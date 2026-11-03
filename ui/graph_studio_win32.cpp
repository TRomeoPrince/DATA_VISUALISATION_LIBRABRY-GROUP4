// Group 4 Graph Studio
// Native Windows user interface written in C++17 using the Win32 API.
// The calculation and SVG export still use our existing library modules.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#include "DataLoader.hpp"
#include "DataProcessor.hpp"
#include "Statistics.hpp"
#include "CountPlot.hpp"
#include "Histogram.hpp"
#include "KDEPlot.hpp"
#include "Axis.hpp"
#include "Annotation.hpp"
#include "Renderer.hpp"
#include "ImageExporter.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef DATAVIZ_SAMPLE_CSV_PATH
#define DATAVIZ_SAMPLE_CSV_PATH "data/input/sample.csv"
#endif

namespace {
constexpr int ID_BROWSE = 1001;
constexpr int ID_TYPE = 1002;
constexpr int ID_COLUMN = 1003;
constexpr int ID_TITLE = 1004;
constexpr int ID_BINS = 1005;
constexpr int ID_GENERATE = 1006;
constexpr int ID_OPEN_SVG = 1007;
constexpr int ID_OPEN_FOLDER = 1008;

struct Studio {
    HWND window = nullptr;
    HWND type = nullptr;
    HWND column = nullptr;
    HWND title = nullptr;
    HWND bins = nullptr;
    HWND binsLabel = nullptr;
    HWND fileLabel = nullptr;
    HWND status = nullptr;
    HWND openSvg = nullptr;
    HFONT font = nullptr;
    std::string csvPath = DATAVIZ_SAMPLE_CSV_PATH;
    std::string exportedPath;
    std::string graphTitle;
    std::string xLabel;
    std::string yLabel;
    DataLoader::Table table;
    std::vector<std::string> headers;
    CountPlot::Counts categoryCounts;
    std::vector<HistogramBin> histogram;
    std::vector<KDEPoint> density;
    int chartKind = 0;
    bool hasGraph = false;
} studio;

std::string controlText(HWND control) {
    const int count = GetWindowTextLengthA(control);
    std::string result(static_cast<std::size_t>(count + 1), '\0');
    GetWindowTextA(control, result.data(), count + 1);
    result.resize(static_cast<std::size_t>(count));
    return result;
}

HWND addControl(const char* kind, const char* text, DWORD style, int id,
                int x, int y, int width, int height) {
    HWND handle = CreateWindowExA(
        0, kind, text, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, studio.window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleA(nullptr), nullptr);
    if (handle && studio.font) {
        SendMessageA(handle, WM_SETFONT, reinterpret_cast<WPARAM>(studio.font), TRUE);
    }
    return handle;
}

void setStatus(const std::string& message) {
    SetWindowTextA(studio.status, message.c_str());
}

void showProblem(const std::string& message) {
    setStatus(message);
    MessageBoxA(studio.window, message.c_str(), "Graph Studio", MB_OK | MB_ICONWARNING);
}

std::string numericText(double value, int precision = 1) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

void chooseDefaultColumn() {
    const int type = static_cast<int>(SendMessageA(studio.type, CB_GETCURSEL, 0, 0));
    const int selected = type == 2 ? 1 : 2;
    const int count = static_cast<int>(studio.headers.size());
    if (count > 0) {
        SendMessageA(studio.column, CB_SETCURSEL, std::min(selected, count - 1), 0);
    }
}

void updateGraphType() {
    const int type = static_cast<int>(SendMessageA(studio.type, CB_GETCURSEL, 0, 0));
    studio.chartKind = type < 0 ? 0 : type;
    const bool isHistogram = studio.chartKind == 0;
    EnableWindow(studio.bins, isHistogram);
    EnableWindow(studio.binsLabel, isHistogram);
    const char* titles[] = {"Student Marks Histogram", "Student Marks KDE Plot",
                            "Students by Programme"};
    SetWindowTextA(studio.title, titles[studio.chartKind]);
    chooseDefaultColumn();
}

void loadCsv(const std::string& filename) {
    auto rows = DataProcessor::cleanRows(DataLoader::loadCSV(filename));
    if (rows.size() < 2 || rows.front().empty()) {
        throw std::runtime_error("CSV needs a header row and at least one data row.");
    }
    // The table and header list are stored so the user can change graph columns.
    studio.table = std::move(rows);
    studio.headers = studio.table.front();
    studio.csvPath = filename;
    SendMessageA(studio.column, CB_RESETCONTENT, 0, 0);
    for (std::size_t i = 0; i < studio.headers.size(); ++i) {
        const std::string label = studio.headers[i].empty()
            ? ("Column " + std::to_string(i + 1)) : studio.headers[i];
        SendMessageA(studio.column, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(label.c_str()));
    }
    chooseDefaultColumn();
    const std::string shortName = std::filesystem::path(filename).filename().string();
    SetWindowTextA(studio.fileLabel, shortName.c_str());
    setStatus("Loaded " + shortName + ". Select a graph and click Generate.");
}

void browseCsv() {
    char selected[MAX_PATH] = {};
    OPENFILENAMEA fileDialog{};
    fileDialog.lStructSize = sizeof(fileDialog);
    fileDialog.hwndOwner = studio.window;
    fileDialog.lpstrFile = selected;
    fileDialog.nMaxFile = MAX_PATH;
    fileDialog.lpstrFilter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0\0";
    fileDialog.nFilterIndex = 1;
    fileDialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&fileDialog)) {
        try {
            loadCsv(selected);
            studio.hasGraph = false;
            EnableWindow(studio.openSvg, FALSE);
            InvalidateRect(studio.window, nullptr, TRUE);
        } catch (const std::exception& error) {
            showProblem(error.what());
        }
    }
}

