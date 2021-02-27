#include "servicepipeprovider.h"

#include <QLocalSocket>


void ServicePipeProvider::readyRead()
{
	QByteArray data;
	QLocalSocket* client = (QLocalSocket*)sender();

	if (client->bytesAvailable()) {
		data = client->readAll();
		emit dataIsReady(data);
    }
}


void ServicePipeProvider::handleConnection()
{
	QLocalSocket *clientConnection = server->nextPendingConnection();
	connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
	connect(clientConnection, &QLocalSocket::readyRead, this, &ServicePipeProvider::readyRead);
}


void ServicePipeProvider::setMaxConnections(int max)
{
	server->setMaxPendingConnections(max);
}


bool ServicePipeProvider::start()
{
    QLocalServer::removeServer(serverName);
    if (!server->listen(serverName)) {
        return false;
    }
    connect(server, &QLocalServer::newConnection, this, &ServicePipeProvider::handleConnection);
    return true;
}


ServicePipeProvider::ServicePipeProvider(QString serverName, QObject *parent)
    : QObject(parent), serverName(serverName), server(new QLocalServer(this))
{}


ServicePipeProvider::~ServicePipeProvider()
{
	if (server && server->isListening()) {
		server->close();
	}
	delete server;
}
