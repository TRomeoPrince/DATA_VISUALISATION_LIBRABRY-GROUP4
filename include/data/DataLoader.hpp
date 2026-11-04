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
    // Flat JSON records: [{"Month":"Jan","Sales":120}, ...],
    // or a JSON table: [["Month","Sales"],["Jan",120],...].
    static Table loadJSON(const std::string& filePath);
    // DVL1 binary format: a magic signature and length-prefixed UTF-8 cells.
    static Table loadBinary(const std::string& filePath);
    static void saveBinary(const Table& table,const std::string& filePath);
    static Table loadJSON(const std::string& filePath);
    static Table loadBinary(const std::string& filePath);
};

#endif
