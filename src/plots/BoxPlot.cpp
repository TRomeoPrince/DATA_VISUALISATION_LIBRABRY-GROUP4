#include "BoxPlot.hpp"
#include "Statistics.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
BoxSummary BoxPlot::calculate(const std::vector<double>& v){
if(v.empty())throw std::invalid_argument("Box plot needs data");
for(double x:v)if(!std::isfinite(x))throw std::invalid_argument("Non-finite box value");
double q1=Statistics::firstQuartile(v),q3=Statistics::thirdQuartile(v),iqr=q3-q1;
BoxSummary b{0,q1,Statistics::median(v),q3,0,{}};std::vector<double> inside;
for(double x:v){if(x<q1-1.5*iqr||x>q3+1.5*iqr)b.outliers.push_back(x);else inside.push_back(x);}
if(inside.empty())throw std::logic_error("No whisker values");
b.minimum=*std::min_element(inside.begin(),inside.end());
b.maximum=*std::max_element(inside.begin(),inside.end());return b;}