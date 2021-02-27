TEMPLATE = lib
CONFIG += qt plugin
QT += qml

DESTDIR = imports/Perri
TARGET = qmlqperriplugin


SOURCES += qperriplugin.cpp \
    timemodel.cpp

HEADERS += qperriplugin.h \
    timemodel.h

DISTFILES += \
    led.qml \
    tst.qml
