#pragma once
#include <vector>
#include <string>

namespace nexa {

struct Tensor {
    std::vector<float> data;
    std::vector<int>   shape;

    Tensor() {}
    Tensor(const std::vector<float>& d, const std::vector<int>& s)
        : data(d), shape(s) {}
};

Tensor matmul(const Tensor& A, const Tensor& B);

} // namespace nexa

// ─────────────────────────────────────────────────────────────────────────────
// LLVM Bridge — all functions callable from compiled Nexa IR
// ─────────────────────────────────────────────────────────────────────────────
extern "C" {

// ── Tensor ops ────────────────────────────────
void*  ai_create_matrix(int rows, int cols);
void   ai_set_value(void* ptr, int r, int c, float val);
void*  ai_matmul(void* a, void* b);
void   ai_print(void* ptr);
void*  ai_zeros(int rows, int cols);
void*  ai_ones(int rows, int cols);
float  ai_sum(void* ptr);
float  ai_mean(void* ptr);
float  ai_max(void* ptr);
float  ai_min(void* ptr);
void*  ai_reshape(void* ptr, int r, int c);
void*  ai_shape(void* ptr);
float  ai_get_value(void* ptr, int r, int c);

// ── CSV ops ───────────────────────────────────
// Read a numeric CSV file into a Tensor (floats).
// Header row is skipped if skip_header != 0.
void*  csv_read(const char* path, int skip_header);

// Write a Tensor back to a CSV file.
void   csv_write(const char* path, void* tensor_ptr);

// Get number of rows / cols in a CSV-loaded tensor
int    csv_rows(void* tensor_ptr);
int    csv_cols(void* tensor_ptr);

// Get a single cell value
float  csv_get(void* tensor_ptr, int row, int col);

// Set a single cell value
void   csv_set(void* tensor_ptr, int row, int col, float val);

// Get an entire row as a new Tensor (1 x cols)
void*  csv_get_row(void* tensor_ptr, int row);

// Get an entire column as a new Tensor (rows x 1)
void*  csv_get_col(void* tensor_ptr, int col);

// Get a sub-range of columns [col_start, col_end) as a new Tensor
void*  csv_slice_cols(void* tensor_ptr, int col_start, int col_end);

} // extern "C"