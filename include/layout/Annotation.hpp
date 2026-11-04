#ifndef ANNOTATION_HPP
#define ANNOTATION_HPP

#include <string>
#include <vector>

// I want to keep the text and position of each note together.
// Later, the Renderer can place these notes on the graph.
struct AnnotationItem {
    double x;
    double y;
    std::string text;
};

class Annotation {
private:
    // The rest of the library will use this public interface.
    std::vector<AnnotationItem> items;

public:
    void add(double x, double y, const std::string& text);
    void clear();
    const std::vector<AnnotationItem>& getItems() const;
};

#endif
