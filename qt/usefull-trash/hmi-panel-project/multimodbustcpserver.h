#ifndef MULTIMODBUSTCPSERVER_H
#define MULTIMODBUSTCPSERVER_H

#include <map>
#include <QModbusTcpServer>


typedef std::multimap <QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMultiMap;

class MultiModbusTCPServer : public QModbusTcpServer
{
	Q_OBJECT

public:
	MultiModbusTCPServer(QObject *parent = nullptr);
	virtual ~MultiModbusTCPServer() = default;

    void clearRegisterMap();
    void insertRegister(QModbusDataUnit::RegisterType key, quint16 address, quint16 length);
    bool setData(const QModbusDataUnit &newData, bool notify = true);

protected:
    bool writeData(const QModbusDataUnit &unit, bool notify);
    virtual bool writeData(const QModbusDataUnit &unit) override;
	virtual bool readData(QModbusDataUnit *newData) const override;

private:
    using QModbusTcpServer::setMap;
	QModbusDataUnitMultiMap m_modbusDataUnitMultiMap;

};

#endif // MULTIMODBUSTCPSERVER_H
