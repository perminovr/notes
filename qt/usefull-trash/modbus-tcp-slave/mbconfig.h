#ifndef MBCONFIG_H
#define MBCONFIG_H

#include <QtGlobal>
#include <QFile>
#include <QModbusDataUnit>

#include "qmbobject.h"



namespace MbConfig {



struct Option 
{
	enum Type {
		unknown,
		self,
		clicked,
		linkActivated,
		editingFinished,
		valueChanged,
		stateChanged,
		currentIndexChanged

	} type;
	MbField field;
};



enum class QObjectIDExt : int {
	tStart = (int)(QObjectID::tEnd),
	MbCurrentForm,
	tEnd,
};



struct Object
{
	QString type;
	QString name;
	QList <Option> options;
};



struct Form
{
	int id;
	QString name;
	QList <Object> objects;
};



class Config
{
public:
	QList <Form> forms;
	QList <Object> objects;

	Config() = default;
	~Config() = default;
	Config(QFile *file);
	int readFromFile(QFile *file);
	int writeToFile(QFile *file);
};



} // namespace MbConfig

#endif // MBCONFIG_H
