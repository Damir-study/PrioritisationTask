#include "../include/Prioritization.hpp"

#include <chrono>
#include <stdexcept>
#include "../TreeStructures/include/BinaryTree/BinarySearchTree.hpp"
#include "../TreeStructures/include/BinaryTree/BalancedTree.hpp"
#include "../TreeStructures/include/Dictionary/BinaryTreeDictionaryStorage.hpp"
#include "../TreeStructures/include/Dictionary/BinaryHeapDictionaryStorage.hpp"
#include "../TreeStructures/include/PriorityQueue/BinaryTreeStorage.hpp"
#include "../TreeStructures/include/PriorityQueue/BinaryHeapStorage.hpp"

bool PrioritizedTask::operator<(const PrioritizedTask& other) const {
    if (priority != other.priority) return priority > other.priority;
    return task_index < other.task_index;
}

bool PrioritizedTask::operator>(const PrioritizedTask& other) const {
    return other < *this;
}

bool PrioritizedTask::operator==(const PrioritizedTask& other) const {
    if (task_index != other.task_index || priority != other.priority || name != other.name)
        return false;
    if (tags.get_length() != other.tags.get_length()) return false;
    for (int i = 0; i < tags.get_length(); ++i) {
        if (tags[i] != other.tags[i]) return false;
    }
    return true;
}

Dictionary<std::string, int> TaskPrioritizer::create_dictionary(StorageType type) {
    using Pair = DictionaryPair<std::string, int>;
    switch (type) {
        case StorageType::Heap:
            return Dictionary<std::string, int>(new BinaryHeapDictionaryStorage<std::string, int>);
        case StorageType::BinarySearchTree:
            return Dictionary<std::string, int>(
                new BinaryTreeDictionaryStorage<std::string, int>(new BinarySearchTree<Pair>));
        case StorageType::AVL:
            return Dictionary<std::string, int>(
                new BinaryTreeDictionaryStorage<std::string, int>(new BalancedTree<Pair>));
    }
    throw std::invalid_argument("Неверное хранилище словаря");
}

PriorityQueue<PrioritizedTask> TaskPrioritizer::create_queue(StorageType type) {
    switch (type) {
        case StorageType::Heap:
            return PriorityQueue<PrioritizedTask>(new BinaryHeapStorage<PrioritizedTask>(default_compare<PrioritizedTask>));
        case StorageType::BinarySearchTree:
            return PriorityQueue<PrioritizedTask>(
                new BinaryTreeStorage<PrioritizedTask>(new BinarySearchTree<PrioritizedTask>(default_compare<PrioritizedTask>)));
        case StorageType::AVL:
            return PriorityQueue<PrioritizedTask>(
                new BinaryTreeStorage<PrioritizedTask>(new BalancedTree<PrioritizedTask>(default_compare<PrioritizedTask>)));
    }
    throw std::invalid_argument("Неверное хранилище очереди");
}

TaskPrioritizer::TaskPrioritizer(const array_sequence<Incident>& incidents,
                                const array_sequence<Task>& tasks,
                                StorageType dictionary_type, StorageType queue_type)
    : incidents(incidents), tasks(tasks), priorities(create_dictionary(dictionary_type)),
      queue(create_queue(queue_type)) {}

void TaskPrioritizer::prioritize() {
    queue.clear();
    priorities.clear();
    count_time = 0;
    queue_time = 0;
    try {
        auto start = std::chrono::steady_clock::now();
        count_tags();
        count_time = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

        start = std::chrono::steady_clock::now();
        build_queue();
        queue_time = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    } catch (...) {
        queue.clear();
        priorities.clear();
        count_time = 0;
        queue_time = 0;
        throw;
    }
}

bool TaskPrioritizer::empty() const {
    return queue.empty();
}

const PrioritizedTask& TaskPrioritizer::top() const {
    return queue.top();
}

PrioritizedTask TaskPrioritizer::pop() {
    return queue.pop();
}

const Dictionary<std::string, int>& TaskPrioritizer::get_tag_priorities() const {
    return priorities;
}

double TaskPrioritizer::get_count_time() const {
    return count_time;
}

double TaskPrioritizer::get_queue_time() const {
    return queue_time;
}

bool TaskPrioritizer::repeated_tag(const array_sequence<std::string>& tags, int index) {
    if (tags[index].empty()) throw std::invalid_argument("Тег не должен быть пустым");
    for (int i = 0; i < index; ++i) {
        if (tags[i] == tags[index]) return true;
    }
    return false;
}

int TaskPrioritizer::add_priority(int first, int second) {
    if (second < 0) throw std::invalid_argument("Приоритет не должен быть отрицательным");
    return first + second;
}

void TaskPrioritizer::count_tags() {
    for (int i = 0; i < incidents.get_length(); ++i) {
        const Incident& incident = incidents[i];
        if (incident.priority < 0) throw std::invalid_argument("Приоритет не должен быть отрицательным");
        for (int j = 0; j < incident.tags.get_length(); ++j) {
            if (repeated_tag(incident.tags, j)) continue;
            const std::string& tag = incident.tags[j];
            if (priorities.contains_key(tag)) {
                int& priority = priorities.get(tag);
                priority = add_priority(priority, incident.priority);
            }
            else priorities.add(tag, incident.priority);
        }
    }
}

void TaskPrioritizer::build_queue() {
    for (int i = 0; i < tasks.get_length(); ++i) {
        int priority = 0;
        for (int j = 0; j < tasks[i].tags.get_length(); ++j) {
            if (repeated_tag(tasks[i].tags, j)) continue;
            const std::string& tag = tasks[i].tags[j];
            if (priorities.contains_key(tag)) priority = add_priority(priority, priorities.get(tag));
        }
        PrioritizedTask result;
        result.name = tasks[i].name;
        result.tags = tasks[i].tags;
        result.priority = priority;
        result.task_index = i;
        queue.push(result);
    }
}
