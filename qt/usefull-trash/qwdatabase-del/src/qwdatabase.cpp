#include "common.h"
#include "qwdatabase.h"
#include <QVariant>
#include <QtCore>
#include <unistd.h>

enum QWDataBaseFType {
	Data, ReqAll, ReqDevList, ReqDevStat, ReqDevSub
};

#define readParamInit \
	QVariant v; \
	QString prm;

#define readParam(k, def, strg) {\
	v = (strg)->value(k, def);\
	if (v.type() == QVariant::String && v.toString().size())\
		{prm = v.toString();}\
	else\
		{prm = def;} \
}

#define VPN_SECTION "vpn/"
# define VPN_FNAME "fname"
# define VPN_EN "en"
#define DEVCMN_SECTION "devices/"
# define DEVCMN_CNT "cnt"
#define DEV_SECTION "device_%1/"
# define DEV_ID "id"
# define DEV_EN "en"
# define DEV_LVL "lvl"
#define RADIO_SECTION "radio/"
# define RADIO_TOA "toa"
# define RADIO_TOR "tor"
# define RADIO_SP "sp"
# define RADIO_PP "pp"
#define ETH_SECTION "eth/"
# define ETH_NAME "name"
# define ETH_MODE "mode"
# define ETH_IP "ip"
# define ETH_MASK "mask"
# define ETH_GW "gw"
#define SERV_SECTION "server/"
# define SERV_SLOT "slot"
# define SERV_PORT "port"


#define TO_PIPE_FTYPE_INIT(ft) \
	QByteArray ret;\
	QDataStream out(&ret, QIODevice::WriteOnly);\
	out.setVersion(QDataStream::Qt_5_3); \
	out << scast(int, ft);

#define TO_PIPE_FTYPE_PRERET() \
	out.device()->seek(0);\
	out << unsigned(ret.size()-sizeof(unsigned));

#define TO_BYTES_INIT(name) \
	TO_PIPE_FTYPE_INIT(QWDataBaseFType::Data) \
	out << name##_hash;

#define TO_BYTES_RET() \
	TO_PIPE_FTYPE_PRERET() \
	return ret;


void QWDataBase::fromBytes(const QByteArray &data)
{
	QDataStream in(data);
	in.setVersion(QDataStream::Qt_5_3);

	if (data.size() > scast(int, sizeof(unsigned))) {
		int itype;
		unsigned blockSize, hash;
		in >> blockSize >> itype >> hash;

		switch (hash) {
			case vpnEnabled_hash: {
				decltype(m_vpnEnabled) s;
				in >> s;
				setVpnEnabledPrivate(s);
			} break;
			case vpnFileName_hash: {
				decltype(m_vpnFileName) s;
				in >> s;
				m_vpnFileName = "reset"; // always reset on write -> always changed
				setVpnFileNamePrivate(s);
			} break;
			case netParam_hash: {
				decltype(m_netParam) s;
				QHostAddress ip, mask;
				in >> s.ipMode >> s.vpnAddr >> s.gw
					>> ip >> mask;
				s.localAddr.setIp(ip);
				s.localAddr.setNetmask(mask);
				setNetParamPrivate(s);
			} break;
			case serverAddr_hash: {
				decltype(m_serverAddr) s;
				int slot;
				in >> slot >> s.port;
				s.slot = scast(QWDataBase::ServerAddr::Slot, slot);
				setServerAddrPrivate(s);
			} break;
			case radioPrm_hash: {
				decltype(m_radioPrm) s;
				in >> s.timeoutAck >> s.timeoutRepeat
					>> s.slavePause >> s.pollPause;
				setRadioPrmPrivate(s);
			} break;
			case devicesStat_hash: {
				DevStruct::DeviceStat s;
				in >> s.getM(ch.signalLvl) >> s.getM(ch.devEn) >> s.getM(ch.T)
					>> s.getM(ch.reflectedPwr) >> s.getM(ch.outputRF)
					>> s.getM(aux.T) >> s.getM(aux.U) >> s.getM(aux.packetId)
					>> s.getM(id) >> s.getM(caseIsOpened);
				setDevicesStatPrivate(s);
			} break;
			case devicesCtl_hash: {
				DevStruct::DeviceCtl s;
				in >> s.getM(ch.signalLvl) >> s.getM(ch.devEn)
					>> s.getM(id) >> s.getM(packetId);
				setDevicesCtlPrivate(s);
			} break;
			case devicesInfo_hash: {
				ServStruct::DeviceInfo s;
				in >> s.getM(id) >> s.getM(connected) >> s.getM(subscribed);
				setDevicesInfoPrivate(s);
			} break;
			default: {} break;
		}
		emit smthChanged();
	}
}


