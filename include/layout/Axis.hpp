#ifndef AXIS_HPP
#define AXIS_HPP

#include <cstddef>
#include <vector>

// The Axis needs the minimum and maximum data values.
// It also needs the starting and ending drawing positions.
// I will use both ranges when working out where each value should appear.
class Axis {
private:
    // These are the values in the original dataset.
    double minValue;
    double maxValue;
    // These are the positions where the axis is drawn.
    double screenStart;
    double screenEnd;

public:
    Axis(
        double minValue,
        double maxValue,
        double screenStart,
        double screenEnd
    );

    // This maps a data value to its drawing position.
    double scale(double value) const;

    // The tick values mark divisions along the axis.
    std::vector<double> ticks(std::size_t count = 6) const;

    double getMin() const;
    double getMax() const;
    double getScreenStart() const;
    double getScreenEnd() const;

    void setRange(double minValue, double maxValue);
    void setScreenRange(double screenStart, double screenEnd);
};

#endif
