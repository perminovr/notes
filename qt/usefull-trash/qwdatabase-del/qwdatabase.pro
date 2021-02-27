TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS +=

HEADERS += \
	qwdatabase.h


SOURCES += \
	qwdatabase.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/qwdatabase


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

