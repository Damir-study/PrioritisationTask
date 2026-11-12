#ifndef PRIORITIZATION_HPP
#define PRIORITIZATION_HPP

#include <string>
#include "../TreeStructures/external/mutable_array_sequence.h"
#include "../TreeStructures/include/Dictionary/Dictionary.hpp"
#include "../TreeStructures/include/PriorityQueue/PriorityQueue.hpp"

enum class StorageType {
    Heap,
    BinarySearchTree,
    AVL
};

struct Incident {
    std::string name;
    int priority = 0;
    mutable_array_sequence<std::string> tags;
};

struct Task {
    std::string name;
    mutable_array_sequence<std::string> tags;
};

struct PrioritizedTask {
    std::string name;
    mutable_array_sequence<std::string> tags;
    int priority = 0;
    int task_index = 0;

    bool operator<(const PrioritizedTask& other) const;
    bool operator>(const PrioritizedTask& other) const;
    bool operator==(const PrioritizedTask& other) const;
};

class TaskPrioritizer {
private:
    mutable_array_sequence<Incident> incidents;
    mutable_array_sequence<Task> tasks;
    Dictionary<std::string, int> priorities;
    PriorityQueue<PrioritizedTask> queue;
    double count_time = 0;
    double queue_time = 0;

    static Dictionary<std::string, int> create_dictionary(StorageType type);
    static PriorityQueue<PrioritizedTask> create_queue(StorageType type);
    static bool repeated_tag(const array_sequence<std::string>& tags, int index);
    static int add_priority(int first, int second);
    void count_tags();
    void build_queue();

public:
    TaskPrioritizer(const array_sequence<Incident>& incidents,
                    const array_sequence<Task>& tasks,
                    StorageType dictionary_type = StorageType::AVL,
                    StorageType queue_type = StorageType::Heap);

    void prioritize();
    bool empty() const;
    const PrioritizedTask& top() const;
    PrioritizedTask pop();
    const Dictionary<std::string, int>& get_tag_priorities() const;
    double get_count_time() const;
    double get_queue_time() const;
};

#endif // PRIORITIZATION_HPP
