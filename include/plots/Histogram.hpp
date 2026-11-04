#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include <cstddef>
#include <vector>

// A histogram divides numerical values into groups called bins.
// Each bin stores its lower limit, upper limit, and number of values.
struct HistogramBin {
    double lower;
    double upper;
    std::size_t count;
};

class Histogram {
public:
    // The rest of the library will use this public interface.
    static std::vector<HistogramBin> build(
        const std::vector<double>& data,
        std::size_t binCount = 10
    );
};

#endif
