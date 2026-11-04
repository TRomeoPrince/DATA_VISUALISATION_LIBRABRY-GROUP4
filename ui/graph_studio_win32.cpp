// Group 4 Graph Studio — native Win32 application for the reusable C++17 library.
// One desktop interface for all nine plot types, CSV labels, annotations and SVG.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include "DataLoader.hpp"
#include "DataProcessor.hpp"
#include "Statistics.hpp"
#include "CountPlot.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"
#include "LinePlot.hpp"
#include "ScatterPlot.hpp"
#include "BarChart.hpp"
#include "BoxPlot.hpp"
#include "Heatmap.hpp"
#include "PieChart.hpp"
#include "Annotation.hpp"
#include "Renderer.hpp"
#include "SVGExporter.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef DATAVIZ_SAMPLE_CSV_PATH
#define DATAVIZ_SAMPLE_CSV_PATH "data/input/sample.csv"
#endif

namespace {
namespace fs=std::filesystem;
constexpr int CW=900, CH=560;
enum :int {
    ID_BROWSE=101,ID_TYPE,ID_X,ID_Y,ID_TITLE,ID_XLABEL,ID_YLABEL,ID_BINS,
    ID_GENERATE,ID_STATS,ID_NOTE_TEXT,ID_ADD_NOTE,ID_CLEAR_NOTE,ID_AUTO_STATS,
    ID_OPEN_SVG,ID_OPEN_FOLDER
};
constexpr const char* chartNames[]={
    "Histogram","KDE Plot","Count Plot","Bar Chart","Line Plot",
    "Scatter Plot","Box Plot","Heatmap","Pie Chart"
};
struct Chart {
    int kind=-1;
    std::vector<double> x,y;
    std::vector<std::string> labels;
    std::vector<HistogramBin> bins;
    std::vector<KDEPoint> kde;
    BoxSummary box{};
    Heatmap::Matrix matrix;
    std::vector<PieSlice> pie;
    std::string title,xLabel,yLabel,svgPath;
    Annotation visible;
};
struct Studio {
    HWND window=nullptr,preview=nullptr,fileLabel=nullptr,status=nullptr;
    HWND kind=nullptr,x=nullptr,y=nullptr,title=nullptr,xLabel=nullptr,yLabel=nullptr;
    HWND bins=nullptr,noteText=nullptr,autoStats=nullptr,openSvg=nullptr;
    HFONT font=nullptr;
    fs::path csv;
    DataLoader::Table rows;
    std::vector<std::string> headers;
    Annotation notes;
    Chart chart;
    bool placing=false;
} s;
std::string getText(HWND h) {
    if(!h)return "";
    int len=GetWindowTextLengthA(h);
    std::string str(static_cast<std::size_t>(len+1),'\0');
    GetWindowTextA(h,str.data(),len+1);
    str.resize(static_cast<std::size_t>(len));
    return str;
}
int indexOf(HWND h){return static_cast<int>(SendMessageA(h,CB_GETCURSEL,0,0));}
void select(HWND h,int index){SendMessageA(h,CB_SETCURSEL,index,0);}
void set(HWND h,const std::string& v){SetWindowTextA(h,v.c_str());}
std::string fmt(double n,int precision=3){
    std::ostringstream o;o<<std::fixed<<std::setprecision(precision)<<n;return o.str();
}
std::string proper(std::string name) {
    bool start=true;
    for(char& c:name) {
        if(c=='_'||c=='-'){c=' ';start=true;}
        else if(start){c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));start=false;}
    }
    return name;
}
fs::path exeDir(){
    char path[MAX_PATH]={};
    DWORD n=GetModuleFileNameA(nullptr,path,MAX_PATH);
    return n>0&&n<MAX_PATH?fs::path(path).parent_path():fs::current_path();
}
fs::path outputDir(){
#ifdef DATAVIZ_PORTABLE_UI
    return exeDir()/"data"/"output";
#else
    return fs::path(DATAVIZ_SAMPLE_CSV_PATH).parent_path().parent_path()/"output";
#endif
}
fs::path defaultCsv(){
#ifdef DATAVIZ_PORTABLE_UI
    return exeDir()/"data"/"input"/"sample.csv";
#else
    return fs::path(DATAVIZ_SAMPLE_CSV_PATH);
#endif
}
HWND control(const char* cls,const char* text,DWORD extra,int id,int x,int y,int w,int h){
    HWND handle=CreateWindowExA(0,cls,text,WS_CHILD|WS_VISIBLE|extra,x,y,w,h,
        s.window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleA(nullptr),nullptr);
    if(s.font&&handle)SendMessageA(handle,WM_SETFONT,reinterpret_cast<WPARAM>(s.font),TRUE);
    return handle;
}
void status(const std::string& message){set(s.status,message);}
void error(const std::string& message){
    status(message);MessageBoxA(s.window,message.c_str(),"Graph Studio",MB_OK|MB_ICONWARNING);
}
bool parse(const std::string& text,double& out){
    try {
        std::string cleaned=DataProcessor::trim(text);
        if(cleaned.empty())return false;
        std::size_t used=0;out=std::stod(cleaned,&used);
        return used==cleaned.size()&&std::isfinite(out);
    }catch(...){return false;}
}
bool numericColumn(int col){
    if(col<0)return false;
    bool found=false;
    for(std::size_t i=1;i<s.rows.size();++i){
        if(static_cast<std::size_t>(col)>=s.rows[i].size())continue;
        const auto& cell=s.rows[i][col];
        if(DataProcessor::trim(cell).empty())continue;
        double v=0;if(!parse(cell,v))return false;
        found=true;
    }
    return found;
}
void bestColumns(){
    if(s.headers.empty())return;
    const int type=indexOf(s.kind);
    int firstNumeric=-1,secondNumeric=-1,firstCategory=-1;
    for(std::size_t i=0;i<s.headers.size();++i){
        if(numericColumn(static_cast<int>(i))){
            if(firstNumeric<0)firstNumeric=static_cast<int>(i);
            else if(secondNumeric<0)secondNumeric=static_cast<int>(i);
        }else if(firstCategory<0)firstCategory=static_cast<int>(i);
    }
    int x=firstNumeric>=0?firstNumeric:0;
    int y=secondNumeric>=0?secondNumeric:firstNumeric;
    if(type==2||type==3||type==4||type==8)
        x=firstCategory>=0?firstCategory:0;
    if(type==3||type==4){
        y=firstNumeric>=0?firstNumeric:0;
    }
    if(type==5){x=firstNumeric>=0?firstNumeric:0;y=secondNumeric>=0?secondNumeric:x;}
    if(type==7){x=firstNumeric>=0?firstNumeric:0;y=secondNumeric>=0?secondNumeric:x;}
    select(s.x,x);
    if(y>=0)select(s.y,y);
    std::string name=proper(s.csv.stem().string());
    if(name.empty())name="Data Visualization";
    if(type==4)set(s.title,name);
    else set(s.title,name+" - "+chartNames[std::clamp(type,0,8)]);
    set(s.xLabel,s.headers[static_cast<std::size_t>(x)]);
    if(type==0||type==2)set(s.yLabel,"Frequency");
    else if(type==1)set(s.yLabel,"Density");
    else if(type==8)set(s.yLabel,"Proportion");
    else if(y>=0&&static_cast<std::size_t>(y)<s.headers.size())
        set(s.yLabel,s.headers[static_cast<std::size_t>(y)]);
    else set(s.yLabel,"Value");
    EnableWindow(s.bins,type==0);
}
void loadCsv(const fs::path& filename){
    auto rows=DataProcessor::cleanRows(DataLoader::loadCSV(filename.string()));
    if(rows.size()<2||rows.front().empty())
        throw std::runtime_error("CSV needs a header and at least one data row.");
    s.rows=std::move(rows);
    s.headers=s.rows.front();
    s.csv=filename;s.notes.clear();s.chart=Chart{};
    for(HWND c:{s.x,s.y}){
        SendMessageA(c,CB_RESETCONTENT,0,0);
        for(std::size_t i=0;i<s.headers.size();++i){
            std::string name=s.headers[i].empty()?"Column "+std::to_string(i+1):s.headers[i];
            SendMessageA(c,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));
        }
    }
    // Month vs Sales should be a line graph rather than a histogram.
    bool monthly=false;
    for(const auto& h:s.headers){
        std::string lower=h;
        std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
        if(lower=="month"||lower=="date")monthly=true;
    }
    if(monthly)select(s.kind,4);
    bestColumns();
    set(s.fileLabel,filename.filename().string());
    EnableWindow(s.openSvg,FALSE);
    status("CSV loaded. Choose a chart and click Generate.");
    InvalidateRect(s.preview,nullptr,TRUE);
}
void browse(){
    char path[MAX_PATH]={};
    OPENFILENAMEA d{};d.lStructSize=sizeof(d);d.hwndOwner=s.window;
    d.lpstrFile=path;d.nMaxFile=MAX_PATH;
    d.lpstrFilter="CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0\0";
    d.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    if(GetOpenFileNameA(&d))loadCsv(path);
}
std::vector<double> columnNumeric(int col){
    std::vector<double> values;
    if(col<0)return values;
    for(std::size_t r=1;r<s.rows.size();++r){
        if(static_cast<std::size_t>(col)>=s.rows[r].size())continue;
        double x=0;if(parse(s.rows[r][col],x))values.push_back(x);
    }
    return values;
}
std::vector<std::string> columnText(int col){
    std::vector<std::string> values;
    if(col<0)return values;
    for(std::size_t r=1;r<s.rows.size();++r)
        if(static_cast<std::size_t>(col)<s.rows[r].size())
            values.push_back(DataProcessor::trim(s.rows[r][col]));
    return values;
}
Annotation annotationsFor(const Chart& chart,const std::vector<double>& statistics){
    Annotation a=s.notes;
    if(SendMessageA(s.autoStats,BM_GETCHECK,0,0)==BST_CHECKED&&!statistics.empty()){
        a.add(620,70,"Mean: "+fmt(Statistics::mean(statistics),4));
        a.add(620,91,"Median: "+fmt(Statistics::median(statistics),4));
        a.add(620,112,"Q1: "+fmt(Statistics::firstQuartile(statistics),4));
        a.add(620,133,"Q3: "+fmt(Statistics::thirdQuartile(statistics),4));
    }
    return a;
}
void generate(){
    if(s.rows.empty())throw std::runtime_error("Load a CSV first.");
    int kind=indexOf(s.kind),cx=indexOf(s.x),cy=indexOf(s.y);
    if(kind<0||cx<0||cy<0)throw std::runtime_error("Choose chart and columns.");
    Chart c;c.kind=kind;c.title=getText(s.title);
    c.xLabel=getText(s.xLabel);c.yLabel=getText(s.yLabel);
    if(c.title.empty())throw std::runtime_error("Enter a chart heading.");
    std::vector<double> stats;
    std::string svg;
    if(kind==0||kind==1||kind==6){
        std::vector<double> nums=columnNumeric(cx);
        if(nums.empty())throw std::runtime_error("Choose a numerical X column.");
        stats=nums;
        if(kind==0){
            int bins=std::stoi(getText(s.bins));
            if(bins<1||bins>100)throw std::runtime_error("Bins must be 1–100.");
            c.bins=Histogram::build(nums,static_cast<std::size_t>(bins));
            for(const auto& b:c.bins){c.x.push_back((b.lower+b.upper)/2);c.y.push_back(static_cast<double>(b.count));}
        }else if(kind==1){
            c.kde=KDEPlot::estimate(nums,140);
            for(const auto& p:c.kde){c.x.push_back(p.x);c.y.push_back(p.density);}
        }else c.box=BoxPlot::calculate(nums);
    }else if(kind==2||kind==8){
        auto counts=CountPlot::calculate(columnText(cx));
        if(counts.empty())throw std::runtime_error("No categories found.");
        std::vector<std::pair<std::string,double>> pairs;
        for(const auto& item:counts){
            c.labels.push_back(item.first);
            c.x.push_back(static_cast<double>(c.x.size()));
            c.y.push_back(static_cast<double>(item.second));
            pairs.push_back({item.first,static_cast<double>(item.second)});
        }
        if(kind==8)c.pie=PieChart::build(pairs);
    }else if(kind==7){
        std::vector<int> cols;
        for(std::size_t col=0;col<s.headers.size();++col)
            if(numericColumn(static_cast<int>(col)))cols.push_back(static_cast<int>(col));
        if(cols.empty())throw std::runtime_error("Heatmap requires numerical columns.");
        for(std::size_t r=1;r<s.rows.size();++r){
            std::vector<double> row;
            for(int col:cols){
                double value=0;
                if(static_cast<std::size_t>(col)>=s.rows[r].size()||
                   !parse(s.rows[r][col],value)){row.clear();break;}
                row.push_back(value);
            }
            if(!row.empty())c.matrix.push_back(row);
        }
        c.matrix=Heatmap::build(c.matrix);
    }else{
        bool numericX=numericColumn(cx);
        for(std::size_t r=1;r<s.rows.size();++r){
            if(static_cast<std::size_t>(cx)>=s.rows[r].size()||
               static_cast<std::size_t>(cy)>=s.rows[r].size())continue;
            double y=0,x=0;
            if(!parse(s.rows[r][cy],y))continue;
            if(numericX){if(!parse(s.rows[r][cx],x))continue;}
            else x=static_cast<double>(c.x.size());
            c.x.push_back(x);c.y.push_back(y);
            c.labels.push_back(DataProcessor::trim(s.rows[r][cx]));
        }
        if(c.x.empty())throw std::runtime_error("No usable paired X/Y rows.");
        if(kind==5&&!numericX)
            throw std::runtime_error("Scatter Plot requires numerical X and Y.");
        stats=c.y;
    }
    c.visible=annotationsFor(c,stats);
    if(kind==0)svg=Renderer::renderHistogram(c.bins,CW,CH,c.title,c.visible,c.xLabel,c.yLabel);
    if(kind==1)svg=Renderer::renderKDE(c.kde,CW,CH,c.title,c.visible,c.xLabel,c.yLabel);
    if(kind==2){
        CountPlot::Counts counts;
        for(std::size_t i=0;i<c.labels.size();++i)
            counts[c.labels[i]]=static_cast<std::size_t>(c.y[i]);
        svg=Renderer::renderCountPlot(counts,CW,CH,c.title,c.visible,c.xLabel,c.yLabel);
    }
    if(kind==3){
        auto bars=BarChart::build(c.labels,c.y);
        svg=Renderer::renderBarChart(bars,CW,CH,c.title,c.visible,c.xLabel,c.yLabel);
    }
    if(kind==4)svg=Renderer::renderLinePlot(LinePlot::build(c.x,c.y),CW,CH,
        c.title,c.visible,c.xLabel,c.yLabel,numericColumn(cx)?std::vector<std::string>{}:c.labels);
    if(kind==5)svg=Renderer::renderScatterPlot(ScatterPlot::build(c.x,c.y),CW,CH,
        c.title,c.visible,c.xLabel,c.yLabel);
    if(kind==6)svg=Renderer::renderBoxPlot(c.box,CW,CH,c.title,c.visible,c.xLabel);
    if(kind==7)svg=Renderer::renderHeatmap(c.matrix,CW,CH,c.title,c.visible,c.xLabel,c.yLabel);
    if(kind==8)svg=Renderer::renderPieChart(c.pie,CW,CH,c.title,c.visible);
    fs::create_directories(outputDir());
    const char* fileNames[]={
        "histogram.svg","kdeplot.svg","countplot.svg","barchart.svg",
        "lineplot.svg","scatterplot.svg","boxplot.svg","heatmap.svg","piechart.svg"
    };
    c.svgPath=(outputDir()/fileNames[kind]).string();
    SVGExporter::save(svg,c.svgPath);
    s.chart=std::move(c);
    EnableWindow(s.openSvg,TRUE);
    status("Graph generated and SVG saved: "+s.chart.svgPath);
    InvalidateRect(s.preview,nullptr,TRUE);
}
void statsDialog(){
    int y=indexOf(s.y),x=indexOf(s.x);
    auto v=columnNumeric(y);
    if(v.empty())v=columnNumeric(x);
    if(v.empty())throw std::runtime_error("Choose a numerical data column.");
    std::ostringstream out;
    out<<"Count: "<<v.size()<<"\r\n"
       <<"Mean: "<<fmt(Statistics::mean(v),4)<<"\r\n"
       <<"Median: "<<fmt(Statistics::median(v),4)<<"\r\n"
       <<"Minimum: "<<fmt(Statistics::minimum(v),4)<<"\r\n"
       <<"Maximum: "<<fmt(Statistics::maximum(v),4)<<"\r\n"
       <<"Variance: "<<fmt(Statistics::variance(v),4)<<"\r\n"
       <<"Std Dev: "<<fmt(Statistics::standardDeviation(v),4)<<"\r\n"
       <<"Q1: "<<fmt(Statistics::firstQuartile(v),4)<<"\r\n"
       <<"Q3: "<<fmt(Statistics::thirdQuartile(v),4)<<"\r\n"
       <<"IQR: "<<fmt(Statistics::interquartileRange(v),4);
    MessageBoxA(s.window,out.str().c_str(),"Statistics",MB_OK|MB_ICONINFORMATION);
}
void textAt(HDC dc,int x,int y,const std::string& v){
    TextOutA(dc,x,y,v.c_str(),static_cast<int>(v.size()));
}
COLORREF color(std::size_t n){
    const COLORREF palette[]={RGB(35,116,171),RGB(59,182,161),RGB(241,166,72),
        RGB(224,102,81),RGB(125,95,179),RGB(47,155,126)};
    return palette[n%6];
}
void draw(HDC dc){
    RECT r;GetClientRect(s.preview,&r);
    FillRect(dc,&r,reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    if(s.chart.kind<0){
        SetBkMode(dc,TRANSPARENT);
        textAt(dc,55,45,"Choose a CSV and Generate Graph to see a preview.");
        return;
    }
    // Match SVG's 900 x 560 logical canvas coordinates.
    SetMapMode(dc,MM_ANISOTROPIC);
    SetWindowExtEx(dc,CW,CH,nullptr);
    SetViewportExtEx(dc,std::max(1,r.right),std::max(1,r.bottom),nullptr);
    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,RGB(35,48,68));
    SetTextAlign(dc,TA_CENTER);
    textAt(dc,CW/2,16,s.chart.title);
    const int left=82,right=855,top=65,bottom=464;
    const int kind=s.chart.kind;
    if(kind!=8){
        MoveToEx(dc,left,top,nullptr);LineTo(dc,left,bottom);
        LineTo(dc,right,bottom);
        textAt(dc,(left+right)/2,520,s.chart.xLabel);
        SetTextAlign(dc,TA_LEFT);
        textAt(dc,7,245,s.chart.yLabel);
    }
    double xmin=0,xmax=1,ymin=0,ymax=1;
    if(!s.chart.x.empty()&&!s.chart.y.empty()){
        xmin=*std::min_element(s.chart.x.begin(),s.chart.x.end());
        xmax=*std::max_element(s.chart.x.begin(),s.chart.x.end());
        ymin=std::min(0.0,*std::min_element(s.chart.y.begin(),s.chart.y.end()));
        ymax=*std::max_element(s.chart.y.begin(),s.chart.y.end());
        if(xmax==xmin)xmax=xmin+1;
        if(ymax==ymin)ymax=ymin+1;
    }
    auto px=[&](double x)->int{
        return left+static_cast<int>((x-xmin)/(xmax-xmin)*(right-left));
    };
    auto py=[&](double y)->int{
        return bottom-static_cast<int>((y-ymin)/(ymax-ymin)*(bottom-top));
    };
    if(kind<=5){
        // Categoricals use uniform slots; numerical charts use true X.
        const bool cats=(kind==2||kind==3||
            (kind==4&&!s.chart.labels.empty()&&!numericColumn(indexOf(s.x))));
        for(int i=0;i<=4;++i){
            int y=bottom-(bottom-top)*i/4;
            textAt(dc,37,y-7,fmt(ymin+(ymax-ymin)*i/4,4));
        }
        if(cats){
            std::size_t n=s.chart.y.size();
            std::size_t step=std::max<std::size_t>(1,(n+11)/12);
            for(std::size_t i=0;i<n;i+=step){
                int x=left+static_cast<int>((i+0.5)*(right-left)/n);
                SetTextAlign(dc,TA_CENTER);
                textAt(dc,x,bottom+8,s.chart.labels[i]);
                SetTextAlign(dc,TA_LEFT);
            }
        }else{
            for(int i=0;i<=5;++i){
                int x=left+(right-left)*i/5;
                SetTextAlign(dc,TA_CENTER);
                textAt(dc,x,bottom+8,fmt(xmin+(xmax-xmin)*i/5,3));
                SetTextAlign(dc,TA_LEFT);
            }
        }
        if(kind==0||kind==2||kind==3){
            const std::size_t n=s.chart.y.size();
            for(std::size_t i=0;i<n;++i){
                int x1=left+static_cast<int>((i+0.14)*(right-left)/n);
                int x2=left+static_cast<int>((i+0.86)*(right-left)/n);
                HBRUSH brush=CreateSolidBrush(color(i));
                HGDIOBJ old=SelectObject(dc,brush);
                Rectangle(dc,x1,py(s.chart.y[i]),x2,bottom);
                SelectObject(dc,old);DeleteObject(brush);
            }
        }else{
            HPEN pen=CreatePen(PS_SOLID,2,color(kind));
            HGDIOBJ old=SelectObject(dc,pen);
            for(std::size_t i=0;i<s.chart.x.size();++i){
                int x=px(s.chart.x[i]),y=py(s.chart.y[i]);
                if(kind!=5){
                    if(i==0)MoveToEx(dc,x,y,nullptr);
                    else LineTo(dc,x,y);
                }
                Ellipse(dc,x-3,y-3,x+3,y+3);
            }
            SelectObject(dc,old);DeleteObject(pen);
        }
    }else if(kind==6){
        auto b=s.chart.box;
        double low=b.minimum,high=b.maximum;
        for(double v:b.outliers){low=std::min(low,v);high=std::max(high,v);}
        if(low==high)high=low+1;
        auto xx=[&](double v){return left+static_cast<int>((v-low)/(high-low)*(right-left));};
        int mid=(top+bottom)/2;
        MoveToEx(dc,xx(b.minimum),mid,nullptr);LineTo(dc,xx(b.maximum),mid);
        Rectangle(dc,xx(b.q1),mid-35,xx(b.q3),mid+35);
        MoveToEx(dc,xx(b.median),mid-35,nullptr);LineTo(dc,xx(b.median),mid+35);
        for(double v:b.outliers)Ellipse(dc,xx(v)-4,mid-4,xx(v)+4,mid+4);
    }else if(kind==7){
        const auto& matrix=s.chart.matrix;
        double min=matrix[0][0],max=min;
        for(const auto& row:matrix)for(double v:row){min=std::min(min,v);max=std::max(max,v);}
        for(std::size_t row=0;row<matrix.size();++row)
            for(std::size_t col=0;col<matrix[row].size();++col){
                double p=max==min?0.5:(matrix[row][col]-min)/(max-min);
                int x1=left+static_cast<int>(col*(right-left)/matrix[row].size());
                int x2=left+static_cast<int>((col+1)*(right-left)/matrix[row].size());
                int y1=top+static_cast<int>(row*(bottom-top)/matrix.size());
                int y2=top+static_cast<int>((row+1)*(bottom-top)/matrix.size());
                HBRUSH brush=CreateSolidBrush(RGB(static_cast<int>(90+120*p),
                     static_cast<int>(55+180*p),static_cast<int>(175-100*p)));
                HGDIOBJ old=SelectObject(dc,brush);
                Rectangle(dc,x1,y1,x2,y2);
                SelectObject(dc,old);DeleteObject(brush);
            }
    }else if(kind==8){
        // Windows GDI wedge preview, matching proportions of exported SVG.
        const int cx=365,cy=265,rad=175;
        double angle=-3.14159265358979323846/2;
        for(std::size_t i=0;i<s.chart.pie.size();++i){
            const auto& v=s.chart.pie[i];
            double next=angle+6.283185307179586*v.fraction;
            HBRUSH brush=CreateSolidBrush(color(i));
            HGDIOBJ old=SelectObject(dc,brush);
            if(v.fraction>=0.99999)Ellipse(dc,cx-rad,cy-rad,cx+rad,cy+rad);
            else Pie(dc,cx-rad,cy-rad,cx+rad,cy+rad,
                     cx+static_cast<int>(rad*std::cos(next)),
                     cy+static_cast<int>(rad*std::sin(next)),
                     cx+static_cast<int>(rad*std::cos(angle)),
                     cy+static_cast<int>(rad*std::sin(angle)));
            SelectObject(dc,old);DeleteObject(brush);
            textAt(dc,650,95+static_cast<int>(i)*23,
                v.label+" ("+fmt(v.fraction*100,1)+"%)");
            angle=next;
        }
    }
    SetTextColor(dc,RGB(179,45,62));
    for(const auto& note:s.chart.visible.getItems())
        textAt(dc,static_cast<int>(note.x),static_cast<int>(note.y),note.text);
    SetTextColor(dc,RGB(0,0,0));
}
LRESULT CALLBACK previewProc(HWND window,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==WM_LBUTTONDOWN&&s.placing){
        RECT r;GetClientRect(window,&r);
        std::string note=getText(s.noteText);
        if(!note.empty()&&r.right>0&&r.bottom>0){
            s.notes.add(static_cast<double>(LOWORD(lp))*CW/r.right,
                        static_cast<double>(HIWORD(lp))*CH/r.bottom,note);
            s.placing=false;
            try{generate();}catch(const std::exception& e){error(e.what());}
        }
        return 0;
    }
    if(msg==WM_PAINT){
        PAINTSTRUCT ps;HDC dc=BeginPaint(window,&ps);
        draw(dc);EndPaint(window,&ps);return 0;
    }
    return DefWindowProcA(window,msg,wp,lp);
}
void makeControls(){
    s.font=CreateFontA(-17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,"Segoe UI");
    control("STATIC","CSV Dataset",0,0,18,14,285,20);
    s.fileLabel=control("STATIC","No file selected",0,0,18,37,305,23);
    control("BUTTON","Browse CSV",BS_PUSHBUTTON,ID_BROWSE,18,68,304,31);
    control("STATIC","Graph Type",0,0,18,112,286,20);
    s.kind=control("COMBOBOX","",CBS_DROPDOWNLIST|WS_VSCROLL,ID_TYPE,18,134,304,320);
    for(const char* t:chartNames)
        SendMessageA(s.kind,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(t));
    select(s.kind,0);
    control("STATIC","X Column",0,0,18,176,140,20);
    control("STATIC","Y Column",0,0,177,176,140,20);
    s.x=control("COMBOBOX","",CBS_DROPDOWNLIST|WS_VSCROLL,ID_X,18,197,140,240);
    s.y=control("COMBOBOX","",CBS_DROPDOWNLIST|WS_VSCROLL,ID_Y,177,197,145,240);
    control("STATIC","Chart Heading",0,0,18,238,304,20);
    s.title=control("EDIT","",WS_BORDER|ES_AUTOHSCROLL,ID_TITLE,18,260,304,28);
    control("STATIC","X-axis Label",0,0,18,300,150,20);
    control("STATIC","Y-axis Label",0,0,177,300,145,20);
    s.xLabel=control("EDIT","",WS_BORDER|ES_AUTOHSCROLL,ID_XLABEL,18,321,140,28);
    s.yLabel=control("EDIT","",WS_BORDER|ES_AUTOHSCROLL,ID_YLABEL,177,321,145,28);
    control("STATIC","Histogram Bins",0,0,18,363,155,20);
    s.bins=control("EDIT","10",WS_BORDER|ES_AUTOHSCROLL,ID_BINS,177,360,145,28);
    s.autoStats=control("BUTTON","Automatic Statistical Notes",BS_AUTOCHECKBOX,
        ID_AUTO_STATS,18,404,300,27);
    control("STATIC","Annotation Text",0,0,18,444,304,20);
    s.noteText=control("EDIT","",WS_BORDER|ES_AUTOHSCROLL,ID_NOTE_TEXT,18,464,304,29);
    control("BUTTON","Place on Chart",BS_PUSHBUTTON,ID_ADD_NOTE,18,502,146,32);
    control("BUTTON","Clear Notes",BS_PUSHBUTTON,ID_CLEAR_NOTE,175,502,147,32);
    control("BUTTON","Generate Graph",BS_PUSHBUTTON,ID_GENERATE,18,548,146,36);
    control("BUTTON","Statistics",BS_PUSHBUTTON,ID_STATS,175,548,147,36);
    s.openSvg=control("BUTTON","Open Saved SVG",BS_PUSHBUTTON,ID_OPEN_SVG,18,595,146,32);
    control("BUTTON","Open Output Folder",BS_PUSHBUTTON,ID_OPEN_FOLDER,175,595,147,32);
    EnableWindow(s.openSvg,FALSE);
    s.status=control("STATIC","Ready",0,0,18,647,300,90);
    s.preview=CreateWindowExA(WS_EX_CLIENTEDGE,"GraphStudioCanvas","",
        WS_CHILD|WS_VISIBLE,350,54,900,650,s.window,nullptr,
        GetModuleHandleA(nullptr),nullptr);
    if(s.font)SendMessageA(s.preview,WM_SETFONT,reinterpret_cast<WPARAM>(s.font),TRUE);
}
LRESULT CALLBACK mainProc(HWND window,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==WM_CREATE){
        s.window=window;makeControls();
        try{loadCsv(defaultCsv());}catch(const std::exception& e){status(e.what());}
        return 0;
    }
    if(msg==WM_SIZE){
        if(s.preview)MoveWindow(s.preview,350,54,
            std::max(300,static_cast<int>(LOWORD(lp))-370),
            std::max(300,static_cast<int>(HIWORD(lp))-91),TRUE);
        return 0;
    }
    if(msg==WM_COMMAND){
        int id=LOWORD(wp),notification=HIWORD(wp);
        try{
            if(id==ID_TYPE&&notification==CBN_SELCHANGE)bestColumns();
            else if(id==ID_BROWSE&&notification==BN_CLICKED)browse();
            else if(id==ID_GENERATE&&notification==BN_CLICKED)generate();
            else if(id==ID_STATS&&notification==BN_CLICKED)statsDialog();
            else if(id==ID_ADD_NOTE&&notification==BN_CLICKED){
                if(getText(s.noteText).empty())throw std::runtime_error("Enter a note before placing it.");
                if(s.chart.kind<0)throw std::runtime_error("Generate a graph first.");
                s.placing=true;status("Click a position inside the chart preview to place your note.");
            }else if(id==ID_CLEAR_NOTE&&notification==BN_CLICKED){
                s.notes.clear();s.placing=false;
                if(s.chart.kind>=0)generate();
            }else if(id==ID_AUTO_STATS&&notification==BN_CLICKED){
                if(s.chart.kind>=0)generate();
            }else if(id==ID_OPEN_SVG&&notification==BN_CLICKED){
                if(!s.chart.svgPath.empty())
                    ShellExecuteA(window,"open",s.chart.svgPath.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
            }else if(id==ID_OPEN_FOLDER&&notification==BN_CLICKED){
                fs::create_directories(outputDir());
                const auto folder=outputDir().string();
                ShellExecuteA(window,"open",folder.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
            }
        }catch(const std::exception& e){error(e.what());}
        return 0;
    }
    if(msg==WM_DESTROY){
        if(s.font){DeleteObject(s.font);s.font=nullptr;}
        PostQuitMessage(0);return 0;
    }
    return DefWindowProcA(window,msg,wp,lp);
}
int selfTest(){
    try{
        auto file=defaultCsv();
        auto table=DataProcessor::cleanRows(DataLoader::loadCSV(file.string()));
        auto col=DataProcessor::getColumn(table,1);
        auto counts=CountPlot::calculate(col);
        if(counts.empty())return 3;
        fs::create_directories(outputDir());
        const auto output=(outputDir()/"portable_smoke.svg").string();
        SVGExporter::save(Renderer::renderCountPlot(counts,CW,CH,"Portable Self Test"),output);
        return fs::exists(output)&&fs::file_size(output)>50?0:2;
    }catch(...){return 3;}
}
}
int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR command,int show){
    if(std::string(command).find("--self-test")!=std::string::npos)return selfTest();
    WNDCLASSA preview{};preview.lpfnWndProc=previewProc;preview.hInstance=h;
    preview.lpszClassName="GraphStudioCanvas";preview.hCursor=LoadCursor(nullptr,IDC_CROSS);
    preview.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassA(&preview))return 1;
    WNDCLASSA main{};main.lpfnWndProc=mainProc;main.hInstance=h;
    main.lpszClassName="Group4GraphStudio";main.hCursor=LoadCursor(nullptr,IDC_ARROW);
    main.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
    if(!RegisterClassA(&main))return 1;
    HWND window=CreateWindowExA(0,"Group4GraphStudio","Group 4 - Graph Studio",
        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,1310,825,
        nullptr,nullptr,h,nullptr);
    if(!window)return 2;
    ShowWindow(window,show);UpdateWindow(window);
    MSG msg{};
    while(GetMessageA(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageA(&msg);}
    return static_cast<int>(msg.wParam);
}
