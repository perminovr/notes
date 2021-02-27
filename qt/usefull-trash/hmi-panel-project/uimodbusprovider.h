#ifndef UIMODBUSPROVIDER_H
#define UIMODBUSPROVIDER_H

#include "regitem.h"
#include <QList>


class UIModbusProvider : public QObject
{
    Q_OBJECT

public:
	UIModbusProvider(QObject *parent = nullptr);
    ~UIModbusProvider();

    Q_INVOKABLE RegItem *regItem(int type, QObject *item,
            QString property, quint16 address, quint16 length);
    Q_INVOKABLE void endRegistration();

    struct Key {
        RegItem_ns::RegItemT type;
        quint16 address;
        quint16 length;
    };
    QMap <Key, RegItem *> *getItems();

	static void qmlRegister() {
		RegItem::qmlRegister();
		qmlRegisterType<UIModbusProvider>(CONFIG_QML_PACKAGE_NAME, 1, 0, "UIModbusProvider");
	}

public slots:
    void changeValue(RegItem *regItem, const QVariant &val);

signals:
    void newItem(RegItem *item);
    void valueChanged(RegItem *regItem, const QVariant &val);
    void registrationEnd();

private:
    QMap <Key, RegItem *> m_items;
};


bool operator==(const UIModbusProvider::Key &k1, const UIModbusProvider::Key &k2);
bool operator<(const UIModbusProvider::Key &k1, const UIModbusProvider::Key &k2);

#endif // UIMODBUSPROVIDER_H
