#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Annotation.hpp"
#include "CountPlot.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"
#include "LinePlot.hpp"
#include "ScatterPlot.hpp"
#include "BarChart.hpp"
#include "BoxPlot.hpp"
#include "Heatmap.hpp"
#include "PieChart.hpp"
#include "IBasePlot.hpp"

#include <string>

// The Renderer turns the calculated plot values into SVG drawings.
// The different graphs should reuse Axis and Annotations where needed.
class Renderer {
public:
    // The rest of the library will use this public interface.
    static std::string renderCountPlot(
        const CountPlot::Counts& counts,
        int width = 800,
        int height = 500,
        const std::string& title = "Count Plot",
        const Annotation& annotations = Annotation(),
        const std::string& xLabel = "Programme",
        const std::string& yLabel = "Frequency"
    );

    static std::string renderHistogram(
        const std::vector<HistogramBin>& bins,
        int width = 800,
        int height = 500,
        const std::string& title = "Histogram",
        const Annotation& annotations = Annotation(),
        const std::string& xLabel = "Marks",
        const std::string& yLabel = "Frequency"
    );

    static std::string renderKDE(
        const std::vector<KDEPoint>& points,
        int width = 800,
        int height = 500,
        const std::string& title = "KDE Plot",
        const Annotation& annotations = Annotation(),
        const std::string& xLabel = "Marks",
        const std::string& yLabel = "Density"
    );

    static std::string render(const IBasePlot& plot,const RenderOptions& options);
    static std::string renderLinePlot(
        const std::vector<LinePoint>& points,int width=900,int height=560,
        const std::string& title="Line Plot",const Annotation& notes=Annotation(),
        const std::string& xLabel="X",const std::string& yLabel="Y",
        const std::vector<std::string>& categories={}
    );
    static std::string renderScatterPlot(
        const std::vector<ScatterPoint>& points,int width=900,int height=560,
        const std::string& title="Scatter Plot",const Annotation& notes=Annotation(),
        const std::string& xLabel="X",const std::string& yLabel="Y"
    );
    static std::string renderBarChart(
        const std::vector<BarItem>& bars,int width=900,int height=560,
        const std::string& title="Bar Chart",const Annotation& notes=Annotation(),
        const std::string& xLabel="Category",const std::string& yLabel="Value"
    );
    static std::string renderBoxPlot(
        const BoxSummary& box,int width=900,int height=560,
        const std::string& title="Box Plot",const Annotation& notes=Annotation(),
        const std::string& xLabel="Value"
    );
    static std::string renderHeatmap(
        const Heatmap::Matrix& matrix,int width=900,int height=560,
        const std::string& title="Heatmap",const Annotation& notes=Annotation(),
        const std::string& xLabel="Column",const std::string& yLabel="Row"
    );
    static std::string renderPieChart(
        const std::vector<PieSlice>& slices,int width=900,int height=560,
        const std::string& title="Pie Chart",const Annotation& notes=Annotation()
    );
private:
    static std::string escapeXML(const std::string& text);
    static std::string svgStart(int width, int height);
    static std::string svgEnd();
};

#endif
