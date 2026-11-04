#include "Annotation.hpp"
#include "DataLoader.hpp"
#include "DataProcessor.hpp"
#include "Statistics.hpp"
#include "LinePlot.hpp"
#include "ScatterPlot.hpp"
#include "BarChart.hpp"
#include "BoxPlot.hpp"
#include "Heatmap.hpp"
#include "PieChart.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"
#include "CountPlot.hpp"
#include "Renderer.hpp"
#include "SVGExporter.hpp"
#include "ImageExporter.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
void check(const std::string& svg,const std::string& required){
    assert(svg.find("<svg")!=std::string::npos);
    assert(svg.find("</svg>")!=std::string::npos);
    assert(svg.find(required)!=std::string::npos);
}
int main(){
    auto csv=DataLoader::loadCSV("data/input/monthly_sales.csv");
    assert(csv.size()==13);
    assert(csv[0][0]=="Month"&&csv[0][1]=="Sales");
    std::vector<double> xs,ys;std::vector<std::string> months;
    for(std::size_t i=1;i<csv.size();++i){
        months.push_back(csv[i][0]);
        xs.push_back(static_cast<double>(i-1));
        ys.push_back(std::stod(csv[i][1]));
    }
    assert(months.front()=="Jan"&&months.back()=="Dec");
    Annotation notes;notes.add(600,75,"Mean: "+std::to_string(Statistics::mean(ys)));
    check(Renderer::renderLinePlot(LinePlot::build(xs,ys),900,560,
        "Monthly Sales",notes,"Month","Sales",months),">Dec</text>");
    auto bars=BarChart::build(months,ys);
    check(Renderer::renderBarChart(bars,900,560,"Monthly Sales",
        notes,"Month","Sales"),">Jan</text>");
    check(Renderer::renderScatterPlot(ScatterPlot::build(xs,ys),
        900,560,"Sales Scatter",notes,"Month","Sales"),"<circle");
    check(Renderer::renderHistogram(Histogram::build(ys,5),
        900,560,"Sales Histogram",notes,"Sales","Frequency"),"<rect");
    check(Renderer::renderKDE(KDEPlot::estimate(ys,80),
        900,560,"Sales KDE",notes,"Sales","Density"),"<polyline");
    check(Renderer::renderCountPlot(CountPlot::calculate(months),
        900,560,"Months",notes,"Month","Frequency"),">Jan</text>");
    check(Renderer::renderBoxPlot(BoxPlot::calculate(ys),
        900,560,"Sales Box",notes,"Sales"),"<rect");
    check(Renderer::renderHeatmap(Heatmap::build({{1,2,3},{4,5,6}}),
        900,560,"Sales Heatmap",notes,"Column","Row"),"<rect");
    check(Renderer::renderPieChart(PieChart::build({{"First",10},{"Second",20}}),
        900,560,"Sales Pie",notes),"First");
    auto title=Renderer::renderLinePlot(LinePlot::build(xs,ys),
        900,560,"Sales & Margin",notes,"Month","Sales",months);
    check(title,"Sales &amp; Margin");
    const char* file="complete_graphs_test.svg";
    SVGExporter::save(title,file);
    std::ifstream stream(ImageExporter::outputFile(file));
    assert(stream.good());stream.close();
    std::remove(ImageExporter::outputFile(file).c_str());
}