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
#include <cctype>
#include <filesystem>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <utility>
#include <algorithm>

namespace {
class FlatJsonReader {
    const std::string text_;
    std::size_t pos_ = 0;
    void spaces(){
        while(pos_<text_.size() && (text_[pos_]==' '||text_[pos_]=='\n'||
            text_[pos_]=='\r'||text_[pos_]=='\t'))++pos_;
    }
    char peek(){spaces();return pos_<text_.size()?text_[pos_]:'\0';}
    bool consume(char c){
        spaces();
        if(pos_<text_.size()&&text_[pos_]==c){++pos_;return true;}
        return false;
    }
    void require(char c){
        if(!consume(c))throw std::runtime_error("Malformed JSON: expected a delimiter");
    }
    static int hexDigit(char c){
        if(c>='0'&&c<='9')return c-'0';
        if(c>='a'&&c<='f')return c-'a'+10;
        if(c>='A'&&c<='F')return c-'A'+10;
        throw std::runtime_error("Invalid JSON Unicode escape");
    }
    std::uint32_t readHex(){
        if(pos_+4>text_.size())throw std::runtime_error("Truncated JSON Unicode escape");
        std::uint32_t value=0;
        for(int i=0;i<4;++i)value=(value<<4)|hexDigit(text_[pos_++]);
        return value;
    }
    static void utf8(std::string& out,std::uint32_t u){
        if(u<=0x7f)out+=static_cast<char>(u);
        else if(u<=0x7ff){
            out+=static_cast<char>(0xc0|(u>>6));
            out+=static_cast<char>(0x80|(u&63));
        }else if(u<=0xffff){
            out+=static_cast<char>(0xe0|(u>>12));
            out+=static_cast<char>(0x80|((u>>6)&63));
            out+=static_cast<char>(0x80|(u&63));
        }else if(u<=0x10ffff){
            out+=static_cast<char>(0xf0|(u>>18));
            out+=static_cast<char>(0x80|((u>>12)&63));
            out+=static_cast<char>(0x80|((u>>6)&63));
            out+=static_cast<char>(0x80|(u&63));
        }else throw std::runtime_error("Invalid Unicode code point");
    }
    std::string stringValue(){
        require('"');
        std::string out;
        while(pos_<text_.size()){
            const char c=text_[pos_++];
            if(c=='"')return out;
            if(static_cast<unsigned char>(c)<0x20)
                throw std::runtime_error("Unescaped control character in JSON");
            if(c!='\\'){out+=c;continue;}
            if(pos_>=text_.size())throw std::runtime_error("Truncated JSON escape");
            const char escape=text_[pos_++];
            switch(escape){
                case '"':out+='"';break;
                case '\\':out+='\\';break;
                case '/':out+='/';break;
                case 'b':out+='\b';break;
                case 'f':out+='\f';break;
                case 'n':out+='\n';break;
                case 'r':out+='\r';break;
                case 't':out+='\t';break;
                case 'u':{
                    std::uint32_t code=readHex();
                    if(code>=0xd800&&code<=0xdbff){
                        if(pos_+2>text_.size()||text_[pos_]!='\\'||text_[pos_+1]!='u')
                            throw std::runtime_error("Missing Unicode low surrogate");
                        pos_+=2;
                        const std::uint32_t low=readHex();
                        if(low<0xdc00||low>0xdfff)
                            throw std::runtime_error("Invalid Unicode low surrogate");
                        code=0x10000+((code-0xd800)<<10)+(low-0xdc00);
                    }else if(code>=0xdc00&&code<=0xdfff)
                        throw std::runtime_error("Unexpected Unicode low surrogate");
                    utf8(out,code);break;
                }
                default:throw std::runtime_error("Invalid JSON string escape");
            }
        }
        throw std::runtime_error("Unterminated JSON string");
    }
    std::string scalar(){
        if(peek()=='"')return stringValue();
        if(peek()=='{'||peek()=='[')
            throw std::runtime_error("Nested JSON cells are not supported");
        spaces();
        const std::size_t begin=pos_;
        if(text_.compare(pos_,4,"null")==0){pos_+=4;return "";}
        if(text_.compare(pos_,4,"true")==0){pos_+=4;return "true";}
        if(text_.compare(pos_,5,"false")==0){pos_+=5;return "false";}
        if(pos_<text_.size()&&text_[pos_]=='-')++pos_;
        if(pos_>=text_.size())throw std::runtime_error("Invalid JSON scalar");
        if(text_[pos_]=='0')++pos_;
        else{
            if(text_[pos_]<'1'||text_[pos_]>'9')throw std::runtime_error("Invalid JSON number");
            while(pos_<text_.size()&&std::isdigit(static_cast<unsigned char>(text_[pos_])))++pos_;
        }
        if(pos_<text_.size()&&text_[pos_]=='.'){
            ++pos_;
            const std::size_t start=pos_;
            while(pos_<text_.size()&&std::isdigit(static_cast<unsigned char>(text_[pos_])))++pos_;
            if(pos_==start)throw std::runtime_error("Invalid JSON fractional number");
        }
        if(pos_<text_.size()&&(text_[pos_]=='e'||text_[pos_]=='E')){
            ++pos_;
            if(pos_<text_.size()&&(text_[pos_]=='+'||text_[pos_]=='-'))++pos_;
            const std::size_t start=pos_;
            while(pos_<text_.size()&&std::isdigit(static_cast<unsigned char>(text_[pos_])))++pos_;
            if(pos_==start)throw std::runtime_error("Invalid JSON exponent");
        }
        return text_.substr(begin,pos_-begin);
    }
    DataLoader::Row arrayRow(){
        require('[');
        DataLoader::Row row;
        if(consume(']'))return row;
        do{row.push_back(scalar());}while(consume(','));
        require(']');
        return row;
    }
    std::vector<std::pair<std::string,std::string>> objectRow(){
        require('{');
        std::vector<std::pair<std::string,std::string>> row;
        if(consume('}'))return row;
        do{
            if(peek()!='"')throw std::runtime_error("JSON object field names must be quoted");
            std::string key=stringValue();
            require(':');
            const std::string value=scalar();
            for(const auto& e:row)
                if(e.first==key)throw std::runtime_error("Duplicate JSON object key: "+key);
            row.emplace_back(std::move(key),value);
        }while(consume(','));
        require('}');
        return row;
    }
public:
    explicit FlatJsonReader(std::string text):text_(std::move(text)){}
    DataLoader::Table table(){
        require('[');
        DataLoader::Table out;
        if(consume(']')){
            if(peek()!='\0')throw std::runtime_error("Trailing JSON content");
            return out;
        }
        if(peek()=='['){
            do{out.push_back(arrayRow());}while(consume(','));
            require(']');
        }else if(peek()=='{'){
            std::map<std::string,std::size_t> headerMap;
            out.emplace_back();
            do{
                auto fields=objectRow();
                DataLoader::Row cells(out[0].size());
                for(const auto& field:fields){
                    auto found=headerMap.find(field.first);
                    if(found==headerMap.end()){
                        const std::size_t col=out[0].size();
                        headerMap.emplace(field.first,col);
                        out[0].push_back(field.first);
                        cells.resize(out[0].size());
                        for(std::size_t i=1;i<out.size();++i)out[i].resize(out[0].size());
                        cells[col]=field.second;
                    }else cells[found->second]=field.second;
                }
                out.push_back(std::move(cells));
            }while(consume(','));
            require(']');
        }else throw std::runtime_error("JSON table must contain arrays or objects");
        if(peek()!='\0')throw std::runtime_error("Trailing JSON content");
        if(out.size()>1000000)throw std::runtime_error("JSON table exceeds row limit");
        return out;
    }
};
void littleEndian(std::ostream& stream,std::uint32_t n){
    for(int i=0;i<4;++i)stream.put(static_cast<char>((n>>(8*i))&0xff));
    if(!stream.good())throw std::runtime_error("Binary write failed");
}
std::uint32_t littleEndian(std::istream& stream){
    std::uint32_t result=0;
    for(int i=0;i<4;++i){
        const int ch=stream.get();
        if(ch==std::char_traits<char>::eof())throw std::runtime_error("Truncated binary data");
        result|=static_cast<std::uint32_t>(static_cast<unsigned char>(ch))<<(8*i);
    }
    return result;
}
}
DataLoader::Table DataLoader::loadJSON(const std::string& path){
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file)throw std::runtime_error("Cannot open JSON: "+path);
    const auto length=file.tellg();
    if(length<0||length>64*1024*1024)
        throw std::runtime_error("JSON exceeds 64 MB limit");
    file.seekg(0);
    std::string text((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    if(text.size()>=3&&static_cast<unsigned char>(text[0])==0xef&&
       static_cast<unsigned char>(text[1])==0xbb&&static_cast<unsigned char>(text[2])==0xbf)
        text.erase(0,3);
    return FlatJsonReader(std::move(text)).table();
}
DataLoader::Table DataLoader::loadBinary(const std::string& path){
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file)throw std::runtime_error("Cannot open DVL1 binary data: "+path);
    const auto length=file.tellg();
    if(length<0||length>128*1024*1024)
        throw std::runtime_error("Binary dataset exceeds 128 MB limit");
    file.seekg(0);
    char magic[4]={};

    if(!file.read(magic,4)||std::string(magic,4)!="DVZ1")
        throw std::runtime_error("Invalid DVZ1 binary signature");
    const std::uint32_t rows=littleEndian(file);
    const std::uint32_t columns=littleEndian(file);
    if(columns>4096||static_cast<std::uint64_t>(rows)*columns>4000000)
        throw std::runtime_error("Binary column count limit exceeded");
    if(rows>1000000)throw std::runtime_error("Binary row count limit exceeded");
    Table table;
    table.reserve(rows);

    for(std::uint32_t i=0;i<rows;++i){
        Row row;
        row.reserve(columns);
        for(std::uint32_t j=0;j<columns;++j){
            const std::uint32_t size=littleEndian(file);
            if(size>2*1024*1024)throw std::runtime_error("Oversized binary field");
            std::string cell(size,'\0');
            if(size && !file.read(&cell[0],static_cast<std::streamsize>(size)))
                throw std::runtime_error("Truncated binary field");
            row.push_back(std::move(cell));
        }
        table.push_back(std::move(row));
    }
    if(file.peek()!=std::char_traits<char>::eof())
        throw std::runtime_error("Unexpected trailing DVL1 binary content");
    return table;
}
void DataLoader::saveBinary(const Table& table,const std::string& path){
    if(table.size()>1000000)throw std::invalid_argument("Too many binary rows");
    const std::filesystem::path destination(path);
    if(destination.has_parent_path())
        std::filesystem::create_directories(destination.parent_path());
    std::ofstream file(path,std::ios::binary|std::ios::trunc);
    if(!file)throw std::runtime_error("Cannot create binary data: "+path);

    // DVZ1 stores fixed-width tables (matching Graph Studio's CSV columns).
    const std::size_t columns=table.empty()?0:table.front().size();
    if(columns>4096 || table.size()*columns>4000000)
        throw std::invalid_argument("Binary table exceeds column limits");
    for(const Row& row:table)
        if(row.size()!=columns)
            throw std::invalid_argument("DVZ1 requires rectangular tables");
    file.write("DVZ1",4);
    littleEndian(file,static_cast<std::uint32_t>(table.size()));
    littleEndian(file,static_cast<std::uint32_t>(columns));
    for(const Row& row:table){
        for(const auto& cell:row){
            if(cell.size()>2*1024*1024)throw std::invalid_argument("Binary cell too large");
            littleEndian(file,static_cast<std::uint32_t>(cell.size()));
            file.write(cell.data(),static_cast<std::streamsize>(cell.size()));
            if(!file.good())throw std::runtime_error("Binary write failed");
        }
    }
}
