# Windows Graph Studio — build and run

## Get an executable

Open the [GitHub Actions page](https://github.com/TRomeoPrince/DATA_VISUALISATION_LIBRABRY-GROUP4/actions).
Choose a successful C++ Build, Tests and Graph Examples workflow run,
download the **GraphStudio-Windows-x64** artifact and extract its ZIP.

Double-click GraphStudio.exe in the extracted GraphStudio folder, with
data/input beside it. Graph SVGs, PNGs and JPGs are saved into data/output next to the EXE.
The unsigned app may prompt a Windows SmartScreen warning.

## Build from source

Requires Visual Studio with Desktop development with C++, or MSYS2 UCRT64
with a C++17 compiler.

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDATAVIZ_PORTABLE_UI=ON
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure
    .\build\Release\GraphStudio.exe

For MSYS2 UCRT64/Ninja:

    cmake -S . -B build -G Ninja -DDATAVIZ_PORTABLE_UI=ON
    cmake --build build
    ctest --test-dir build --output-on-failure
    .\build\GraphStudio.exe

## Using the app

Browse a CSV or use the packaged samples. A Month/Sales dataset
automatically selects Line Plot. All nine charts are available from
the graph-type selector.

Set X, Y, a graph heading and axis labels. Click Generate Graph to
preview and export SVG. Use Save PNG or Save JPG for bitmap copies. Click Statistics for mean, quartiles and other
calculations. Type a note, click Place on Chart, then click the chart
to position it. Automatic Statistical Notes adds labels for numerical charts.

Open Saved SVG or Open Output Folder to view generated graphs.
Heatmap uses the numerical columns; Pie Chart uses category frequencies.
Scatter Plot requires numerical X and Y.

The portable --self-test checks CSV loading and SVG output without opening
a window. GitHub CI compiles and runs unit tests on Ubuntu and Windows.
Manual visual QA is still recommended before publishing a release.
PNG/JPG encoding is not supported by this version.
