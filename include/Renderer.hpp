#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Annotation.hpp"
#include "CountPlot.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"

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

private:
    static std::string escapeXML(const std::string& text);
    static std::string svgStart(int width, int height);
    static std::string svgEnd();
};

#endif
