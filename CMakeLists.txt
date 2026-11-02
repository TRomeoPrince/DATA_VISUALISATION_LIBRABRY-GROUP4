cmake_minimum_required(VERSION 3.16)

project(DataVisualizationLibrary VERSION 1.0 LANGUAGES CXX)

# The library uses only standard C++17 and does not require plotting packages.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dataviz
    src/DataLoader.cpp
    src/DataProcessor.cpp
    src/Statistics.cpp
    src/CountPlot.cpp
    src/Histogram.cpp
    src/KDEPlot.cpp
    src/Axis.cpp
    src/Annotation.cpp
    src/Renderer.cpp
    src/ImageExporter.cpp
)
target_include_directories(dataviz PUBLIC include)

# This points to the repository's actual output folder, even when an example
# is launched from build/ or build/Debug on another computer.
target_compile_definitions(dataviz PRIVATE
    DATAVIZ_DEFAULT_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/output"
)

# We copy our sample file into the build directory for the graph demonstrations.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data/input")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/data/input/sample.csv"
    "${CMAKE_CURRENT_BINARY_DIR}/data/input/sample.csv"
    COPYONLY
)

# Examples are complete SVG graphs, not separate demos of the helper modules.
foreach(name histogram kdeplot countplot complete_library)
    add_executable(${name}_example examples/${name}_example.cpp)
    target_link_libraries(${name}_example PRIVATE dataviz)
endforeach()

enable_testing()

# Each individual test checks its module's calculations and basic behaviour.
function(add_dataviz_test name)
    add_executable(${name} tests/${name}.cpp)
    target_link_libraries(${name} PRIVATE dataviz)
    # Keep the assert() checks enabled even when using a Release build.
    if(MSVC)
        target_compile_options(${name} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${name} PRIVATE -UNDEBUG)
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

foreach(module dataloader dataprocessor statistics histogram kdeplot
               countplot axis annotation renderer imageexporter)
    add_dataviz_test(test_${module})
endforeach()

# The final test loads one CSV and makes sure all three plots can be exported.
add_dataviz_test(test_integration)

// This small executable lets the local browser UI use the C++ library.
add_executable(graph_ui_cli ui/graph_ui_cli.cpp)
target_link_libraries(graph_ui_cli PRIVATE dataviz)

add_test(
    NAME test_graph_ui_cli
    COMMAND graph_ui_cli
        "${CMAKE_CURRENT_BINARY_DIR}/data/input/sample.csv"
        histogram 2 "UI smoke test" 5 ui_smoke.svg 880
)
