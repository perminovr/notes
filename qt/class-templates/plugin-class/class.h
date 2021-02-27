#ifndef CLASS_NAME_H
#define CLASS_NAME_H

#include <QObject>
#include <QtQml>
#include <QQmlParserStatus>

class _CLASS_NAME_ : public QObject , public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(bool isComplete READ isComplete NOTIFY completed)

public:
    _CLASS_NAME_(QObject *parent = nullptr);
    virtual ~_CLASS_NAME_();

    virtual void classBegin() override;
    virtual void componentComplete() override;
    bool isComplete();

    static void qmlRegister(const char *pkgName, int mj, int mi) {
        qmlRegisterType<_CLASS_NAME_>(pkgName, mj, mi, "_CLASS_NAME_");
    }

public slots:

signals:
    void completed(_CLASS_NAME_ *);

private slots:

private:
    bool m_complete;

};

#endif // CLASS_NAME_H
