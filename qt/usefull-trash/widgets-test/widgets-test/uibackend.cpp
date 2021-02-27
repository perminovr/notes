#include "uibackend.h"
#include <QDebug>


RegItem *UIBackEnd::lastItem()
{
	return this->m_items.back();
}

QList <RegItem*> *UIBackEnd::items() 
{
	return &this->m_items;
}

void UIBackEnd::regItem(RegItem *item)
{
	RegItem *i = new RegItem(item);
	this->m_items.push_back(i);
	emit itemChanged(i);
}

void UIBackEnd::changeValue(quint16 addr, quint16 val)
{
	qInfo() << "yes please" << endl;
	emit valueChanged(addr, val);
}

UIBackEnd::UIBackEnd(QObject *parent) : QObject(parent)
{}








// QObject* UIBackEnd::qmlItem()
// {
//     return 0;
// }


// #include <QtQuick/QQuickItem>

// void UIBackEnd::butSlot()
// {
//     qInfo() << "butSlot " << "" << endl;
// }

// void UIBackEnd::setQmlItem(QObject *item)
// {
//     qInfo() << "setQmlItem " << "" << endl;
//     item->dumpObjectInfo();
//     qInfo() << item->metaObject()->className() << endl;
//     item->setProperty("text", "__but__");

//     qInfo() << "properties -- t:" << item->property("qqt").toUInt() << " x: " << item->property("qqx").toUInt() << endl;

//     //QObject::connect(item, SIGNAL(signalClicked()), this, SLOT(butSlot()));

// }
