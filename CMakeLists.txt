cmake_minimum_required(VERSION 3.16)
project(Prioritization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)

add_library(prioritization src/Prioritization.cpp src/UI/Benchmark.cpp
    TreeStructures/src/tools/TreePrinter.cpp)
target_include_directories(prioritization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(prioritization PRIVATE -Wall -Wextra -pedantic-errors)

add_executable(prioritization_gui MACOSX_BUNDLE gui_main.cpp src/UI/MainWindow.cpp)
target_link_libraries(prioritization_gui PRIVATE prioritization Qt6::Widgets)
target_compile_options(prioritization_gui PRIVATE -Wall -Wextra -pedantic-errors)
set_target_properties(prioritization_gui PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Приоритезация задач"
    MACOSX_BUNDLE_GUI_IDENTIFIER "study.prioritization")

add_executable(prioritization_tests tests.cpp
    googletest/googletest/src/gtest-all.cc googletest/googletest/src/gtest_main.cc)
target_include_directories(prioritization_tests PRIVATE
    googletest/googletest/include googletest/googletest)
target_link_libraries(prioritization_tests PRIVATE prioritization Threads::Threads)

enable_testing()
add_test(NAME prioritization COMMAND prioritization_tests)
