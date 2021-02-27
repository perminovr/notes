#include "regitem.h"


RegItem_ns::RegItemT RegItem::type()
{
    return this->m_type;
}

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

quint16 RegItem::length()
{
    return this->m_length;
}


void RegItem::setType(RegItem_ns::RegItemT type)
{
    this->m_type = type;
    emit this->typeChanged(type);
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

void RegItem::setLength(quint16 length)
{
    this->m_length = length;
    emit this->lengthChanged(length);
}



RegItem::RegItem(RegItem_ns::RegItemT type, QObject *item, QString property,
                 quint16 address, quint16 length, QObject *parent) : QObject(parent)
{
    this->m_type = type;
    this->m_item = item;
    this->m_property = property;
    this->m_address = address;
    this->m_length = length;
}

RegItem::RegItem(RegItem *item, QObject *parent) : QObject(parent)
{
    if (item) {
        this->m_type = item->type();
        this->m_item = item->item();
        this->m_property = item->property();
        this->m_address = item->address();
        this->m_length = item->length();
    }
}

RegItem::RegItem(QObject *parent) : QObject(parent)
{}
