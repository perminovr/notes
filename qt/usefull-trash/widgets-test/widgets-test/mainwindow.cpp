#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QResource>
#include <QQmlApplicationEngine>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    quick = 0;
    engine = 0;
    uiBackEnd = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    qInfo() << "on_horizontalSlider_valueChanged " << value << endl;
}

void MainWindow::on_label_linkActivated(const QString &link)
{
    qInfo() << "on_label_linkActivated " << link << endl;

}

void MainWindow::on_pushButton_clicked()
{
    /* use if qml root != Window (styles don't supported) */
//    if (quick)
//        quick->close();
//    delete quick;
//    quick = new QQuickWidget();
//    quick->setSource(QUrl(QStringLiteral("SecondWindowForm.ui.qml")));
//    quick->show();


    delete engine;
    engine = new QQmlApplicationEngine();

    delete uiBackEnd;
    uiBackEnd = new UIBackEnd();

    engine->rootContext()->setContextProperty("uiBackEnd", uiBackEnd);
    engine->load(QUrl(QStringLiteral("SecondWindow.qml")));
}

void MainWindow::on_pushButton_2_clicked()
{
    delete engine;
    engine = new QQmlApplicationEngine();
    engine->load(QUrl(QStringLiteral("main.qml")));
}

void MainWindow::on_pushButton_3_clicked()
{
    if (uiBackEnd) {
        auto list = uiBackEnd->items();
        qInfo () << "list size: " << list->size() << endl;

        if (list->size()) {
            auto qit = uiBackEnd->lastItem();
            auto item = qit->item();
            auto prop = qit->property();

            item->setProperty(prop.toStdString().c_str(), "123");
        }
    }
}
