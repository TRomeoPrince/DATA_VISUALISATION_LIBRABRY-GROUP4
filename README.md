# Group 4 C++ Data Visualization Library — Draft

A C++17 visualization library and native Windows Graph Studio app.

## Features

**Nine charts:** Histogram, KDE Plot, Count Plot, Bar Chart, Line Plot,
Scatter Plot, Box Plot, Heatmap and Pie Chart.

**Data and statistics:** CSV parsing (including quoted commas and multiline
cells), cleaning, paired numerical columns, mean, median, mode, quartiles,
interquartile range, variance, standard deviation and Tukey box-plot outliers.

**Visualization:** automatically suggested chart headings, editable titles,
categorical month/subject X-axis labels, X/Y-axis labels, readable numeric
ticks, themes and palettes, interactive annotations and statistical notes.

**Output:** SVG vector images. The Windows app shows a GDI graph preview.
PNG/JPG export is not implemented; SVG is the supported export format.

## Architecture

- include/data and src/data: DataLoader, DataProcessor, Statistics
- include/core and src/core: Config, Theme, ColorMap, Utils
- include/plots and src/plots: nine plot algorithms
- include/layout and src/layout: Axis, Annotation
- include/rendering and src/rendering: Renderer and IBasePlot interface
- include/export and src/export: SVGExporter, ImageExporter
- ui: Graph Studio Windows desktop application
- tests, examples, data/input, data/output

IBasePlot is under rendering, alongside Renderer. Renderer contains the
low-level drawing/font work; Annotation owns text and its placement. There
is no separate FontRenderer or ExtendedRenderer.

## Build and test

Requires CMake 3.16+ and a C++17 compiler.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure

On Windows, build a portable executable:

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDATAVIZ_PORTABLE_UI=ON
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

Windows builds are packaged as the GraphStudio-Windows-x64 artifact by
GitHub Actions; see BUILD_WINDOWS.md.

## Example

    #include "LinePlot.hpp"
    #include "Renderer.hpp"
    #include "SVGExporter.hpp"

    auto values=LinePlot::build({0,1,2},{120,145,135});
    auto svg=Renderer::renderLinePlot(
        values,900,560,"Monthly Sales",Annotation(),"Month","Sales",
        {"Jan","Feb","Mar"});
    SVGExporter::save(svg,"monthly_sales.svg");

The existing three plot types, tests and four examples remain available.
The nine plot algorithms are separate from rendering and file export.

This Draft repository is separate from the official Group 4 repository.
