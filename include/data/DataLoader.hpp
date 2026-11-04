#ifndef DATALOADER_HPP
#define DATALOADER_HPP

#include <string>
#include <vector>

// I need to read the CSV file before I can make any graph.
// Each row is stored as a vector of text values.
class DataLoader {
public:
    // The rest of the library will use this public interface.
    using Row = std::vector<std::string>;
    using Table = std::vector<Row>;

    static Table loadCSV(const std::string& filePath, char delimiter = ',');
};

#endif
