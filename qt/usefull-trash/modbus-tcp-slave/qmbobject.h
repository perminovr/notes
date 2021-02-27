#ifndef QMBOBJECT_H
#define QMBOBJECT_H

#include <QModbusDataUnit>

#include <QtGlobal>
#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLCDNumber>
#include <QSlider>
#include <QDial>
#include <QProgressBar>



union QUniObject {
	QObject *obj;
	QPushButton *pbtn;
	QLineEdit *ledit;
	QSpinBox *spinb;
	QLabel *txtlbl;
};



struct QMbObject;



struct MbField {
	QModbusDataUnit::RegisterType regType;
	quint16 address;
	quint16 len;
	enum QObjectSignalType {
		Void,
		Int,
		QString,
	} sigType;
};



struct QMbObject {
	QObject *self;
	QList <MbField*> fieldList;
};



enum class QObjectID : int {
	unknown,
	PushButton,
	LineEdit,
	SpinBox,
	Label,
	RadioButton,
	CheckBox,
	ComboBox,
	LCDNumber,
	ProgressBar,
	Slider,
	Dial,
	tEnd
};



static QObjectID GetQObjectID(const QString &className)
{
	const char *cn = className.toStdString().c_str();
	if (strcmp(cn, QPushButton::staticMetaObject.className()) == 0)
		return QObjectID::PushButton;
	if (strcmp(cn, QLineEdit::staticMetaObject.className()) == 0)
		return QObjectID::LineEdit;
	if (strcmp(cn, QSpinBox::staticMetaObject.className()) == 0)
		return QObjectID::SpinBox;
	if (strcmp(cn, QLabel::staticMetaObject.className()) == 0)
		return QObjectID::Label;
	if (strcmp(cn, QRadioButton::staticMetaObject.className()) == 0)
		return QObjectID::RadioButton;
	if (strcmp(cn, QCheckBox::staticMetaObject.className()) == 0)
		return QObjectID::CheckBox;
	if (strcmp(cn, QComboBox::staticMetaObject.className()) == 0)
		return QObjectID::ComboBox;
	if (strcmp(cn, QLCDNumber::staticMetaObject.className()) == 0)
		return QObjectID::LCDNumber;
	if (strcmp(cn, QSlider::staticMetaObject.className()) == 0)
		return QObjectID::Slider;
	if (strcmp(cn, QDial::staticMetaObject.className()) == 0)
		return QObjectID::Dial;
	return QObjectID::unknown;
}



static QObjectID GetQObjectID(QObject *object)
{
	if (qobject_cast<QPushButton*>(object))
		return QObjectID::PushButton;
	if (qobject_cast<QLineEdit*>(object))
		return QObjectID::LineEdit;
	if (qobject_cast<QSpinBox*>(object))
		return QObjectID::SpinBox;
	if (qobject_cast<QLabel*>(object))
		return QObjectID::Label;
	if (qobject_cast<QRadioButton*>(object))
		return QObjectID::RadioButton;
	if (qobject_cast<QCheckBox*>(object))
		return QObjectID::CheckBox;
	if (qobject_cast<QComboBox*>(object))
		return QObjectID::ComboBox;
	if (qobject_cast<QLCDNumber*>(object))
		return QObjectID::LCDNumber;
	if (qobject_cast<QSlider*>(object))
		return QObjectID::Slider;
	if (qobject_cast<QDial*>(object))
		return QObjectID::Dial;
	return QObjectID::unknown;
}



#endif // QMBOBJECT_H
