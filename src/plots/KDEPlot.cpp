#include "KDEPlot.hpp"

#include "Statistics.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double PI = 3.14159265358979323846;
}

// This kernel provides one bell-shaped contribution to the density.
double KDEPlot::gaussianKernel(double u) {
    return std::exp(-0.5 * u * u) / std::sqrt(2.0 * PI);
}

double KDEPlot::automaticBandwidth(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 1.0;
    }

    double deviation = Statistics::standardDeviation(data);

    if (deviation <= 0.0) {
        return 1.0;
    }

    // Silverman's common rule-of-thumb bandwidth.
    return 1.06 * deviation *
           std::pow(static_cast<double>(data.size()), -0.2);
}

std::vector<KDEPoint> KDEPlot::estimate(
    const std::vector<double>& data,
    std::size_t pointCount,
    double bandwidth
) {
    if (data.empty()) {
        throw std::invalid_argument("KDE requires at least one data value.");
    }

    if (pointCount < 2) {
        throw std::invalid_argument("KDE point count must be at least two.");
    }

    if (bandwidth <= 0.0) {
        bandwidth = automaticBandwidth(data);
    }

    double minValue = *std::min_element(data.begin(), data.end());
    double maxValue = *std::max_element(data.begin(), data.end());

    // Extend the evaluated range so the curve can taper at both sides.
    double start = minValue - 3.0 * bandwidth;
    double end = maxValue + 3.0 * bandwidth;
    double step = (end - start) / static_cast<double>(pointCount - 1);

    std::vector<KDEPoint> points;
    points.reserve(pointCount);

    // Calculate the density at each point so the Renderer can join them.
    for (std::size_t i = 0; i < pointCount; ++i) {
        double x = start + static_cast<double>(i) * step;
        double densitySum = 0.0;

        for (double value : data) {
            double u = (x - value) / bandwidth;
            densitySum += gaussianKernel(u);
        }

        double density = densitySum /
            (static_cast<double>(data.size()) * bandwidth);

        points.push_back({x, density});
    }

    return points;
}