void QtDataBase::fromBytes(const QByteArray &data, QLocalSocket *client)
{
	QDataStream in(data);
	in.setVersion(QDataStream::Qt_5_3);

	if (data.size() > scast(int, sizeof(unsigned))) {
		int itype;
		QWDataBaseFType ftype;
		unsigned blockSize;
		in >> blockSize >> itype;

		ftype = scast(QWDataBaseFType, itype);

		if (ftype == QWDataBaseFType::Data) {
			QWDataBase::fromBytes(data);
		} else {
			switch (ftype) {
				case QWDataBaseFType::ReqAll: {
					client->write( vpnEnabled_toBytes() );
					client->write( vpnFileName_toBytes() );
					client->write( netParam_toBytes() );
					client->write( serverAddr_toBytes() );
					client->write( radioPrm_toBytes() );
					for (auto &x : m_devicesStat) {
						int id = x.getM(id);
						client->write( devicesStat_toBytes(id) );
					}
					for (auto &x : m_devicesCtl) {
						int id = x.getM(id);
						client->write( devicesCtl_toBytes(id) );
					}
					for (auto &x : m_devicesInfo) {
						int id = x.getM(id);
						client->write( devicesInfo_toBytes(id) );
					}
				} break;
				case ReqDevList: {
					// todo
				} break;
				case ReqDevStat: {
					int id;
					in >> id;
					// todo
				} break;
				case ReqDevSub: {
					int id;
					bool subscribe;
					in >> id >> subscribe;
					// todo
				} break;
				default: {} break;
			}
		}
	}
}


QByteArray QWDataBase::vpnEnabled_toBytes() const
{
	TO_BYTES_INIT(vpnEnabled)
	out << m_vpnEnabled;
	TO_BYTES_RET()
}


QByteArray QWDataBase::vpnFileName_toBytes() const
{
	TO_BYTES_INIT(vpnFileName)
	out << m_vpnFileName;
	TO_BYTES_RET()
}


QByteArray QWDataBase::netParam_toBytes() const
{
	TO_BYTES_INIT(netParam)
	out << m_netParam.ipMode
		<< m_netParam.vpnAddr
		<< m_netParam.gw
		<< m_netParam.localAddr.ip()
		<< m_netParam.localAddr.netmask();
	TO_BYTES_RET()
}


QByteArray QWDataBase::serverAddr_toBytes() const
{
	TO_BYTES_INIT(serverAddr)
	out << scast(int, m_serverAddr.slot) << m_serverAddr.port;
	TO_BYTES_RET()
}


QByteArray QWDataBase::radioPrm_toBytes() const
{
	TO_BYTES_INIT(radioPrm)
	out << m_radioPrm.timeoutAck << m_radioPrm.timeoutRepeat
		<< m_radioPrm.slavePause << m_radioPrm.pollPause;
	TO_BYTES_RET()
}


QByteArray QWDataBase::devicesStat_toBytes(const int &t) const
{
	auto it = this->m_devicesStat.find(t);
	if (it != this->m_devicesStat.end()) {
		TO_BYTES_INIT(devicesStat)
		out << it->m.ch.signalLvl << it->m.ch.devEn << it->m.ch.T
			<< it->m.ch.reflectedPwr << it->m.ch.outputRF
			<< it->m.aux.T << it->m.aux.U << it->m.aux.packetId
			<< it->m.id << it->m.caseIsOpened;
		TO_BYTES_RET()
	}
	return QByteArray();
}


QByteArray QWDataBase::devicesCtl_toBytes(const int &t) const
{
	auto it = this->m_devicesCtl.find(t);
	if (it != this->m_devicesCtl.end()) {
		TO_BYTES_INIT(devicesCtl)
		out << it->m.ch.signalLvl << it->m.ch.devEn
			<< it->m.id << it->m.packetId;
		TO_BYTES_RET()
	}
	return QByteArray();
}


QByteArray QWDataBase::devicesInfo_toBytes(const int &t) const
{
	auto it = this->m_devicesInfo.find(t);
	if (it != this->m_devicesInfo.end()) {
		TO_BYTES_INIT(devicesInfo)
		out << it->m.id << it->m.connected << it->m.subscribed;
		TO_BYTES_RET()
	}
	return QByteArray();
}


