
#include "timemodel.h"

TimeModel::TimeModel(QObject *parent) : QObject(parent)
{

}

int TimeModel::hour() const
{
	return m_hour;
}
