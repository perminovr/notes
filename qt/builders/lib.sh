
OBJ_NAME=$1
WD=$2

if [ "$OBJ_NAME" == "" -o -d "$OBJ_NAME" ]; then
	echo "1 - object name"
	echo "2 - workdir (maybe undef)"	
	exit 1
fi

if [ "$WD" != "" -a -d "$WD" ]; then
	cd $WD
fi

OBJ_NAME_UPPER=`echo $OBJ_NAME | tr [a-z] [A-Z]`

mkdir -p $OBJ_NAME/src
mkdir -p $OBJ_NAME/include
touch $OBJ_NAME/$OBJ_NAME.pri
touch $OBJ_NAME/$OBJ_NAME.pro
touch $OBJ_NAME/src/$OBJ_NAME.cpp
touch $OBJ_NAME/include/$OBJ_NAME.h

cd $OBJ_NAME



# include ################################################
cat << EOF > include/$OBJ_NAME.h
#ifndef ${OBJ_NAME_UPPER}_H
#define ${OBJ_NAME_UPPER}_H

#include <QObject>

class __CLASSNAME__ : public QObject
{
    Q_OBJECT
public:

    __CLASSNAME__(QObject *parent = nullptr);
    virtual ~__CLASSNAME__();

public slots:
signals:
private slots:
private:
};

#endif // ${OBJ_NAME_UPPER}_H
EOF



# source #################################################
cat << EOF > src/$OBJ_NAME.cpp
#include "$OBJ_NAME.h"


__CLASSNAME__::__CLASSNAME__(QObject *parent) : QObject(parent)
{
}


__CLASSNAME__::~__CLASSNAME__()
{
}

EOF



# pri ####################################################
cat << EOF > $OBJ_NAME.pri
LIBTARGET = $OBJ_NAME
BASEDIR   = \$\${PWD}
INCLUDEPATH *= \$\${BASEDIR}/include
LIBS += -l\$\${LIBTARGET}

EOF



# pro ####################################################
cat << EOF > $OBJ_NAME.pro
TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += 
LIBS += 

HEADERS += \\
	$OBJ_NAME.h

SOURCES += \\
	$OBJ_NAME.cpp
	

TARGET = \$\${PRJ_LIB}/$OBJ_NAME


VPATH += \\
	include \\
	src

INCLUDEPATH += \\
	include \\
	src

EOF
