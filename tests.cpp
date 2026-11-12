#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

#include "include/Prioritization.hpp"
#include "include/UI/Benchmark.hpp"

Incident make_incident(int priority, const array_sequence<std::string>& tags) {
    Incident result;
    result.priority = priority;
    for (int i = 0; i < tags.get_length(); ++i) result.tags.append(tags[i]);
    return result;
}

Task make_task(const std::string& name, const array_sequence<std::string>& tags) {
    Task result;
    result.name = name;
    for (int i = 0; i < tags.get_length(); ++i) result.tags.append(tags[i]);
    return result;
}

TEST(prioritization_test, default_usage_and_owned_data) {
    mutable_array_sequence<std::string> login_server_tags;
    login_server_tags.append("login");
    login_server_tags.append("server");
    mutable_array_sequence<std::string> server_tags;
    server_tags.append("server");
    mutable_array_sequence<std::string> login_tags;
    login_tags.append("login");

    mutable_array_sequence<Incident> incidents;
    incidents.append(make_incident(5, login_server_tags));
    incidents.append(make_incident(3, server_tags));
    mutable_array_sequence<Task> tasks;
    Task login = make_task("Fix login", login_tags);
    tasks.append(login);
    Task server = make_task("Fix server", server_tags);
    tasks.append(server);

    TaskPrioritizer prioritizer(incidents, tasks);
    incidents[0].priority = 100;
    tasks[1].name = "Changed";
    tasks.pop();
    prioritizer.prioritize();

    EXPECT_EQ(prioritizer.top().name, "Fix server");
    EXPECT_EQ(prioritizer.top().priority, 8);
    PrioritizedTask first = prioritizer.pop();
    EXPECT_EQ(first.name, "Fix server");
    ASSERT_EQ(first.tags.get_length(), 1);
    EXPECT_EQ(first.tags[0], "server");
    EXPECT_EQ(first.priority, 8);
    PrioritizedTask second = prioritizer.pop();
    EXPECT_EQ(second.name, "Fix login");
    EXPECT_EQ(second.priority, 5);
    EXPECT_TRUE(prioritizer.empty());
    EXPECT_EQ(prioritizer.get_tag_priorities().get("server"), 8);
    EXPECT_THROW(prioritizer.pop(), std::runtime_error);
    EXPECT_GE(prioritizer.get_count_time(), 0);
    EXPECT_GE(prioritizer.get_queue_time(), 0);
}

TEST(prioritization_test, extracted_task_owns_name_and_tags) {
    mutable_array_sequence<std::string> a_b_tags;
    a_b_tags.append("a");
    a_b_tags.append("b");

    PrioritizedTask result;
    {
        mutable_array_sequence<Incident> incidents;
        incidents.append(make_incident(5, a_b_tags));
        mutable_array_sequence<Task> tasks;
        Task task = make_task("Fix both", a_b_tags);
        tasks.append(task);
        TaskPrioritizer prioritizer(incidents, tasks);
        prioritizer.prioritize();
        result = prioritizer.pop();
    }
    EXPECT_EQ(result.name, "Fix both");
    EXPECT_EQ(result.priority, 10);
    ASSERT_EQ(result.tags.get_length(), 2);
    EXPECT_EQ(result.tags[0], "a");
    EXPECT_EQ(result.tags[1], "b");
}

TEST(prioritization_test, weighted_tags_and_task_order) {
    mutable_array_sequence<std::string> login_server_tags;
    login_server_tags.append("login");
    login_server_tags.append("server");
    mutable_array_sequence<std::string> server_tags;
    server_tags.append("server");
    mutable_array_sequence<std::string> login_tags;
    login_tags.append("login");
    mutable_array_sequence<std::string> unknown_tags;
    unknown_tags.append("unknown");

    mutable_array_sequence<Incident> incidents;
    incidents.append(make_incident(5, login_server_tags));
    incidents.append(make_incident(3, server_tags));
    mutable_array_sequence<Task> tasks;
    tasks.append(make_task("login", login_tags));
    tasks.append(make_task("server", server_tags));
    tasks.append(make_task("login server", login_server_tags));
    tasks.append(make_task("unknown", unknown_tags));
    for (StorageType dictionary_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
        for (StorageType queue_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
            TaskPrioritizer prioritizer(incidents, tasks, dictionary_type, queue_type);
            prioritizer.prioritize();
            EXPECT_EQ(prioritizer.get_tag_priorities().get("login"), 5);
            EXPECT_EQ(prioritizer.get_tag_priorities().get("server"), 8);
            EXPECT_EQ(prioritizer.get_tag_priorities().get_size(), 2u);
            const int indices[] = {2, 1, 0, 3};
            const int weights[] = {13, 8, 5, 0};
            for (int i = 0; i < 4; ++i) {
                PrioritizedTask result = prioritizer.pop();
                EXPECT_EQ(result.name, tasks[indices[i]].name);
                EXPECT_EQ(result.priority, weights[i]);
                EXPECT_EQ(result.tags.get_length(), tasks[indices[i]].tags.get_length());
            }
            EXPECT_TRUE(prioritizer.empty());
        }
    }
}

