CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic-errors

SOURCES = src/Prioritization.cpp src/UI/Benchmark.cpp TreeStructures/src/tools/TreePrinter.cpp
HEADERS = $(wildcard include/*.hpp include/UI/*.hpp)
TREE_FILES = $(wildcard TreeStructures/include/*/*.hpp TreeStructures/src/*/*.tpp)
SEQUENCE_FILES = $(wildcard TreeStructures/external/*.h TreeStructures/external/*.tpp)
GTEST = googletest/googletest

.PHONY: all test gui

all: gui

prioritization_tests: tests.cpp $(SOURCES) $(HEADERS) $(TREE_FILES) $(SEQUENCE_FILES)
	$(CXX) $(CXXFLAGS) -I. -I$(GTEST)/include -I$(GTEST) tests.cpp $(SOURCES) $(GTEST)/src/gtest-all.cc $(GTEST)/src/gtest_main.cc -pthread -o prioritization_tests

test: prioritization_tests
	./prioritization_tests

gui:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --target prioritization_gui -j2
