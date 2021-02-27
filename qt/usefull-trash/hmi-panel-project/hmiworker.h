#ifndef HMIWORKER_H
#define HMIWORKER_H

#include "multimodbustcpserver.h"
#include <QObject>

#include "servicepipeprovider.h"
#include "hmiconfig.h"
#include "uimodbusprovider.h"


class HMIWorker : public QObject
{
    Q_OBJECT
public:
    HMIWorker();
    ~HMIWorker();

public slots:
    void start();

signals:
    void finished();

private slots:    
    void dataWritten(QModbusDataUnit::RegisterType , int , int );
    void stateChanged(QModbusDevice::State );
    void errorOccurred(QModbusDevice::Error );

    void serviceDataIsReady(const QByteArray &);

    void newItem(RegItem *);
    void registrationEnd();
    void valueChanged(RegItem *, const QVariant &);

    void setDefaultScreenIfNull(QObject *object, const QUrl &url);

private:
    HMIConfig config;
    ServicePipeProvider *servicePipe;

    MultiModbusTCPServer *mbDev;

    UIModbusProvider *uiMbProvider;
    QQmlApplicationEngine *engine;

};

#endif // HMIWORKER_H