void generateGraph() {
    try {
        const int index = static_cast<int>(
            SendMessageA(studio.column, CB_GETCURSEL, 0, 0));
        if (index < 0 || static_cast<std::size_t>(index) >= studio.headers.size()) {
            throw std::runtime_error("Choose a valid CSV column.");
        }
        const auto column = DataProcessor::getColumn(studio.table,
                                                     static_cast<std::size_t>(index));
        if (column.empty()) {
            throw std::runtime_error("The selected CSV column is empty.");
        }
        studio.chartKind = static_cast<int>(
            SendMessageA(studio.type, CB_GETCURSEL, 0, 0));
        studio.graphTitle = DataProcessor::trim(controlText(studio.title));
        if (studio.graphTitle.empty()) {
            throw std::runtime_error("Enter a graph title.");
        }
        studio.xLabel = studio.headers[static_cast<std::size_t>(index)];
        studio.yLabel = studio.chartKind == 1 ? "Density" : "Frequency";
        std::string svg;
        std::string filename;

        // Each type calls the appropriate plot module, then the shared Renderer.
        if (studio.chartKind == 2) {
            studio.categoryCounts = CountPlot::calculate(column);
            if (studio.categoryCounts.empty()) {
                throw std::runtime_error("No non-empty categories were found.");
            }
            svg = Renderer::renderCountPlot(studio.categoryCounts, 880, 560,
                                             studio.graphTitle, Annotation(),
                                             studio.xLabel, studio.yLabel);
            filename = "ui_countplot.svg";
        } else if (studio.chartKind == 0 || studio.chartKind == 1) {
            auto values = DataProcessor::toNumeric(column);
            // Inf and NaN cannot be positioned correctly on an axis.
            values.erase(std::remove_if(values.begin(), values.end(),
                                        [](double x) { return !std::isfinite(x); }),
                         values.end());
            if (values.empty()) {
                throw std::runtime_error("Choose a column containing valid numbers.");
            }
            if (studio.chartKind == 0) {
                const std::string rawBins = controlText(studio.bins);
                const int bins = std::stoi(rawBins);
                if (bins < 1 || bins > 100) {
                    throw std::invalid_argument("Histogram bins must be between 1 and 100.");
                }
                studio.histogram = Histogram::build(values,
                                                    static_cast<std::size_t>(bins));
                svg = Renderer::renderHistogram(studio.histogram, 880, 560,
                                                  studio.graphTitle, Annotation(),
                                                  studio.xLabel, studio.yLabel);
                filename = "ui_histogram.svg";
            } else {
                studio.density = KDEPlot::estimate(values, 160);
                svg = Renderer::renderKDE(studio.density, 880, 560,
                                           studio.graphTitle, Annotation(),
                                           studio.xLabel, studio.yLabel);
                filename = "ui_kdeplot.svg";
            }
        } else {
            throw std::runtime_error("Choose a graph type.");
        }

        ImageExporter::saveSVG(svg, filename);
        studio.exportedPath = ImageExporter::outputFile(filename);
        studio.hasGraph = true;
        EnableWindow(studio.openSvg, TRUE);
        setStatus("Saved SVG in data/output: " + filename);
        InvalidateRect(studio.window, nullptr, TRUE);
    } catch (const std::exception& error) {
        showProblem(std::string("Could not generate graph: ") + error.what());
    }
}

