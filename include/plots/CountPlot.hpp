#ifndef COUNTPLOT_HPP
#define COUNTPLOT_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

// CountPlot counts how many times each category appears.
// The Renderer will use these results to draw the bars.
class CountPlot {
public:
    // The rest of the library will use this public interface.
    using Counts = std::map<std::string, std::size_t>;

    static Counts calculate(const std::vector<std::string>& categories);
};

#endif
