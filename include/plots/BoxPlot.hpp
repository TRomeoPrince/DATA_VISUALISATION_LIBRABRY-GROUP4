#pragma once
#include <vector>
struct BoxSummary {double minimum,q1,median,q3,maximum;std::vector<double> outliers;};
class BoxPlot {public:static BoxSummary calculate(const std::vector<double>& values);};