#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickView>
#include <QQuickWindow>
#include <QQuickItem>
#include <QtQuickWidgets/QQuickWidget>

#include "uibackend.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_horizontalSlider_valueChanged(int value);

    void on_label_linkActivated(const QString &link);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;
    int cnt;

    QQuickWidget *quick;
    QQmlApplicationEngine *engine;
    UIBackEnd *uiBackEnd;
};
#endif // MAINWINDOW_H
