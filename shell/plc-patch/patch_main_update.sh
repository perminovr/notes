#!/bin/sh
MYNAME="tc71x_v3.2.2.10775_patch1"
MYVER="001"

# updates m5sum
updates_list="d4703204e85d435f95c6e53d29f14d44"
libnetlinklayer_so="a8706c08efdc3f7a2b7b337f8eaadec1"
networking="fe0936e43aa536651db0ab9dc341dae4"
elesyfunctions_sh="7d371e7405cfe2f123ae1a96a4e42f6d"



# base paths
MYPATH=/home/root/$MYNAME
CSPATH=/home/root/CoDeSysSP
CHPATH=/home/root/Channel
SOPATH=/home/root/Libs
FILESPATH=/home/root/update
UPDATEPATH=/var/run/patch_update
RESULTPATH=/var/run/update-script-result
UPDATEDIR="elsytmkupdate"
SYSUP="sysup"
if test -e /dev/sda1 ; then
	SDAPATH=/run/media/sda1
elif test -e /dev/sda ; then
	SDAPATH=/run/media/sda
else
	SDAPATH=
fi

# logging
LOGDIR="${SDAPATH}/${UPDATEDIR}"
LOGFILE="${LOGDIR}/update.log"
log() {
	echo "$1"
	if [ -d "$LOGDIR" ]; then
		echo "$1" >> "$LOGFILE"
	fi
}


cleanup() {
	log "cleanup"
	rm -rf $FILESPATH $UPDATEPATH
	sync
}

RESULTFILE="${LOGDIR}/result.txt"
model=`cat /proc/device-tree/model`
exit_ok() {
	cleanup
	log "$MYNAME: patched OK"
	if [ -d "$LOGDIR" ]; then
		echo "$model " `hostname` " ${MYNAME}_v${MYVER}: patched OK" >> $RESULTFILE
	fi
	echo 0 > $RESULTPATH
	exit 0
}

exit_fail() {
	cleanup
	if [ -d "$LOGDIR" ]; then
		echo "$model " `hostname` "${MYNAME}_v${MYVER}: patching FAIL" >> $RESULTFILE
	fi
	# fail, load system
	echo 2 > $RESULTPATH
	exit 2
}


log "$MYNAME: main_update.elesysh started"


# system version checking
SYSVER="`echo $MYNAME | sed "s/.*_v\(.*\)_.*/\1/"`"
if [ -f /home/root/Share/build.version ]; then
	REALSYSVER="`cat /home/root/Share/build.version | tr -d '\n'`"	
elif [ -f /home/root/release ]; then
	REALSYSVER="`cat /home/root/release | tr -d '\n'`"	
fi
if [ "$SYSVER" != "$REALSYSVER" ]; then
	log "  require version is $SYSVER but system version is $REALSYSVER"
	exit_fail
fi


# previous patch version checking
if [ -f $MYPATH ]; then
	OLDVER="`cat $MYPATH | head -1 | tr -d '\n'`"
	if [ -n "$OLDVER" -a -n "$MYVER" ]; then
		if [ $OLDVER -eq $MYVER ]; then
			log "  $MYNAME with ver $MYVER already done"
			exit_ok
		elif [ $OLDVER -gt $MYVER ]; then
			log "  installed version $OLDVER greater than $MYVER: fatal error"
			exit_fail
		fi
	else
		log "  couldn't get versions [$OLDVER] [$MYVER]"
		exit_fail	
	fi
fi


log "  zip unpacking started"
MYZIPDIR=${SDAPATH}/${UPDATEDIR}/${SYSUP}
ZIPFILE="${MYNAME}*.zip"
for UPDZIP in `find $MYZIPDIR -type f -name "$ZIPFILE"`; do
	if [ -f "$UPDZIP" ]; then break; fi
done
if [ -f "$UPDZIP" ]; then
	mkdir -p $UPDATEPATH ; cd $UPDATEPATH
	# list all files in zip
	ziplist="`unzip -l $UPDZIP | tail -n +4 | head -n -2 | tr -s ' ' | cut -d" " -f5 | sort -u`"
	for fname in $ziplist; do
		hf=`echo $fname | tr '.' '_'`
		if [ "$hf" != "" ]; then
			eval "hash=\$$hf" # hash in this script
			# if hash is presented
			if [ "$hash" != "" ]; then
				unzip "$UPDZIP" $fname
				realhash="`md5sum $fname | cut -d" " -f1`"
				log "    $fname hash     $hash"
				log "    $fname realhash $realhash"
				if [ "$hash" == "$realhash" ]; then
					log "      $fname extracted"
				else
					rm -rf "$fname"
				fi
			fi
		fi
	done
else
	log "  zip not found"
	exit_fail
fi
log "  zip unpacking end"


if [ ! -d $UPDATEPATH ]; then
	log "$UPDATEPATH is not exist"
	exit_fail
fi
cd $UPDATEPATH


if [ ! -f updates_list ]; then
	log "updates_list not found"
	exit_fail	
fi


log "  libs copying started"
LIBSU=`find $UPDATEPATH -type f -name "*.so"`
for lib in $LIBSU; do
	lib=`basename $lib`
	LIBS="$LIBS $lib"
done
log "    Found: $LIBS"
for lib in $LIBS; do
	# CoDeSysSP
	if [ -f $CSPATH/$lib ]; then 
		log "    cp $lib to $CSPATH/$lib"
		cp -f $lib $CSPATH/$lib
	# Channel
	elif [ -f $CHPATH/$lib ]; then
		log "    cp $lib to $CHPATH/$lib"
		cp -f $lib $CHPATH/$lib
	# Libs
	else
		log "    cp $lib to $SOPATH/$lib"
		cp -f $lib $SOPATH/$lib
	fi
done
log "  libs copying done"




# specific for this patch
log "  file system fixing started"
mount -o remount,rw / /
REGFILELIST=" 
/etc/init.d/elesyfunctions.sh 
/etc/init.d/networking 
"
for rf in $REGFILELIST; do
	fname=`basename $rf`
	if [ -f $UPDATEPATH/$fname ]; then
		log "    cp $fname to $rf"
		cp -f $fname $rf
		chmod u+rwx $rf
	else
		log "    $fname not found"
		mount -o remount,ro / /
		exit_fail
	fi
done
mount -o remount,ro / /
sync
log "  file system fixing done"
# specific end




echo $MYVER > $MYPATH
DATE=`date "+%Y-%m-%d %H:%M:%S"`
echo "PLC DT -- " $DATE >> $MYPATH
cat updates_list >> $MYPATH
cat $MYPATH > $CSPATH/$MYNAME
log "$MYNAME: main_update.elesysh done"


cleanup
log "$MYNAME: patched SUCCESS"
log "reboot"
reboot
sleep 100


