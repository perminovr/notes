#ifndef TIMEMODEL_H
#define TIMEMODEL_H

#include <QObject>

class TimeModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hour READ hour NOTIFY timeChanged)

public:
    explicit TimeModel(QObject *parent = nullptr);
    ~TimeModel() = default;

    int hour() const;

signals:
    void timeChanged();

private:
    int m_hour;

};

#endif // TIMEMODEL_H
