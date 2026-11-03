#include "Renderer.hpp"

#include "Axis.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
constexpr double LEFT = 70.0;
constexpr double RIGHT = 30.0;
constexpr double TOP = 55.0;
constexpr double BOTTOM = 70.0;


// Each graph should say what the horizontal and vertical values represent.
std::string axisLabels(int width, int height,
                       const std::string& xLabel, const std::string& yLabel) {
    std::ostringstream svg;
    svg << "<text x=\"" << width / 2 << "\" y=\"" << (height - 18)
        << "\" text-anchor=\"middle\" font-size=\"15\""
        << " font-family=\"sans-serif\">" << xLabel << "</text>\n";
    svg << "<text x=\"22\" y=\"" << height / 2
        << "\" transform=\"rotate(-90 22 " << height / 2 << ")\""
        << " text-anchor=\"middle\" font-size=\"15\""
        << " font-family=\"sans-serif\">" << yLabel << "</text>\n";
    return svg.str();
}

std::string number(double value, int precision = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}
}

// I keep the plotting margins in one place so the graphs use the same layout.
std::string Renderer::escapeXML(const std::string& text) {
    std::string result;

    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c; break;
        }
    }

    return result;
}

std::string Renderer::svgStart(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Renderer width and height must be positive.");
    }

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
        << width << "\" height=\"" << height
        << "\" viewBox=\"0 0 " << width << " " << height << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    return svg.str();
}

std::string Renderer::svgEnd() {
    return "</svg>\n";
}

std::string Renderer::renderCountPlot(
    const CountPlot::Counts& counts,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (counts.empty()) {
        throw std::invalid_argument("Cannot render an empty count plot.");
    }

    std::size_t maxCount = 1;
    for (const auto& item : counts) {
        maxCount = std::max(maxCount, item.second);
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis yAxis(0.0, static_cast<double>(maxCount), plotBottom, plotTop);

    double availableWidth = plotRight - plotLeft;
    double slotWidth = availableWidth / static_cast<double>(counts.size());
    double barWidth = slotWidth * 0.7;

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    std::size_t index = 0;
    for (const auto& item : counts) {
        double x = plotLeft + index * slotWidth + (slotWidth - barWidth) / 2.0;
        double y = yAxis.scale(static_cast<double>(item.second));
        double barHeight = plotBottom - y;

        svg << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << barWidth << "\" height=\"" << barHeight
            << "\" fill=\"steelblue\"/>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (plotBottom + 20)
            << "\" text-anchor=\"middle\" font-size=\"12\""
            << " font-family=\"sans-serif\">"
            << escapeXML(item.first) << "</text>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (y - 5)
            << "\" text-anchor=\"middle\" font-size=\"12\""
            << " font-family=\"sans-serif\">"
            << item.second << "</text>\n";

        ++index;
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<line x1=\"" << (plotLeft - 5) << "\" y1=\"" << y
            << "\" x2=\"" << plotLeft << "\" y2=\"" << y
            << "\" stroke=\"black\"/>\n";
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}

std::string Renderer::renderHistogram(
    const std::vector<HistogramBin>& bins,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (bins.empty()) {
        throw std::invalid_argument("Cannot render an empty histogram.");
    }

    std::size_t maxCount = 1;
    for (const auto& bin : bins) {
        maxCount = std::max(maxCount, bin.count);
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis yAxis(0.0, static_cast<double>(maxCount), plotBottom, plotTop);

    double availableWidth = plotRight - plotLeft;
    double barWidth = availableWidth / static_cast<double>(bins.size());

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    for (std::size_t i = 0; i < bins.size(); ++i) {
        const auto& bin = bins[i];
        double x = plotLeft + i * barWidth;
        double y = yAxis.scale(static_cast<double>(bin.count));

        svg << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << barWidth
            << "\" height=\"" << (plotBottom - y)
            << "\" fill=\"seagreen\" stroke=\"white\"/>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (plotBottom + 18)
            << "\" text-anchor=\"middle\" font-size=\"10\""
            << " font-family=\"sans-serif\">"
            << number(bin.lower, 1) << "</text>\n";
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}

std::string Renderer::renderKDE(
    const std::vector<KDEPoint>& points,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (points.size() < 2) {
        throw std::invalid_argument("KDE rendering needs at least two points.");
    }

    double minX = points.front().x;
    double maxX = points.front().x;
    double maxDensity = points.front().density;

    for (const auto& point : points) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        maxDensity = std::max(maxDensity, point.density);
    }

    if (minX == maxX) {
        maxX = minX + 1.0;
    }

    if (maxDensity <= 0.0) {
        maxDensity = 1.0;
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis xAxis(minX, maxX, plotLeft, plotRight);
    Axis yAxis(0.0, maxDensity, plotBottom, plotTop);

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<polyline fill=\"none\" stroke=\"darkorange\""
        << " stroke-width=\"3\" points=\"";

    for (const auto& point : points) {
        svg << xAxis.scale(point.x) << ","
            << yAxis.scale(point.density) << " ";
    }

    svg << "\"/>\n";

    for (double tick : xAxis.ticks()) {
        double x = xAxis.scale(tick);
        svg << "<text x=\"" << x << "\" y=\"" << (plotBottom + 20)
            << "\" text-anchor=\"middle\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 3) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}
