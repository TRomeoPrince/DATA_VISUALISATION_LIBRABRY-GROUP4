# Group 4 C++ Data Visualization Library — Draft

A C++17 visualization library and native Windows Graph Studio app.

## Features

**Nine charts:** Histogram, KDE Plot, Count Plot, Bar Chart, Line Plot,
Scatter Plot, Box Plot, Heatmap and Pie Chart.

**Data and statistics:** CSV parsing (including quoted commas and multiline
cells), flat JSON table imports, DVL1 binary table import/export, cleaning,
paired numerical columns, mean, median, mode, quartiles,
interquartile range, variance, standard deviation and Tukey box-plot outliers.

**Visualization:** automatically suggested chart headings, editable titles,
categorical month/subject X-axis labels, X/Y-axis labels, readable numeric
ticks, themes and palettes, interactive annotations and statistical notes.

**Output:** SVG vector graphs on all platforms; PNG and JPEG exports through the
native Windows GDI+ raster backend. Graph Studio previews the chart and provides
**Open SVG**, **Save PNG**, and **Save JPG** actions. The raster exporters accept an
RGBA pixel buffer; SVGExporter saves vector markup.

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


### Additional data formats

The library can import CSV, flat JSON object arrays (e.g.
`[{"Month":"Jan","Sales":120}]`), and JSON table arrays
(e.g. `[["Month","Sales"],["Jan",120]]`). Nested JSON values are intentionally
rejected to prevent ambiguous matrix columns.

Binary datasets use the documented **DVL1** format: four-byte `DVL1` magic,
a little-endian unsigned 32-bit row count, then for each row a little-endian
column count, and for each cell its little-endian byte length followed by its
UTF-8 bytes. Use `DataLoader::saveBinary` and `DataLoader::loadBinary`.
Graph Studio's graphical file picker currently focuses on CSV.

## Data formats

Graph Studio accepts CSV, flat JSON arrays of objects/rows and .dvz binary tables.
DVZ1 is a documented little-endian format: magic DVZ1, uint32 row count,
uint32 column count, then per-cell uint32 UTF-8 byte length and bytes.
The binary table's first row contains headers. Nested JSON is not supported.
