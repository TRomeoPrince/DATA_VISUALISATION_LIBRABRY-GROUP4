#include "ScatterPlot.hpp"
#include <cmath>
#include <stdexcept>
std::vector<ScatterPoint> ScatterPlot::build(const std::vector<double>& x,const std::vector<double>& y){
if(x.empty()||x.size()!=y.size())throw std::invalid_argument("Scatter X/Y sizes must match and be nonempty");
std::vector<ScatterPoint> out;out.reserve(x.size());
for(std::size_t i=0;i<x.size();++i){if(!std::isfinite(x[i])||!std::isfinite(y[i]))
throw std::invalid_argument("Non-finite scatter point");out.push_back({x[i],y[i]});}
return out;}