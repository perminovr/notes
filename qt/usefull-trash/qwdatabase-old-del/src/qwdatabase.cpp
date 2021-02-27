#include "common.h"
#include "qwdatabase.h"
#include <QVariant>
#include <QtCore>
#include <unistd.h>

#define readParamInit	\
	QVariant v;			\
	QString prm;

#define readParam(k, def, strg) {		\
	v = (strg)->value(k, def);			\
	if (v.type() == QVariant::String && v.toString().size())	\
		prm = v.toString();				\
	else								\
		prm = def;						\
}

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


void QWDataBase::fromBytes(const QByteArray &data)
{
	QDataStream in(data);
	in.setVersion(QDataStream::Qt_5_3);

	if (data.size() > scast(int, sizeof(unsigned))) {
		unsigned blockSize, hash;
		in >> blockSize >> hash;

		switch (hash) {
			case netParam_hash: {
				decltype(m_netParam) s;
				QHostAddress ip, mask;
				in >> s.ipMode >> ip >> mask;
				s.localAddr.setIp(ip);
				s.localAddr.setNetmask(mask);
				setNetParamPrivate(s);
			} break;
			case gateway_hash: {
				decltype(m_gateway) s;
				in >> s;
				setGatewayPrivate(s);
			} break;
			case serverAddr_hash: {
				decltype(m_serverAddr) s;
				in >> s.ip >> s.port;
				setServerAddrPrivate(s);
			} break;
			case radioPrm_hash: {
				decltype(m_radioPrm) s;
				in >> s.timeoutAck >> s.timeoutRepeat
					>> s.slavePause >> s.pollPause;
				setRadioPrmPrivate(s);
			} break;
			case devicesStat_hash[0]:
			case devicesStat_hash[1]:
			case devicesStat_hash[2]:
			case devicesStat_hash[3]:
			case devicesStat_hash[4]:
			case devicesStat_hash[5]: {
				typeof(m_devicesStat[0]) s;
				int idx = -1;
				for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
					if (devicesStat_hash[i] == hash) {
						idx = i; break;
					}
				}
				if (idx >= 0) {
					in >> s.m.ch.signalLvl >> s.m.ch.devEn >> s.m.ch.T
						>> s.m.ch.reflectedPwr >> s.m.ch.outputRF
						>> s.m.aux.T >> s.m.aux.U >> s.m.aux.packetId
						>> s.m.id >> s.m.caseIsOpened;
					setDevicesStatPrivate(s, idx);
				}
			} break;
			case devicesCtl_hash[0]:
			case devicesCtl_hash[1]:
			case devicesCtl_hash[2]:
			case devicesCtl_hash[3]:
			case devicesCtl_hash[4]:
			case devicesCtl_hash[5]: {
				typeof(m_devicesCtl[0]) s;
				int idx = -1;
				for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
					if (devicesStat_hash[i] == hash) {
						idx = i; break;
					}
				}
				if (idx >= 0) {
					in >> s.m.ch.signalLvl >> s.m.ch.devEn
						>> s.m.id >> s.m.packetId;
					setDevicesCtlPrivate(s, idx);
				}
			} break;
			default: {} break;
		}
		emit smthChanged();
	}
}


QByteArray QWDataBase::toBytes(unsigned hash)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_3);

	out << unsigned(0) << hash;

	switch (hash) {
		case netParam_hash: {
			out << m_netParam.ipMode << m_netParam.localAddr.ip()
				<< m_netParam.localAddr.netmask();
		} break;
		case gateway_hash: {
			out << m_gateway;
		} break;
		case serverAddr_hash: {
			out << m_serverAddr.ip << m_serverAddr.port;
		} break;
		case radioPrm_hash: {
			out << m_radioPrm.timeoutAck << m_radioPrm.timeoutRepeat
				<< m_radioPrm.slavePause << m_radioPrm.pollPause;
		} break;
		case devicesStat_hash[0]:
		case devicesStat_hash[1]:
		case devicesStat_hash[2]:
		case devicesStat_hash[3]:
		case devicesStat_hash[4]:
		case devicesStat_hash[5]: {
			for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
				if (devicesStat_hash[i] == hash) {
					auto &x = m_devicesStat[i];
					out << x.m.ch.signalLvl << x.m.ch.devEn << x.m.ch.T
						<< x.m.ch.reflectedPwr << x.m.ch.outputRF
						<< x.m.aux.T << x.m.aux.U << x.m.aux.packetId
						<< x.m.id << x.m.caseIsOpened;
					break;
				}
			}
		} break;
		case devicesCtl_hash[0]:
		case devicesCtl_hash[1]:
		case devicesCtl_hash[2]:
		case devicesCtl_hash[3]:
		case devicesCtl_hash[4]:
		case devicesCtl_hash[5]: {
			for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
				if (devicesCtl_hash[i] == hash) {
					auto &x = m_devicesCtl[i];
					out << x.m.ch.signalLvl << x.m.ch.devEn
						<< x.m.id << x.m.packetId;
					break;
				}
			}
		} break;
		default: {
			return QByteArray();
		} break;
	}

	out.device()->seek(0);
	out << unsigned(data.size() - sizeof(unsigned));

	return data;
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

	// devices

	for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
		QString pname;

		pname = QString(DEV_SECTION DEV_ID).arg(i);
		readParam(pname, "0", this->settings);
		this->m_devicesCtl[i].m.id = scast(quint16, prm.toInt());

		pname = QString(DEV_SECTION DEV_EN).arg(i);
		readParam(pname, "0", this->settings);
		this->m_devicesCtl[i].m.ch.devEn = scast(bool, prm.toInt());

		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		readParam(pname, "0", this->settings);
		this->m_devicesCtl[i].m.ch.signalLvl = scast(quint8, prm.toInt());
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
	this->m_gateway = QHostAddress(prm);

}


void QtDataBase::save()
{
	// devices
	for (int i = 0; i < QWDATABASE_DEVICES_CNT; ++i) {
		QString pname;
		pname = QString(DEV_SECTION DEV_ID).arg(i);
		this->settings->setValue(pname, scast(int, this->m_devicesCtl[i].m.id));
		pname = QString(DEV_SECTION DEV_EN).arg(i);
		this->settings->setValue(pname, scast(int, this->m_devicesCtl[i].m.ch.devEn));
		pname = QString(DEV_SECTION DEV_LVL).arg(i);
		this->settings->setValue(pname, scast(int, this->m_devicesCtl[i].m.ch.signalLvl));
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
	this->settings->setValue(ETH_SECTION ETH_GW, this->m_gateway.toString());

	this->settings->sync();
	sync();
}


void QtDataBase::handleChanges(unsigned hash)
{
	QByteArray data = QWDataBase::toBytes(hash);
	for (auto &x : clients) {
		x->write(data);
	}
}


void WtDataBase::handleChanges(unsigned hash)
{
	QByteArray data = QWDataBase::toBytes(hash);
	client->write(data);
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
						this->fromBytes(client->readAll());
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
}


QtDataBase::~QtDataBase()
{}


WtDataBase::~WtDataBase()
{}


