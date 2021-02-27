#include "uimodbusprovider.h"
#include <QDebug>


template <class T>
static inline bool interval_conj(const T &as, const T &ae, const T &bs, const T &be)
{
    return (((as) <= (be)) && ((ae) >= (bs)));
}


RegItem *UIModbusProvider::regItem(int type, QObject *item,
        QString property, quint16 address, quint16 length)
{
    RegItem_ns::RegItemT regType = static_cast<RegItem_ns::RegItemT>(type);
    Key key = {regType, address, length};
    RegItem *i = 0;
    if (m_items.find(key) == m_items.end()) {
        i = new RegItem(regType, item, property, address, length, this); // todo delete
        this->m_items.insert(key, i);
    }
    emit newItem(i);
    return i;
}

QMap <UIModbusProvider::Key, RegItem *> *UIModbusProvider::getItems()
{
	return &this->m_items;
}

void UIModbusProvider::changeValue(RegItem *regItem, const QVariant &val)
{
    emit valueChanged(regItem, val);
}

void UIModbusProvider::endRegistration()
{
    emit registrationEnd();
}

UIModbusProvider::UIModbusProvider(QObject *parent) : QObject(parent)
{}

UIModbusProvider::~UIModbusProvider()
{
    for (auto &x : this->m_items)
        delete x;
}


bool operator==(const UIModbusProvider::Key &k1, const UIModbusProvider::Key &k2) {
    return (k1.type == k2.type) &&
            interval_conj(k1.address, quint16(k1.address+k1.length-1), k2.address, quint16(k2.address+k2.length-1));
}

bool operator<(const UIModbusProvider::Key &k1, const UIModbusProvider::Key &k2) {
    return (k1.type < k2.type)? true :
            (k1 == k2)? false :
            (k1.type == k2.type && k1.address < k2.address);
}

