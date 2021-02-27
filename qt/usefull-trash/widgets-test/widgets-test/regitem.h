#ifndef REGITEM_H
#define REGITEM_H

#include <QObject>
#include <QString>
#include <QtGlobal>

#include <QtQml>

class RegItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* item READ item WRITE setItem NOTIFY itemChanged)
	Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
	Q_PROPERTY(quint16 address READ address WRITE setAddress NOTIFY addressChanged)

public:
    RegItem(RegItem *item, QObject *parent = nullptr);
    RegItem(QObject *parent = nullptr);
	~RegItem() = default;

    QObject *item();
    QString &property();
    quint16 address();

	static void qmlRegister() {
		qmlRegisterType<RegItem>("mypackage", 1, 0, "RegItem");
	}

public slots:
	void setItem(QObject *);
	void setProperty(const QString &);
	void setAddress(quint16 );

signals:
	void itemChanged(QObject *);
	void propertyChanged(QString &);
	void addressChanged(quint16 );

private:
    QObject *m_item;
    QString m_property;
    quint16 m_address;

};

#endif // REGITEM_H
