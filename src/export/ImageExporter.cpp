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

#include <vector>
#include <cstdint>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gdiplus.h>
#include <cwchar>
#endif
namespace {
void checkRaster(const std::vector<std::uint8_t>& rgba,int width,int height){
    if(width<=0||height<=0||width>16000||height>16000)
        throw std::invalid_argument("Invalid image dimensions.");
    if(rgba.size()!=static_cast<std::size_t>(width)*static_cast<std::size_t>(height)*4)
        throw std::invalid_argument("RGBA byte count must equal width*height*4.");
}
#ifdef _WIN32
struct GdiSession {
    ULONG_PTR token=0;
    GdiSession(){
        Gdiplus::GdiplusStartupInput init;
        if(Gdiplus::GdiplusStartup(&token,&init,nullptr)!=Gdiplus::Ok)
            throw std::runtime_error("GDI+ startup failed.");
    }
    ~GdiSession(){if(token)Gdiplus::GdiplusShutdown(token);}
};
CLSID codecFor(const wchar_t* mime){
    UINT count=0,bytes=0;
    if(Gdiplus::GetImageEncodersSize(&count,&bytes)!=Gdiplus::Ok||!bytes)
        throw std::runtime_error("No image encoders available.");
    std::vector<BYTE> buffer(bytes);
    auto* codecs=reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    if(Gdiplus::GetImageEncoders(count,bytes,codecs)!=Gdiplus::Ok)
        throw std::runtime_error("Cannot enumerate image encoders.");
    for(UINT i=0;i<count;++i)
        if(std::wcscmp(codecs[i].MimeType,mime)==0)return codecs[i].Clsid;
    throw std::runtime_error("Requested image encoder is unavailable.");
}
void encodeRaster(const std::vector<std::uint8_t>& rgba,int width,int height,
                  const std::string& filePath,bool jpeg,int quality){
    if(jpeg&&(quality<1||quality>100))
        throw std::invalid_argument("JPEG quality must be 1..100.");
    GdiSession session;
    Gdiplus::Bitmap bitmap(width,height,PixelFormat32bppARGB);
    if(bitmap.GetLastStatus()!=Gdiplus::Ok)
        throw std::runtime_error("Cannot allocate image.");
    Gdiplus::Rect bounds(0,0,width,height);
    Gdiplus::BitmapData pixels{};
    if(bitmap.LockBits(&bounds,Gdiplus::ImageLockModeWrite,PixelFormat32bppARGB,&pixels)!=Gdiplus::Ok)
        throw std::runtime_error("Cannot lock image pixels.");
    for(int y=0;y<height;++y){
        auto* dest=reinterpret_cast<BYTE*>(pixels.Scan0)+
                    static_cast<std::ptrdiff_t>(y)*pixels.Stride;
        for(int x=0;x<width;++x){
            const auto* src=&rgba[(static_cast<std::size_t>(y)*width+x)*4];
            dest[x*4+0]=src[2];dest[x*4+1]=src[1];
            dest[x*4+2]=src[0];dest[x*4+3]=src[3];
        }
    }
    bitmap.UnlockBits(&pixels);
    auto path=ImageExporter::outputFile(filePath);
    std::filesystem::path destination(path);
    if(destination.has_parent_path())
        std::filesystem::create_directories(destination.parent_path());
    const auto codec=codecFor(jpeg?L"image/jpeg":L"image/png");
    Gdiplus::Status outcome;
    if(jpeg){
        Gdiplus::EncoderParameters settings{};
        settings.Count=1;
        ULONG q=static_cast<ULONG>(quality);
        settings.Parameter[0].Guid=Gdiplus::EncoderQuality;
        settings.Parameter[0].Type=Gdiplus::EncoderParameterValueTypeLong;
        settings.Parameter[0].NumberOfValues=1;
        settings.Parameter[0].Value=&q;
        outcome=bitmap.Save(destination.wstring().c_str(),&codec,&settings);
    }else outcome=bitmap.Save(destination.wstring().c_str(),&codec,nullptr);
    if(outcome!=Gdiplus::Ok)
        throw std::runtime_error("Image encoding failed: "+destination.string());
}
#endif
}
void ImageExporter::savePNG(const std::vector<std::uint8_t>& rgba,int width,
                            int height,const std::string& filePath){
    checkRaster(rgba,width,height);
#ifdef _WIN32
    encodeRaster(rgba,width,height,filePath,false,100);
#else
    (void)filePath;
    throw std::runtime_error("PNG raster encoding requires the Windows GDI+ backend.");
#endif
}
void ImageExporter::saveJPG(const std::vector<std::uint8_t>& rgba,int width,
                            int height,const std::string& filePath,int quality){
    checkRaster(rgba,width,height);
#ifdef _WIN32
    encodeRaster(rgba,width,height,filePath,true,quality);
#else
    (void)filePath;(void)quality;
    throw std::runtime_error("JPG encoding requires the Windows GDI+ backend.");
#endif
}
