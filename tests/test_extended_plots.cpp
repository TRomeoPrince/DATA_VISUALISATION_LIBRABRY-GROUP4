#include "LinePlot.hpp"
#include "ScatterPlot.hpp"
#include "BarChart.hpp"
#include "BoxPlot.hpp"
#include "Heatmap.hpp"
#include "PieChart.hpp"
#include "ColorMap.hpp"
#include "Statistics.hpp"
#include "Renderer.hpp"
#include "IBasePlot.hpp"
#include <cassert>
#include <stdexcept>
struct Custom: IBasePlot {
    std::string renderSVG(const RenderOptions& opts) const override {
        return "<svg>"+opts.title+"</svg>";
    }
};
int main(){
    assert(Statistics::firstQuartile({1,2,3,4,5})==2);
    assert(Statistics::thirdQuartile({1,2,3,4,5})==4);
    assert(Statistics::interquartileRange({1,2,3,4,5})==2);
    assert(Statistics::mode({1,2,2,3})[0]==2);
    auto line=LinePlot::build({0,1,2},{20,30,25});
    auto scatter=ScatterPlot::build({1,2},{3,4});
    auto bars=BarChart::build({"Jan","Feb"},{23,45});
    auto box=BoxPlot::calculate({1,2,3,4,5,100});
    auto heat=Heatmap::build({{1,2},{3,4}});
    auto pie=PieChart::build({{"A",2},{"B",3}});
    Annotation notes;notes.add(110,75,"Sales & growth");
    auto svg=Renderer::renderLinePlot(line,900,560,"Monthly Sales",notes,"Month","Sales",{"Jan","Feb","Mar"});
    assert(svg.find(">Jan</text>")!=std::string::npos);
    assert(svg.find("Sales &amp; growth")!=std::string::npos);
    assert(Renderer::renderScatterPlot(scatter).find("<circle")!=std::string::npos);
    auto numericLine=Renderer::renderLinePlot(LinePlot::build({1.0,2.0},{0.0,1.0}));
    assert(numericLine.find(">1.000</text>")!=std::string::npos);
    assert(numericLine.find(">0.0000</text>")!=std::string::npos);
    auto heatSvg=Renderer::renderHeatmap({{1.25,2.0},{3.0,4.0}});
    assert(heatSvg.find(">1.2500</text>")!=std::string::npos);
    assert(Renderer::renderBarChart(bars).find("Jan")!=std::string::npos);
    assert(Renderer::renderBoxPlot(box).find("<rect")!=std::string::npos);
    assert(Renderer::renderHeatmap(heat).find("<rect")!=std::string::npos);
    assert(Renderer::renderPieChart(pie).find("<path")!=std::string::npos);
    assert(ColorMap::viridis(0)!=ColorMap::viridis(1));
    Custom custom;RenderOptions options;options.title="Custom";
    assert(Renderer::render(custom,options).find("Custom")!=std::string::npos);
    bool failed=false;
    try{LinePlot::build({1},{2,3});}catch(const std::invalid_argument&){failed=true;}
    assert(failed);
}