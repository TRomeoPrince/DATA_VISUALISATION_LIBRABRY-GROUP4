#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <cstddef>
#include <vector>

// The Statistics functions calculate results that plots can use.
// They are static because we do not need to store a Statistics object.
class Statistics {
public:
    // The rest of the library will use this public interface.
    static std::size_t count(const std::vector<double>& data);
    static double mean(const std::vector<double>& data);
    static double minimum(const std::vector<double>& data);
    static double maximum(const std::vector<double>& data);
    static double median(std::vector<double> data);
    static double variance(const std::vector<double>& data);
    static double standardDeviation(const std::vector<double>& data);
};

#endif