void QtDataBase::readIfaces()
{
	readParamInit;

	readParam(ETH_SECTION ETH_NAME, "eth0", this->settings);
	this->m_ethName = prm;
}


void QtDataBase::read()
{
	readParamInit;

	// vpn

	readParam(VPN_SECTION VPN_FNAME, "", this->settings);
	this->m_vpnFileName = prm;

	readParam(VPN_SECTION VPN_EN, "0", this->settings);
	this->m_vpnEnabled = scast(bool, prm.toInt());

	// devices

	readParam(DEVCMN_SECTION DEVCMN_CNT, "0", this->settings);
	int sz = prm.toInt();

	for (int i = 0; i < sz; ++i) {
		QString pname;
		DevStruct::DeviceCtl ctl;

		pname = QString(DEV_SECTION DEV_ID).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(id) = scast(quint16, prm.toInt());

		pname = QString(DEV_SECTION DEV_EN).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(ch.devEn) = scast(bool, prm.toInt());

		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		readParam(pname, "0", this->settings);
		ctl.getM(ch.signalLvl) = scast(quint8, prm.toInt());

		this->m_devicesCtl.insert(ctl.getM(id), ctl);
	}

	// radio

	readParam(RADIO_SECTION RADIO_TOA, "200", this->settings);
	this->m_radioPrm.timeoutAck = prm.toInt();

	readParam(RADIO_SECTION RADIO_TOR, "1000", this->settings);
	this->m_radioPrm.timeoutRepeat = prm.toInt();

	readParam(RADIO_SECTION RADIO_SP, "0", this->settings);
	this->m_radioPrm.slavePause = prm.toInt();

	readParam(RADIO_SECTION RADIO_PP, "1000", this->settings);
	this->m_radioPrm.pollPause = prm.toInt();

	// eth

	readParam(ETH_SECTION ETH_MODE, "0", this->settings);
	this->m_netParam.ipMode = prm.toInt();

	readParam(ETH_SECTION ETH_IP, "192.168.1.150", this->settings);
	this->m_netParam.localAddr.setIp(QHostAddress(prm));

	readParam(ETH_SECTION ETH_MASK, "255.255.0.0", this->settings);
	this->m_netParam.localAddr.setNetmask(QHostAddress(prm));

	readParam(ETH_SECTION ETH_GW, "192.168.1.1", this->settings);
	this->m_netParam.gw = QHostAddress(prm);

	// server

	readParam(SERV_SECTION SERV_SLOT, "0", this->settings);
	this->m_serverAddr.slot = scast(ServerAddr::Slot, prm.toInt());

	readParam(SERV_SECTION SERV_PORT, "22000", this->settings);
	this->m_serverAddr.port = prm.toInt();

}


void QtDataBase::save()
{
	// vpn
	this->settings->setValue(VPN_SECTION VPN_FNAME, this->m_vpnFileName);
	this->settings->setValue(VPN_SECTION VPN_EN, scast(int, this->m_vpnEnabled));
	// devices
	int sz = this->m_devicesCtl.size();
	this->settings->setValue(DEVCMN_SECTION DEVCMN_CNT, sz);
	int i = 0;
	for (auto &x : this->m_devicesCtl) {
		QString pname;
		pname = QString(DEV_SECTION DEV_ID).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(id)));
		pname = QString(DEV_SECTION DEV_EN).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(ch.devEn)));
		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		this->settings->setValue(pname, scast(int, x.getM(ch.signalLvl)));
		i++;
	}
	// radio
	this->settings->setValue(RADIO_SECTION RADIO_TOA, this->m_radioPrm.timeoutAck);
	this->settings->setValue(RADIO_SECTION RADIO_TOR, this->m_radioPrm.timeoutRepeat);
	this->settings->setValue(RADIO_SECTION RADIO_SP, this->m_radioPrm.slavePause);
	this->settings->setValue(RADIO_SECTION RADIO_PP, this->m_radioPrm.pollPause);
	// eth
	this->settings->setValue(ETH_SECTION ETH_NAME, this->m_ethName);
	this->settings->setValue(ETH_SECTION ETH_MODE, this->m_netParam.ipMode);
	this->settings->setValue(ETH_SECTION ETH_IP, this->m_netParam.localAddr.ip().toString());
	this->settings->setValue(ETH_SECTION ETH_MASK, this->m_netParam.localAddr.netmask().toString());
	this->settings->setValue(ETH_SECTION ETH_GW, this->m_netParam.gw.toString());
	// server
	this->settings->setValue(SERV_SECTION SERV_SLOT, scast(int, this->m_serverAddr.slot));
	this->settings->setValue(SERV_SECTION SERV_PORT, this->m_serverAddr.port);

	this->settings->sync();
	sync();
}


