#include "Renderer.hpp"

#include "Axis.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
constexpr double LEFT = 70.0;
constexpr double RIGHT = 30.0;
constexpr double TOP = 55.0;
constexpr double BOTTOM = 70.0;


// Each graph should say what the horizontal and vertical values represent.
std::string axisLabels(int width, int height,
                       const std::string& xLabel, const std::string& yLabel) {
    std::ostringstream svg;
    svg << "<text x=\"" << width / 2 << "\" y=\"" << (height - 18)
        << "\" text-anchor=\"middle\" font-size=\"15\""
        << " font-family=\"sans-serif\">" << xLabel << "</text>\n";
    svg << "<text x=\"22\" y=\"" << height / 2
        << "\" transform=\"rotate(-90 22 " << height / 2 << ")\""
        << " text-anchor=\"middle\" font-size=\"15\""
        << " font-family=\"sans-serif\">" << yLabel << "</text>\n";
    return svg.str();
}

std::string number(double value, int precision = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}
}

// I keep the plotting margins in one place so the graphs use the same layout.
std::string Renderer::escapeXML(const std::string& text) {
    std::string result;

    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c; break;
        }
    }

    return result;
}

std::string Renderer::svgStart(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Renderer width and height must be positive.");
    }

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
        << width << "\" height=\"" << height
        << "\" viewBox=\"0 0 " << width << " " << height << "\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    return svg.str();
}

std::string Renderer::svgEnd() {
    return "</svg>\n";
}

