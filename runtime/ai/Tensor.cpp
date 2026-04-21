#include "Tensor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

namespace nexa {

Tensor matmul(const Tensor& A, const Tensor& B) {
    int m = A.shape[0];
    int n = A.shape[1];
    int p = B.shape[1];
    std::vector<float> result(m * p, 0.0f);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < p; j++) {
            float sum = 0;
            for (int k = 0; k < n; k++)
                sum += A.data[i*n+k] * B.data[k*p+j];
            result[i*p+j] = sum;
        }
    return Tensor(result, {m, p});
}

} // namespace nexa

extern "C" {

// ─────────────────────────────────────────────
// Tensor ops
// ─────────────────────────────────────────────

void* ai_create_matrix(int rows, int cols) {
    std::vector<float> data(rows * cols, 0.0f);
    return new nexa::Tensor(data, {rows, cols});
}

void ai_set_value(void* ptr, int r, int c, float val) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    t->data[r * t->shape[1] + c] = val;
}

float ai_get_value(void* ptr, int r, int c) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    return t->data[r * t->shape[1] + c];
}

void* ai_matmul(void* a_ptr, void* b_ptr) {
    auto* A = static_cast<nexa::Tensor*>(a_ptr);
    auto* B = static_cast<nexa::Tensor*>(b_ptr);
    nexa::Tensor result = nexa::matmul(*A, *B);
    return new nexa::Tensor(result.data, result.shape);
}

void ai_print(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    int rows = t->shape[0];
    int cols = t->shape[1];
    std::cout << "[";
    for (int i = 0; i < rows; i++) {
        if (i > 0) std::cout << " ";
        std::cout << "[";
        for (int j = 0; j < cols; j++) {
            std::cout << t->data[i * cols + j];
            if (j < cols - 1) std::cout << ", ";
        }
        std::cout << "]";
        if (i < rows - 1) std::cout << ",\n";
    }
    std::cout << "]" << std::endl;
}

void* ai_zeros(int rows, int cols) {
    return new nexa::Tensor(std::vector<float>(rows * cols, 0.0f), {rows, cols});
}

void* ai_ones(int rows, int cols) {
    return new nexa::Tensor(std::vector<float>(rows * cols, 1.0f), {rows, cols});
}

float ai_sum(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    float s = 0;
    for (float v : t->data) s += v;
    return s;
}

float ai_mean(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    if (t->data.empty()) return 0.0f;
    float s = 0;
    for (float v : t->data) s += v;
    return s / t->data.size();
}

float ai_max(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    if (t->data.empty()) return 0.0f;
    float m = t->data[0];
    for (float v : t->data) if (v > m) m = v;
    return m;
}

float ai_min(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    if (t->data.empty()) return 0.0f;
    float m = t->data[0];
    for (float v : t->data) if (v < m) m = v;
    return m;
}

void* ai_reshape(void* ptr, int r, int c) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    auto* result = new nexa::Tensor(t->data, {r, c});
    return result;
}

void* ai_shape(void* ptr) {
    auto* t = static_cast<nexa::Tensor*>(ptr);
    auto* result = new nexa::Tensor(
        {(float)t->shape[0], (float)t->shape[1]}, {1, 2});
    return result;
}

// ─────────────────────────────────────────────
// CSV ops
// ─────────────────────────────────────────────

// Helper: trim whitespace from a string
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// Helper: split a line by delimiter
static std::vector<std::string> splitLine(const std::string& line, char delim = ',') {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, delim))
        tokens.push_back(trim(token));
    return tokens;
}

void* csv_read(const char* path, int skip_header) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "[nexa runtime] error: cannot open CSV '%s'\n", path);
        return nullptr;
    }

    std::vector<std::vector<float>> rows;
    std::string line;
    bool first = true;

    while (std::getline(f, line)) {
        if (trim(line).empty()) continue;

        // Skip header row if requested
        if (first && skip_header) {
            first = false;
            continue;
        }
        first = false;

        auto tokens = splitLine(line);
        std::vector<float> row;
        for (auto& tok : tokens) {
            try {
                row.push_back(std::stof(tok));
            } catch (...) {
                // Non-numeric cell — store 0 and warn
                fprintf(stderr, "[nexa runtime] warning: non-numeric cell '%s' in CSV, using 0\n",
                        tok.c_str());
                row.push_back(0.0f);
            }
        }
        if (!row.empty())
            rows.push_back(row);
    }

    if (rows.empty()) {
        fprintf(stderr, "[nexa runtime] warning: CSV '%s' is empty\n", path);
        return new nexa::Tensor({}, {0, 0});
    }

    int nrows = (int)rows.size();
    int ncols = (int)rows[0].size();

    std::vector<float> data;
    data.reserve(nrows * ncols);
    for (auto& r : rows) {
        // Pad or truncate rows to ncols for a clean matrix
        r.resize(ncols, 0.0f);
        for (float v : r) data.push_back(v);
    }

    return new nexa::Tensor(data, {nrows, ncols});
}

void csv_write(const char* path, void* tensor_ptr) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return;

    std::ofstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "[nexa runtime] error: cannot open '%s' for CSV writing\n", path);
        return;
    }

    int rows = t->shape[0];
    int cols = t->shape[1];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            f << t->data[i * cols + j];
            if (j < cols - 1) f << ",";
        }
        f << "\n";
    }
}

int csv_rows(void* tensor_ptr) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    return t ? t->shape[0] : 0;
}

int csv_cols(void* tensor_ptr) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    return t ? t->shape[1] : 0;
}

float csv_get(void* tensor_ptr, int row, int col) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return 0.0f;
    return t->data[row * t->shape[1] + col];
}

void csv_set(void* tensor_ptr, int row, int col, float val) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return;
    t->data[row * t->shape[1] + col] = val;
}

void* csv_get_row(void* tensor_ptr, int row) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return nullptr;
    int cols = t->shape[1];
    std::vector<float> rowData(t->data.begin() + row * cols,
                               t->data.begin() + row * cols + cols);
    return new nexa::Tensor(rowData, {1, cols});
}

void* csv_get_col(void* tensor_ptr, int col) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return nullptr;
    int rows = t->shape[0];
    int cols = t->shape[1];
    std::vector<float> colData;
    colData.reserve(rows);
    for (int i = 0; i < rows; i++)
        colData.push_back(t->data[i * cols + col]);
    return new nexa::Tensor(colData, {rows, 1});
}

void* csv_slice_cols(void* tensor_ptr, int col_start, int col_end) {
    auto* t = static_cast<nexa::Tensor*>(tensor_ptr);
    if (!t) return nullptr;
    int rows = t->shape[0];
    int cols = t->shape[1];
    int newCols = col_end - col_start;
    std::vector<float> data;
    data.reserve(rows * newCols);
    for (int i = 0; i < rows; i++)
        for (int j = col_start; j < col_end && j < cols; j++)
            data.push_back(t->data[i * cols + j]);
    return new nexa::Tensor(data, {rows, newCols});
}

} // extern "C"