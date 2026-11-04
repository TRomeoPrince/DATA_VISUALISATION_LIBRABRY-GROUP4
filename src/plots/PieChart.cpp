#include "PieChart.hpp"
#include <cmath>
#include <stdexcept>
std::vector<PieSlice> PieChart::build(const std::vector<std::pair<std::string,double>>& values){
if(values.empty())throw std::invalid_argument("Pie chart empty");
double sum=0;
for(const auto& p:values){if(!std::isfinite(p.second)||p.second<0)
throw std::invalid_argument("Pie values must be nonnegative and finite");sum+=p.second;}
if(sum<=0)throw std::invalid_argument("Pie total must be positive");
std::vector<PieSlice> out;for(const auto& p:values)if(p.second>0)
out.push_back({p.first,p.second,p.second/sum});return out;}