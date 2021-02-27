
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
touch $OBJ_NAME/$OBJ_NAME.pro
touch $OBJ_NAME/src/$OBJ_NAME.cpp

cd $OBJ_NAME



# main ################################################
cat << EOF > src/$OBJ_NAME.cpp
#include <QtCore>


int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	return app.exec();
}

EOF



# pro #################################################
cat << EOF > $OBJ_NAME.pro
TEMPLATE = app
QT += core
CONFIG +=
LIBS += 

HEADERS += \\

SOURCES += \\
	$OBJ_NAME.cpp


TARGET = \$\${PRJ_BIN}/$OBJ_NAME


CONFIG += c++11

VPATH += \\
	src \\
	include

INCLUDEPATH += \\
	src \\
	include

EOF

