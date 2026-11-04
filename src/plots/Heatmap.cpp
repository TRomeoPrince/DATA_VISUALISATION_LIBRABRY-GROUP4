#include "Heatmap.hpp"
#include <cmath>
#include <stdexcept>
Heatmap::Matrix Heatmap::build(const Matrix& values){
if(values.empty()||values.front().empty())throw std::invalid_argument("Heatmap empty");
const auto width=values.front().size();
for(const auto& r:values){if(r.size()!=width)throw std::invalid_argument("Heatmap rows must have equal width");
for(double x:r)if(!std::isfinite(x))throw std::invalid_argument("Non-finite heatmap");}
return values;}