void openSvg() {
    if (studio.exportedPath.empty()) return;
    const HINSTANCE result = ShellExecuteA(
        studio.window, "open", studio.exportedPath.c_str(),
        nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(result) <= 32) {
        showProblem("Could not open the SVG file. Open data/output manually.");
    }
}

void openOutputFolder() {
    try {
        const auto folder = std::filesystem::path(
            ImageExporter::outputFile("example.svg")).parent_path();
        std::filesystem::create_directories(folder);
        const HINSTANCE result = ShellExecuteA(studio.window, "open",
                                               folder.string().c_str(),
                                               nullptr, nullptr, SW_SHOWNORMAL);
        if (reinterpret_cast<INT_PTR>(result) <= 32) {
            showProblem("Could not open the output folder.");
        }
    } catch (const std::exception& error) {
        showProblem(error.what());
    }
}

void print(HDC dc, int x, int y, const std::string& text) {
    TextOutA(dc, x, y, text.c_str(), static_cast<int>(text.size()));
}

void renderPreview(HDC dc) {
    RECT outer{338, 110, 1045, 700};
    const HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &outer, white);
    DeleteObject(white);
    FrameRect(dc, &outer, reinterpret_cast<HBRUSH>(GetStockObject(LTGRAY_BRUSH)));

    SetBkMode(dc, TRANSPARENT);
    if (!studio.hasGraph) {
        SetTextColor(dc, RGB(110, 125, 145));
        print(dc, 500, 365, "Your generated chart will appear here.");
        print(dc, 470, 395, "The SVG will be saved to data/output.");
        return;
    }

    SetTextColor(dc, RGB(22, 42, 65));
    RECT titleRect{410, 142, 995, 180};
    DrawTextA(dc, studio.graphTitle.c_str(), -1, &titleRect,
              DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    constexpr int left = 425;
    constexpr int top = 210;
    constexpr int right = 994;
    constexpr int bottom = 575;

    const HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(65, 80, 98));
    const HGDIOBJ oldPen = SelectObject(dc, axisPen);
    MoveToEx(dc, left, top, nullptr);
    LineTo(dc, left, bottom);
    LineTo(dc, right, bottom);
    SelectObject(dc, oldPen);
    DeleteObject(axisPen);

    double highest = 1.0;
    if (studio.chartKind == 2) {
        for (const auto& item : studio.categoryCounts)
            highest = std::max(highest, static_cast<double>(item.second));
    } else if (studio.chartKind == 0) {
        for (const auto& item : studio.histogram)
            highest = std::max(highest, static_cast<double>(item.count));
    } else {
        for (const auto& point : studio.density)
            highest = std::max(highest, point.density);
    }
    Axis vertical(0.0, highest, bottom, top);

    const HPEN gridPen = CreatePen(PS_DOT, 1, RGB(215, 224, 234));
    SelectObject(dc, gridPen);
    SetTextColor(dc, RGB(70, 88, 106));
    for (double tick : vertical.ticks(6)) {
        const int y = static_cast<int>(vertical.scale(tick));
        MoveToEx(dc, left, y, nullptr);
        LineTo(dc, right, y);
        print(dc, left - 67, y - 9,
              numericText(tick, studio.chartKind == 1 ? 3 : 1));
    }
    SelectObject(dc, oldPen);
    DeleteObject(gridPen);

    if (studio.chartKind == 2 || studio.chartKind == 0) {
        const std::size_t count = studio.chartKind == 2
            ? studio.categoryCounts.size() : studio.histogram.size();
        const double spacing = static_cast<double>(right - left) /
                               static_cast<double>(std::max<std::size_t>(1, count));
        const HBRUSH barBrush = CreateSolidBrush(
            studio.chartKind == 2 ? RGB(55, 105, 195) : RGB(40, 150, 116));
        const HGDIOBJ oldBrush = SelectObject(dc, barBrush);
        std::size_t index = 0;
        auto drawBar = [&](double frequency, const std::string& label) {
            const int x1 = left + static_cast<int>((index + 0.12) * spacing);
            const int x2 = left + static_cast<int>((index + 0.88) * spacing);
            const int y = static_cast<int>(vertical.scale(frequency));
            Rectangle(dc, x1, y, std::max(x1 + 1, x2), bottom);
            if (count <= 12) {
                const std::string text = label.size() > 9
                    ? label.substr(0, 9) : label;
                RECT labelRect{x1 - 4, bottom + 9, x2 + 8, bottom + 39};
                DrawTextA(dc, text.c_str(), -1, &labelRect,
                          DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            }
            ++index;
        };
        if (studio.chartKind == 2) {
            for (const auto& item : studio.categoryCounts)
                drawBar(static_cast<double>(item.second), item.first);
        } else {
            for (const auto& item : studio.histogram)
                drawBar(static_cast<double>(item.count), numericText(item.lower));
        }
        SelectObject(dc, oldBrush);
        DeleteObject(barBrush);
    } else if (!studio.density.empty()) {
        double minX = studio.density.front().x;
        double maxX = studio.density.back().x;
        if (minX == maxX) maxX += 1.0;
        Axis horizontal(minX, maxX, left, right);
        const HPEN curvePen = CreatePen(PS_SOLID, 3, RGB(220, 125, 35));
        SelectObject(dc, curvePen);
        bool first = true;
        for (const auto& point : studio.density) {
            const int x = static_cast<int>(horizontal.scale(point.x));
            const int y = static_cast<int>(vertical.scale(point.density));
            if (first) {
                MoveToEx(dc, x, y, nullptr);
                first = false;
            } else {
                LineTo(dc, x, y);
            }
        }
        SelectObject(dc, oldPen);
        DeleteObject(curvePen);
        for (double tick : horizontal.ticks(6)) {
            const int x = static_cast<int>(horizontal.scale(tick));
            print(dc, x - 12, bottom + 8, numericText(tick));
        }
    }

    SetTextColor(dc, RGB(38, 60, 82));
    RECT xlabelRect{left, 628, right, 660};
    DrawTextA(dc, studio.xLabel.c_str(), -1, &xlabelRect,
              DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    print(dc, 362, 187, studio.yLabel);
}

void createControls() {
    studio.font = CreateFontA(
        -18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");

    addControl("STATIC", "GROUP 4  |  C++ GRAPH STUDIO", 0, 0,
               28, 23, 460, 30);
    addControl("STATIC", "Preview  |  Generated by the C++ visualization library",
               0, 0, 350, 72, 685, 25);

    addControl("STATIC", "1. Choose CSV dataset", 0, 0,
               28, 88, 280, 24);
    addControl("BUTTON", "Browse CSV...", WS_TABSTOP | BS_PUSHBUTTON,
               ID_BROWSE, 28, 117, 282, 39);
    studio.fileLabel = addControl("STATIC", "sample.csv", 0, 0,
                                  28, 165, 286, 27);

    addControl("STATIC", "2. Graph type", 0, 0,
               28, 208, 280, 24);
    studio.type = addControl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL,
                             ID_TYPE, 28, 236, 282, 170);
    SendMessageA(studio.type, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>("Histogram"));
    SendMessageA(studio.type, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>("KDE Plot"));
    SendMessageA(studio.type, CB_ADDSTRING, 0,
                 reinterpret_cast<LPARAM>("Count Plot"));
    SendMessageA(studio.type, CB_SETCURSEL, 0, 0);

    addControl("STATIC", "3. Data column", 0, 0,
               28, 290, 280, 24);
    studio.column = addControl("COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL,
                               ID_COLUMN, 28, 317, 282, 220);

    addControl("STATIC", "4. Chart title", 0, 0,
               28, 371, 280, 24);
    studio.title = addControl("EDIT", "Student Marks Histogram",
                              WS_BORDER | ES_AUTOHSCROLL, ID_TITLE,
                              28, 397, 282, 33);

    studio.binsLabel = addControl("STATIC", "5. Number of histogram bins", 0, 0,
                                  28, 448, 282, 24);
    studio.bins = addControl("EDIT", "5", WS_BORDER | ES_NUMBER,
                             ID_BINS, 28, 477, 282, 33);

    addControl("BUTTON", "Generate SVG Graph", WS_TABSTOP | BS_PUSHBUTTON,
               ID_GENERATE, 28, 536, 282, 44);
    studio.openSvg = addControl("BUTTON", "Open Saved SVG",
                                WS_TABSTOP | BS_PUSHBUTTON,
                                ID_OPEN_SVG, 28, 594, 282, 38);
    EnableWindow(studio.openSvg, FALSE);
    addControl("BUTTON", "Open data/output Folder",
               WS_TABSTOP | BS_PUSHBUTTON,
               ID_OPEN_FOLDER, 28, 646, 282, 38);
    studio.status = addControl("STATIC", "Ready to generate a chart.",
                               SS_LEFT, 0, 350, 715, 680, 24);

    updateGraphType();
    try {
        loadCsv(studio.csvPath);
    } catch (const std::exception& error) {
        setStatus(std::string("Choose a CSV file: ") + error.what());
    }
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message,
                                 WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            studio.window = window;
            createControls();
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case ID_BROWSE:
                    if (HIWORD(wparam) == BN_CLICKED) browseCsv();
                    return 0;
                case ID_TYPE:
                    if (HIWORD(wparam) == CBN_SELCHANGE) {
                        updateGraphType();
                        studio.hasGraph = false;
                        EnableWindow(studio.openSvg, FALSE);
                        InvalidateRect(window, nullptr, TRUE);
                    }
                    return 0;
                case ID_GENERATE:
                    if (HIWORD(wparam) == BN_CLICKED) generateGraph();
                    return 0;
                case ID_OPEN_SVG:
                    if (HIWORD(wparam) == BN_CLICKED) openSvg();
                    return 0;
                case ID_OPEN_FOLDER:
                    if (HIWORD(wparam) == BN_CLICKED) openOutputFolder();
                    return 0;
                default:
                    break;
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            const HDC dc = BeginPaint(window, &paint);
            const HGDIOBJ oldFont = studio.font
                ? SelectObject(dc, studio.font) : nullptr;
            renderPreview(dc);
            if (oldFont) SelectObject(dc, oldFont);
            EndPaint(window, &paint);
            return 0;
        }
        case WM_DESTROY:
            if (studio.font) DeleteObject(studio.font);
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}
}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
    const char* windowClass = "Group4GraphStudioNative";
    WNDCLASSA wc{};
    wc.lpfnWndProc = windowProcedure;
    wc.hInstance = instance;
    wc.lpszClassName = windowClass;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    if (!RegisterClassA(&wc)) {
        MessageBoxA(nullptr, "Could not register Graph Studio.",
                    "Graph Studio", MB_OK | MB_ICONERROR);
        return 1;
    }
    HWND window = CreateWindowExA(
        0, windowClass, "Group 4 - C++ Data Visualization Library",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 1070, 790,
        nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, show);
    UpdateWindow(window);
    MSG msg{};
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return static_cast<int>(msg.wParam);
}
