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
#include <cstdint>
#include <sstream>
#include <unordered_map>
#include <utility>
namespace {
struct JSON {
    const std::string& src;
    std::size_t pos=0;
    void skip(){while(pos<src.size()&&(src[pos]==' '||src[pos]=='\n'||src[pos]=='\r'||src[pos]=='\t'))++pos;}
    char peek(){skip();return pos<src.size()?src[pos]:'\0';}
    bool take(char c){skip();if(pos<src.size()&&src[pos]==c){++pos;return true;}return false;}
    void expect(char c){if(!take(c))throw std::runtime_error("Malformed JSON document");}
    std::string str(){
        expect('"');std::string result;
        while(pos<src.size()){
            char c=src[pos++];
            if(c=='"')return result;
            if(static_cast<unsigned char>(c)<32)throw std::runtime_error("Invalid JSON text");
            if(c!='\\'){result.push_back(c);continue;}
            if(pos>=src.size())break;
            char e=src[pos++];
            switch(e){
                case '"':result.push_back('"');break;
                case '\\':result.push_back('\\');break;
                case '/':result.push_back('/');break;
                case 'n':result.push_back('\n');break;
                case 'r':result.push_back('\r');break;
                case 't':result.push_back('\t');break;
                case 'b':result.push_back('\b');break;
                case 'f':result.push_back('\f');break;
                default:throw std::runtime_error("Unsupported JSON character escape");
            }
        }
        throw std::runtime_error("Unterminated JSON string");
    }
    std::string scalar(){
        char c=peek();
        if(c=='"')return str();
        if(c=='['||c=='{'||c=='\0')throw std::runtime_error("Only flat JSON tables are supported");
        auto start=pos;
        while(pos<src.size()){
            c=src[pos];
            if(c==','||c=='}'||c==']'||c==' '||c=='\r'||c=='\n'||c=='\t')break;
            ++pos;
        }
        std::string value=src.substr(start,pos-start);
        if(value=="null")return "";
        if(value=="true"||value=="false")return value;
        if(value.empty())throw std::runtime_error("Missing JSON value");
        try{
            std::size_t n=0;
            (void)std::stod(value,&n);
            if(n!=value.size())throw std::runtime_error("Invalid JSON number");
        }catch(const std::exception&){throw std::runtime_error("Invalid JSON scalar value");}
        return value;
    }
    DataLoader::Row array(){
        expect('[');DataLoader::Row row;
        if(take(']'))return row;
        do{row.push_back(scalar());}while(take(','));
        expect(']');return row;
    }
    std::vector<std::pair<std::string,std::string>> object(){
        expect('{');std::vector<std::pair<std::string,std::string>> row;
        if(take('}'))return row;
        do{auto key=str();expect(':');row.emplace_back(key,scalar());}while(take(','));
        expect('}');return row;
    }
    DataLoader::Table table(){
        expect('[');DataLoader::Table result;
        if(take(']'))return result;
        const bool objects=peek()=='{';
        std::unordered_map<std::string,std::size_t> keys;
        do{
            if(objects){
                auto values=object();
                if(result.empty())result.push_back({});
                DataLoader::Row row(result.front().size(),"");
                for(const auto& item:values){
                    auto it=keys.find(item.first);
                    std::size_t index=0;
                    if(it==keys.end()){
                        index=keys.size();keys.emplace(item.first,index);
                        result.front().push_back(item.first);
                        for(std::size_t i=1;i<result.size();++i)result[i].push_back("");
                        row.push_back("");
                    }else index=it->second;
                    row[index]=item.second;
                }
                result.push_back(std::move(row));
            }else{
                if(peek()!='[')throw std::runtime_error("Expected JSON array row");
                result.push_back(array());
            }
        }while(take(','));
        expect(']');
        if(peek()!='\0')throw std::runtime_error("Unexpected JSON text");
        return result;
    }
};
std::uint32_t u32(std::istream& stream){
    unsigned char b[4]{};
    stream.read(reinterpret_cast<char*>(b),4);
    if(!stream)throw std::runtime_error("Truncated DVZ binary file");
    return static_cast<std::uint32_t>(b[0]) |
           (static_cast<std::uint32_t>(b[1])<<8) |
           (static_cast<std::uint32_t>(b[2])<<16) |
           (static_cast<std::uint32_t>(b[3])<<24);
}
}
DataLoader::Table DataLoader::loadJSON(const std::string& path){
    std::ifstream file(path,std::ios::binary);
    if(!file)throw std::runtime_error("Cannot open JSON: "+path);
    std::ostringstream contents;contents<<file.rdbuf();
    std::string text=contents.str();
    JSON parser{text};return parser.table();
}
DataLoader::Table DataLoader::loadBinary(const std::string& path){
    std::ifstream file(path,std::ios::binary);
    if(!file)throw std::runtime_error("Cannot open binary data: "+path);
    char magic[4]{};
    file.read(magic,4);
    if(!file||std::string(magic,4)!="DVZ1")
        throw std::runtime_error("Binary input requires DVZ1 format");
    auto rows=u32(file),cols=u32(file);
    if(rows>1000000||cols>10000||
       static_cast<std::uint64_t>(rows)*cols>10000000)
       throw std::runtime_error("Binary table is too large");
    Table result;result.reserve(rows);
    for(std::uint32_t r=0;r<rows;++r){
        Row row;row.reserve(cols);
        for(std::uint32_t c=0;c<cols;++c){
            auto len=u32(file);
            if(len>16000000)throw std::runtime_error("Binary field is too large");
            std::string value(len,'\0');
            if(len)file.read(&value[0],len);
            if(!file)throw std::runtime_error("Truncated binary field");
            row.push_back(std::move(value));
        }
        result.push_back(std::move(row));
    }
    if(file.peek()!=std::char_traits<char>::eof())
        throw std::runtime_error("Unexpected binary trailing data");
    return result;
}
