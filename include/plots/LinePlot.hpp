#pragma once
#include <vector>
struct LinePoint {double x,y;};
class LinePlot {
public:
    static std::vector<LinePoint> build(const std::vector<double>& x,
                                        const std::vector<double>& y);
};