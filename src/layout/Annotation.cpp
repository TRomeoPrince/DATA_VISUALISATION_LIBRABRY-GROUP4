#include "Annotation.hpp"

// I keep the notes in the order they were added to the graph.
void Annotation::add(double x, double y, const std::string& text) {
    items.push_back({x, y, text});
}

void Annotation::clear() {
    items.clear();
}

const std::vector<AnnotationItem>& Annotation::getItems() const {
    return items;
}
