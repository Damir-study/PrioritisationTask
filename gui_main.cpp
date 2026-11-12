#include <QApplication>
#include "include/UI/MainWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Приоритезация задач");
    app.setStyle("Fusion");
    MainWindow window;
    window.show();
    return app.exec();
}
