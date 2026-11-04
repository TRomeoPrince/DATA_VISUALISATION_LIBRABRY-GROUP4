#ifndef KDEPLOT_HPP
#define KDEPLOT_HPP

#include <cstddef>
#include <vector>

// KDE gives us points for a smooth density curve.
// Each point has its x position and estimated density.
struct KDEPoint {
    double x;
    double density;
};

class KDEPlot {
public:
    // The rest of the library will use this public interface.
    static std::vector<KDEPoint> estimate(
        const std::vector<double>& data,
        std::size_t pointCount = 100,
        double bandwidth = 0.0
    );

private:
    static double gaussianKernel(double u);
    static double automaticBandwidth(const std::vector<double>& data);
};

#endif