std::string Renderer::renderCountPlot(
    const CountPlot::Counts& counts,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (counts.empty()) {
        throw std::invalid_argument("Cannot render an empty count plot.");
    }

    std::size_t maxCount = 1;
    for (const auto& item : counts) {
        maxCount = std::max(maxCount, item.second);
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis yAxis(0.0, static_cast<double>(maxCount), plotBottom, plotTop);

    double availableWidth = plotRight - plotLeft;
    double slotWidth = availableWidth / static_cast<double>(counts.size());
    double barWidth = slotWidth * 0.7;

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    std::size_t index = 0;
    for (const auto& item : counts) {
        double x = plotLeft + index * slotWidth + (slotWidth - barWidth) / 2.0;
        double y = yAxis.scale(static_cast<double>(item.second));
        double barHeight = plotBottom - y;

        svg << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << barWidth << "\" height=\"" << barHeight
            << "\" fill=\"steelblue\"/>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (plotBottom + 20)
            << "\" text-anchor=\"middle\" font-size=\"12\""
            << " font-family=\"sans-serif\">"
            << escapeXML(item.first) << "</text>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (y - 5)
            << "\" text-anchor=\"middle\" font-size=\"12\""
            << " font-family=\"sans-serif\">"
            << item.second << "</text>\n";

        ++index;
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<line x1=\"" << (plotLeft - 5) << "\" y1=\"" << y
            << "\" x2=\"" << plotLeft << "\" y2=\"" << y
            << "\" stroke=\"black\"/>\n";
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}

std::string Renderer::renderHistogram(
    const std::vector<HistogramBin>& bins,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (bins.empty()) {
        throw std::invalid_argument("Cannot render an empty histogram.");
    }

    std::size_t maxCount = 1;
    for (const auto& bin : bins) {
        maxCount = std::max(maxCount, bin.count);
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis yAxis(0.0, static_cast<double>(maxCount), plotBottom, plotTop);

    double availableWidth = plotRight - plotLeft;
    double barWidth = availableWidth / static_cast<double>(bins.size());

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    for (std::size_t i = 0; i < bins.size(); ++i) {
        const auto& bin = bins[i];
        double x = plotLeft + i * barWidth;
        double y = yAxis.scale(static_cast<double>(bin.count));

        svg << "<rect x=\"" << x << "\" y=\"" << y
            << "\" width=\"" << barWidth
            << "\" height=\"" << (plotBottom - y)
            << "\" fill=\"seagreen\" stroke=\"white\"/>\n";

        svg << "<text x=\"" << (x + barWidth / 2.0)
            << "\" y=\"" << (plotBottom + 18)
            << "\" text-anchor=\"middle\" font-size=\"10\""
            << " font-family=\"sans-serif\">"
            << number(bin.lower, 1) << "</text>\n";
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}

std::string Renderer::renderKDE(
    const std::vector<KDEPoint>& points,
    int width,
    int height,
    const std::string& title,
    const Annotation& annotations,
    const std::string& xLabel,
    const std::string& yLabel
) {
    if (points.size() < 2) {
        throw std::invalid_argument("KDE rendering needs at least two points.");
    }

    double minX = points.front().x;
    double maxX = points.front().x;
    double maxDensity = points.front().density;

    for (const auto& point : points) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        maxDensity = std::max(maxDensity, point.density);
    }

    if (minX == maxX) {
        maxX = minX + 1.0;
    }

    if (maxDensity <= 0.0) {
        maxDensity = 1.0;
    }

    double plotLeft = LEFT;
    double plotRight = width - RIGHT;
    double plotTop = TOP;
    double plotBottom = height - BOTTOM;

    Axis xAxis(minX, maxX, plotLeft, plotRight);
    Axis yAxis(0.0, maxDensity, plotBottom, plotTop);

    std::ostringstream svg;
    svg << svgStart(width, height);

    svg << "<text x=\"" << width / 2
        << "\" y=\"30\" text-anchor=\"middle\" font-size=\"22\""
        << " font-family=\"sans-serif\">"
        << escapeXML(title) << "</text>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotBottom
        << "\" x2=\"" << plotRight << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<line x1=\"" << plotLeft << "\" y1=\"" << plotTop
        << "\" x2=\"" << plotLeft << "\" y2=\"" << plotBottom
        << "\" stroke=\"black\"/>\n";

    svg << "<polyline fill=\"none\" stroke=\"darkorange\""
        << " stroke-width=\"3\" points=\"";

    for (const auto& point : points) {
        svg << xAxis.scale(point.x) << ","
            << yAxis.scale(point.density) << " ";
    }

    svg << "\"/>\n";

    for (double tick : xAxis.ticks()) {
        double x = xAxis.scale(tick);
        svg << "<text x=\"" << x << "\" y=\"" << (plotBottom + 20)
            << "\" text-anchor=\"middle\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 1) << "</text>\n";
    }

    for (double tick : yAxis.ticks()) {
        double y = yAxis.scale(tick);
        svg << "<text x=\"" << (plotLeft - 10) << "\" y=\"" << (y + 4)
            << "\" text-anchor=\"end\" font-size=\"11\""
            << " font-family=\"sans-serif\">"
            << number(tick, 3) << "</text>\n";
    }

    for (const auto& note : annotations.getItems()) {
        svg << "<text x=\"" << note.x << "\" y=\"" << note.y
            << "\" font-size=\"13\" font-family=\"sans-serif\""
            << " fill=\"darkred\">"
            << escapeXML(note.text) << "</text>\n";
    }

    svg << axisLabels(width, height, escapeXML(xLabel), escapeXML(yLabel));
    svg << svgEnd();
    return svg.str();
}

#include "Config.hpp"
#include "Theme.hpp"
#include "ColorMap.hpp"
#include "Utils.hpp"
#include <cmath>
namespace {
double mapData(double x,double x0,double x1,double a,double b) {
    return a==b?(x0+x1)/2:x0+(x-a)*(x1-x0)/(b-a);
}
struct Canvas {
    int w,h;double left,right,top,bottom;
    std::ostringstream out;
    Canvas(int width,int height,const std::string& title,const std::string& xlabel,
           const std::string& ylabel,bool axes=true):w(width),h(height) {
        Config c;c.width=width;c.height=height;c.validate();
        left=c.left;right=w-c.right;top=c.top;bottom=h-c.bottom;
        Theme theme=Theme::light();
        out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""<<w
           <<"\" height=\""<<h<<"\" viewBox=\"0 0 "<<w<<" "<<h<<"\">\n";
        out<<"<rect width=\"100%\" height=\"100%\" fill=\""<<theme.background<<"\"/>\n";
        out<<"<text x=\""<<w/2<<"\" y=\"30\" font-size=\"22\" font-family=\"sans-serif\" font-weight=\"bold\" text-anchor=\"middle\">"
           <<Utils::escapeXML(title)<<"</text>\n";
        if(axes) {
            out<<"<line x1=\""<<left<<"\" y1=\""<<top<<"\" x2=\""<<left<<"\" y2=\""<<bottom<<"\" stroke=\""<<theme.foreground<<"\"/>\n";
            out<<"<line x1=\""<<left<<"\" y1=\""<<bottom<<"\" x2=\""<<right<<"\" y2=\""<<bottom<<"\" stroke=\""<<theme.foreground<<"\"/>\n";
            out<<"<text x=\""<<(left+right)/2<<"\" y=\""<<h-18<<"\" font-family=\"sans-serif\" font-size=\"15\" text-anchor=\"middle\">"<<Utils::escapeXML(xlabel)<<"</text>\n";
            out<<"<text x=\"22\" y=\""<<(top+bottom)/2<<"\" transform=\"rotate(-90 22 "<<(top+bottom)/2
               <<")\" font-family=\"sans-serif\" font-size=\"15\" text-anchor=\"middle\">"<<Utils::escapeXML(ylabel)<<"</text>\n";
        }
    }
    void ticks(double xmin,double xmax,double ymin,double ymax,bool showX=true) {
        if(ymin==ymax){ymin-=0.5;ymax+=0.5;}
        if(xmin==xmax){xmin-=0.5;xmax+=0.5;}
        for(int i=0;i<=5;++i){
            double v=ymin+(ymax-ymin)*i/5.0, y=mapData(v,bottom,top,ymin,ymax);
            out<<"<line x1=\""<<left<<"\" y1=\""<<y<<"\" x2=\""<<right<<"\" y2=\""<<y<<"\" stroke=\"#e1e7ec\" stroke-width=\"0.7\"/>\n";
            out<<"<text x=\""<<left-9<<"\" y=\""<<y+4<<"\" text-anchor=\"end\" font-family=\"sans-serif\" font-size=\"10\">"<<Utils::decimal(v,4)<<"</text>\n";
            double x=xmin+(xmax-xmin)*i/5.0;
            if(showX)out<<"<text x=\""<<mapData(x,left,right,xmin,xmax)<<"\" y=\""<<bottom+20
               <<"\" text-anchor=\"middle\" font-family=\"sans-serif\" font-size=\"10\">"<<Utils::decimal(x,3)<<"</text>\n";
        }
    }
    void categories(const std::vector<std::string>& labels,const std::vector<double>& x,double xmin,double xmax){
        if(labels.size()!=x.size())throw std::invalid_argument("Category labels and points differ.");
        std::size_t step=std::max<std::size_t>(1,(labels.size()+11)/12);
        for(std::size_t i=0;i<labels.size();i+=step)
            out<<"<text x=\""<<mapData(x[i],left,right,xmin,xmax)<<"\" y=\""<<bottom+20
               <<"\" font-family=\"sans-serif\" font-size=\"11\" text-anchor=\"middle\">"
               <<Utils::escapeXML(labels[i])<<"</text>\n";
    }
    std::string finish(const Annotation& notes) {
        for(const auto& n:notes.getItems())
            out<<"<text x=\""<<n.x<<"\" y=\""<<n.y
               <<"\" font-family=\"sans-serif\" font-size=\"13\" fill=\"#b43346\">"
               <<Utils::escapeXML(n.text)<<"</text>\n";
        out<<"</svg>\n";return out.str();
    }
};
template<class T>
void range(const std::vector<T>& points,double& xmin,double& xmax,double& ymin,double& ymax){
    if(points.empty())throw std::invalid_argument("Cannot render an empty plot.");
    xmin=xmax=points.front().x;ymin=ymax=points.front().y;
    for(const auto& p:points){xmin=std::min(xmin,p.x);xmax=std::max(xmax,p.x);
        ymin=std::min(ymin,p.y);ymax=std::max(ymax,p.y);}
    if(xmin==xmax){xmin-=0.5;xmax+=0.5;}
    if(ymin==ymax){ymin-=0.5;ymax+=0.5;}
    double pad=(ymax-ymin)*0.08;ymin-=pad;ymax+=pad;
}
}
std::string Renderer::render(const IBasePlot& plot,const RenderOptions& opts){
    Config c;c.width=opts.width;c.height=opts.height;c.validate();
    return plot.renderSVG(opts);
}
std::string Renderer::renderLinePlot(const std::vector<LinePoint>& data,int w,int h,
    const std::string& title,const Annotation& notes,const std::string& xl,const std::string& yl,
    const std::vector<std::string>& labels){
    if(!labels.empty()&&labels.size()!=data.size())throw std::invalid_argument("Line labels must match points.");
    double xmin,xmax,ymin,ymax;range(data,xmin,xmax,ymin,ymax);
    Canvas c(w,h,title,xl,yl);
    if(labels.empty())c.ticks(xmin,xmax,ymin,ymax);
    else{
        c.ticks(xmin,xmax,ymin,ymax,false);
        std::vector<double> xv;for(const auto& p:data)xv.push_back(p.x);
        c.categories(labels,xv,xmin,xmax);
    }
    c.out<<"<polyline fill=\"none\" stroke=\"#2374ab\" stroke-width=\"3\" points=\"";
    for(const auto& p:data)c.out<<mapData(p.x,c.left,c.right,xmin,xmax)<<","
                                 <<mapData(p.y,c.bottom,c.top,ymin,ymax)<<" ";
    c.out<<"\"/>\n";
    for(const auto& p:data)c.out<<"<circle cx=\""<<mapData(p.x,c.left,c.right,xmin,xmax)
        <<"\" cy=\""<<mapData(p.y,c.bottom,c.top,ymin,ymax)<<"\" r=\"3\" fill=\"#2374ab\"/>\n";
    return c.finish(notes);
}
std::string Renderer::renderScatterPlot(const std::vector<ScatterPoint>& data,int w,int h,
    const std::string& title,const Annotation& notes,const std::string& xl,const std::string& yl){
    double xmin,xmax,ymin,ymax;range(data,xmin,xmax,ymin,ymax);
    Canvas c(w,h,title,xl,yl);c.ticks(xmin,xmax,ymin,ymax);
    for(const auto& p:data)c.out<<"<circle cx=\""<<mapData(p.x,c.left,c.right,xmin,xmax)
        <<"\" cy=\""<<mapData(p.y,c.bottom,c.top,ymin,ymax)
        <<"\" r=\"4\" fill=\"#2374ab\" opacity=\"0.8\"/>\n";
    return c.finish(notes);
}
std::string Renderer::renderBarChart(const std::vector<BarItem>& data,int w,int h,
    const std::string& title,const Annotation& notes,const std::string& xl,const std::string& yl){
    if(data.empty())throw std::invalid_argument("Empty bar chart.");
    double min=0,max=0;std::vector<std::string> labels;std::vector<double> positions;
    for(std::size_t i=0;i<data.size();++i){min=std::min(min,data[i].value);max=std::max(max,data[i].value);
        labels.push_back(data[i].category);positions.push_back(static_cast<double>(i)+0.5);}
    if(min==max)max+=1;
    Canvas c(w,h,title,xl,yl);
    c.ticks(0,static_cast<double>(data.size()),min,max,false);
    c.categories(labels,positions,0,static_cast<double>(data.size()));
    double zero=mapData(0,c.bottom,c.top,min,max);
    double slot=(c.right-c.left)/data.size();
    for(std::size_t i=0;i<data.size();++i){
        double y=mapData(data[i].value,c.bottom,c.top,min,max);
        c.out<<"<rect x=\""<<c.left+(i+0.13)*slot<<"\" y=\""<<std::min(y,zero)
             <<"\" width=\""<<slot*0.74<<"\" height=\""<<std::abs(y-zero)
             <<"\" fill=\""<<ColorMap::categorical(i)<<"\"/>\n";
    }
    return c.finish(notes);
}
std::string Renderer::renderBoxPlot(const BoxSummary& data,int w,int h,
    const std::string& title,const Annotation& notes,const std::string& xl){
    double min=data.minimum,max=data.maximum;
    for(double v:data.outliers){min=std::min(min,v);max=std::max(max,v);}
    if(min==max){min-=0.5;max+=0.5;}
    Canvas c(w,h,title,xl,"");
    auto x=[&](double v){return mapData(v,c.left,c.right,min,max);};
    double m=(c.top+c.bottom)/2;
    c.out<<"<line x1=\""<<x(data.minimum)<<"\" x2=\""<<x(data.maximum)
         <<"\" y1=\""<<m<<"\" y2=\""<<m<<"\" stroke=\"#25384a\" stroke-width=\"2\"/>\n";
    c.out<<"<rect x=\""<<x(data.q1)<<"\" y=\""<<m-38<<"\" width=\""<<x(data.q3)-x(data.q1)
         <<"\" height=\"76\" fill=\"#a6dcee\" stroke=\"#2374ab\" stroke-width=\"2\"/>\n";
    c.out<<"<line x1=\""<<x(data.median)<<"\" x2=\""<<x(data.median)
         <<"\" y1=\""<<m-38<<"\" y2=\""<<m+38<<"\" stroke=\"#d85740\" stroke-width=\"3\"/>\n";
    for(double v:{data.minimum,data.maximum})
        c.out<<"<line x1=\""<<x(v)<<"\" x2=\""<<x(v)<<"\" y1=\""<<m-15
             <<"\" y2=\""<<m+15<<"\" stroke=\"#25384a\" stroke-width=\"2\"/>\n";
    for(double v:data.outliers)c.out<<"<circle cx=\""<<x(v)<<"\" cy=\""<<m
                                     <<"\" r=\"5\" fill=\"#d85740\"/>\n";
    c.ticks(min,max,0,1);return c.finish(notes);
}
std::string Renderer::renderHeatmap(const Heatmap::Matrix& input,int w,int h,
    const std::string& title,const Annotation& notes,const std::string& xl,const std::string& yl){
    auto data=Heatmap::build(input);
    Canvas c(w,h,title,xl,yl);
    double min=data.front().front(),max=min;
    for(const auto& row:data)for(double v:row){min=std::min(min,v);max=std::max(max,v);}
    double cw=(c.right-c.left)/data.front().size(),ch=(c.bottom-c.top)/data.size();
    for(std::size_t r=0;r<data.size();++r)for(std::size_t col=0;col<data[r].size();++col){
        double a=max==min?0.5:(data[r][col]-min)/(max-min);
        c.out<<"<rect x=\""<<c.left+col*cw<<"\" y=\""<<c.top+r*ch
             <<"\" width=\""<<cw<<"\" height=\""<<ch
             <<"\" stroke=\"#fff\" stroke-width=\"1\" fill=\""<<ColorMap::viridis(a)<<"\"/>\n";
        if(data.size()<=12&&data[r].size()<=12)
            c.out<<"<text x=\""<<c.left+(col+0.5)*cw<<"\" y=\""<<c.top+(r+0.5)*ch+4
                 <<"\" font-family=\"sans-serif\" font-size=\"11\" text-anchor=\"middle\" fill=\"white\">"
                 <<Utils::decimal(data[r][col],2)<<"</text>\n";
    }
    return c.finish(notes);
}
std::string Renderer::renderPieChart(const std::vector<PieSlice>& data,int w,int h,
    const std::string& title,const Annotation& notes){
    if(data.empty())throw std::invalid_argument("Empty pie chart.");
    Canvas c(w,h,title,"","",false);
    constexpr double pi=3.14159265358979323846;
    double cx=w*0.39,cy=h*0.53,r=std::min(w*0.29,h*0.33),angle=-pi/2;
    for(std::size_t i=0;i<data.size();++i){
        const auto& v=data[i];double next=angle+2*pi*v.fraction;
        if(v.fraction>0.999999){
            c.out<<"<circle cx=\""<<cx<<"\" cy=\""<<cy<<"\" r=\""<<r
                 <<"\" fill=\""<<ColorMap::categorical(i)<<"\"/>\n";
        }else{
            c.out<<"<path d=\"M "<<cx<<" "<<cy<<" L "<<cx+r*std::cos(angle)
                 <<" "<<cy+r*std::sin(angle)<<" A "<<r<<" "<<r<<" 0 "
                 <<(v.fraction>0.5?1:0)<<" 1 "<<cx+r*std::cos(next)
                 <<" "<<cy+r*std::sin(next)<<" Z\" fill=\""<<ColorMap::categorical(i)
                 <<"\" stroke=\"white\"/>\n";
        }
        double ly=c.top+25+i*23;
        c.out<<"<rect x=\""<<w*0.72<<"\" y=\""<<ly-12<<"\" width=\"12\" height=\"12\" fill=\""
             <<ColorMap::categorical(i)<<"\"/>\n";
        c.out<<"<text x=\""<<w*0.72+17<<"\" y=\""<<ly-1
             <<"\" font-family=\"sans-serif\" font-size=\"12\">"<<Utils::escapeXML(v.label)
             <<" ("<<Utils::decimal(v.fraction*100,1)<<"%)</text>\n";
        angle=next;
    }
    return c.finish(notes);
}
