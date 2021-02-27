#include "hmiworker.h"
#include "hmivariant.h"

#include <QQuickWindow>
#include <QDebug>
#include <QUrl>



void HMIWorker::newItem(RegItem *item)
{
    if (item) {
        qDebug() << "newItem " << item->type() << item->address() << item->length() << item->property() << endl;
		// server is busy while updating @ref registrationEnd
        mbDev->setValue(QModbusServer::DeviceBusy, 1);
        QModbusDataUnit::RegisterType regType = (item->type() == RegItem_ns::ReadOnly)?
                QModbusDataUnit::InputRegisters : QModbusDataUnit::HoldingRegisters;
        mbDev->insertRegister(regType, item->address(), item->length());
    } else {
        qWarning() << "newItem is empty" << endl;
    }
}


void HMIWorker::valueChanged(RegItem *item, const QVariant &val)
{
    if (item) {
        qDebug() << "valueChanged " << item->address() << val << endl;
        HMIVariant::Array array;
        bool ok = HMIVariant::QVariantToArray(val, array, item->length());
        if (ok) {
            QModbusDataUnit::RegisterType regType = (item->type() == RegItem_ns::ReadOnly)?
                    QModbusDataUnit::InputRegisters : QModbusDataUnit::HoldingRegisters;
            ok = mbDev->setData( {regType, item->address(), array}, false );
        }
    }
}


void HMIWorker::registrationEnd()
{
    qDebug() << "registrationEnd" << endl;

    // update all registers
    for (auto &x : *uiMbProvider->getItems()) {
        QObject *obj = x->item();
        if (obj) {
            valueChanged(x, obj->property(x->property().toStdString().c_str()));
        }
    }
	// config update end
    mbDev->setValue(QModbusServer::DeviceBusy, 0);

	// run server
    switch (mbDev->state()) {
        case QModbusDevice::UnconnectedState:
        case QModbusDevice::ClosingState: {
            mbDev->setConnectionParameter(QModbusDevice::NetworkPortParameter, (int)config.mbConfig.port);
            mbDev->setConnectionParameter(QModbusDevice::NetworkAddressParameter, config.mbConfig.ip);
            mbDev->setServerAddress(config.mbConfig.addr);
            mbDev->connectDevice();
        } break;
        default: { /* NOP */ } break;
    }
}


void HMIWorker::setDefaultScreenIfNull(QObject *object, const QUrl &url)
{
    Q_UNUSED(url);

    if (!object) {
        // start default window
        engine->load(QUrl(CONFIG_DEFAULT_QML_FILE_NAME));
    }
}


void HMIWorker::start()
{
    qDebug() << "start (reload)" << endl;

	// read config
    QFile file(CONFIG_UCONF_FILE_NAME);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    bool ok = config.readFromFile(&file) == 0;
    file.close();

    // todo config version control

    // check user screen
    bool defaultScreen = true;
    if (ok && config.startScreen.size() != 0) {
        QFile file(config.startScreen);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (file.isOpen()) {
            file.close();
            defaultScreen = false;
        }
    }

    // create UI engine
    if (engine) {
        disconnect(engine, &QQmlApplicationEngine::objectCreated, this, &HMIWorker::setDefaultScreenIfNull);
    }

    delete engine;
    engine = new QQmlApplicationEngine(this);

    connect(engine, &QQmlApplicationEngine::objectCreated, this, &HMIWorker::setDefaultScreenIfNull);

    // choose screen
    if (defaultScreen) {
        setDefaultScreenIfNull(0, {});
    } else {
        // setup modbus if user UI
        if (uiMbProvider) {
            disconnect(uiMbProvider, &UIModbusProvider::newItem, this, &HMIWorker::newItem);
            disconnect(uiMbProvider, &UIModbusProvider::valueChanged, this, &HMIWorker::valueChanged);
        }

        delete uiMbProvider;
        uiMbProvider = new UIModbusProvider(this);

        connect(uiMbProvider, &UIModbusProvider::newItem, this, &HMIWorker::newItem);
        connect(uiMbProvider, &UIModbusProvider::valueChanged, this, &HMIWorker::valueChanged);
        connect(uiMbProvider, &UIModbusProvider::registrationEnd, this, &HMIWorker::registrationEnd);

        switch (mbDev->state()) {
            case QModbusDevice::ConnectingState:
            case QModbusDevice::ConnectedState: {
                mbDev->disconnectDevice();
            } break;
            default: { /* NOP */ } break;
        }
        mbDev->clearRegisterMap();

        engine->rootContext()->setContextProperty("uiMbProvider", uiMbProvider);
        engine->load(QUrl(config.startScreen));
    }
}


void HMIWorker::serviceDataIsReady(const QByteArray &data)
{
    qDebug() << "serviceDataIsReady: " << data.toStdString().c_str() << endl;
	start(); // todo 
}


void HMIWorker::dataWritten(QModbusDataUnit::RegisterType regType, int address, int size)
{
    qDebug() << "dataWritten " << regType << address << size << endl;

    RegItem_ns::RegItemT type = (regType == QModbusDataUnit::InputRegisters)?
            RegItem_ns::ReadOnly : RegItem_ns::ReadWrite;
    auto map = uiMbProvider->getItems();

    int endAddr = address + size - 1;
    int curAddr = address;
    while (curAddr <= endAddr) {
        auto mi = map->find( {type, (quint16)curAddr, 1} );
        if (mi != map->end()) {
            bool ok;
            RegItem *regItem = mi.value();
            QModbusDataUnit unit {regType, regItem->address(), regItem->length()};
            ok = mbDev->data(&unit);
            if (ok) {
                QObject *item = regItem->item();
                if (item) {
                    const char *prop = regItem->property().toStdString().c_str();
                    QVariant var = item->property(prop);
                    ok = HMIVariant::ArrayToQVariant(unit.values(), var);
                    if (ok) {
                        item->setProperty(prop, var);
                    }
                }
            }
            curAddr += regItem->length();
        } else {
            break;
        }
    }
}


void HMIWorker::stateChanged(QModbusDevice::State state)
{
    qDebug() << "stateChanged " << state << endl;
}


void HMIWorker::errorOccurred(QModbusDevice::Error error)
{
    qDebug() << "errorOccurred " << error << endl;
}


HMIWorker::HMIWorker()
{
    engine = nullptr;
    uiMbProvider = nullptr;
    mbDev = new MultiModbusTCPServer(this);
	servicePipe = new ServicePipeProvider(CONFIG_SERV_SOCK_NAME, this);

    if (!servicePipe->start())
        qFatal("Couldn't start server %s", CONFIG_SERV_SOCK_NAME);

    UIModbusProvider::qmlRegister();

	connect(servicePipe, &ServicePipeProvider::dataIsReady, this, &HMIWorker::serviceDataIsReady);

    connect(mbDev, &QModbusServer::dataWritten, this, &HMIWorker::dataWritten);
    connect(mbDev, &QModbusServer::stateChanged, this, &HMIWorker::stateChanged);
    connect(mbDev, &QModbusServer::errorOccurred, this, &HMIWorker::errorOccurred);
}


HMIWorker::~HMIWorker()
{
    if (mbDev) {
        mbDev->disconnectDevice();
    }
	delete servicePipe;
    delete mbDev;
    delete uiMbProvider;
    delete engine;
}
