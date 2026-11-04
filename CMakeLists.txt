cmake_minimum_required(VERSION 3.16)
project(DataVisualizationLibrary VERSION 2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(DATAVIZ_PORTABLE_UI "Build relocatable Windows Graph Studio" OFF)
if(MSVC AND DATAVIZ_PORTABLE_UI)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
add_library(dataviz
    src/data/DataLoader.cpp
    src/data/DataProcessor.cpp
    src/data/Statistics.cpp
    src/plots/CountPlot.cpp
    src/plots/Histogram.cpp
    src/plots/KDEPlot.cpp
    src/layout/Axis.cpp
    src/layout/Annotation.cpp
    src/rendering/Renderer.cpp
    src/export/ImageExporter.cpp
    src/core/Config.cpp
    src/core/Theme.cpp
    src/core/ColorMap.cpp
    src/core/Utils.cpp
    src/plots/LinePlot.cpp
    src/plots/ScatterPlot.cpp
    src/plots/BarChart.cpp
    src/plots/BoxPlot.cpp
    src/plots/Heatmap.cpp
    src/plots/PieChart.cpp
    src/export/SVGExporter.cpp
)
target_include_directories(dataviz PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/data
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/plots
    ${CMAKE_CURRENT_SOURCE_DIR}/include/layout
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/include/export
)
if(WIN32)
    target_link_libraries(dataviz PUBLIC gdiplus)
endif()
target_compile_definitions(dataviz PRIVATE
    DATAVIZ_DEFAULT_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/output"
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data/input")
foreach(sample sample.csv monthly_sales.csv student_marks.csv subject_marks.csv subject_preferences.csv)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/data/input/${sample}"
                   "${CMAKE_CURRENT_BINARY_DIR}/data/input/${sample}" COPYONLY)
endforeach()
foreach(name histogram kdeplot countplot complete_library)
    add_executable(${name}_example examples/${name}_example.cpp)
    target_link_libraries(${name}_example PRIVATE dataviz)
endforeach()
enable_testing()
function(add_dataviz_test name)
    add_executable(${name} tests/${name}.cpp)
    target_link_libraries(${name} PRIVATE dataviz)
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
add_dataviz_test(test_integration)
add_dataviz_test(test_extended_plots)
add_dataviz_test(test_csv_quotes)
add_dataviz_test(test_complete_graphs)
add_dataviz_test(test_raster_export)
if(WIN32)
    add_executable(graph_studio WIN32 ui/graph_studio_win32.cpp)
    target_link_libraries(graph_studio PRIVATE dataviz user32 gdi32 comdlg32 shell32)
    add_test(NAME test_graphstudio_selftest COMMAND graph_studio --self-test)
    if(DATAVIZ_PORTABLE_UI)
        target_compile_definitions(graph_studio PRIVATE DATAVIZ_PORTABLE_UI=1)
        set_target_properties(graph_studio PROPERTIES OUTPUT_NAME GraphStudio)
    else()
        target_compile_definitions(graph_studio PRIVATE
            DATAVIZ_SAMPLE_CSV_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/input/sample.csv")
    endif()
endif()
