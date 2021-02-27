QT += quick virtualkeyboard \
    core quickwidgets serialbus network

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VPATH += \
    hmiconfig/include \
    hmiconfig/src

SUBDIRS += \
    hmiconfig/src

SOURCES += \
    hmiconfig.cpp \
    hmivariant.cpp \
    hmiworker.cpp \
    main.cpp \
    multimodbustcpserver.cpp \
    regitem.cpp \
    servicepipeprovider.cpp \
    uimodbusprovider.cpp

RESOURCES += qml.qrc \
    unused.qrc

TRANSLATIONS += \
    hmi-panel-project_en_150.ts

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

INCLUDEPATH += \
    hmiconfig/include

HEADERS += \
    config.h \
    hmiconfig.h \
    hmiconfig/include/hmiconfig.h \
    hmivariant.h \
    hmiworker.h \
    multimodbustcpserver.h \
    regitem.h \
    servicepipeprovider.h \
    uimodbusprovider.h