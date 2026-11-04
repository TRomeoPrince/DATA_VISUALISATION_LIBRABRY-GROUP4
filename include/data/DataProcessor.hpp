#ifndef DATAPROCESSOR_HPP
#define DATAPROCESSOR_HPP

#include <cstddef>
#include <string>
#include <vector>

// The data we load may have extra spaces or invalid values.
// These functions will prepare it before we calculate the graphs.
class DataProcessor {
public:
    // The rest of the library will use this public interface.
    using Table = std::vector<std::vector<std::string>>;

    static std::string trim(const std::string& value);

    static Table cleanRows(const Table& table);

    static std::vector<std::string> getColumn(
        const Table& table,
        std::size_t columnIndex,
        bool skipHeader = true
    );

    static std::vector<double> toNumeric(
        const std::vector<std::string>& values,
        bool ignoreInvalid = true
    );
};

#endif
