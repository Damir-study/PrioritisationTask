#include "../../include/UI/Benchmark.hpp"

#include <chrono>
#include <fstream>
#include <random>
#include <stdexcept>

using Clock = std::chrono::steady_clock;

const char* storage_name(StorageType type) {
    switch (type) {
        case StorageType::Heap: return "Heap";
        case StorageType::BinarySearchTree: return "BST";
        case StorageType::AVL: return "AVL";
    }
    throw std::invalid_argument("Неверное хранилище");
}

double elapsed_ms(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

void generate_data(mutable_array_sequence<Incident>& incidents,
                   mutable_array_sequence<Task>& tasks,
                   int count, int tag_count, unsigned int seed) {
    if (count < 0 || tag_count < 1) throw std::invalid_argument("Неверные параметры генерации");
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> tag(0, tag_count - 1);
    std::uniform_int_distribution<int> priority(1, 10);

    mutable_array_sequence<Incident> new_incidents;
    mutable_array_sequence<Task> new_tasks;
    for (int i = 0; i < count; ++i) {
        Incident incident;
        incident.name = "incident" + std::to_string(i);
        incident.priority = priority(random);
        Task task;
        task.name = "task" + std::to_string(i);
        for (int j = 0; j < 3; ++j) {
            incident.tags.append("tag" + std::to_string(tag(random)));
            task.tags.append("tag" + std::to_string(tag(random)));
        }
        new_incidents.append(incident);
        new_tasks.append(task);
    }
    incidents = new_incidents;
    tasks = new_tasks;
}

BenchmarkResult measure_prioritization(const array_sequence<Incident>& incidents,
                                       const array_sequence<Task>& tasks,
                                       StorageType dictionary_type, StorageType queue_type) {
    BenchmarkResult measurement;
    measurement.dictionary_type = dictionary_type;
    measurement.queue_type = queue_type;
    for (int repeat = 0; repeat < 3; ++repeat) {
        TaskPrioritizer prioritizer(incidents, tasks, dictionary_type, queue_type);

        prioritizer.prioritize();
        measurement.count_time += prioritizer.get_count_time();
        measurement.queue_time += prioritizer.get_queue_time();

        auto start = Clock::now();
        while (!prioritizer.empty()) prioritizer.pop();
        measurement.extract_time += elapsed_ms(start);
    }
    measurement.count_time /= 3;
    measurement.queue_time /= 3;
    measurement.extract_time /= 3;
    return measurement;
}

void save_benchmarks(const array_sequence<BenchmarkResult>& results,
                     int count, int tag_count, const std::string& path) {
    std::ofstream file(path);
    if (!file) throw std::runtime_error("Не удалось открыть файл результатов");
    file << "count,tag_count,dictionary,queue,count_ms,queue_ms,extract_ms,total_ms\n";
    for (int i = 0; i < results.get_length(); ++i) {
        const BenchmarkResult& result = results[i];
        file << count << ',' << tag_count << ',' << storage_name(result.dictionary_type) << ','
             << storage_name(result.queue_type) << ',' << result.count_time << ',' << result.queue_time << ','
             << result.extract_time << ',' << result.total_time() << '\n';
    }
    file.close();
    if (!file) throw std::runtime_error("Не удалось записать результаты");
}
