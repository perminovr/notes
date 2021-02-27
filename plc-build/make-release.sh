#!/bin/bash

# $1 = work dir
# $2 = pass phrase for pem key
# $3 = full version
# $4 = release dir

DT=`date +%Y-%m-%d-%H-%M`
WORKDIR="$1"
PASSPEM="$2"
FULLVERS="$3"
RELEASEDIR="$4"

INC="build.names.inc"

# 1 - data name
# 2 - sign name
sign_file() {
	data="$1"
	sign="$2"
	openssl dgst -passin pass:$PASSPEM -sha256 -sign $PRIVATEKEY -out $data.sign $data
	openssl enc -base64 -in $data.sign -out $sign
	rm -rf $data.sign
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

cd $WORKDIR
WORKDIR=`pwd`

# names of spl, mlo, etc
. $INC

# Создаем архив системы
zip -j $RELEASEDIR/XPLCX_sys_v${FULLVERS}.zip \
	${_spl} ${_mlo}  ${_uboot} \
	${_dtbs} ${_krnl} ${_ubi} \
	XPLCX.spl XPLCX.mlo XPLCX.img \
	release changelog.txt cmd-line.sh

# Создаем архив обновления

# extract key-pair
PRIVATEKEY="$WORKDIR/.keys/companytag.key"
install -d -m 700 .keys
cd .keys
gpg --batch --yes --passphrase $PASSPEM --output keys.tar -d ../keys
tar -xf keys.tar

# list update-files names
#
# ...
# update.update
#
if [ -f $PRIVATEKEY ]; then
	TMPUPDATE="tmpupdate"
	mkdir $TMPUPDATE
	TMPUPDATE=`pwd`/$TMPUPDATE

	cp $WORKDIR/XPLCX.mlo			$TMPUPDATE/MLO-XPLCX.mlo
	ln -sf	MLO-XPLCX.mlo			$TMPUPDATE/MLO-XPLCX-1.mlo
	ln -sf	MLO-XPLCX.mlo			$TMPUPDATE/MLO-XPLCX-2.mlo
	#
	cp $WORKDIR/XPLCX.img			$TMPUPDATE/u-boot-XPLCX.img.uboot
	ln -sf	u-boot-XPLCX.img.uboot	$TMPUPDATE/u-boot-XPLCX-1.img.uboot
	ln -sf	u-boot-XPLCX.img.uboot	$TMPUPDATE/u-boot-XPLCX-2.img.uboot
	#
	cp $WORKDIR/XPLCX-1.dtb			$TMPUPDATE/zImage-XPLCX-1.dtb
	cp $WORKDIR/XPLCX-2.dtb			$TMPUPDATE/zImage-XPLCX-2.dtb
	#
	cp $WORKDIR/k.img.update		$TMPUPDATE/kernel.fs

	cd $TMPUPDATE
	#
	UPDFILES="zfiles.update.tar"
	tar -cvf ../${UPDFILES} *
	# clean up
	rm -rf *

	mv $WORKDIR/update.sh 			$TMPUPDATE/update.sh
	mv ../${UPDFILES} 				$TMPUPDATE/${UPDFILES}

	# compute tar hash and write to update script
	upd="$UPDFILES"
	sum=`md5sum $upd | cut -d" " -f1`
	upd=`echo "$upd" | tr '.' "_"`
	write_prop "$upd" "$sum" "update.sh" "6"

	# sign
	sign_file "update.sh" "update.sh.sign"
	sign_file "$UPDFILES" "$UPDFILES.sign"
	#
	cp $WORKDIR/release				$TMPUPDATE/release
	cp $WORKDIR/changelog.txt		$TMPUPDATE/changelog.txt
	zip -j $RELEASEDIR/XPLCX_upd_v${FULLVERS}.zip *
else
	echo "key sign FAIL"
	exit 1
fi

exit 0