void QtDataBase::handleChanges(const QByteArray &data)
{
	if (data.size()) {
		for (auto &x : clients) {
			x->write(data);
		}
	}
}


void WtDataBase::handleChanges(const QByteArray &data)
{
	if (data.size()) client->write(data);
}


void WtDataBase::pp_requestAll() const
{
	TO_PIPE_FTYPE_INIT(QWDataBaseFType::ReqAll)
	TO_PIPE_FTYPE_PRERET()
	client->write(ret);
}


void WtDataBase::pp_requestDevicesList() const
{
	TO_PIPE_FTYPE_INIT(QWDataBaseFType::ReqDevList)
	TO_PIPE_FTYPE_PRERET()
	client->write(ret);
}
void WtDataBase::p_requestStat(int id) const
{
	TO_PIPE_FTYPE_INIT(QWDataBaseFType::ReqDevStat)
	out << id;
	TO_PIPE_FTYPE_PRERET()
	client->write(ret);
}


void WtDataBase::p_requestSubscrbe(int id, bool subscribe) const
{
	TO_PIPE_FTYPE_INIT(QWDataBaseFType::ReqDevSub)
	out << id << subscribe;
	TO_PIPE_FTYPE_PRERET()
	client->write(ret);
}


QtDataBase::QtDataBase(QObject *parent) : QWDataBase(parent)
{
	server = new QLocalServer(this);
	QLocalServer::removeServer(QStringLiteral(CONFIG_QWDATABASE_PIPE_NAME));
	server->setMaxPendingConnections(CONFIG_QWDATABASE_CONNECTIONS_MAX);
	if (server->listen(QStringLiteral(CONFIG_QWDATABASE_PIPE_NAME))) {
		connect(server, &QLocalServer::newConnection, this, [this](){
			QLocalSocket *client = server->nextPendingConnection();
			if (client) {
				clients.push_back(client);
				connect(client, &QLocalSocket::disconnected, this, [this](){
					QLocalSocket *client = qobject_cast<QLocalSocket*>(sender());
					if (client) {
						this->clients.removeOne(client);
						client->deleteLater();
					}
				});
				connect(client, &QLocalSocket::readyRead, this, [this](){
					QLocalSocket *client = qobject_cast<QLocalSocket*>(sender());
					if (client) {
						this->fromBytes(client->readAll(), client);
					}
				});
			}
		});
	}
	QCoreApplication::setOrganizationName("E");
	QCoreApplication::setOrganizationDomain("E.ru");
	QCoreApplication::setApplicationName(QStringLiteral(PROJECT_NAME));
	settings = new QSettings(QStringLiteral(CONFIG_PARAMS_FILE_PATH), QSettings::IniFormat, this);
	connect(this, &QtDataBase::smthChanged, this, &QtDataBase::save);
}


WtDataBase::WtDataBase(QObject *parent) : QWDataBase(parent)
{
	client = new QLocalSocket(this);
	connect(client, &QLocalSocket::readyRead, this, [this](){
		if (this->client) {
			this->fromBytes(this->client->readAll());
		}
	});
	client->connectToServer(QStringLiteral(CONFIG_QWDATABASE_PIPE_NAME), QIODevice::ReadWrite);

	// self connections
	connect(this, &WtDataBase::p_requestAll, this, &WtDataBase::pp_requestAll);
	connect(this, &WtDataBase::p_requestDevicesList, this, &WtDataBase::pp_requestDevicesList);





	DevStruct::DeviceStat stat;
	stat.getM(id) = 0;
	stat.getM(aux.T) = 25;
	stat.getM(aux.U) = 4999;
	stat.getM(ch.signalLvl) = 75;
	stat.getM(ch.devEn) = 1;
	stat.getM(ch.outputRF) = 700;
	stat.getM(ch.reflectedPwr) = 1001;
	stat.getM(ch.T) = 10;
	m_devicesStat.insert(0, stat);
	stat.getM(id) = 1;
	m_devicesStat.insert(1, stat);

	QTimer *t = new QTimer(this);
	t->setInterval(1000);
	t->start(3000);
	connect(t, &QTimer::timeout, this, [this](){
		for (auto &x : m_devicesStat) {
			emit devicesStatChanged(x.getM(id));
		}
	});
}


QtDataBase::~QtDataBase()
{}


WtDataBase::~WtDataBase()
{}


