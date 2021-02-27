#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mbObjWiz = new MbObjectWizard;

    connect(mbObjWiz, &MbObjectWizard::accepted, this, &MainWindow::NewMbObject);
}



void MainWindow::NewMbObject(const MbObject &object)
{

}



MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    mbObjWiz->show();
}
