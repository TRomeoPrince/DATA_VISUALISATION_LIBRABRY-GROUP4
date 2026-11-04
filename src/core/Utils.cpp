#include "Utils.hpp"
#include <filesystem>
#include <iomanip>
#include <sstream>
std::string Utils::escapeXML(const std::string& text){
std::string out;for(char c:text){
switch(c){case '&':out+="&amp;";break;case '<':out+="&lt;";break;
case '>':out+="&gt;";break;case '"':out+="&quot;";break;
case '\'':out+="&apos;";break;default:out+=c;}}return out;}
std::string Utils::decimal(double x,int p){std::ostringstream s;
s<<std::fixed<<std::setprecision(p)<<x;return s.str();}
void Utils::ensureParent(const std::string& path){
auto p=std::filesystem::path(path).parent_path();
if(!p.empty())std::filesystem::create_directories(p);}