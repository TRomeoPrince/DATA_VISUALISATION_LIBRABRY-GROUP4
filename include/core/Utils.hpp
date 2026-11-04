#pragma once
#include <string>
class Utils {
public:
    static std::string escapeXML(const std::string& text);
    static std::string decimal(double value,int places=3);
    static void ensureParent(const std::string& path);
};