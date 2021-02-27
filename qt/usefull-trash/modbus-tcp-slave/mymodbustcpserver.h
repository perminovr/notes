#ifndef MYMODBUSTCPSERVER_H
#define MYMODBUSTCPSERVER_H

#include <QModbusTcpServer>

typedef QMultiMap <QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMultiMap;

class MyModbusTcpServer : public QModbusTcpServer
{
    Q_OBJECT

public:
    MyModbusTcpServer(QObject *parent = nullptr);
    virtual ~MyModbusTcpServer() = default;

    virtual bool setMap(const QModbusDataUnitMultiMap &map);

protected:
    virtual bool writeData(const QModbusDataUnit &unit) override;
    virtual bool readData(QModbusDataUnit *newData) const override;

private:
    using QModbusTcpServer::setMap;
    QModbusDataUnitMultiMap m_modbusDataUnitMultiMap;

};

#endif // MYMODBUSTCPSERVER_H
