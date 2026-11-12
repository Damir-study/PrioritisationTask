#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include "Benchmark.hpp"

class QCloseEvent;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QThread;
class QWidget;

class MainWindow : public QMainWindow {
private:
    QTableWidget* incidents_table;
    QTableWidget* tasks_table;
    QTableWidget* priorities_table;
    QTableWidget* result_table;
    QTableWidget* benchmark_table;
    QSpinBox* data_count;
    QSpinBox* data_tag_count;
    QSpinBox* benchmark_count;
    QSpinBox* benchmark_tag_count;
    QComboBox* dictionary_storage;
    QComboBox* queue_storage;
    QLabel* data_summary;
    QLabel* result_summary;
    QLabel* benchmark_summary;
    QProgressBar* progress;
    QWidget* data_page;
    QGroupBox* result_controls;
    QGroupBox* benchmark_controls;
    QPushButton* save_button;
    QThread* worker = nullptr;
    bool pending_close = false;
    int measured_count = 0;
    int measured_tag_count = 0;
    mutable_array_sequence<BenchmarkResult> measurements;

    QWidget* create_data_page();
    QWidget* create_result_page();
    QWidget* create_benchmark_page();
    void load_example();
    void generate();
    void set_data(const array_sequence<Incident>& incidents, const array_sequence<Task>& tasks);
    void read_data(mutable_array_sequence<Incident>& incidents, mutable_array_sequence<Task>& tasks) const;
    void invalidate_result();
    void run_prioritization();
    void run_benchmarks();
    void save_results();
    void append_measurement(const BenchmarkResult& result);
    void watch_worker();
    void show_error(const QString& message);

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
};

#endif // MAIN_WINDOW_HPP
