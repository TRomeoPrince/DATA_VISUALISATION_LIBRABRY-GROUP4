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


#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdiplus.h>
#endif

namespace {
using Byte=std::uint8_t;
using Bytes=std::vector<Byte>;
void validateImage(const Bytes& rgb,int width,int height){
    if(width<=0||height<=0)throw std::invalid_argument("Image dimensions must be positive");
    const auto w=static_cast<std::size_t>(width),h=static_cast<std::size_t>(height);
    if(w>std::numeric_limits<std::size_t>::max()/3/h ||
       rgb.size()!=w*h*3)throw std::invalid_argument("Expected width*height*3 RGB bytes");
}
void be32(Bytes& out,std::uint32_t n){
    out.push_back(static_cast<Byte>(n>>24));
    out.push_back(static_cast<Byte>(n>>16));
    out.push_back(static_cast<Byte>(n>>8));
    out.push_back(static_cast<Byte>(n));
}
std::uint32_t crc32(const Bytes& bytes,std::size_t from,std::size_t until){
    std::uint32_t crc=0xffffffffu;
    for(std::size_t i=from;i<until;++i){
        crc^=bytes[i];
        for(int k=0;k<8;++k)crc=(crc>>1)^(0xedb88320u&static_cast<std::uint32_t>(-(static_cast<int>(crc&1))));
    }
    return crc^0xffffffffu;
}
void chunk(Bytes& result,const char* type,const Bytes& data){
    be32(result,static_cast<std::uint32_t>(data.size()));
    const std::size_t start=result.size();
    for(int i=0;i<4;++i)result.push_back(static_cast<Byte>(type[i]));
    result.insert(result.end(),data.begin(),data.end());
    be32(result,crc32(result,start,result.size()));
}
void saveBytes(const Bytes& bytes,const std::string& filePath){
    const std::filesystem::path destination(ImageExporter::outputFile(filePath));
    if(destination.has_parent_path())std::filesystem::create_directories(destination.parent_path());
    std::ofstream file(destination,std::ios::binary);
    if(!file)throw std::runtime_error("Could not create image: "+destination.string());
    file.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    if(!file.good())throw std::runtime_error("Writing image failed: "+destination.string());
}
}
void ImageExporter::savePNG(const std::vector<std::uint8_t>& rgb,int width,int height,
                            const std::string& filePath){
    validateImage(rgb,width,height);
    const auto w=static_cast<std::size_t>(width),h=static_cast<std::size_t>(height);
    Bytes raw;
    raw.reserve(h*(w*3+1));
    for(std::size_t y=0;y<h;++y){
        raw.push_back(0); // PNG filter type 0
        raw.insert(raw.end(),rgb.begin()+static_cast<std::ptrdiff_t>(y*w*3),
                   rgb.begin()+static_cast<std::ptrdiff_t>((y+1)*w*3));
    }
    // Standards-compliant zlib stream with stored (uncompressed) DEFLATE blocks.
    Bytes zlib{0x78,0x01};
    std::size_t offset=0;
    while(offset<raw.size()){
        const std::size_t count=std::min<std::size_t>(65535,raw.size()-offset);
        const bool final=offset+count==raw.size();
        zlib.push_back(final?1:0);
        const auto length=static_cast<std::uint16_t>(count);
        const std::uint16_t inverse=static_cast<std::uint16_t>(~length);
        for(std::uint16_t n:{length,inverse}){
            zlib.push_back(static_cast<Byte>(n));
            zlib.push_back(static_cast<Byte>(n>>8));
        }
        zlib.insert(zlib.end(),raw.begin()+static_cast<std::ptrdiff_t>(offset),
                    raw.begin()+static_cast<std::ptrdiff_t>(offset+count));
        offset+=count;
    }
    std::uint32_t a=1,b=0;
    for(Byte v:raw){a=(a+v)%65521;b=(b+a)%65521;}
    be32(zlib,(b<<16)|a);
    Bytes png{137,80,78,71,13,10,26,10};
    Bytes ihdr;
    be32(ihdr,static_cast<std::uint32_t>(width));
    be32(ihdr,static_cast<std::uint32_t>(height));
    ihdr.insert(ihdr.end(),{8,2,0,0,0}); // RGB, 8-bit
    chunk(png,"IHDR",ihdr);
    chunk(png,"IDAT",zlib);
    chunk(png,"IEND",{});
    saveBytes(png,filePath);
}
void ImageExporter::saveJPG(const std::vector<std::uint8_t>& rgb,int width,int height,
                            const std::string& filePath,int quality){
    validateImage(rgb,width,height);
    if(quality<1||quality>100)throw std::invalid_argument("JPEG quality must be 1..100");
#ifdef _WIN32
    using namespace Gdiplus;
    GdiplusStartupInput input;
    ULONG_PTR token=0;
    if(GdiplusStartup(&token,&input,nullptr)!=Ok)
        throw std::runtime_error("Windows JPEG encoder could not initialize");
    try{
        Bitmap bitmap(width,height,PixelFormat24bppRGB);
        Rect rect(0,0,width,height);
        BitmapData locked{};
        if(bitmap.LockBits(&rect,ImageLockModeWrite,PixelFormat24bppRGB,&locked)!=Ok)
            throw std::runtime_error("JPEG bitmap access failed");
        for(int y=0;y<height;++y){
            const auto row=static_cast<const std::uint8_t*>(rgb.data())+
                           static_cast<std::size_t>(y)*static_cast<std::size_t>(width)*3;
            auto* pixels=static_cast<std::uint8_t*>(locked.Scan0)+
                         static_cast<std::ptrdiff_t>(y)*locked.Stride;
            for(int x=0;x<width;++x){
                pixels[x*3]=row[x*3+2];
                pixels[x*3+1]=row[x*3+1];
                pixels[x*3+2]=row[x*3];
            }
        }
        bitmap.UnlockBits(&locked);
        UINT codecCount=0,bytes=0;
        GetImageEncodersSize(&codecCount,&bytes);
        if(bytes==0)throw std::runtime_error("No Windows JPEG encoder found");
        std::vector<BYTE> buffer(bytes);
        auto* codecs=reinterpret_cast<ImageCodecInfo*>(buffer.data());
        if(GetImageEncoders(codecCount,bytes,codecs)!=Ok)
            throw std::runtime_error("Cannot enumerate JPEG encoders");
        CLSID jpeg{};
        bool found=false;
        for(UINT i=0;i<codecCount;++i){
            if(codecs[i].MimeType&&std::wstring(codecs[i].MimeType)==L"image/jpeg"){
                jpeg=codecs[i].Clsid;found=true;break;
            }
        }
        if(!found)throw std::runtime_error("No JPEG encoder available");
        EncoderParameters params{};
        ULONG q=static_cast<ULONG>(quality);
        params.Count=1;
        params.Parameter[0].Guid=EncoderQuality;
        params.Parameter[0].Type=EncoderParameterValueTypeLong;
        params.Parameter[0].NumberOfValues=1;
        params.Parameter[0].Value=&q;
        const auto destination=std::filesystem::path(outputFile(filePath));
        if(destination.has_parent_path())std::filesystem::create_directories(destination.parent_path());
        if(bitmap.Save(destination.wstring().c_str(),&jpeg,&params)!=Ok)
            throw std::runtime_error("JPEG encoding failed");
        GdiplusShutdown(token);
    }catch(...){GdiplusShutdown(token);throw;}
#else
    (void)filePath;
    throw std::runtime_error("JPG export requires Windows GDI+");
#endif
}
