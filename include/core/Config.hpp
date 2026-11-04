#pragma once
struct Config {
    int width=900,height=560;
    double left=80,right=40,top=60,bottom=85;
    void validate() const;
};
// Shared display policy: axis values and descriptive statistics only.
// Computation retains full double precision; rounding happens when displayed.
struct AxisPrecision {
    static constexpr int x = 3;
    static constexpr int y = 4;
    static constexpr int statistics = 4;
};
