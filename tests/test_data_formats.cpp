#include "DataLoader.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
int main(){
    const char* a="format_records.json";
    const char* b="format_arrays.json";
    const char* c="format_data.dvl";
    {
        std::ofstream f(a,std::ios::binary);
        f<<"[{\"Month\":\"Jan\",\"Sales\":120.5,\"Active\":true},"
          <<"{\"Month\":\"Feb\",\"Sales\":200,\"Note\":\"line\\nnext\"},"
          <<"{\"Month\":\"Mar\",\"Sales\":null,\"Unicode\":\"\\u00E4\"}]";
    }
    auto records=DataLoader::loadJSON(a);
    assert(records.size()==4);
    assert(records[0].size()==5);
    assert(records[0][0]=="Month");
    assert(records[0][1]=="Sales");
    assert(records[1][0]=="Jan");
    assert(records[2][1]=="200");
    assert(records[2][3]=="line\nnext");
    assert(records[3][1]=="");
    assert(records[3][4]==std::string("\xC3\xA4",2));
    {
        std::ofstream f(b);
        f<<"[[\"Month\",\"Sales\"],[\"Jan\",15],[\"Feb\",2.5e2]]";
    }
    auto array=DataLoader::loadJSON(b);
    assert(array.size()==3&&array[2][1]=="2.5e2");
    DataLoader::saveBinary(records,c);
    auto roundtrip=DataLoader::loadBinary(c);
    assert(roundtrip==records);
    bool threw=false;
    try{
        std::ofstream f(a);f<<"[{\"A\": {\"deep\":1}}]";
        f.close();
        (void)DataLoader::loadJSON(a);
    }catch(const std::runtime_error&){threw=true;}
    assert(threw);
    std::remove(a);std::remove(b);std::remove(c);
}
