#ifndef HMICONFIG_H
#define HMICONFIG_H

#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>


struct ModbusConfig 
{
	quint16 addr;
	QString ip;
	quint16 port;
};


struct HMIConfig
{
	QString version;
    QString startScreen;
	ModbusConfig mbConfig;


    HMIConfig() = default;
    ~HMIConfig() = default;
    int readFromFile(QFile *file);
    int writeToFile(QFile *file);

private:
	int ParseStartScreen(QXmlStreamReader &reader);
	int ParseModbus(QXmlStreamReader &reader);
};

#endif // HMICONFIG_H
