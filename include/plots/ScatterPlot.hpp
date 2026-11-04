#pragma once
#include <vector>
struct ScatterPoint {double x,y;};
class ScatterPlot {
public:
    static std::vector<ScatterPoint> build(const std::vector<double>& x,
                                           const std::vector<double>& y);
};