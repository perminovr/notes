#include "class.h"


void _CLASS_NAME_::classBegin()
{}


void _CLASS_NAME_::componentComplete()
{
    this->m_complete = true;
    emit completed(this);
}


bool _CLASS_NAME_::isComplete()
{
    return this->m_complete;
}


_CLASS_NAME_::_CLASS_NAME_(QObject *parent) : QObject(parent)
{
    this->m_complete = false;
}


_CLASS_NAME_::~_CLASS_NAME_()
{

}
