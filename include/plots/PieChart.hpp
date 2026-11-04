#pragma once
#include <string>
#include <utility>
#include <vector>
struct PieSlice {std::string label;double value,fraction;};
class PieChart {
public:
    static std::vector<PieSlice> build(const std::vector<std::pair<std::string,double>>& values);
};