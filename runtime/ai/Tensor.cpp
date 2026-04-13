#include "Tensor.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

namespace nexa {

// Your existing matmul implementation
Tensor matmul(const Tensor& A, const Tensor& B)
{
    int m = A.shape[0];
    int n = A.shape[1];
    int p = B.shape[1];

    std::vector<float> result(m * p);

    for (int i = 0; i < m; i++)
        for (int j = 0; j < p; j++)
        {
            float sum = 0;

            for (int k = 0; k < n; k++)
                sum += A.data[i*n+k] * B.data[k*p+j];

            result[i*p+j] = sum;
        }

    return Tensor(result, {m, p});
}

} // namespace nexa

// ==========================================================
// LLVM BRIDGE (EXTERN "C")
// These functions are what the CodeGen actually calls.
// ==========================================================

extern "C" {

    // Creates the C++ Tensor object and returns it as an opaque pointer
    void* ai_create_matrix(int rows, int cols) {
        std::vector<float> data(rows * cols, 0.0f);
        nexa::Tensor* t = new nexa::Tensor(data, {rows, cols});
        return static_cast<void*>(t);
    }

    // Fills the data from Nexa source code into the Tensor object
    void ai_set_value(void* ptr, int r, int c, float val) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        int cols = t->shape[1];
        t->data[r * cols + c] = val;
    }

    void* ai_zeros(int rows, int cols) {
        std::vector<float> data(rows* cols, 0.0f);
        return static_cast<void*>(new nexa::Tensor(data, {rows, cols}));
    }

    float ai_sum(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        return std::accumulate(t->data.begin(), t->data.end(), 0.0f);
    }

    float ai_mean(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        if (t->data.empty()) return 0.0f;
        return ai_sum(ptr) / t->data.size();
    }

    float ai_max(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        if (t->data.empty()) return 0.0f;
        return *std::max_element(t->data.begin(), t->data.end());
    }

    float ai_min(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        if (t->data.empty()) return 0.0f;
        return *std::min_element(t->data.begin(), t->data.end());
    }

    void* ai_reshape(void* ptr, int r, int c) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        if (r*c != (int)t->data.size()) {
            std::cerr << "[Runtime] ERROR: Reshape dimensions " << r << "x" << c
                    << "do not match total elements " << t->data.size() << "\n";
            return ptr;
        }
        return static_cast<void*>(new nexa::Tensor(t->data, {r, c}));
    }

    void* ai_shape(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
        int rows = t->shape[0];
        int cols = t->shape[1];

        std::cout << "Shape: (" << rows << ", " << cols << ")" << std::endl;

        int* shape_arr = new int[2];
        shape_arr[0] = rows;
        shape_arr[1] = cols;
        return static_cast<void*>(shape_arr);
    }

    // Performs the matmul and returns a NEW Tensor object pointer
    void* ai_matmul(void* a_ptr, void* b_ptr) {
        nexa::Tensor* A = static_cast<nexa::Tensor*>(a_ptr);
        nexa::Tensor* B = static_cast<nexa::Tensor*>(b_ptr);
        
        nexa::Tensor result = nexa::matmul(*A, *B);
        
        // Move result to heap so it survives the return to the Nexa program
        nexa::Tensor* heapResult = new nexa::Tensor(result.data, result.shape);
        return static_cast<void*>(heapResult);
    }

    // Used by the 'print' statement in Nexa
    void ai_print(void* ptr) {
        nexa::Tensor* t = static_cast<nexa::Tensor*>(ptr);
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
}