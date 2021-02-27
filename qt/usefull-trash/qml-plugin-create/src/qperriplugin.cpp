
#include "qperriplugin.h"
#include "timemodel.h"


QPerriPlugin::QPerriPlugin(QObject *parent) : QQmlExtensionPlugin(parent)
{

}


void QPerriPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("Perri"));
    qmlRegisterType<TimeModel>(uri, 1, 0, "Time");
}
