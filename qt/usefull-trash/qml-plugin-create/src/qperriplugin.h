#ifndef QPERRIPLUGIN_H
#define QPERRIPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class QPerriPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
	explicit QPerriPlugin(QObject *parent = nullptr);
	~QPerriPlugin() = default;
	void registerTypes(const char *uri) override;

signals:

};

#endif // QPERRIPLUGIN_H
