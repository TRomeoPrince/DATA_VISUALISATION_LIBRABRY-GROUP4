#include "ColorMap.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace {
std::string rgb(int r,int g,int b){std::ostringstream out;
out<<"#"<<std::hex<<std::setfill('0')<<std::setw(2)<<r
<<std::setw(2)<<g<<std::setw(2)<<b;return out.str();}
}
std::string ColorMap::viridis(double x){
 if(!std::isfinite(x))throw std::invalid_argument("Non-finite color value");
 x=std::max(0.0,std::min(1.0,x));const int palette[5][3]={
 {68,1,84},{59,82,139},{33,145,140},{94,201,97},{253,231,37}};
 double p=x*4;int i=std::min(3,static_cast<int>(p));double a=p-i;
 return rgb(static_cast<int>(std::lround(palette[i][0]*(1-a)+palette[i+1][0]*a)),
 static_cast<int>(std::lround(palette[i][1]*(1-a)+palette[i+1][1]*a)),
 static_cast<int>(std::lround(palette[i][2]*(1-a)+palette[i+1][2]*a)));
}
std::string ColorMap::categorical(std::size_t i){
static constexpr const char* colors[]={"#247ba0","#70c1b3","#f3a712",
"#e76f51","#7867ac","#2a9d8f","#ef476f","#118ab2","#8d6e63"};
return colors[i%9];}