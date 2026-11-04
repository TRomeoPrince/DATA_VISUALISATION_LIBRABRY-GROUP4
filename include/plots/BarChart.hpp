#pragma once
#include <string>
#include <vector>
struct BarItem {std::string category;double value;};
class BarChart {
public:
    static std::vector<BarItem> build(const std::vector<std::string>& labels,
                                      const std::vector<double>& values);
};