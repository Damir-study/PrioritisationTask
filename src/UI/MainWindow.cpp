#include "../../include/UI/MainWindow.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QThread>
#include <QVBoxLayout>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include "../../TreeStructures/include/Dictionary/DictionaryPair.hpp"

Q_DECLARE_METATYPE(StorageType)

QTableWidget* create_table(const QStringList& headers, bool editable) {
    auto* table = new QTableWidget(0, headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setDefaultSectionSize(34);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);
    table->setWordWrap(false);
    if (!editable) table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    return table;
}

QPushButton* create_button(const QString& text, bool primary = false) {
    auto* button = new QPushButton(text);
    button->setCursor(Qt::PointingHandCursor);
    if (primary) button->setProperty("primary", true);
    return button;
}

QSpinBox* create_count(int value, int maximum) {
    auto* count = new QSpinBox;
    count->setRange(1, maximum);
    count->setValue(value);
    count->setGroupSeparatorShown(true);
    count->setMinimumWidth(105);
    return count;
}

QComboBox* create_storage() {
    auto* storage = new QComboBox;
    storage->addItem("Бинарная куча", QVariant::fromValue(StorageType::Heap));
    storage->addItem("Дерево поиска", QVariant::fromValue(StorageType::BinarySearchTree));
    storage->addItem("AVL-дерево", QVariant::fromValue(StorageType::AVL));
    storage->setCurrentIndex(storage->findData(QVariant::fromValue(StorageType::AVL)));
    storage->setMinimumWidth(160);
    return storage;
}

void set_cell(QTableWidget* table, int row, int column, const QString& value) {
    auto* item = new QTableWidgetItem(value);
    item->setToolTip(value);
    table->setItem(row, column, item);
}

QString cell_text(const QTableWidget* table, int row, int column) {
    const auto* item = table->item(row, column);
    return item ? item->text().trimmed() : QString();
}

QString tags_text(const array_sequence<std::string>& tags) {
    QString text;
    for (int i = 0; i < tags.get_length(); ++i) {
        if (i) text += ' ';
        text += QString::fromStdString(tags[i]);
    }
    return text;
}

mutable_array_sequence<std::string> parse_tags(const QString& text) {
    mutable_array_sequence<std::string> tags;
    std::istringstream input(text.toStdString());
    std::string tag;
    while (input >> tag) tags.append(tag);
    return tags;
}

