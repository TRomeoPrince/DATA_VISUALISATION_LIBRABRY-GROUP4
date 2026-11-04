#include "SVGExporter.hpp"
#include "ImageExporter.hpp"
#include <stdexcept>
void SVGExporter::save(const std::string& svg,const std::string& path){
if(svg.find("<svg")==std::string::npos||svg.find("</svg>")==std::string::npos)
throw std::invalid_argument("Invalid SVG document");
ImageExporter::saveSVG(svg,path);}