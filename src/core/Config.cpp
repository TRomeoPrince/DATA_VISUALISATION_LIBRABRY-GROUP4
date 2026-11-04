#include "Config.hpp"
#include <stdexcept>
void Config::validate() const {
    if(width<240||height<200||left<0||right<0||top<0||bottom<0||
       left+right>=width-40||top+bottom>=height-40)
       throw std::invalid_argument("Invalid graph canvas or margins.");
}