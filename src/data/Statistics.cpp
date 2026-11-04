#include "Statistics.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace {
void ensureNotEmpty(const std::vector<double>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Statistics cannot be calculated for empty data.");
    }
}
}

// The functions here calculate values used by the different graphs.
std::size_t Statistics::count(const std::vector<double>& data) {
    return data.size();
}

double Statistics::mean(const std::vector<double>& data) {
    ensureNotEmpty(data);
    double total = std::accumulate(data.begin(), data.end(), 0.0);
    return total / static_cast<double>(data.size());
}

double Statistics::minimum(const std::vector<double>& data) {
    ensureNotEmpty(data);
    return *std::min_element(data.begin(), data.end());
}

double Statistics::maximum(const std::vector<double>& data) {
    ensureNotEmpty(data);
    return *std::max_element(data.begin(), data.end());
}

double Statistics::median(std::vector<double> data) {
    ensureNotEmpty(data);
    // Sorting a copy means we do not change the caller's original values.
    std::sort(data.begin(), data.end());

    std::size_t middle = data.size() / 2;

    if (data.size() % 2 == 0) {
        return (data[middle - 1] + data[middle]) / 2.0;
    }

    return data[middle];
}

double Statistics::variance(const std::vector<double>& data) {
    ensureNotEmpty(data);

    double avg = mean(data);
    double totalSquaredDifference = 0.0;

    for (double value : data) {
        double difference = value - avg;
        totalSquaredDifference += difference * difference;
    }

    return totalSquaredDifference / static_cast<double>(data.size());
}

double Statistics::standardDeviation(const std::vector<double>& data) {
    return std::sqrt(variance(data));
}
