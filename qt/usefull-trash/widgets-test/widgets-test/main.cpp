#include "mainwindow.h"
#include "uibackend.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    UIBackEnd::qmlRegister();

    return a.exec();
}