TEST(prioritization_test, repeated_tags_equal_priorities_and_rebuilding) {
    mutable_array_sequence<std::string> a_a_a_tags;
    a_a_a_tags.append("a");
    a_a_a_tags.append("a");
    a_a_a_tags.append("a");
    mutable_array_sequence<std::string> b_tags;
    b_tags.append("b");
    mutable_array_sequence<std::string> a_a_tags;
    a_a_tags.append("a");
    a_a_tags.append("a");
    mutable_array_sequence<std::string> a_tags;
    a_tags.append("a");
    mutable_array_sequence<std::string> b_unknown_tags;
    b_unknown_tags.append("b");
    b_unknown_tags.append("unknown");
    mutable_array_sequence<std::string> empty_tags;

    mutable_array_sequence<Incident> incidents;
    incidents.append(make_incident(4, a_a_a_tags));
    incidents.append(make_incident(0, b_tags));
    mutable_array_sequence<Task> tasks;
    tasks.append(make_task("a a", a_a_tags));
    tasks.append(make_task("a", a_tags));
    tasks.append(make_task("b unknown", b_unknown_tags));
    tasks.append(make_task("", empty_tags));
    for (StorageType dictionary_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
        for (StorageType queue_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
            TaskPrioritizer prioritizer(incidents, tasks, dictionary_type, queue_type);
            prioritizer.prioritize();
            prioritizer.pop();
            prioritizer.prioritize();
            EXPECT_EQ(prioritizer.get_tag_priorities().get("a"), 4);
            for (int i = 0; i < 4; ++i) {
                PrioritizedTask result = prioritizer.pop();
                EXPECT_EQ(result.name, tasks[i].name);
                EXPECT_EQ(result.priority, i < 2 ? 4 : 0);
            }
            EXPECT_TRUE(prioritizer.empty());
        }
    }
}

TEST(prioritization_test, empty_input) {
    mutable_array_sequence<std::string> missing_tags;
    missing_tags.append("missing");
    mutable_array_sequence<std::string> a_tags;
    a_tags.append("a");

    mutable_array_sequence<Incident> incidents;
    mutable_array_sequence<Task> tasks;
    for (StorageType type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
        TaskPrioritizer empty(incidents, tasks, type, type);
        empty.prioritize();
        EXPECT_TRUE(empty.empty());
        EXPECT_TRUE(empty.get_tag_priorities().is_empty());
        EXPECT_THROW(empty.top(), std::runtime_error);

        tasks.append(make_task("missing", missing_tags));
        TaskPrioritizer no_incidents(incidents, tasks, type, type);
        no_incidents.prioritize();
        PrioritizedTask result = no_incidents.pop();
        EXPECT_EQ(result.name, "missing");
        EXPECT_EQ(result.priority, 0);
        EXPECT_EQ(result.tags[0], "missing");
        tasks.pop();

        incidents.append(make_incident(1, a_tags));
        TaskPrioritizer no_tasks(incidents, tasks, type, type);
        no_tasks.prioritize();
        EXPECT_TRUE(no_tasks.empty());
        EXPECT_EQ(no_tasks.get_tag_priorities().get("a"), 1);
        incidents.pop();
    }
}

TEST(prioritization_test, invalid_input) {
    mutable_array_sequence<std::string> a_tags;
    a_tags.append("a");

    mutable_array_sequence<Incident> incidents;
    mutable_array_sequence<Task> tasks;
    EXPECT_THROW(TaskPrioritizer(incidents, tasks, static_cast<StorageType>(-1)), std::invalid_argument);
    EXPECT_THROW(TaskPrioritizer(incidents, tasks, StorageType::AVL, static_cast<StorageType>(-1)), std::invalid_argument);
    incidents.append(make_incident(-1, a_tags));
    TaskPrioritizer negative(incidents, tasks);
    EXPECT_THROW(negative.prioritize(), std::invalid_argument);
    incidents[0].priority = 1;
    incidents[0].tags.append("");
    TaskPrioritizer empty_incident_tag(incidents, tasks);
    EXPECT_THROW(empty_incident_tag.prioritize(), std::invalid_argument);
    EXPECT_TRUE(empty_incident_tag.get_tag_priorities().is_empty());
    incidents[0].tags.pop();
    tasks.append(make_task("a", a_tags));
    Task invalid_task;
    invalid_task.tags.append("");
    tasks.append(invalid_task);
    TaskPrioritizer empty_task_tag(incidents, tasks);
    EXPECT_THROW(empty_task_tag.prioritize(), std::invalid_argument);
    EXPECT_TRUE(empty_task_tag.empty());
    EXPECT_TRUE(empty_task_tag.get_tag_priorities().is_empty());
}

TEST(prioritization_test, generator_parameters) {
    mutable_array_sequence<Incident> incidents;
    mutable_array_sequence<Task> tasks;
    EXPECT_THROW(generate_data(incidents, tasks, -1, 10, 42), std::invalid_argument);
    EXPECT_THROW(generate_data(incidents, tasks, 1, 0, 42), std::invalid_argument);
    generate_data(incidents, tasks, 5, 1, 42);
    generate_data(incidents, tasks, 0, 1, 42);
    EXPECT_EQ(incidents.get_length(), 0);
    EXPECT_EQ(tasks.get_length(), 0);
}