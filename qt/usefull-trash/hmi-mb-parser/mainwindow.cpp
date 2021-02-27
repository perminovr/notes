#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFile>


enum ElementID {
    unknown,
    mbconfig,
    mbobject,
    address,
    len,
    form,
    objects,
    count,
    opt,
};



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    reader = new QXmlStreamReader;
}

MainWindow::~MainWindow()
{
    delete ui;
}


ElementID GetElementID(const QStringRef &name)
{
    if (name == "mbconfig")
        return ElementID::mbconfig;
    if (name == "mbobject")
        return ElementID::mbobject;
    if (name == "address")
        return ElementID::address;
    if (name == "len")
        return ElementID::len;
    if (name == "form")
        return ElementID::form;
    if (name == "objects")
        return ElementID::objects;
    if (name == "count")
        return ElementID::count;
    if (name == "opt")
        return ElementID::opt;
    return unknown;
}


int GetAttr(QXmlStreamReader *reader, const QString &attrName, QString &value)
{
    QXmlStreamAttributes attr = reader->attributes();
    if (attr.hasAttribute(attrName)) {
        value = attr.value(attrName).toString();
        return 0;
    } else {
        qCritical() << reader->name() << " " << attrName << " not found. stop" << endl;
        return 1;
    }
}



void MainWindow::on_pushButton_clicked()
{
    int error = 0;
    QXmlStreamAttributes attr;
    QString attrValue;
    QXmlStreamReader::TokenType token;
    QFile* file = new QFile("../../hmi-mb-configurator/mbconfig.xml");

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "couldn't open file" << endl;
        return;
    }
    reader->setDevice(file);

    // mbconfig
    token = reader->readNext();
    if (token != QXmlStreamReader::StartElement)
        return;
    if (GetElementID(reader->name()) != ElementID::mbconfig)
        return;
    if (GetAttr(reader, "version", attrValue) != 0)
        return;
    int v1,v2,v3,v4;
    if (sscanf(attrValue.toStdString().c_str(), "%d.%d.%d.%d", &v1,&v2,&v3,&v4) != 4)
        return;
    // todo vesion control

    // form / mbobject
    token = reader->readNext();
    if (token != QXmlStreamReader::StartElement)
        return;
    while (!reader->atEnd() && !reader->hasError() && !error) {

    }


    return;

    int error = 0;
    while (!reader->atEnd() && !reader->hasError() && !error) {
        token = reader->readNext();
        if (token == QXmlStreamReader::StartDocument)
            continue;
        if (token == QXmlStreamReader::StartElement) {
            qDebug() << reader->name() << endl;
            switch ( GetElementID(reader->name()) ) {
                case ElementID::mbconfig: {
                    error = GetAttr(reader, "version", attrValue);
                } break;
                case ElementID::mbobject: {
                    error = GetAttr(reader, "name", attrValue);
                } break;
                case ElementID::address: {

                } break;
                case ElementID::len: {

                } break;
                case ElementID::form: {

                } break;
                case ElementID::objects: {

                } break;
                case ElementID::count: {

                } break;
                case ElementID::opt: {

                } break;
                default: {

                } break;
            }
        }
    }
}
