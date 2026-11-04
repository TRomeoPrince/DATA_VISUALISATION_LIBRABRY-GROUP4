#pragma once
#include <string>
struct RenderOptions { int width=900,height=560; std::string title,xLabel,yLabel; };
class IBasePlot {
public:
    virtual ~IBasePlot()=default;
    virtual std::string renderSVG(const RenderOptions&) const=0;
};