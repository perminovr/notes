#ifndef QWDATABASE_H
#define QWDATABASE_H

#include <QObject>
#include <QByteArray>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLinkedList>
#include <QNetworkAddressEntry>
#include <QSettings>

#include "rfjammersctl.h"
#define QWDATABASE_DEVICES_CNT RFJAMMERS_DEVICES_CNT


static unsigned constexpr const_hash(const char *s, int idx = 0) {
	return (*s)? static_cast<unsigned int>(*s) + 33 * const_hash(s+1) + idx : 5381;
}

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (!(this->m_##name == t)) { this->m_##name = t; this->handleChanges( name##_hash ); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	protected: void setter##Private(type t) { if (!(this->m_##name == t)) { this->m_##name = t; emit this->notifier(); } } \
	protected: static constexpr unsigned name##_hash = const_hash( #name); \
	protected: type m_##name;

#undef Q_PROPERTY_IMPLEMENTATION2
#define Q_PROPERTY_IMPLEMENTATION2(type, name, getter, setter, notifier, size) \
	public slots: void setter(type t, int idx) { if (!(this->m_##name[idx] == t)) { this->m_##name[idx] = t; this->handleChanges( name##_hash[idx] ); } } \
	public: type getter(int idx) const { return this->m_##name[idx]; } \
	Q_SIGNAL void notifier(int idx); \
	protected: void setter##Private(type t, int idx) { if (!(this->m_##name[idx] == t)) { this->m_##name[idx] = t; emit this->notifier(idx); } } \
	protected: static constexpr ConstHashesArray_##name name##_hash = ConstHashesArray_##name(); \
	protected: type m_##name[size];

#define CONST_HASHES_ARRAY_CLASS(pfx, sz) \
class ConstHashesArray_##pfx {\
public:\
	constexpr ConstHashesArray_##pfx() : data() {\
		for(int i = 0; i < sz; ++i) { data[i] = const_hash( #pfx, i); }\
	}\
	constexpr unsigned operator[](int i) const { return data[i]; }\
private: unsigned data[sz];\
};

CONST_HASHES_ARRAY_CLASS(devicesStat, QWDATABASE_DEVICES_CNT)
CONST_HASHES_ARRAY_CLASS(devicesCtl, QWDATABASE_DEVICES_CNT)


class QWDataBase : public QObject
{
public /*typedef*/:
	struct DeviceNetParams {
		int ipMode;
		QNetworkAddressEntry localAddr;
		//
		DeviceNetParams() { ipMode = 0; }
		DeviceNetParams(const DeviceNetParams &prm) { *this = prm; }
		~DeviceNetParams() = default;
		void operator=(const DeviceNetParams &prm) {
			localAddr.setIp(prm.localAddr.ip());
			localAddr.setNetmask(prm.localAddr.netmask());
			ipMode = prm.ipMode;
		}
		bool operator==(const DeviceNetParams &prm) { return localAddr == prm.localAddr && ipMode == prm.ipMode; }
	};
	struct SlotIPv4 {
		QHostAddress ip;
		quint16 port;
		//
		SlotIPv4() { port = 0; }
		SlotIPv4(const SlotIPv4 &slot) { *this = slot; }
		~SlotIPv4() = default;
		void operator=(const SlotIPv4 &slot) { ip = slot.ip; port = slot.port; }
		bool operator==(const SlotIPv4 &slot) { return ip == slot.ip && port == slot.port; }
	};

private:
	Q_OBJECT
	Q_PROPERTY(QWDataBase::DeviceNetParams netParam READ netParam WRITE setNetParam NOTIFY netParamChanged)
	Q_PROPERTY(QHostAddress gateway READ gateway WRITE setGateway NOTIFY gatewayChanged)
	Q_PROPERTY(QWDataBase::SlotIPv4 serverAddr READ serverAddr WRITE setServerAddr NOTIFY serverAddrChanged)
	Q_PROPERTY(ServStruct::RadioParams radioPrm READ radioPrm WRITE setRadioPrm NOTIFY radioPrmChanged)
	//Q_PROPERTY(DevStruct::DeviceStat devicesStat READ devicesStat WRITE setDevicesStat NOTIFY devicesStatChanged)
	//Q_PROPERTY(DevStruct::DeviceCtl devicesCtl READ devicesCtl WRITE setDevicesCtl NOTIFY devicesCtlChanged)

	Q_PROPERTY_IMPLEMENTATION(QWDataBase::DeviceNetParams , netParam , netParam , setNetParam , netParamChanged)
	Q_PROPERTY_IMPLEMENTATION(QHostAddress , gateway , gateway , setGateway , gatewayChanged)
	Q_PROPERTY_IMPLEMENTATION(QWDataBase::SlotIPv4 , serverAddr , serverAddr , setServerAddr , serverAddrChanged)
	Q_PROPERTY_IMPLEMENTATION(ServStruct::RadioParams , radioPrm , radioPrm , setRadioPrm , radioPrmChanged)
	Q_PROPERTY_IMPLEMENTATION2(DevStruct::DeviceStat , devicesStat , devicesStat , setDevicesStat , devicesStatChanged , QWDATABASE_DEVICES_CNT)
	Q_PROPERTY_IMPLEMENTATION2(DevStruct::DeviceCtl , devicesCtl , devicesCtl , setDevicesCtl , devicesCtlChanged , QWDATABASE_DEVICES_CNT)

public:
	QWDataBase(QObject *parent = nullptr) : QObject(parent) {
		qRegisterMetaType<ServStruct::RadioParams>("ServStruct::RadioParams");
		qRegisterMetaType<DevStruct::DeviceStat>("DevStruct::DeviceStat");
		qRegisterMetaType<DevStruct::DeviceCtl>("DevStruct::DeviceCtl");
		qRegisterMetaType<QWDataBase::DeviceNetParams>("QWDataBase::DeviceNetParams"); // todo
	}
	virtual ~QWDataBase() {}

signals:
	void smthChanged();

protected:
	QString m_ethName;

	virtual void handleChanges(unsigned hash) = 0;
	QByteArray toBytes(unsigned hash);
	void fromBytes(const QByteArray &array);
};


class QtDataBase : public QWDataBase
{
	Q_OBJECT
public:
	QtDataBase(QObject *parent = nullptr);
	virtual ~QtDataBase();

protected:
	virtual void handleChanges(unsigned hash) override;

private slots:
	void save();

private:
	QLocalServer *server;
	QLinkedList <QLocalSocket *> clients;
	QSettings *settings;

	void read();
	void readIfaces();
};

class WtDataBase : public QWDataBase
{
	Q_OBJECT
public:
	WtDataBase(QObject *parent = nullptr);
	virtual ~WtDataBase();

protected:
	virtual void handleChanges(unsigned hash) override;

private:
	QLocalSocket *client;
};

#endif /* QWDATABASE_H */
