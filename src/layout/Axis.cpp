#include "Axis.hpp"

#include <stdexcept>

// I check the data range first so the scale formula cannot divide by zero.
Axis::Axis(
    double minValue,
    double maxValue,
    double screenStart,
    double screenEnd
)
    : minValue(minValue),
      maxValue(maxValue),
      screenStart(screenStart),
      screenEnd(screenEnd) {
    if (minValue == maxValue) {
        throw std::invalid_argument("Axis minimum and maximum cannot be equal.");
    }
}

double Axis::scale(double value) const {
    // Find the fraction of the data range, then map it onto the screen range.
    return screenStart +
           (value - minValue) *
           (screenEnd - screenStart) /
           (maxValue - minValue);
}

std::vector<double> Axis::ticks(std::size_t count) const {
    if (count < 2) {
        throw std::invalid_argument("Axis needs at least two tick values.");
    }

    std::vector<double> values;
    values.reserve(count);

    // We have one fewer interval than the number of tick marks.
    double step = (maxValue - minValue) /
                  static_cast<double>(count - 1);

    for (std::size_t i = 0; i < count; ++i) {
        values.push_back(minValue + static_cast<double>(i) * step);
    }

    return values;
}

double Axis::getMin() const {
    return minValue;
}

double Axis::getMax() const {
    return maxValue;
}

double Axis::getScreenStart() const {
    return screenStart;
}

double Axis::getScreenEnd() const {
    return screenEnd;
}

void Axis::setRange(double newMinValue, double newMaxValue) {
    if (newMinValue == newMaxValue) {
        throw std::invalid_argument("Axis minimum and maximum cannot be equal.");
    }

    minValue = newMinValue;
    maxValue = newMaxValue;
}

void Axis::setScreenRange(double newScreenStart, double newScreenEnd) {
    screenStart = newScreenStart;
    screenEnd = newScreenEnd;
}
