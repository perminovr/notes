#ifndef QWDATABASE_H
#define QWDATABASE_H

#include <QObject>
#include <QByteArray>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLinkedList>
#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QSettings>
#include <QDataStream>
#include <QMap>
#include <QDebug>

#include "common.h"


static unsigned constexpr const_hash(const char *s, int idx = 0) {
	return (*s)? static_cast<unsigned int>(*s) + 33 * const_hash(s+1) + idx : 5381;
}

#define DEVICE_SETTER(name)\
	int id = t.getM(id);\
	auto it = this->m_##name.find(id);\
	if (it != this->m_##name.end()) {\
		if (*it == t) { return; } else { *it = t; } \
	} else { this->m_##name.insert(id, t); }

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(const type &t) { if (!(this->m_##name == t)) { this->m_##name = t; this->handleChanges( name##_toBytes() ); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	protected: void setter##Private(const type &t) { if (!(this->m_##name == t)) { this->m_##name = t; emit this->notifier(); } } \
	protected: QByteArray name##_toBytes() const; \
	protected: static constexpr unsigned name##_hash = const_hash( #name); \
	protected: type m_##name; \

#undef Q_PROPERTY_IMPLEMENTATION2
#define Q_PROPERTY_IMPLEMENTATION2(ktype, type, name, getter, setter, notifier) \
	public slots: void setter(const type &t) { DEVICE_SETTER(name); this->handleChanges( name##_toBytes(id) ); } \
	public: type getter(const ktype &t) const { auto it = this->m_##name.find(t); return (it != this->m_##name.end())? *it : type(); } \
	Q_SIGNAL void notifier(const ktype &t); \
	protected: void setter##Private(const type &t) { DEVICE_SETTER(name); emit this->name##Changed(id); } \
	protected: QByteArray name##_toBytes(const ktype &t) const; \
	protected: static constexpr unsigned name##_hash = const_hash( #name); \
	protected: QMap<ktype, type> m_##name; \


class QWDataBase : public QObject
{
public /*typedef*/:
	struct DeviceNetParams {
		int ipMode;
		QHostAddress vpnAddr;
		QHostAddress gw;
		QNetworkAddressEntry localAddr;
		//
		DeviceNetParams() { ipMode = 0; }
		DeviceNetParams(const DeviceNetParams &prm) { *this = prm; }
		~DeviceNetParams() = default;
		void operator=(const DeviceNetParams &prm) {
			localAddr.setIp(prm.localAddr.ip());
			localAddr.setNetmask(prm.localAddr.netmask());
			vpnAddr = prm.vpnAddr;
			gw = prm.gw;
			ipMode = prm.ipMode;
		}
		bool operator==(const DeviceNetParams &prm) {
			return vpnAddr == prm.vpnAddr && gw == prm.gw && localAddr == prm.localAddr && ipMode == prm.ipMode;
		}
	};
	struct ServerAddr {
		enum Slot { Vpn, Device };
		Slot slot;
		int port;
		//
		ServerAddr() { slot = Slot::Vpn; port = 0; }
		ServerAddr(const ServerAddr &s) { *this = s; }
		~ServerAddr() = default;
		void operator=(const ServerAddr &s) { slot = s.slot; port = s.port; }
		bool operator==(const ServerAddr &s) { return slot == s.slot && port == s.port; }
	};

private:
	Q_OBJECT
	Q_PROPERTY(bool vpnEnabled READ vpnEnabled WRITE setVpnEnabled NOTIFY vpnEnabledChanged)
	Q_PROPERTY(QString vpnFileName READ vpnFileName WRITE setVpnFileName NOTIFY vpnFileNameChanged)
	Q_PROPERTY(QWDataBase::DeviceNetParams netParam READ netParam WRITE setNetParam NOTIFY netParamChanged)
	Q_PROPERTY(QWDataBase::ServerAddr serverAddr READ serverAddr WRITE setServerAddr NOTIFY serverAddrChanged)
	Q_PROPERTY(ServStruct::RadioParams radioPrm READ radioPrm WRITE setRadioPrm NOTIFY radioPrmChanged)
	// Q_PROPERTY(DevStruct::DeviceStat devicesStat READ devicesStat WRITE setDevicesStat NOTIFY devicesStatChanged)
	// Q_PROPERTY(DevStruct::DeviceCtl devicesCtl READ devicesCtl WRITE setDevicesCtl NOTIFY devicesCtlChanged)

	Q_PROPERTY_IMPLEMENTATION(bool , vpnEnabled , vpnEnabled , setVpnEnabled , vpnEnabledChanged)
	Q_PROPERTY_IMPLEMENTATION(QString , vpnFileName , vpnFileName , setVpnFileName , vpnFileNameChanged)
	Q_PROPERTY_IMPLEMENTATION(QWDataBase::DeviceNetParams , netParam , netParam , setNetParam , netParamChanged)
	Q_PROPERTY_IMPLEMENTATION(QWDataBase::ServerAddr , serverAddr , serverAddr , setServerAddr , serverAddrChanged)
	Q_PROPERTY_IMPLEMENTATION(ServStruct::RadioParams , radioPrm , radioPrm , setRadioPrm , radioPrmChanged)
	Q_PROPERTY_IMPLEMENTATION2(int , DevStruct::DeviceStat , devicesStat , devicesStat , setDevicesStat , devicesStatChanged)
	Q_PROPERTY_IMPLEMENTATION2(int , DevStruct::DeviceCtl , devicesCtl , devicesCtl , setDevicesCtl , devicesCtlChanged)

public:
	QWDataBase(QObject *parent = nullptr) : QObject(parent) {
		qRegisterMetaType<QWDataBase::DeviceNetParams>("QWDataBase::DeviceNetParams");
		qRegisterMetaType<QWDataBase::ServerAddr>("QWDataBase::ServerAddr");
		qRegisterMetaType<ServStruct::RadioParams>("ServStruct::RadioParams");
		qRegisterMetaType<DevStruct::DeviceStat>("DevStruct::DeviceStat");
		qRegisterMetaType<DevStruct::DeviceCtl>("DevStruct::DeviceCtl");
	}
	virtual ~QWDataBase() {}

signals:
	void smthChanged();

protected:
	QString m_ethName;

	virtual void handleChanges(const QByteArray &data) = 0;
	void fromBytes(const QByteArray &data);
};


class QtDataBase : public QWDataBase
{
	Q_OBJECT
public:
	QtDataBase(QObject *parent = nullptr);
	virtual ~QtDataBase();

protected:
	virtual void handleChanges(const QByteArray &data) override;
	void fromBytes(const QByteArray &data, QLocalSocket *client);

signals:
	void devicesListRequested();

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

	// safe calls from wt
	void requestAll() { emit p_requestAll(); }
	void requestDevicesList() { emit p_requestDevicesList(); }
	void requestStat(int id) { emit p_requestStat(id); }
	void requestSubscrbe(int id, bool subscribe) { emit p_requestSubscrbe(id, subscribe); }

signals:
	void p_requestAll();
	void p_requestDevicesList();
	void p_requestStat(int id);
	void p_requestSubscrbe(int id, bool subscribe);

private slots:
	void pp_requestAll() const;
	void pp_requestDevicesList() const;
	void p_requestStat(int id) const;
	void p_requestSubscrbe(int id, bool subscribe) const;

protected:
	virtual void handleChanges(const QByteArray &data) override;

private:
	QLocalSocket *client;
};

#endif /* QWDATABASE_H */
