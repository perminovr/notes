#!/bin/sh

MAINSH_NAME="patch_main_update.elesysh"
SYSPREFIX="tc71x"
SYSVER="3.2.2.10775"

PATCHNAME="${SYSPREFIX}_v${SYSVER}_patch1"
PATCHVER="001"

DESCRIPTION="Патч $PATCHNAME версии $PATCHVER
Состав патча:
	/home/root/Libs/libnetlinklayer.so
		исправлена функция protocolsend: добавлена проверка буфера 
		(pc->sendbuf) перед отправкой
	/etc/init.d/elesyfunction.sh
		добавлена обработка результата работы скрипта
	/etc/init.d/networking
		оригинальный файл
"
KEYPEM="hcs@elesy.ru-key.pem"

UPDATES="elesyfunctions.sh networking"
# all *.so
UPDATES="$UPDATES `find * -type f -regex ".*\.\(so\)"`"


# 1 - data name
sign_data() {
	openssl dgst -passin pass:$PASSPEM -sha256 -sign $KEYPEM -out $1.sign $1
	openssl enc -base64 -in $1.sign -out $1.elesign
	rm -rf $1.sign	
}

# 1 - prop name
# 2 - prop val
# 3 - file name
# 4 - string number
write_prop() {
	if [ "`grep "$1=" $3`" == "" ]; then
		sed -i "$4i$1=\"$2\"" $3
	else
		sed -i "s/$1=.*$/$1=\"$2\"/" $3
	fi
}

echo -n "PEM-pass: "
read -s PASSPEM
echo ""

#echo -n "zip-name: "
#read ZIPNAME
#echo ""


# write ver and name
write_prop "MYNAME" "$PATCHNAME" "$MAINSH_NAME" "2"
write_prop "MYVER" "$PATCHVER" "$MAINSH_NAME" "3"


# compute md5 for all updates
UPDATES="`echo $UPDATES | sort -u`"
echo -n "Updates: "
echo $UPDATES
echo $UPDATES | tr ' ' '\n' > updates_list
UPDATES="$UPDATES updates_list"
for upd in $UPDATES; do
	sum=`md5sum $upd | cut -d" " -f1`
	upd=`echo "$upd" | tr '.' "_"`
	write_prop "$upd" "$sum" "$MAINSH_NAME" "6"
done

ESHS=`ls | grep ".elesysh$"`
echo -n "Scripts: "
echo $ESHS
for esh in $ESHS; do
	sign_data $esh
done

printf "$DESCRIPTION" > description.txt

zip -r ${PATCHNAME}_v${PATCHVER}.zip ${UPDATES} *.elesysh *.elesysh.elesign updates_list build.sh description.txt
rm -rf *.elesign

echo "done"
