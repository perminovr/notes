#include "mbconfig.h"

#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>



using namespace MbConfig;



enum ElementID {
	unknown,
	mbconfig,
	form,
	mbobject,
	opt,
	regtype,
	address,
	len,
};



static QObjectIDExt QString_to_ObjectIDExt(const QString &className);
static QString ObjectIDExt_to_QString(QObjectIDExt id);
//
static ElementID QStringRef_to_ElementID(const QStringRef &name);
static QString ElementID_to_QString(ElementID id);
//
static QModbusDataUnit::RegisterType QString_to_RegisterType(const QString &type);
static QString RegisterType_to_QString(QModbusDataUnit::RegisterType type);
//
static Option::Type QString_to_OptType(const QString &type);
static QString OptType_to_QString(Option::Type type);



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



int ParseOpt(QXmlStreamReader &reader, MbConfig::Option &opt)
{
	Option::Type type;
	MbField field;
	//
	bool ok;
	QString attrValue;
	QXmlStreamAttributes attr;

	if (GetAttr(reader, "type", attrValue) != 0)
		return 1;
	type = QString_to_OptType(attrValue);
	if (type == Option::unknown)
		return 1;

	// regtype
	{
		// start
		if (MoveToNextToken(reader, QXmlStreamReader::StartElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::regtype)
			return 1;
		// value
		if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
			return 1;
		attrValue = reader.text().toString();
		field.regType = QString_to_RegisterType(attrValue);
		if (field.regType == QModbusDataUnit::RegisterType::Invalid)
			return 1;
		// end
		if (MoveToNextToken(reader, QXmlStreamReader::EndElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::regtype)
			return 1;
	}

	// address
	{
		// start
		if (MoveToNextToken(reader, QXmlStreamReader::StartElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::address)
			return 1;
		// value
		if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
			return 1;
		attrValue = reader.text().toString();
		field.address = attrValue.toInt(&ok, 10);
		if (!ok)
			return 1;
		// end
		if (MoveToNextToken(reader, QXmlStreamReader::EndElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::address)
			return 1;
	}

	// len
	{
		// start
		if (MoveToNextToken(reader, QXmlStreamReader::StartElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::len)
			return 1;
		// value
		if (MoveToNextToken(reader, QXmlStreamReader::Characters) != 0)
			return 1;
		attrValue = reader.text().toString();
		field.len = attrValue.toInt(&ok, 10);
		if (!ok)
			return 1;
		// end
		if (MoveToNextToken(reader, QXmlStreamReader::EndElement) != 0)
			return 1;
		if (QStringRef_to_ElementID(reader.name()) != ElementID::len)
			return 1;
	} 

	// end
	if (MoveToNextToken(reader, QXmlStreamReader::EndElement) != 0)
		return 1;
	if (QStringRef_to_ElementID(reader.name()) != ElementID::opt)
		return 1;

	opt.type = type;
	opt.field = field;

	return 0;
}



int ParseObject(QXmlStreamReader &reader, MbConfig::Object &obj)
{
	QString type;
	QString name;
	QList <Option> options;
	//
	QString attrValue;
	QXmlStreamReader::TokenType token;

	if (GetAttr(reader, "type", attrValue) != 0)
		return 1;
	type = attrValue;
	if (GetAttr(reader, "name", attrValue) != 0)
		return 1;
	name = attrValue;

	while (true) {
		token = reader.readNext();
		if (reader.atEnd() || reader.hasError()) {
			return 1;
		}
		if (token == QXmlStreamReader::StartElement) {
			switch ( QStringRef_to_ElementID(reader.name()) ) {
				case ElementID::opt: {
					MbConfig::Option opt;
					if (ParseOpt(reader, opt) != 0) {
						return 1;						
					}
					options.push_back(opt);
				} break;
				default: {
					return 1;
				} break;
			}
			continue;
		} else if (token == QXmlStreamReader::EndElement) {
			if ( QStringRef_to_ElementID(reader.name()) == ElementID::mbobject ) {
				break; // success -> end
			}
		}
	}

	obj.name = name;
	obj.type = type;
	obj.options = options;

	return 0;
}



int ParseForm(QXmlStreamReader &reader, MbConfig::Form &form)
{
	int id;
	QString name;
	QList <Object> objects;
	//
	bool ok;
	QString attrValue;
	QXmlStreamReader::TokenType token;

	if (GetAttr(reader, "name", attrValue) != 0)
		return 1;
	name = attrValue;
	if (GetAttr(reader, "id", attrValue) != 0)
		return 1;
	id = attrValue.toInt(&ok, 10);
	if (!ok)
		return 1;

	while (true) {
		token = reader.readNext();
		if (reader.atEnd() || reader.hasError()) {
			return 1;
		}
		if (token == QXmlStreamReader::StartElement) {
			switch ( QStringRef_to_ElementID(reader.name()) ) {
				case ElementID::mbobject: {
					MbConfig::Object obj;
					if (ParseObject(reader, obj) != 0) {
						return 1;
					}
					objects.push_back(obj);
				} break;
				default: {
					return 1;
				} break;
			}
			continue;
		} else if (token == QXmlStreamReader::EndElement) {
			if ( QStringRef_to_ElementID(reader.name()) == ElementID::form ) {
				break; // success -> end
			}
		}
	}

	form.name = name;
	form.id = id;
	form.objects = objects;
		
	return 0;
}



int Config::readFromFile(QFile *file)
{
	QXmlStreamReader::TokenType token;
	QXmlStreamReader reader(file);
	QString attrValue;

	QList <Form> forms;
	QList <Object> objects;

	// mbconfig
	token = reader.readNext();
	if (token != QXmlStreamReader::StartDocument)
		return 1;
	token = reader.readNext();
	if (token != QXmlStreamReader::StartElement)
		return 1;
	if (QStringRef_to_ElementID(reader.name()) != ElementID::mbconfig)
		return 1;
	if (GetAttr(reader, "version", attrValue) != 0)
		return 1;
	int v1,v2,v3,v4;
	if (sscanf(attrValue.toStdString().c_str(), "%d.%d.%d.%d", &v1,&v2,&v3,&v4) != 4)
		return 1;
	// todo version control

	while (true) {
		token = reader.readNext();
		if (reader.atEnd() || reader.hasError()) {
			return 1;
		}
		if (token == QXmlStreamReader::StartElement) {
			switch ( QStringRef_to_ElementID(reader.name()) ) {
				case ElementID::form: {
					MbConfig::Form form;
					if (ParseForm(reader, form) != 0) {
						return 1;
					}
					forms.push_back(form);
				} break;
				case ElementID::mbobject: {
					MbConfig::Object obj;
					if (ParseObject(reader, obj) != 0) {
						return 1;
					}
					objects.push_back(obj);			   
				} break;
				default: { /* NOP */ } break;
			}
		} else if (token == QXmlStreamReader::EndElement) {
			if ( QStringRef_to_ElementID(reader.name()) == ElementID::mbconfig ) {
				break; // success -> end
			}
		}
	}

	this->forms = forms;
	this->objects = objects;

	return 0;
}



int Config::writeToFile(QFile *file)
{
	QXmlStreamWriter writer(file);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();

	// writer.writeStartElement("bookmark");
    // writer.writeAttribute("href", "http://qt-project.org/");
    // writer.writeTextElement("title", "Qt Project");
    // writer.writeEndElement(); // bookmark

	writer.writeStartElement( ElementID_to_QString(ElementID::mbconfig) );
	writer.writeAttribute("version", ""); // todo
	for (auto &x : this->objects) {
		writer.writeStartElement( ElementID_to_QString(ElementID::mbobject) );
		writer.writeAttribute("type", x.type);
		writer.writeAttribute("name", x.name);
		for (auto &o : x.options) {

		}
		writer.writeEndElement(); // mbobject
	}
	writer.writeEndElement(); // mbconfig

	writer.writeEndDocument();
}



Config::Config(QFile *file)
{
	readFromFile(file);
}



static QObjectIDExt QString_to_ObjectIDExt(const QString &className)
{
	QObjectID id = GetQObjectID(className);
	if (id == QObjectID::unknown) {
		if (className == "MbCurrentForm")
			return QObjectIDExt::MbCurrentForm;
		return (QObjectIDExt)QObjectID::unknown;
	}
	return (QObjectIDExt)id;
}



static QString ObjectIDExt_to_QString(QObjectIDExt id)
{

}



static ElementID QStringRef_to_ElementID(const QStringRef &name)
{
	if (name == "mbconfig")
		return ElementID::mbconfig;
	if (name == "form")
		return ElementID::form;
	if (name == "mbobject")
		return ElementID::mbobject;
	if (name == "opt")
		return ElementID::opt;
	if (name == "regtype")
		return ElementID::regtype;
	if (name == "address")
		return ElementID::address;
	if (name == "len")
		return ElementID::len;
	return ElementID::unknown;
}



static QString ElementID_to_QString(ElementID id)
{
	switch (id) {
		case ElementID::mbconfig:
			return "mbconfig";
		case ElementID::form:
			return "form";
		case ElementID::mbobject:
			return "mbobject";
		case ElementID::opt:
			return "opt";
		case ElementID::regtype:
			return "regtype";
		case ElementID::address:
			return "address";
		case ElementID::len:
			return "len";
		default:
			return "unknown";
	}
}



static QModbusDataUnit::RegisterType QString_to_RegisterType(const QString &type)
{
	if (type == "IR")
		return QModbusDataUnit::RegisterType::InputRegisters;
	if (type == "HR")
		return QModbusDataUnit::RegisterType::HoldingRegisters;
	if (type == "C")
		return QModbusDataUnit::RegisterType::Coils;
	if (type == "DI")
		return QModbusDataUnit::RegisterType::DiscreteInputs;
	return QModbusDataUnit::RegisterType::Invalid;
}



static QString RegisterType_to_QString(QModbusDataUnit::RegisterType type)
{
	switch (type) {
		case QModbusDataUnit::RegisterType::InputRegisters:
			return "IR";
		case QModbusDataUnit::RegisterType::HoldingRegisters:
			return "HR";
		case QModbusDataUnit::RegisterType::Coils:
			return "C";
		case QModbusDataUnit::RegisterType::DiscreteInputs:
			return "DI";
		default:
			return "INV";
	}
}



static Option::Type QString_to_OptType(const QString &type)
{
	if (type == "self")
		return Option::self;
	if (type == "clicked")
		return Option::clicked;
	if (type == "linkActivated")
		return Option::linkActivated;
	return Option::unknown;
}



static QString OptType_to_QString(Option::Type type)
{
	switch (type) {
		case Option::self:
			return "self";
		case Option::clicked:
			return "clicked";
		case Option::linkActivated:
			return "linkActivated";
		default:
			return "unknown";
	}
}