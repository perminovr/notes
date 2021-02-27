#ifndef UIBACKEND_H
#define UIBACKEND_H

#include "regitem.h"
#include <QList>


class UIBackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(RegItem* item READ lastItem WRITE regItem NOTIFY itemChanged)

public:
    UIBackEnd(QObject *parent = nullptr);
    ~UIBackEnd() = default;

    RegItem *lastItem();
    QList <RegItem*> *items();

	static void qmlRegister() {
		RegItem::qmlRegister();
		qmlRegisterType<UIBackEnd>("mypackage", 1, 0, "UIBackEnd");
	}

public slots:
    void regItem(RegItem *item);
    void changeValue(quint16 addr, quint16 val);

signals:
    void itemChanged(RegItem *item);
	void valueChanged(quint16 addr, quint16 val);

private:
    QList <RegItem *> m_items;
};




// namespace UIBackEndNS
// {
//     Q_NAMESPACE
//     enum UIt {
//         UIt1 = 1,
//         UIt2 = 2,
//     };
//     Q_ENUM_NS(UIt)
// }
//    qmlRegisterUncreatableMetaObject(UIBackEndNS::staticMetaObject,
//      "io.qt.examples.uibackend", 1, 0,
//      "UIBackEndNS",
//      "Error: only enums"
//    );

#endif // UIBACKEND_H
