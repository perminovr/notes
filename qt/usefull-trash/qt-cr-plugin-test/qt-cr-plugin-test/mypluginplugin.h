#ifndef QT_CR_PLUGIN_TEST_H
#define QT_CR_PLUGIN_TEST_H

#include "myplugin_global.h"

#include <extensionsystem/iplugin.h>

namespace MyPlugin {
namespace Internal {

class MyPluginPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "MyPlugin.json")

public:
    MyPluginPlugin();
    ~MyPluginPlugin() override;

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

private:
    void triggerAction();
};

} // namespace Internal
} // namespace MyPlugin

#endif // QT_CR_PLUGIN_TEST_H
