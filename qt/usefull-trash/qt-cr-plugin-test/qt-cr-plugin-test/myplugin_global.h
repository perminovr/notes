#ifndef QT_CR_PLUGIN_TEST_GLOBAL_H
#define QT_CR_PLUGIN_TEST_GLOBAL_H

#include <QtGlobal>

#if defined(MYPLUGIN_LIBRARY)
#  define MYPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MYPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QT_CR_PLUGIN_TEST_GLOBAL_H
