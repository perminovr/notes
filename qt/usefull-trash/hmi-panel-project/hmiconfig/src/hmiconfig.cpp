#include "hmiconfig.h"
#include <QHostAddress>


enum XMLConfigID {
    unknown,
    hmiconfig,
    startscreen,
    modbus,
    server_addr,
    ip,
    port,
};


static XMLConfigID QStringRef_to_XMLConfigID(const QStringRef &name);
static const char *XMLConfigID_to_String(XMLConfigID id);


int MoveToNextToken(QXmlStreamReader &reader, QXmlStreamReader::TokenType type)
{
    QXmlStreamReader::TokenType token;
    while (true) {
        token = reader.readNext();
        if (reader.atEnd() || reader.hasError())
            return 1;
        if (token == type)
            return 0;
    }
}


int GetAttr(QXmlStreamReader &reader, const QString &attrName, QString &value)
{
    QXmlStreamAttributes attr = reader.attributes();
    if (attr.hasAttribute(attrName)) {
        value = attr.value(attrName).toString();
        return 0;
    } else {
        return 1;
    }
}


int HMIConfig::ParseStartScreen(QXmlStreamReader &reader)
{
	if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
		return 1;
	QString attrValue;
	attrValue = reader.text().toString();
	this->startScreen = attrValue;
	return 0;
}


int HMIConfig::ParseModbus(QXmlStreamReader &reader)
{
	bool ok;
	QString attrValue;
	bool addr = false, ip = false, port = false;
	QXmlStreamReader::TokenType token;

	while (true) {
		token = reader.readNext();
		if (reader.atEnd() || reader.hasError()) {
			return 1;
		}
		if (token == QXmlStreamReader::StartElement) {
			switch ( QStringRef_to_XMLConfigID(reader.name()) ) {
				case XMLConfigID::server_addr: {
					if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
						return 1;
					attrValue = reader.text().toString();
					this->mbConfig.addr = (quint16)attrValue.toUInt(&ok);
					if (!ok || this->mbConfig.addr > 255)
						return 1;
					addr = true;
				} break;
				case XMLConfigID::ip: {
					if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
						return 1;
					attrValue = reader.text().toString();
					this->mbConfig.ip = attrValue;
					QHostAddress hostAddr;
					if ( !hostAddr.setAddress(this->mbConfig.ip) )
						return 1;
					ip = true;
				} break;
				case XMLConfigID::port: {
					if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
						return 1;
					attrValue = reader.text().toString();
					this->mbConfig.port = (quint16)attrValue.toUInt(&ok);
					if (!ok)
						return 1;
					port = true;
				} break;
                default: { /* NOP */ } break;
			}
		} else if (token == QXmlStreamReader::EndElement) {
			if ( QStringRef_to_XMLConfigID(reader.name()) == XMLConfigID::modbus ) {
				if (addr && ip && port) {
					break; // success -> end
				} else {
					return 1;
				}
			}
		}
	}
	
	return 0;
}


int HMIConfig::readFromFile(QFile *file)
{
    if (file->isOpen()) {
        QXmlStreamReader::TokenType token;
        QXmlStreamReader reader(file);
        QString attrValue;
        token = reader.readNext();
        if (token != QXmlStreamReader::StartDocument)
            return 1;
        token = reader.readNext();
        if (token != QXmlStreamReader::StartElement)
            return 1;
        // hmiconfig
        if (QStringRef_to_XMLConfigID(reader.name()) != XMLConfigID::hmiconfig)
            return 1;
        if (GetAttr(reader, "version", attrValue) != 0)
            return 1;
		this->version = attrValue;

        while (true) {
            token = reader.readNext();
            if (reader.atEnd() || reader.hasError()) {
                return 1;
            }
            if (token == QXmlStreamReader::StartElement) {
                switch ( QStringRef_to_XMLConfigID(reader.name()) ) {
                    case XMLConfigID::startscreen: {
						if (ParseStartScreen(reader) != 0)
							return 1;
                    } break;
					case XMLConfigID::modbus: {
						if (ParseModbus(reader) != 0)
							return 1;
					} break;
                    default: { /* NOP */ } break;
                }
            } else if (token == QXmlStreamReader::EndElement) {
                if ( QStringRef_to_XMLConfigID(reader.name()) == XMLConfigID::hmiconfig ) {
                    break; // success -> end
                }
            }
        }
        return 0;
    }
    return 1;
}


int HMIConfig::writeToFile(QFile *file)
{
    if (file->isOpen()) {
		QXmlStreamWriter writer(file);
		writer.setAutoFormatting(true);

		writer.writeStartDocument();
		writer.writeStartElement( XMLConfigID_to_String(XMLConfigID::hmiconfig) );
		writer.writeAttribute("version", this->version);
		{
			writer.writeTextElement( XMLConfigID_to_String(XMLConfigID::startscreen), this->startScreen);
			writer.writeStartElement( XMLConfigID_to_String(XMLConfigID::modbus) );
			{
				writer.writeTextElement( XMLConfigID_to_String(XMLConfigID::server_addr), QString::number(this->mbConfig.addr));
				writer.writeTextElement( XMLConfigID_to_String(XMLConfigID::ip), this->mbConfig.ip);
				writer.writeTextElement( XMLConfigID_to_String(XMLConfigID::port), QString::number(this->mbConfig.port));				
			}
			writer.writeEndElement(); // modbus
		}
		writer.writeEndElement(); // hmiconfig
		writer.writeEndDocument();

        return 0;
    }
    return 1;
}


static XMLConfigID QStringRef_to_XMLConfigID(const QStringRef &name)
{
    if (name == "hmiconfig")
        return XMLConfigID::hmiconfig;
    if (name == "startscreen")
        return XMLConfigID::startscreen;
    if (name == "modbus")
    	return XMLConfigID::modbus;
    if (name == "server_addr")
    	return XMLConfigID::server_addr;
    if (name == "ip")
    	return XMLConfigID::ip;
    if (name == "port")
    	return XMLConfigID::port;
    return XMLConfigID::unknown;
}


static const char *XMLConfigID_to_String(XMLConfigID id)
{
	switch (id) {
		case XMLConfigID::hmiconfig:
			return "hmiconfig";
		case XMLConfigID::startscreen:
			return "startscreen";
		case XMLConfigID::modbus:
			return "modbus";
		case XMLConfigID::server_addr:
			return "server_addr";
		case XMLConfigID::ip:
			return "ip";
		case XMLConfigID::port:
			return "port";
		default:
			return 0;
	}
}
