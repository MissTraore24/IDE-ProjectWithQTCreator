#include "idewindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    IDEWindow w;
    w.show();
    return a.exec();
}
