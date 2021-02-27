#include "regitem.h"

QObject *RegItem::item()
{
	return this->m_item;
}

QString &RegItem::property()
{
	return this->m_property;
}

quint16 RegItem::address()
{
	return this->m_address;
}

void RegItem::setItem(QObject *item)
{
	this->m_item = item;
	emit this->itemChanged(item);
}

void RegItem::setProperty(const QString &property)
{
	this->m_property = property;
	emit this->propertyChanged(this->m_property);
}

void RegItem::setAddress(quint16 address)
{
	this->m_address = address;
	emit this->addressChanged(address);
}

RegItem::RegItem(RegItem *item, QObject *parent) : QObject(parent)
{
    if (item) {
        this->m_item = item->item();
        this->m_property = item->property();
        this->m_address = item->address();
    }
}

RegItem::RegItem(QObject *parent) : QObject(parent)
{}
