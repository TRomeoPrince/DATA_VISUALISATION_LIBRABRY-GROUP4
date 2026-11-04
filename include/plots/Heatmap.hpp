#pragma once
#include <vector>
class Heatmap {
public:
    using Matrix=std::vector<std::vector<double>>;
    static Matrix build(const Matrix& values);
};