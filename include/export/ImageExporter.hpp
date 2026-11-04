#ifndef IMAGEEXPORTER_HPP
#define IMAGEEXPORTER_HPP

#include <string>

// The Renderer gives us the SVG text, but we still need to save it.
// By default I want the finished images in the project's data/output folder.
class ImageExporter {
public:
    // This tells us the full location where a graph will be saved.
    static std::string outputFile(const std::string& filename);

    // Save the SVG and create its destination folder if necessary.
    static void saveSVG(const std::string& svg, const std::string& filePath);
};

#endif
