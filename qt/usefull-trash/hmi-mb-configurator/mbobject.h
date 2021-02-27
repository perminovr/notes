#ifndef MBOBJECT_H
#define MBOBJECT_H

#include <QtGlobal>


class MbObject
{
public:
    enum Type {
        nDef,
        PushButton,
    };
    MbObject();

private:
    MbObject::Type type;
    quint16 address;
};

#endif // MBOBJECT_H
