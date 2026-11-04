#include "Histogram.hpp"

#include <algorithm>
#include <stdexcept>

// I find the range and bin width before counting the values.
std::vector<HistogramBin> Histogram::build(
    const std::vector<double>& data,
    std::size_t binCount
) {
    if (data.empty()) {
        throw std::invalid_argument("Histogram requires at least one data value.");
    }

    if (binCount == 0) {
        throw std::invalid_argument("Histogram bin count must be greater than zero.");
    }

    double minValue = *std::min_element(data.begin(), data.end());
    double maxValue = *std::max_element(data.begin(), data.end());

    if (minValue == maxValue) {
        return {{minValue, maxValue, data.size()}};
    }

    double binWidth = (maxValue - minValue) / static_cast<double>(binCount);

    std::vector<HistogramBin> bins;
    bins.reserve(binCount);

    for (std::size_t i = 0; i < binCount; ++i) {
        double lower = minValue + static_cast<double>(i) * binWidth;
        double upper = (i == binCount - 1)
            ? maxValue
            : minValue + static_cast<double>(i + 1) * binWidth;

        bins.push_back({lower, upper, 0});
    }

    // A value on the final boundary must still count in the last bin.
    for (double value : data) {
        std::size_t index = static_cast<std::size_t>(
            (value - minValue) / binWidth
        );

        if (index >= binCount) {
            index = binCount - 1;
        }

        ++bins[index].count;
    }

    return bins;
}
