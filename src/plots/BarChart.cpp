#include "BarChart.hpp"
#include <cmath>
#include <stdexcept>
std::vector<BarItem> BarChart::build(const std::vector<std::string>& labels,const std::vector<double>& values){
if(labels.empty()||labels.size()!=values.size())throw std::invalid_argument("Bar labels/values must match");
std::vector<BarItem> out;out.reserve(labels.size());
for(std::size_t i=0;i<labels.size();++i){if(!std::isfinite(values[i]))
throw std::invalid_argument("Non-finite bar value");out.push_back({labels[i],values[i]});}return out;}