CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic-errors

FILES = $(wildcard include/Abstract/*.hpp include/BinaryTree/*.hpp include/BinaryTreeDictionary/*.hpp include/Dictionary/*.hpp include/BinaryHeap/*.hpp include/PriorityQueue/*.hpp include/tools/*.hpp)
TEMPLATES = $(wildcard src/BinaryTree/*.tpp src/BinarytreeDictionary/*.tpp src/Dictionary/*.tpp src/BinaryHeap/*.tpp src/PriorityQueue/*.tpp)
GTEST = googletest/googletest

.PHONY: all test

all:
	$(CXX) $(CXXFLAGS) -fsyntax-only -x c++ $(FILES)

main: main.cpp src/UI/Benchmark.cpp src/tools/TreePrinter.cpp include/UI/Input.hpp include/UI/Benchmark.hpp $(FILES) $(TEMPLATES)
	$(CXX) $(CXXFLAGS) -O2 -I. main.cpp src/UI/Benchmark.cpp src/tools/TreePrinter.cpp -o main

tree_tests: tests.cpp src/tools/TreePrinter.cpp $(FILES) $(TEMPLATES)
	$(CXX) $(CXXFLAGS) -I. -I$(GTEST)/include -I$(GTEST) tests.cpp src/tools/TreePrinter.cpp $(GTEST)/src/gtest-all.cc $(GTEST)/src/gtest_main.cc -pthread -o tree_tests

test: tree_tests
	./tree_tests
