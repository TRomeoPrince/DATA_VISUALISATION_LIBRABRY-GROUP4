# Windows Graph Studio: download, run and build

The user interface is **native C++17 (Win32 + GDI)**. It does not need Python,
HTML, JavaScript, Qt, a web browser, or an internet connection to generate graphs.

## Option A: Run without coding or using the terminal

1. On Windows 10/11 (x64), open the [GitHub Releases page](https://github.com/TRomeoPrince/DATA_VISUALISATION_LIBRABRY-GROUP4/releases).
2. Download the **GraphStudio-Windows-x64.zip** attachment for the latest version.
3. Extract the **entire** ZIP into a writable folder such as
   `Documents\GraphStudio`. Do not double-click the EXE while it is still inside
   the ZIP.
4. Open the extracted `GraphStudio` directory and double-click
   **GraphStudio.exe**. You do not need to install a compiler, CMake, or Python.
5. The application opens the packaged sample CSV. You can choose your own CSV
   using **Browse CSV**, select Histogram / KDE / Count Plot, pick a column and
   click **Generate SVG Graph**. Use **Open Saved SVG** to see the result.

The downloaded folder should look like:

```text
GraphStudio/
├── GraphStudio.exe
├── START_HERE.txt
└── data/
    ├── input/
    │   ├── sample.csv
    │   ├── student_marks.csv
    │   ├── subject_marks.csv
    │   ├── subject_preferences.csv
    │   └── monthly_sales.csv
    └── output/                 (SVG images appear after generating)
```

The packaged application determines file locations relative to
`GraphStudio.exe`, **not** from a GitHub Actions runner's absolute build path.
The GUI sends SVG files to `data/output/` next to the executable. To keep your
generated graphs, back up that folder before replacing an old release.

The app currently offers Histogram, KDE Plot and Count Plot. A marks-by-subject
bar chart is a separately planned chart type and is not included yet.

This application is currently Windows x64 only. It is not digitally signed, so
Windows SmartScreen may display a warning. Only run a file you trust and verify
you downloaded it from this repository.

## Option B: Build your own Windows executable

Requirements: Windows 10/11, CMake 3.16+ and a C++17 compiler. For a simple
redistributable EXE, use Visual Studio 2022 with Desktop development with C++.
From a PowerShell terminal **at the repository root**, run:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDATAVIZ_PORTABLE_UI=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The desktop program is at `build\Release\GraphStudio.exe`.
To use this as a portable application, **copy the EXE** into a writable
`GraphStudio` folder, create `GraphStudio\data\input` and copy
`data\input\*.csv` from the repository into it. The program creates
`data\output` on the first export.

For the developer build without `-DDATAVIZ_PORTABLE_UI=ON`, the executable
may instead be named `graph_studio.exe` and sample/output paths use the
developer source checkout.

## Option C: Download a CI artifact

On the repository's [Actions page](https://github.com/TRomeoPrince/DATA_VISUALISATION_LIBRABRY-GROUP4/actions),
open a successful **C++ Build, Tests and Graph Examples** run. Under
**Artifacts**, download **GraphStudio-Windows-x64** and extract the inner ZIP.
Artifacts may expire and often require GitHub sign-in; GitHub **Releases** are
preferable for regular users.

## How maintainers publish a versioned release

After merging tested changes to `main`, tag the commit and push the tag:

```powershell
git switch main
git pull --ff-only origin main
git tag v1.0.0
git push origin v1.0.0
```

Use a new version number if `v1.0.0` already exists. The GitHub Actions
Windows job will compile the portable EXE with the MSVC static C++ runtime,
run all C++ tests, ZIP the app and sample CSVs, extract the ZIP and invoke a
headless `--self-test` to verify that it reads CSV and writes an SVG from a
different working directory. After success, it uploads the ZIP as a CI artifact
and publishes the same ZIP under that GitHub Release.

**Verification boundaries:** The CI self-test checks build success, runtime
dependencies, CSV loading and SVG output. It does not test clicking buttons,
window appearance, CSV file picker or other manual GUI interactions.

## Source tree and current build tools

The underlying portable library is built from `include/` and `src/` through
`CMakeLists.txt`; executable examples are in `examples/`, and automated tests
are in `tests/`. This is all C++17. On Linux/macOS the library and console
examples remain buildable, but this Win32 GUI is not available.
