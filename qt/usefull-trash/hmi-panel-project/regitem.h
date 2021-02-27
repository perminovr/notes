#ifndef REGITEM_H
#define REGITEM_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QtQml>

#include "config.h"


 namespace RegItem_ns
 {
     Q_NAMESPACE
     enum RegItemT {
         ReadOnly = 1,
         ReadWrite = 2,
     };
     Q_ENUM_NS(RegItemT)
 }


class RegItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RegItem_ns::RegItemT type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QObject* item READ item WRITE setItem NOTIFY itemChanged)
    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    Q_PROPERTY(quint16 address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(quint16 length READ length WRITE setLength NOTIFY lengthChanged)

public:
    RegItem(RegItem_ns::RegItemT type, QObject *item, QString property,
            quint16 address, quint16 length, QObject *parent = nullptr);
    RegItem(RegItem *item, QObject *parent = nullptr);
    RegItem(QObject *parent = nullptr);
    ~RegItem() = default;

    RegItem_ns::RegItemT type();
    QObject *item();
    QString &property();
    quint16 address();
    quint16 length();

    static void qmlRegister() {
        qmlRegisterUncreatableMetaObject(RegItem_ns::staticMetaObject,
                CONFIG_QML_PACKAGE_NAME, 1, 0,
                "RegItemType", "Error: only enums");
        qmlRegisterType<RegItem>(CONFIG_QML_PACKAGE_NAME, 1, 0, "RegItem");
    }

public slots:
    void setType(RegItem_ns::RegItemT );
    void setItem(QObject *);
    void setProperty(const QString &);
    void setAddress(quint16 );
    void setLength(quint16 );

signals:
    void typeChanged(RegItem_ns::RegItemT );
    void itemChanged(QObject *);
    void propertyChanged(QString &);
    void addressChanged(quint16 );
    void lengthChanged(quint16 );

private:
    RegItem_ns::RegItemT m_type;
    QObject *m_item;
    QString m_property;
    quint16 m_address;
    quint16 m_length;

};

#endif // REGITEM_H
