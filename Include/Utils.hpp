#ifndef UTILS_HPP
#define UTILS_HPP
#include <tuple>

namespace Arrithmetic {
    std::pair<float, float> quadratic_solver(const float& a, const float& b, const float& c);

    void cubic_solver(const float& b, const float& c, const float& d, float* output);
};



#endif // !UTILS_HPP
