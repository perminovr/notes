#ifndef SERVICEPIPEPROVIDER_H
#define SERVICEPIPEPROVIDER_H

#include <QLocalServer>
#include <QObject>

class ServicePipeProvider : public QObject
{
    Q_OBJECT

public:
	ServicePipeProvider(QString serverName, QObject *parent = nullptr);
	~ServicePipeProvider();

    bool start();
	void setMaxConnections(int max);

signals:
	void dataIsReady(const QByteArray &data);

private:
    QString serverName;
	QLocalServer *server;

	void handleConnection();
	void readyRead();
};

#endif // SERVICEPIPEPROVIDER_H
