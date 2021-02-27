#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qmbobject.h"
#include "mbconfig.h"

#include <QMainWindow>
#include <mymodbustcpserver.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	QList <QMbObject*> &GetMbList();

public slots:
	void dataWritten(QModbusDataUnit::RegisterType table, int address, int size);
	void stateChanged(QModbusDevice::State state);
	void errorOccurred(QModbusDevice::Error error);

	// qobjects
	void voidSlot();
	void intSlot(int);
	void qStringSlot(const QString &);

private:
	Ui::MainWindow *ui;

    MyModbusTcpServer *mbDev;

	QVector <QWidget*> *mbUiList;
	QList <QMbObject*> *mbObjectsList;

	MbConfig::Config *config;

	QMbObject *FindMbObject(QObject *qobject);
	void mbVoidHandler(QMbObject *obj, MbField *field);
	void mbIntHandler(QMbObject *obj, MbField *field, int);
	void mbQStringHandler(QMbObject *obj, MbField *field, const QString &);
};

#endif // MAINWINDOW_H
