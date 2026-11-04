#include "DataLoader.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
int main(){
    const char* path="quoted_csv_test.csv";
    {
        std::ofstream f(path,std::ios::binary);
        f<<"Name,Note,Value\r\n"
         <<"\"A, B\",\"He said \"\"yes\"\"\",12\r\n"
         <<"C,\"two\nlines\",8\r\n"
         <<"D,tail,\r\n";
    }
    auto t=DataLoader::loadCSV(path);std::remove(path);
    assert(t.size()==4);
    assert(t[0][0]=="Name"&&t[0][2]=="Value");
    assert(t[1][0]=="A, B");
    assert(t[1][1]=="He said \"yes\"");
    assert(t[2][1]=="two\nlines");
    assert(t[3][2]=="");
}