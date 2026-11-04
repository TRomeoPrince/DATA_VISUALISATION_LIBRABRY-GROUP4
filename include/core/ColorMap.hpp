#pragma once
#include <cstddef>
#include <string>
class ColorMap {
public:
    static std::string viridis(double fraction);
    static std::string categorical(std::size_t index);
};