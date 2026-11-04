#include "ImageExporter.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

std::string ImageExporter::outputFile(const std::string& filename) {
    if (filename.empty()) {
        throw std::invalid_argument("Output filename cannot be empty.");
    }

    std::filesystem::path destination(filename);

    // If only a filename is given, I want it inside data/output.
    // Absolute and explicitly named directory paths still work normally.
    if (!destination.is_absolute() && !destination.has_parent_path()) {
#ifdef DATAVIZ_DEFAULT_OUTPUT_DIR
        destination = std::filesystem::path(DATAVIZ_DEFAULT_OUTPUT_DIR) / destination;
#else
        destination = std::filesystem::path("data") / "output" / destination;
#endif
    }

    return destination.string();
}

// Writing the SVG to a file is the final step after rendering.
void ImageExporter::saveSVG(
    const std::string& svg,
    const std::string& filePath
) {
    const std::filesystem::path destination(outputFile(filePath));

    // The output directory might not exist on another person's computer.
    if (destination.has_parent_path()) {
        std::filesystem::create_directories(destination.parent_path());
    }

    std::ofstream file(destination);

    if (!file.is_open()) {
        throw std::runtime_error("Could not create output file: " + destination.string());
    }

    file << svg;

    if (!file.good()) {
        throw std::runtime_error("Failed while writing output file: " + destination.string());
    }
}
