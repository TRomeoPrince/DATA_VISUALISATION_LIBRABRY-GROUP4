#include "DataLoader.hpp"
#include <fstream>
#include <stdexcept>
DataLoader::Table DataLoader::loadCSV(const std::string& filename,char delimiter){
    std::ifstream file(filename,std::ios::binary);
    if(!file)throw std::runtime_error("Could not open CSV: "+filename);
    if(file.peek()==0xEF){
        char bom[3]={};file.read(bom,3);
        if(file.gcount()!=3||static_cast<unsigned char>(bom[1])!=0xBB||
           static_cast<unsigned char>(bom[2])!=0xBF){
            file.clear();file.seekg(0);
        }
    }
    Table result;Row row;std::string cell;
    bool quoted=false,started=false;
    char ch=0;
    while(file.get(ch)){
        started=true;
        if(quoted){
            if(ch=='"'){
                if(file.peek()=='"'){file.get(ch);cell+='"';}
                else quoted=false;
            }else cell+=ch;
            continue;
        }
        if(ch=='"'){
            if(!cell.empty())throw std::runtime_error("Unexpected quote in CSV field");
            quoted=true;
        }else if(ch==delimiter){
            row.push_back(cell);cell.clear();
        }else if(ch=='\r'||ch=='\n'){
            if(ch=='\r'&&file.peek()=='\n')file.get(ch);
            row.push_back(cell);cell.clear();
            result.push_back(std::move(row));row=Row{};
            started=false;
        }else cell+=ch;
    }
    if(quoted)throw std::runtime_error("CSV contains an unclosed quoted field");
    if(started||!row.empty()||!cell.empty()){
        row.push_back(cell);result.push_back(std::move(row));
    }
    return result;
}