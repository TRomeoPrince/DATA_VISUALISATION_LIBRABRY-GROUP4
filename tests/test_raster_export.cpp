#include "ImageExporter.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <vector>
int main(){
    std::vector<std::uint8_t> rgba={
        255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,255,255
    };
    bool rejected=false;
    try{ImageExporter::savePNG({},2,2,"bad.png");}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected);
#ifdef _WIN32
    ImageExporter::savePNG(rgba,2,2,"test_raster.png");
    ImageExporter::saveJPG(rgba,2,2,"test_raster.jpg",85);
    for(const char* name:{"test_raster.png","test_raster.jpg"}){
        const auto path=ImageExporter::outputFile(name);
        std::ifstream input(path,std::ios::binary);
        assert(input.good());
        input.seekg(0,std::ios::end);
        assert(input.tellg()>20);
        input.close();
        std::remove(path.c_str());
    }
#else
    bool unsupported=false;
    try{ImageExporter::savePNG(rgba,2,2,"test.png");}
    catch(const std::runtime_error&){unsupported=true;}
    assert(unsupported);
#endif
}