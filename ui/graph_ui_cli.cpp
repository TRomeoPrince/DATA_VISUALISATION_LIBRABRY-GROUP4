#include "DataLoader.hpp"
#include "DataProcessor.hpp"
#include "CountPlot.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"
#include "Renderer.hpp"
#include "ImageExporter.hpp"
#include <algorithm>
#include <cctype>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// This small command-line program connects our C++ library to the browser UI.
// All graph calculations still happen inside the existing group modules.
int main(int argc, char* argv[]) {
    try {
        if (argc != 8) {
            throw std::invalid_argument(
                "Usage: graph_ui_cli <csv> <plot> <column> <title> <bins> <filename> <width>");
        }
        const std::string csvPath = argv[1];
        const std::string plot = argv[2];
        std::size_t column = static_cast<std::size_t>(std::stoul(argv[3]));
        const std::string title = argv[4];
        const std::size_t bins = static_cast<std::size_t>(std::stoul(argv[5]));
        const std::string filename = argv[6];
        const int width = std::stoi(argv[7]);

        if (width < 400 || width > 1600 || bins < 1 || bins > 100 ||
            filename.empty() || filename.find('/') != std::string::npos ||
            filename.find('\\') != std::string::npos ||
            filename == "." || filename == ".." ||
            filename.size() < 5 || filename.substr(filename.size() - 4) != ".svg") {
            throw std::invalid_argument("Invalid graph settings or output filename.");
        }
        const auto rows = DataProcessor::cleanRows(DataLoader::loadCSV(csvPath));
        if (rows.size() < 2 || column >= rows.front().size()) {
            throw std::invalid_argument("CSV must have a header, at least one data row, and a valid column.");
        }
        const auto values = DataProcessor::getColumn(rows, column);
        if (values.empty()) {
            throw std::invalid_argument("Selected column is empty.");
        }

        std::string svg;
        if (plot == "count") {
            svg = Renderer::renderCountPlot(
                CountPlot::calculate(values), width, 520, title);
        } else if (plot == "histogram" || plot == "kde") {
            const auto numeric = DataProcessor::toNumeric(values);
            if (numeric.empty()) {
                throw std::invalid_argument("Select a numeric column with valid numbers.");
            }
            if (plot == "histogram") {
                svg = Renderer::renderHistogram(
                    Histogram::build(numeric, bins), width, 520, title);
            } else {
                svg = Renderer::renderKDE(
                    KDEPlot::estimate(numeric, 160), width, 520, title);
            }
        } else {
            throw std::invalid_argument("Unknown plot type.");
        }
        ImageExporter::saveSVG(svg, filename);
        std::cout << ImageExporter::outputFile(filename) << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Graph error: " << e.what() << "\n";
        return 1;
    }
}
