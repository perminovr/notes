#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QUrl>



void MainWindow::dataWritten(QModbusDataUnit::RegisterType table, int address, int size)
{
	quint16 value;
	for (int i = 0; i < size; ++i) {
		mbDev->data(table, address + i, &value);
		qInfo() << "wr value = " << value << endl;
	}
}



void MainWindow::stateChanged(QModbusDevice::State state)
{
	bool connected = (state != QModbusDevice::UnconnectedState);
	qInfo() << "connected = " << connected << endl;
}



void MainWindow::errorOccurred(QModbusDevice::Error error)
{
	if (error == QModbusDevice::NoError || !mbDev)
		return;
	qInfo() << "error = " << error << " ("
			   << mbDev->errorString() << ")"
			   << endl;
}



MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	bool ok;
	ui->setupUi(this);

	// create
    mbDev = new MyModbusTcpServer(this);

	// setup modbus data
    QModbusDataUnitMultiMap reg;
	reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, 10 });
	reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 10 });
    reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 1000, 10 });
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 1000, 10 });
	mbDev->setMap(reg);

	// callbacks
	connect(mbDev, &QModbusServer::dataWritten, this, &MainWindow::dataWritten);
	connect(mbDev, &QModbusServer::stateChanged, this, &MainWindow::stateChanged);
	connect(mbDev, &QModbusServer::errorOccurred, this, &MainWindow::errorOccurred);

	// modbus options (statuses)
	/*
		DiagnosticRegister,
		ExceptionStatusOffset,
		DeviceBusy,
		AsciiInputDelimiter,
		ListenOnlyMode,
		ServerIdentifier,
		RunIndicatorStatus,
		AdditionalData,
		DeviceIdentification,
	*/
	mbDev->setValue(QModbusServer::DeviceBusy, 0);

	// setup data values
	ok = true;
//	for (int i = 0; i < 10; ++i) {
//		ok &= mbDev->setData(QModbusDataUnit::InputRegisters, i, /*value*/ i+100);
//		ok &= mbDev->setData(QModbusDataUnit::HoldingRegisters, i, /*value*/ i+200);
//	}
    ok &= mbDev->setData(QModbusDataUnit::InputRegisters, 0, 1);
    ok &= mbDev->setData(QModbusDataUnit::InputRegisters, 1000, 2);
    ok &= mbDev->setData(QModbusDataUnit::HoldingRegisters, 0, 3);
    ok &= mbDev->setData(QModbusDataUnit::HoldingRegisters, 1000, 4);
	if (!ok) {
		qInfo() << "not ok" << endl;
	}

	// connection
    const QUrl url = QUrl::fromUserInput("192.168.1.3:10020");
	mbDev->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
	mbDev->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
	mbDev->setServerAddress(1);
	mbDev->connectDevice();


       return;



	config = new MbConfig::Config();
	QFile* file = new QFile("../../hmi-mb-configurator/mbconfig.xml");
	file->open(QIODevice::ReadOnly | QIODevice::Text);
//	config->setFromFile(file);
	delete file;

	return;


	// Qobject with mb test
	{
		QMbObject *mbObj;
		MbField *mbField;

		mbUiList = new QVector <QWidget*>();
		mbObjectsList = new QList <QMbObject*>();

		// pushButton
		mbObj = new QMbObject;
		mbObj->self = ui->pushButton;
		mbField = new MbField({ // clicked
			QModbusDataUnit::InputRegisters, 1000, 1,
			MbField::QObjectSignalType::Void
		});
		mbObj->fieldList.push_back(mbField);
		mbObjectsList->push_back(mbObj);
		connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::voidSlot);

		// lineEdit
		mbObj = new QMbObject;
		mbObj->self = ui->lineEdit;
		mbField = new MbField({ // self
			QModbusDataUnit::HoldingRegisters, 1001, 1,
			MbField::QObjectSignalType::QString
		});
		mbObj->fieldList.push_back(mbField);
		mbField = new MbField({ // editingFinished
			QModbusDataUnit::InputRegisters, 1002, 1,
			MbField::QObjectSignalType::Void
		});
		mbObj->fieldList.push_back(mbField);
		mbObjectsList->push_back(mbObj);
		connect(ui->lineEdit, &QLineEdit::editingFinished, this, &MainWindow::voidSlot);

		// spinBox
		mbObj = new QMbObject;
		mbObj->self = ui->spinBox;
		mbField = new MbField({ // clicked
			QModbusDataUnit::InputRegisters, 1003, 1,
			MbField::QObjectSignalType::Int
		});
		mbObj->fieldList.push_back(mbField);
		mbObjectsList->push_back(mbObj);
		connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::intSlot);
	}
}



MainWindow::~MainWindow()
{
	if (mbDev)
		mbDev->disconnectDevice();
	delete mbDev;
	for (auto &x : *mbUiList) {
		delete x;
	}
	delete mbUiList;
	for (auto &x : *mbObjectsList) {
		for (auto &y : x->fieldList) {
			delete y;
		}
		delete x;
	}
	delete mbObjectsList;
	delete ui;
}















void MainWindow::mbVoidHandler(QMbObject *obj, MbField *field)
{
	QUniObject quo;
	quo.obj = obj->self;
	switch ( GetQObjectID(obj->self) )
	{
		case QObjectID::PushButton: {
			quo.pbtn->setText("clicked");
		} break;
		case QObjectID::LineEdit: {
		} break;
		default: {
		}
		break;
	}

}



void MainWindow::mbIntHandler(QMbObject *obj, MbField *field, int val)
{
	QUniObject quo;
	quo.obj = obj->self;
	switch ( GetQObjectID(obj->self) )
	{
		case QObjectID::SpinBox: {
			for (quint16 a = field->address; a < field->address + field->len; ++a)
				mbDev->setData(field->regType, a, val);
		} break;
		default: {
		}
		break;
	}
}



void MainWindow::mbQStringHandler(QMbObject *obj, MbField *field, const QString &val)
{

}



QMbObject *MainWindow::FindMbObject(QObject *qobject)
{
	for (auto &x : *mbObjectsList) {
		if (x->self == qobject)
			return x;
	}
	return 0;
}



void MainWindow::voidSlot()
{
	QMbObject *obj = FindMbObject(sender());
	if (obj) {
		for (auto &x : obj->fieldList) {
			if (x->sigType == MbField::QObjectSignalType::Void) {
				mbVoidHandler(obj, x);
			}
		}
	}
}



void MainWindow::intSlot(int val)
{
	QMbObject *obj = FindMbObject(sender());
	if (obj) {
		for (auto &x : obj->fieldList) {
			if (x->sigType == MbField::QObjectSignalType::Int) {
				mbIntHandler(obj, x, val);
			}
		}
	}
}



void MainWindow::qStringSlot(const QString &str)
{
	QMbObject *obj = FindMbObject(sender());
	if (obj) {
		for (auto &x : obj->fieldList) {
			if (x->sigType == MbField::QObjectSignalType::QString) {
				mbQStringHandler(obj, x, str);
			}
		}
	}
}
