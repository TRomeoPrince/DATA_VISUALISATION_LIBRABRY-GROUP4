#include "ImageExporter.hpp"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
int main(){
    const std::vector<std::uint8_t> rgb={
        255,0,0, 0,255,0,
        0,0,255, 255,255,0
    };
    ImageExporter::savePNG(rgb,2,2,"test_raster.png");
    const auto path=ImageExporter::outputFile("test_raster.png");
    std::ifstream file(path,std::ios::binary);
    assert(file.good());
    unsigned char signature[8]{};
    file.read(reinterpret_cast<char*>(signature),8);
    assert(file.gcount()==8&&signature[0]==137&&signature[1]=='P'&&signature[2]=='N'&&signature[3]=='G');
    file.close();
    std::filesystem::remove(path);
    bool rejected=false;
    try{ImageExporter::savePNG({0,0,0},2,2,"wrong.png");}
    catch(const std::invalid_argument&){rejected=true;}
    assert(rejected);
#ifdef _WIN32
    ImageExporter::saveJPG(rgb,2,2,"test_raster.jpg",90);
    const auto jpg=ImageExporter::outputFile("test_raster.jpg");
    std::ifstream input(jpg,std::ios::binary);
    assert(input.good());
    unsigned char marker[2]{};
    input.read(reinterpret_cast<char*>(marker),2);
    assert(marker[0]==0xff&&marker[1]==0xd8);
    input.close();
    std::filesystem::remove(jpg);
#endif
}
