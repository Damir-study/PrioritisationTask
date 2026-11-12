#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include "../Prioritization.hpp"

struct BenchmarkResult {
    StorageType dictionary_type = StorageType::AVL;
    StorageType queue_type = StorageType::Heap;
    double count_time = 0;
    double queue_time = 0;
    double extract_time = 0;

    double total_time() const { return count_time + queue_time + extract_time; }
};

const char* storage_name(StorageType type);

void generate_data(mutable_array_sequence<Incident>& incidents,
                   mutable_array_sequence<Task>& tasks,
                   int count, int tag_count, unsigned int seed);
BenchmarkResult measure_prioritization(const array_sequence<Incident>& incidents,
                                       const array_sequence<Task>& tasks,
                                       StorageType dictionary_type, StorageType queue_type);
void save_benchmarks(const array_sequence<BenchmarkResult>& results,
                     int count, int tag_count, const std::string& path);

#endif // BENCHMARK_HPP
