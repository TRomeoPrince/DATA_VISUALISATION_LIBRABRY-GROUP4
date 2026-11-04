#include "DataLoader.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
void writeU32(std::ofstream& f,std::uint32_t x){
    char data[4]={static_cast<char>(x&255),static_cast<char>((x>>8)&255),
                  static_cast<char>((x>>16)&255),static_cast<char>((x>>24)&255)};
    f.write(data,4);
}
void writeCell(std::ofstream& f,const std::string& text){
    writeU32(f,static_cast<std::uint32_t>(text.size()));
    f.write(text.data(),static_cast<std::streamsize>(text.size()));
}
int main(){
    {
        std::ofstream f("test_objects.json");
        f<<"[{\"Month\":\"Jan\",\"Sales\":120},{\"Month\":\"Feb\",\"Sales\":145},"
            "{\"Month\":\"Mar\",\"Note\":\"A & B\"}]";
    }
    auto objects=DataLoader::loadJSON("test_objects.json");
    assert(objects.size()==4&&objects[0].size()==3);
    assert(objects[0][0]=="Month"&&objects[0][1]=="Sales");
    assert(objects[1][0]=="Jan"&&objects[2][1]=="145");
    assert(objects[3][1].empty()&&objects[3][2]=="A & B");
    std::remove("test_objects.json");
    {
        std::ofstream f("test_arrays.json");
        f<<"[[\"Month\",\"Sales\"],[\"Jan\",120],[\"Feb\",145]]";
    }
    auto rows=DataLoader::loadJSON("test_arrays.json");
    assert(rows.size()==3&&rows[2][0]=="Feb");
    std::remove("test_arrays.json");
    {
        std::ofstream f("test_data.dvz",std::ios::binary);
        f.write("DVZ1",4);writeU32(f,3);writeU32(f,2);
        for(const std::string& cell:{"Month","Sales","Jan","120","Feb","145"})
            writeCell(f,cell);
    }
    auto binary=DataLoader::loadBinary("test_data.dvz");
    assert(binary.size()==3&&binary[2][1]=="145");
    std::remove("test_data.dvz");
    {
        std::ofstream f("test_invalid.json");
        f<<"[{\"unsupported\":[1,2]}]";
    }
    bool threw=false;
    try{DataLoader::loadJSON("test_invalid.json");}
    catch(const std::runtime_error&){threw=true;}
    assert(threw);
    std::remove("test_invalid.json");
}