# C++ Data Visualization Library — Group 4

A reusable **C++17 data visualization library** that loads and prepares data,
calculates statistics, and creates **Count Plots, Histograms, and KDE Plots**.
The graphs are rendered as SVG images, without an external graphics library.

This project was organized by Group 4 as a practical application of C++
programming, modular design, and Object-Oriented Programming (OOP).

> **Integration note:** This README describes the integrated reference
> implementation in [Draft-Group4](https://github.com/TRomeoPrince/Draft-Group4).
> It is prepared for the group's
> [official repository](https://github.com/Ahumuza-Blessing/DATA_VISUALISATION_LIBRARY)
> **when the matching source files and CMake configuration are transferred**.
> The official repository currently has some different filenames and module
> interfaces, so copying this README alone will not make that code match.

## 1. Group members and responsibilities

The names and allocations below follow the group's project records.

| Group member | Assigned module | Responsibility |
| --- | --- | --- |
| **ADONGA PHILLIP** | Data Loader | Read input datasets, initially CSV files |
| **ATWINE JEANES** | Data Processing | Clean and prepare loaded data |
| **AHUMUZA BLESSING** | Statistics | Implement descriptive statistical calculations |
| **EGERU MICHAEL** | Count Plot | Calculate frequencies of categorical values |
| **SERUGYA FAHAD** | Histogram | Group numerical values into bins |
| **MUGERWA SHADRACK STEPHEN** | KDE Plot | Estimate smooth numerical density curves |
| **TWINOMUJUNI ROMEO** | Axis | Calculate graph scales and tick values |
| **WALUGEMBE MUHAISINAH N.** | Annotations | Manage notes and labels added to graphs |
| **KISAKYE JEMIMA** | Renderer | Draw graph elements and produce SVG markup |
| **NAMUBIRU LOUISE** | Image Exporter | Save rendered SVG content as image files |

Each member works on a separate module. During integration, the modules share
their outputs rather than repeating calculations or drawing code.

## 2. Project objectives

- Apply C++ and relevant OOP concepts to a group software project.
- Develop a library with reusable and separately testable components.
- Read CSV datasets and prepare numerical or categorical values.
- Calculate basic statistical summaries.
- Produce complete Count Plots, Histograms, and KDE Plots.
- Include titles, axis labels, scales, tick marks, and optional annotations.
- Export the completed graphs as SVG files.
- Verify both individual modules and the combined library.

## 3. How the library works

```text
                  Input CSV file
                         |
                     DataLoader
                         |
                    DataProcessor
                         |
             +-----------+-----------+
             |                       |
      Categorical data         Numerical data
             |                       |
          CountPlot           Statistics
             |                       |
             |                +------+------+
             |                |             |
             |             Histogram      KDEPlot
             |                |             |
             +----------------+-------------+
                              |
                           Renderer
                     (uses Axis and
                    optional Annotation)
                              |
                         SVG content
                              |
                        ImageExporter
                              |
                       data/output/*.svg
```

For example, a histogram calculates its bins first, then gives those bins to
the **Renderer**, which uses **Axis** for scaling and places any requested
**Annotation** items on the graph. The **ImageExporter** saves the finished SVG.

The Renderer is shared by the three plot types. Each graph example calls the
other modules it needs, rather than implementing its own axis, drawing system,
or export code.

## 4. Main modules

| Module in this draft | What it does |
| --- | --- |
| `DataLoader` | Reads a CSV file into rows and columns of text |
| `DataProcessor` | Trims text, removes blank rows, extracts columns, and converts numerical text |
| `Statistics` | Calculates count, mean, median, minimum, maximum, variance, and standard deviation |
| `CountPlot` | Counts occurrences of categorical values |
| `Histogram` | Divides numeric data into bins and counts their values |
| `KDEPlot` | Uses a Gaussian kernel to estimate density points |
| `Axis` | Maps numerical values to drawing coordinates and generates ticks |
| `Annotation` | Stores positioned notes to appear on a plot |
| `Renderer` | Renders SVG graphs with title, axes, scales, labels, and optional annotations |
| `ImageExporter` | Creates the output directory if needed and saves SVG files |

**Important distinction:** In this version, `Annotation` stores additional
notes, while the plot title and axis labels are added by `Renderer`.
The module's class name is **`Annotation`** (singular), even though its
assigned project area is called *Annotations*.

## 5. Project structure

This is the structure of the **integrated draft** that the README documents.

```text
Draft-Group4/
├── data/
│   ├── input/
│   │   └── sample.csv
│   └── output/
│       └── README.md
├── include/
│   ├── DataLoader.hpp
│   ├── DataProcessor.hpp
│   ├── Statistics.hpp
│   ├── CountPlot.hpp
│   ├── Histogram.hpp
│   ├── KDEPlot.hpp
│   ├── Axis.hpp
│   ├── Annotation.hpp
│   ├── Renderer.hpp
│   └── ImageExporter.hpp
├── src/
│   ├── DataLoader.cpp
│   ├── DataProcessor.cpp
│   ├── Statistics.cpp
│   ├── CountPlot.cpp
│   ├── Histogram.cpp
│   ├── KDEPlot.cpp
│   ├── Axis.cpp
│   ├── Annotation.cpp
│   ├── Renderer.cpp
│   └── ImageExporter.cpp
├── examples/
│   ├── countplot_example.cpp
│   ├── histogram_example.cpp
│   ├── kdeplot_example.cpp
│   └── complete_library_example.cpp
├── tests/
│   ├── test_dataloader.cpp
│   ├── test_dataprocessor.cpp
│   ├── test_statistics.cpp
│   ├── test_countplot.cpp
│   ├── test_histogram.cpp
│   ├── test_kdeplot.cpp
│   ├── test_axis.cpp
│   ├── test_annotation.cpp
│   ├── test_renderer.cpp
│   ├── test_imageexporter.cpp
│   └── test_integration.cpp
├── .github/workflows/ci.yml
├── .gitignore
├── CMakeLists.txt
└── README.md
```

The official group's repository also keeps **weekly reports** in a `reports/`
folder. The reports are not included in this draft reference.

## 6. Examples: finished graphs

The `examples/` folder contains **four demonstration programs**:

| Example | Output |
| --- | --- |
| `countplot_example.cpp` | Complete categorical count plot |
| `histogram_example.cpp` | Complete histogram of numerical marks |
| `kdeplot_example.cpp` | Complete KDE density curve |
| `complete_library_example.cpp` | Uses the main modules together and creates all three graphs |

Each graph example includes a title, labeled axes, scales/ticks, and the
required plot elements. Notes can be added through the Annotation module.
The examples use `data/input/sample.csv` and export finished SVG files.

An **example** demonstrates how someone can use the library. It does not
replace a correctness test.

## 7. Tests: individual modules and full integration

The group uses **two levels of testing**:

1. **Individual module tests:** one test for each of the ten modules.
   They check calculations, expected results, and selected invalid inputs.
2. **Integration test:** `test_integration.cpp` checks the full flow:
   CSV loading → processing → plot calculations → rendering → SVG export.

`tests/` therefore contains **ten individual tests and one integration test**.

A **test** verifies an expected result and reports success or failure.
A complete graph example may look correct but is not a substitute for tests.

## 8. Requirements and build instructions

- A C++17-compatible compiler, such as GCC, Clang, or MSVC.
- CMake **3.16 or newer**.
- The C++ standard library.

No third-party plotting or graphics framework is required.

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The test command runs all eleven registered tests. GitHub Actions also builds
the code, runs the tests, executes the four examples, and verifies the SVG
exports when changes are pushed.

### Run the example programs

On Linux, or when using a single-configuration generator:

```bash
cd build
./countplot_example
./histogram_example
./kdeplot_example
./complete_library_example
```

On Windows, the executable location depends on the CMake generator. For a
Visual Studio build, an example may be `build/Debug/histogram_example.exe`.
Start it with **`build/` as the working directory**, since the sample CSV is
copied to `build/data/input/sample.csv` during CMake configuration.

## 9. Image output

By default, **all generated SVG images are saved to the project's
`data/output/` folder**, not to the build directory. The output folder is
created automatically if it does not exist.

```text
data/output/
├── countplot_example.svg
├── histogram_example.svg
├── kdeplot_example.svg
├── complete_countplot.svg
├── complete_histogram.svg
└── complete_kdeplot.svg
```

For example:

```cpp
ImageExporter::saveSVG(svg, "mygraph.svg");
```

This creates `data/output/mygraph.svg`. If an explicit output path is
provided, ImageExporter uses that requested path instead.

Generated `.svg` files are ignored by Git in this draft; running examples or
GitHub Actions does **not** automatically commit the images to GitHub.

## 10. C++ and OOP design

This draft uses **C++17** and separates declarations in `.hpp` files from
implementations in matching `.cpp` files.

- **Encapsulation:** `Axis` keeps its range values private and exposes
  public methods for scaling, ticks, and updates. `Annotation` keeps its
  collection of notes private.
- **Reusability:** plot-calculation classes expose public functions that other
  modules can call. Several of these functions are `static`.
- **Separation of responsibilities:** plot modules calculate data, `Axis`
  handles scaling, `Renderer` draws SVG, and `ImageExporter` saves files.

**Not yet implemented:** This reference version has no class-inheritance
hierarchy and no virtual-function polymorphism. Group reports should describe
these concepts as implemented only after they exist in the actual code.

**Namespace clarification:** `dataviz` is the **CMake library target name**,
not a C++ namespace in this reference implementation. The classes currently
use the global namespace; do not claim that the C++ code is inside
`namespace dataviz` unless that is implemented separately.

## 11. Naming conventions and integration

- Use consistent **PascalCase** for the public module filenames, e.g.,
  `Axis.hpp` and `Axis.cpp`.
- Match the case exactly in `#include` directives. Linux builds are
  case-sensitive.
- Keep headers in `include/` and implementations in `src/`.
- Keep a module's calculations separate from SVG drawing.
- Create individual tests and verify the integrated build before merging code.
- Use feature branches and pull requests when combining the group's work.

This README is **ready to accompany the tested reference implementation** into
the official project. Before importing it, transfer or reconcile the matching
C++ source files, example programs, tests, and `CMakeLists.txt`. The official
repository currently uses some different interfaces, capitalization, and a
different folder layout, so copying this README **by itself** would leave parts
of the documentation inaccurate.

## 12. Project references and documentation

The team's earlier planning referenced **Seaborn** for visualization ideas,
**NumPy** for histogram concepts, **SciPy** for Gaussian KDE, and
**Matplotlib** for plotting concepts. They are **conceptual references**, not
runtime dependencies of this C++ implementation.

The official repository's `reports/` folder contains the group's progress
reports and module assignments. Each member should keep their own report
consistent with the code they have actually implemented and tested.

## 13. Graph Studio — native C++ desktop interface (Windows)

Graph Studio is a **Windows desktop program written entirely in C++17**.
It uses the built-in Windows API (Win32) for its window and controls, and GDI
for an interactive chart preview. Its graph calculations and SVG export call
our existing C++ classes. There is **no Python, HTML, CSS, JavaScript, web
server, Qt or other third-party GUI requirement**.

### Build the application

On Windows with a C++17 toolchain (including MSYS2 UCRT64/GCC or MSVC):

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Then run the desktop UI. When using Ninja with MSYS2, the executable is
usually at `build/graph_studio.exe`:

```powershell
.\build\graph_studio.exe
```

For a Visual Studio generator, use:

```powershell
.\build\Debug\graph_studio.exe
```

If Windows reports a missing DLL when using MSYS2, launch VS Code from an
MSYS2 UCRT64 terminal or make sure `C:\msys64\ucrt64\bin` is in your PATH.

### Using Graph Studio

1. The provided `data/input/sample.csv` loads by default. Click
   **Browse CSV** to select another local CSV file.
2. Select **Histogram**, **KDE Plot**, or **Count Plot**.
3. Choose a CSV column, enter a title, and (for histograms) set the bin count.
4. Click **Generate SVG Graph** to draw a native C++ chart preview and export
   the actual graph through the library.
5. Use **Open Saved SVG** or **Open data/output Folder** to view the output.

Files are created in the project directory:

```text
data/output/
  ui_histogram.svg
  ui_kdeplot.svg
  ui_countplot.svg
```

Graph Studio's preview uses native Windows GDI; the saved SVG uses the
project's Renderer and ImageExporter. Both use the same calculated plot data.
The UI also passes the selected CSV column name to the Renderer as the
horizontal-axis label.

**Scope:** The desktop UI is Windows-only, but the underlying C++ library,
command-line examples, and individual/integration tests remain portable.
The existing CSV loader is intentionally simple and does not fully support
quoted commas or multiline CSV fields. Windows CI checks that Graph Studio
compiles; graphical interaction must also be tested manually on Windows.