QGroupBox* create_editor(const QString& title, QTableWidget* table, bool incident) {
    auto* group = new QGroupBox(title);
    auto* layout = new QVBoxLayout(group);
    auto* buttons = new QHBoxLayout;
    auto* add = create_button("+ Добавить");
    auto* remove = create_button("Удалить строку");
    auto* clear = create_button("Очистить");
    buttons->addWidget(add);
    buttons->addWidget(remove);
    buttons->addStretch();
    buttons->addWidget(clear);
    layout->addLayout(buttons);
    layout->addWidget(table);
    QObject::connect(add, &QPushButton::clicked, table, [table, incident]() {
        int row = table->rowCount();
        table->insertRow(row);
        set_cell(table, row, 0, "");
        if (incident) set_cell(table, row, 1, "1");
        set_cell(table, row, incident ? 2 : 1, "");
        table->setCurrentCell(row, 0);
        table->editItem(table->item(row, 0));
    });
    QObject::connect(remove, &QPushButton::clicked, table, [table]() {
        if (table->currentRow() >= 0) table->removeRow(table->currentRow());
    });
    QObject::connect(clear, &QPushButton::clicked, table, [table]() { table->setRowCount(0); });
    return group;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Приоритезация задач — Лабораторная работа №2");
    resize(1180, 780);
    setMinimumSize(920, 640);
    auto* central = new QWidget;
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(28, 22, 28, 14);
    layout->setSpacing(14);
    auto* title = new QLabel("Приоритезация задач");
    title->setObjectName("title");
    auto* subtitle = new QLabel("Инциденты → веса тегов → порядок выполнения");
    subtitle->setObjectName("subtitle");
    layout->addWidget(title);
    layout->addWidget(subtitle);
    auto* tabs = new QTabWidget;
    tabs->setObjectName("tabs");
    data_page = create_data_page();
    tabs->addTab(data_page, "01  Данные");
    tabs->addTab(create_result_page(), "02  Приоритезация");
    tabs->addTab(create_benchmark_page(), "03  Сравнение скорости");
    layout->addWidget(tabs);
    setCentralWidget(central);
    progress = new QProgressBar;
    progress->setFixedWidth(180);
    progress->setRange(0, 9);
    progress->hide();
    statusBar()->addPermanentWidget(progress);

    setStyleSheet(R"(
        QMainWindow { background: #f4f6fa; }
        QWidget { color: #25324a; font-size: 13px; }
        QLabel#title { font-size: 27px; font-weight: 700; }
        QLabel#subtitle { color: #68758b; font-size: 14px; margin-bottom: 6px; }
        QTabWidget::pane { background: white; border: 1px solid #dce2ec; border-radius: 8px; }
        QTabBar::tab { padding: 12px 22px; margin-right: 5px; color: #68758b; }
        QTabBar::tab:selected { color: #235bd4; background: white; font-weight: 600; }
        QGroupBox { border: 1px solid #dce2ec; border-radius: 7px; margin-top: 15px;
                    padding: 14px 10px 10px; font-weight: 600; }
        QGroupBox::title { subcontrol-origin: margin; left: 13px; padding: 0 5px; }
        QPushButton { background: #f1f4f9; border: 1px solid #d8dfeb; border-radius: 5px;
                      padding: 8px 13px; }
        QPushButton:hover { background: #e5ebf6; }
        QPushButton[primary="true"] { background: #2862db; color: white; border-color: #2862db; }
        QPushButton[primary="true"]:hover { background: #1e50bb; }
        QPushButton:disabled { background: #edf0f5; color: #99a2b2; border-color: #e1e5ec; }
        QSpinBox, QComboBox { background: white; border: 1px solid #d8dfeb;
                             border-radius: 4px; padding: 6px; }
        QTableWidget { background: white; alternate-background-color: #f7f9fc;
                       border: 1px solid #e2e7ef; border-radius: 4px;
                       selection-background-color: #deebff; selection-color: #183b7b; }
        QHeaderView::section { background: #eef2f8; color: #55637a; border: none;
                               padding: 9px 7px; font-weight: 600; }
        QStatusBar { background: #f4f6fa; color: #68758b; }
        QProgressBar { border: 1px solid #d8dfeb; border-radius: 4px; text-align: center; }
        QProgressBar::chunk { background: #8fb3f9; }
    )");

    connect(incidents_table, &QTableWidget::cellChanged, this, [this]() { invalidate_result(); });
    connect(tasks_table, &QTableWidget::cellChanged, this, [this]() { invalidate_result(); });
    connect(incidents_table->model(), &QAbstractItemModel::rowsRemoved, this, [this]() { invalidate_result(); });
    connect(tasks_table->model(), &QAbstractItemModel::rowsRemoved, this, [this]() { invalidate_result(); });
    connect(dictionary_storage, &QComboBox::currentIndexChanged, this, [this]() { invalidate_result(); });
    connect(queue_storage, &QComboBox::currentIndexChanged, this, [this]() { invalidate_result(); });
    load_example();
}

MainWindow::~MainWindow() {
    if (worker) {
        worker->requestInterruption();
        worker->wait();
        delete worker;
    }
}

QWidget* MainWindow::create_data_page() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    auto* controls = new QHBoxLayout;
    data_count = create_count(100, 10000);
    data_tag_count = create_count(20, 10000);
    auto* generate_button = create_button("Сгенерировать", true);
    auto* example_button = create_button("Загрузить пример");
    controls->addWidget(new QLabel("Инцидентов и заданий:"));
    controls->addWidget(data_count);
    controls->addWidget(new QLabel("Возможных тегов:"));
    controls->addWidget(data_tag_count);
    controls->addWidget(generate_button);
    controls->addStretch();
    controls->addWidget(example_button);
    layout->addLayout(controls);
    auto* hint = new QLabel("Редактируй ячейки двойным нажатием. Теги разделяются пробелами, приоритет инцидента ≥ 0.");
    hint->setWordWrap(true);
    layout->addWidget(hint);
    incidents_table = create_table({"Инцидент", "Приоритет", "Теги"}, true);
    incidents_table->setObjectName("incidents_table");
    incidents_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tasks_table = create_table({"Задание", "Теги"}, true);
    tasks_table->setObjectName("tasks_table");
    auto* splitter = new QSplitter;
    splitter->addWidget(create_editor("Инциденты", incidents_table, true));
    splitter->addWidget(create_editor("Задания", tasks_table, false));
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter, 1);
    data_summary = new QLabel;
    layout->addWidget(data_summary);
    connect(generate_button, &QPushButton::clicked, this, [this]() { generate(); });
    connect(example_button, &QPushButton::clicked, this, [this]() { load_example(); });
    return page;
}

QWidget* MainWindow::create_result_page() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 10, 16, 16);
    result_controls = new QGroupBox("Хранилища");
    auto* controls = new QHBoxLayout(result_controls);
    dictionary_storage = create_storage();
    queue_storage = create_storage();
    queue_storage->setCurrentIndex(queue_storage->findData(QVariant::fromValue(StorageType::Heap)));
    auto* run = create_button("Рассчитать приоритеты", true);
    run->setObjectName("run_prioritization");
    controls->addWidget(new QLabel("Словарь:"));
    controls->addWidget(dictionary_storage);
    controls->addSpacing(16);
    controls->addWidget(new QLabel("Очередь:"));
    controls->addWidget(queue_storage);
    controls->addStretch();
    controls->addWidget(run);
    layout->addWidget(result_controls);
    auto* hint = new QLabel("Вес тега — сумма приоритетов его инцидентов. Приоритет задания — сумма весов его тегов.\n"
                           "Повторы тега учитываются один раз. При равенстве приоритетов сохраняется исходный порядок.");
    hint->setWordWrap(true);
    layout->addWidget(hint);
    auto* splitter = new QSplitter;
    auto* priorities = new QGroupBox("Веса тегов");
    auto* priorities_layout = new QVBoxLayout(priorities);
    priorities_table = create_table({"Тег", "Вес"}, false);
    priorities_table->setObjectName("priorities_table");
    priorities_layout->addWidget(priorities_table);
    auto* result = new QGroupBox("Порядок выполнения заданий");
    auto* result_layout = new QVBoxLayout(result);
    result_table = create_table({"Место", "Задание", "Приоритет", "Теги"}, false);
    result_table->setObjectName("result_table");
    result_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    result_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    result_layout->addWidget(result_table);
    splitter->addWidget(priorities);
    splitter->addWidget(result);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter, 1);
    result_summary = new QLabel("Нажми «Рассчитать приоритеты», чтобы получить результат.");
    result_summary->setWordWrap(true);
    layout->addWidget(result_summary);
    connect(run, &QPushButton::clicked, this, [this]() { run_prioritization(); });
    return page;
}

QWidget* MainWindow::create_benchmark_page() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 10, 16, 16);
    benchmark_controls = new QGroupBox("Параметры сравнения");
    auto* controls = new QHBoxLayout(benchmark_controls);
    benchmark_count = create_count(1000, 100000);
    benchmark_tag_count = create_count(100, 100000);
    auto* run = create_button("Сравнить все хранилища", true);
    run->setObjectName("run_benchmarks");
    controls->addWidget(new QLabel("Инцидентов и заданий:"));
    controls->addWidget(benchmark_count);
    controls->addWidget(new QLabel("Возможных тегов:"));
    controls->addWidget(benchmark_tag_count);
    controls->addStretch();
    controls->addWidget(run);
    layout->addWidget(benchmark_controls);
    auto* hint = new QLabel("Все 9 сочетаний используют одинаковые сгенерированные данные. Среднее за 3 запуска, время в мс.\n"
                           "Генерация и отображение в замер не входят.");
    hint->setWordWrap(true);
    layout->addWidget(hint);
    benchmark_table = create_table({"Словарь", "Очередь", "Подсчёт тегов", "Построение очереди", "Извлечение", "Всего, мс"}, false);
    benchmark_table->setObjectName("benchmark_table");
    layout->addWidget(benchmark_table);
    auto* bottom = new QHBoxLayout;
    benchmark_summary = new QLabel("Результаты появятся после запуска сравнения.");
    benchmark_summary->setWordWrap(true);
    save_button = create_button("Сохранить CSV");
    save_button->setEnabled(false);
    bottom->addWidget(benchmark_summary, 1);
    bottom->addWidget(save_button);
    layout->addLayout(bottom);
    connect(run, &QPushButton::clicked, this, [this]() { run_benchmarks(); });
    connect(save_button, &QPushButton::clicked, this, [this]() { save_results(); });
    return page;
}

void MainWindow::set_data(const array_sequence<Incident>& incidents, const array_sequence<Task>& tasks) {
    QSignalBlocker incident_signals(incidents_table);
    QSignalBlocker task_signals(tasks_table);
    incidents_table->setRowCount(incidents.get_length());
    for (int i = 0; i < incidents.get_length(); ++i) {
        set_cell(incidents_table, i, 0, QString::fromStdString(incidents[i].name));
        set_cell(incidents_table, i, 1, QString::number(incidents[i].priority));
        set_cell(incidents_table, i, 2, tags_text(incidents[i].tags));
    }
    tasks_table->setRowCount(tasks.get_length());
    for (int i = 0; i < tasks.get_length(); ++i) {
        set_cell(tasks_table, i, 0, QString::fromStdString(tasks[i].name));
        set_cell(tasks_table, i, 1, tags_text(tasks[i].tags));
    }
    invalidate_result();
}

void MainWindow::load_example() {
    mutable_array_sequence<std::string> login_server_tags;
    login_server_tags.append("login");
    login_server_tags.append("server");
    mutable_array_sequence<std::string> catalog_server_tags;
    catalog_server_tags.append("catalog");
    catalog_server_tags.append("server");
    mutable_array_sequence<std::string> cart_server_tags;
    cart_server_tags.append("cart");
    cart_server_tags.append("server");
    mutable_array_sequence<std::string> payment_cart_tags;
    payment_cart_tags.append("payment");
    payment_cart_tags.append("cart");
    mutable_array_sequence<std::string> payment_email_tags;
    payment_email_tags.append("payment");
    payment_email_tags.append("email");
    mutable_array_sequence<std::string> delivery_email_tags;
    delivery_email_tags.append("delivery");
    delivery_email_tags.append("email");
    mutable_array_sequence<std::string> mobile_login_tags;
    mobile_login_tags.append("mobile");
    mobile_login_tags.append("login");
    mutable_array_sequence<std::string> login_security_tags;
    login_security_tags.append("login");
    login_security_tags.append("security");
    mutable_array_sequence<std::string> mobile_catalog_tags;
    mobile_catalog_tags.append("mobile");
    mobile_catalog_tags.append("catalog");
    mutable_array_sequence<std::string> server_database_tags;
    server_database_tags.append("server");
    server_database_tags.append("database");
    mutable_array_sequence<std::string> email_tags;
    email_tags.append("email");
    mutable_array_sequence<std::string> delivery_tags;
    delivery_tags.append("delivery");
    mutable_array_sequence<std::string> server_tags;
    server_tags.append("server");
    mutable_array_sequence<std::string> cart_tags;
    cart_tags.append("cart");
    mutable_array_sequence<std::string> docs_tags;
    docs_tags.append("docs");
    mutable_array_sequence<std::string> design_tags;
    design_tags.append("design");

    mutable_array_sequence<Incident> incidents;
    incidents.append(Incident{"Покупатели не могут войти", 10, login_server_tags});
    incidents.append(Incident{"Каталог медленно открывается", 6, catalog_server_tags});
    incidents.append(Incident{"Товары пропадают из корзины", 4, cart_server_tags});
    incidents.append(Incident{"Ошибка при оплате заказа", 8, payment_cart_tags});
    incidents.append(Incident{"Не приходят письма сброса пароля", 2, email_tags});
    incidents.append(Incident{"Нет подтверждения оплаты", 5, payment_email_tags});
    incidents.append(Incident{"Не обновляется статус доставки", 3, delivery_tags});
    incidents.append(Incident{"Не приходят письма о доставке", 2, delivery_email_tags});
    incidents.append(Incident{"Приложение вылетает при входе", 7, mobile_login_tags});
    incidents.append(Incident{"Подозрительные входы в аккаунты", 9, login_security_tags});
    incidents.append(Incident{"Каталог не открывается в приложении", 4, mobile_catalog_tags});
    incidents.append(Incident{"База данных отвечает с задержкой", 6, server_database_tags});

    mutable_array_sequence<Task> tasks;
    tasks.append(Task{"Исправить сервер авторизации", login_server_tags});
    tasks.append(Task{"Ускорить каталог", catalog_server_tags});
    tasks.append(Task{"Исправить оплату корзины", payment_cart_tags});
    tasks.append(Task{"Проверить уведомления об оплате", payment_email_tags});
    tasks.append(Task{"Восстановить письма сброса пароля", email_tags});
    tasks.append(Task{"Исправить статусы доставки", delivery_tags});
    tasks.append(Task{"Исправить уведомления о доставке", delivery_email_tags});
    tasks.append(Task{"Исправить вход в мобильном приложении", mobile_login_tags});
    tasks.append(Task{"Усилить защиту аккаунтов", login_security_tags});
    tasks.append(Task{"Исправить каталог в приложении", mobile_catalog_tags});
    tasks.append(Task{"Оптимизировать базу данных", server_database_tags});
    tasks.append(Task{"Оптимизировать сервер", server_tags});
    tasks.append(Task{"Исправить сохранение корзины", cart_tags});
    tasks.append(Task{"Обновить шаблоны писем", email_tags});
    tasks.append(Task{"Обновить документацию", docs_tags});
    tasks.append(Task{"Обновить внешний вид сайта", design_tags});
    set_data(incidents, tasks);
    statusBar()->showMessage("Пример загружен: 12 инцидентов и 16 заданий. Данные можно редактировать.");
}

void MainWindow::generate() {
    try {
        mutable_array_sequence<Incident> incidents;
        mutable_array_sequence<Task> tasks;
        generate_data(incidents, tasks, data_count->value(), data_tag_count->value(), 42);
        set_data(incidents, tasks);
        statusBar()->showMessage("Данные сгенерированы.");
    } catch (const std::exception& error) { show_error(QString::fromUtf8(error.what())); }
}

void MainWindow::read_data(mutable_array_sequence<Incident>& incidents, mutable_array_sequence<Task>& tasks) const {
    for (int i = 0; i < incidents_table->rowCount(); ++i) {
        Incident incident;
        incident.name = cell_text(incidents_table, i, 0).toStdString();
        if (incident.name.empty()) throw std::invalid_argument("Укажи название инцидента в строке " + std::to_string(i + 1));
        bool valid;
        incident.priority = cell_text(incidents_table, i, 1).toInt(&valid);
        if (!valid || incident.priority < 0)
            throw std::invalid_argument("Неверный приоритет инцидента в строке " + std::to_string(i + 1));
        incident.tags = parse_tags(cell_text(incidents_table, i, 2));
        incidents.append(incident);
    }
    for (int i = 0; i < tasks_table->rowCount(); ++i) {
        Task task;
        task.name = cell_text(tasks_table, i, 0).toStdString();
        if (task.name.empty()) throw std::invalid_argument("Укажи название задания в строке " + std::to_string(i + 1));
        task.tags = parse_tags(cell_text(tasks_table, i, 1));
        tasks.append(task);
    }
}

void MainWindow::invalidate_result() {
    priorities_table->setRowCount(0);
    result_table->setRowCount(0);
    result_summary->setText("Нажми «Рассчитать приоритеты», чтобы получить результат.");
    data_summary->setText(QString("Инцидентов: %1   •   Заданий: %2").arg(incidents_table->rowCount()).arg(tasks_table->rowCount()));
}

void MainWindow::watch_worker() {
    data_page->setEnabled(false);
    result_controls->setEnabled(false);
    benchmark_controls->setEnabled(false);
    save_button->setEnabled(false);
    progress->show();
    connect(worker, &QThread::finished, this, [this]() {
        QThread* finished = worker;
        worker = nullptr;
        finished->deleteLater();
        data_page->setEnabled(true);
        result_controls->setEnabled(true);
        benchmark_controls->setEnabled(true);
        save_button->setEnabled(measurements.get_length() == 9);
        progress->hide();
        if (pending_close) close();
    });
    worker->start();
}

void MainWindow::run_prioritization() {
    if (worker) return;
    try {
        mutable_array_sequence<Incident> incidents;
        mutable_array_sequence<Task> tasks;
        read_data(incidents, tasks);
        StorageType dictionary_type = dictionary_storage->currentData().value<StorageType>();
        StorageType queue_type = queue_storage->currentData().value<StorageType>();
        invalidate_result();
        result_summary->setText("Вычисление приоритетов…");
        statusBar()->showMessage("Вычисление приоритетов…");
        progress->setRange(0, 0);
        worker = QThread::create([this, incidents, tasks, dictionary_type, queue_type]() {
            try {
                TaskPrioritizer prioritizer(incidents, tasks, dictionary_type, queue_type);
                auto start = std::chrono::steady_clock::now();
                prioritizer.prioritize();
                mutable_array_sequence<PrioritizedTask> order;
                while (!prioritizer.empty()) order.append(prioritizer.pop());
                double time = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
                auto priorities = prioritizer.get_tag_priorities();
                mutable_array_sequence<DictionaryPair<std::string, int>> weights;
                for (int i = 0; i < incidents.get_length(); ++i) {
                    for (int j = 0; j < incidents[i].tags.get_length(); ++j) {
                        const std::string& tag = incidents[i].tags[j];
                        if (priorities.contains_key(tag)) {
                            weights.append(DictionaryPair<std::string, int>(tag, priorities.get(tag), default_compare<std::string>));
                            priorities.remove(tag);
                        }
                    }
                }
                QMetaObject::invokeMethod(this, [this, order, weights, time]() {
                    priorities_table->setRowCount(weights.get_length());
                    for (int i = 0; i < weights.get_length(); ++i) {
                        set_cell(priorities_table, i, 0, QString::fromStdString(weights[i].key));
                        set_cell(priorities_table, i, 1, QString::number(weights[i].value));
                    }
                    result_table->setRowCount(order.get_length());
                    for (int i = 0; i < order.get_length(); ++i) {
                        const PrioritizedTask& task = order[i];
                        set_cell(result_table, i, 0, QString::number(i + 1));
                        set_cell(result_table, i, 1, QString::fromStdString(task.name));
                        set_cell(result_table, i, 2, QString::number(order[i].priority));
                        set_cell(result_table, i, 3, tags_text(task.tags));
                    }
                    result_summary->setText(QString("Заданий: %1   •   Тегов: %2   •   Время расчёта: %3 мс")
                        .arg(order.get_length()).arg(weights.get_length()).arg(time, 0, 'f', 3));
                    statusBar()->showMessage("Приоритеты рассчитаны.");
                }, Qt::QueuedConnection);
            } catch (const std::exception& error) {
                QString message = QString::fromUtf8(error.what());
                QMetaObject::invokeMethod(this, [this, message]() { show_error(message); }, Qt::QueuedConnection);
            }
        });
        watch_worker();
    } catch (const std::exception& error) { show_error(QString::fromUtf8(error.what())); }
}

void MainWindow::append_measurement(const BenchmarkResult& result) {
    measurements.append(result);
    int row = benchmark_table->rowCount();
    benchmark_table->insertRow(row);
    set_cell(benchmark_table, row, 0, storage_name(result.dictionary_type));
    set_cell(benchmark_table, row, 1, storage_name(result.queue_type));
    set_cell(benchmark_table, row, 2, QString::number(result.count_time, 'f', 3));
    set_cell(benchmark_table, row, 3, QString::number(result.queue_time, 'f', 3));
    set_cell(benchmark_table, row, 4, QString::number(result.extract_time, 'f', 3));
    set_cell(benchmark_table, row, 5, QString::number(result.total_time(), 'f', 3));
    for (int column = 2; column < benchmark_table->columnCount(); ++column)
        benchmark_table->item(row, column)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    progress->setValue(row + 1);
    if (row == 8) {
        int best = 0;
        for (int i = 1; i < measurements.get_length(); ++i) {
            if (measurements[i].total_time() < measurements[best].total_time()) best = i;
        }
        for (int column = 0; column < benchmark_table->columnCount(); ++column) {
            benchmark_table->item(best, column)->setBackground(QColor("#e4f3e9"));
            QFont font = benchmark_table->item(best, column)->font();
            font.setBold(true);
            benchmark_table->item(best, column)->setFont(font);
        }
        benchmark_summary->setText(QString("Самое быстрое сочетание: %1 / %2 (%3 мс).")
            .arg(storage_name(measurements[best].dictionary_type)).arg(storage_name(measurements[best].queue_type))
            .arg(measurements[best].total_time(), 0, 'f', 3));
        statusBar()->showMessage("Сравнение завершено.");
    }
    else benchmark_summary->setText(QString("Измерено сочетаний: %1 из 9").arg(row + 1));
}

void MainWindow::run_benchmarks() {
    if (worker) return;
    measured_count = benchmark_count->value();
    measured_tag_count = benchmark_tag_count->value();
    int count = measured_count;
    int tag_count = measured_tag_count;
    measurements = mutable_array_sequence<BenchmarkResult>();
    benchmark_table->setRowCount(0);
    benchmark_summary->setText("Генерация данных…");
    statusBar()->showMessage("Сравнение скорости…");
    progress->setRange(0, 9);
    progress->setValue(0);
    worker = QThread::create([this, count, tag_count]() {
        try {
            mutable_array_sequence<Incident> incidents;
            mutable_array_sequence<Task> tasks;
            generate_data(incidents, tasks, count, tag_count, 42);
            for (StorageType dictionary_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
                for (StorageType queue_type : {StorageType::Heap, StorageType::BinarySearchTree, StorageType::AVL}) {
                    if (QThread::currentThread()->isInterruptionRequested()) return;
                    BenchmarkResult result = measure_prioritization(incidents, tasks, dictionary_type, queue_type);
                    QMetaObject::invokeMethod(this, [this, result]() { append_measurement(result); }, Qt::QueuedConnection);
                }
            }
        } catch (const std::exception& error) {
            QString message = QString::fromUtf8(error.what());
            QMetaObject::invokeMethod(this, [this, message]() { show_error(message); }, Qt::QueuedConnection);
        }
    });
    watch_worker();
}

void MainWindow::save_results() {
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    dialog.setWindowTitle("Сохранить результаты");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("CSV (*.csv)");
    dialog.setDefaultSuffix("csv");
    dialog.setDirectory(QDir::homePath());
    dialog.selectFile("prioritization.csv");
    dialog.setLabelText(QFileDialog::Accept, "Сохранить");
    dialog.setLabelText(QFileDialog::Reject, "Отмена");
    if (dialog.exec() != QDialog::Accepted) return;
    QString path = dialog.selectedFiles().first();
    try {
        save_benchmarks(measurements, measured_count, measured_tag_count, path.toStdString());
        statusBar()->showMessage("Результаты сохранены: " + path);
    } catch (const std::exception& error) { show_error(QString::fromUtf8(error.what())); }
}

void MainWindow::show_error(const QString& message) {
    result_summary->setText("Расчёт не выполнен.");
    if (measurements.get_length() != 9) benchmark_summary->setText("Сравнение не завершено.");
    statusBar()->showMessage(message);
    QMessageBox::warning(this, "Ошибка", message);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (worker) {
        pending_close = true;
        worker->requestInterruption();
        statusBar()->showMessage("Завершение текущего расчёта…");
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}
