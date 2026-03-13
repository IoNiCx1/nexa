#ifndef NEXA_TENSOR_H
#define NEXA_TENSOR_H

#include <vector>

namespace nexa {

class Tensor {

public:

    std::vector<float> data;
    std::vector<int> shape;

    Tensor(const std::vector<float>& d,
           const std::vector<int>& s)
        : data(d), shape(s) {}

};

Tensor matmul(const Tensor& A, const Tensor& B);

}

#endif
