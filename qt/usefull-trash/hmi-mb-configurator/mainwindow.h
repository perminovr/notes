#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mbobjectwizard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void NewMbObject(const MbObject &object);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    MbObjectWizard *mbObjWiz;
};
#endif // MAINWINDOW_H
