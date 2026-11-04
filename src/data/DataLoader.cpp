#include "DataLoader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

// Read the file line by line and separate each line into its columns.
DataLoader::Table DataLoader::loadCSV(const std::string& filePath, char delimiter) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filePath);
    }

    Table table;
    std::string line;

    while (std::getline(file, line)) {
        Row row;
        std::stringstream stream(line);
        std::string cell;

        while (std::getline(stream, cell, delimiter)) {
            row.push_back(cell);
        }

        // Preserve a trailing empty CSV field, for example: a,b,
        if (!line.empty() && line.back() == delimiter) {
            row.push_back("");
        }

        table.push_back(row);
    }

    return table;
}
