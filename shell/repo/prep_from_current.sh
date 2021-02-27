#!/bin/bash

svn_repo="http://trac.plant.elesy.com/usvn/svn/tm7-project"
BUILD_ENV_FILE=".build_script_env"

Usage() {
	echo "\
		Prepare workspace from svn-current:
		-d | --device
			device name from svn repo (eg 'hmi01', 'elsyma-m01')
		[--local-meta DIR]
			choose meta-working directory
			def: DEVICE-ti			
		[--local-u-boot DIR]
			choose meta-u-boot directory
			def: DEVICE-u-boot
		[--local-project DIR]	
			choose meta-project directory
			def: DEVICE-project
	"
	exit 1
}



while true; do
	case "$1" in
	-d | --device)
		DEVICE="$2"; shift 2
	;;
	--local-meta)
		meta_devdir="$2"; shift 2
	;;
	--local-u-boot)
		uboot_devdir="$2"; shift 2
	;;
	--local-project)
		proj_devdir="$2"; shift 2
	;;
	--help)
		Usage		
	;;
	* )
		break
	;;
	esac
done



YOCTO_META_NAME="meta-elesy-${DEVICE}"
trunk="${svn_repo}/trunk/boards/${DEVICE}"
[ "$meta_devdir" == "" ] && meta_devdir="${DEVICE}-ti"
[ "$uboot_devdir" == "" ] && uboot_devdir="${DEVICE}-u-boot"
[ "$proj_devdir" == "" ] && proj_devdir="${DEVICE}-project"
META_SVN_LOCAL="${meta_devdir}/sources/${YOCTO_META_NAME}"
UBOOT_SVN_LOCAL="${uboot_devdir}"
PROJ_SVN_LOCAL="${proj_devdir}"
BUILD_SVN_LOCAL="."



svn ls -R $trunk > .svnls
if [ "$?" != "0" ]; then
	echo "Error! $trunk not found!"
	echo
	Usage
fi
#
BDIRS=`svn ls $trunk`
BLDFILES=`grep "build" .svnls | grep "/[^$]"`
#
rm -rf .svnls



echo "WARNING! All files from the local repo copy will be lost!"
echo "WARNING! You should commit all changes!"
read -n 1 -p "continue? [y/n/q]: " ucmd
[ "$ucmd" == "q" ] && exit 1
if [ "$ucmd" == "y" ]; then
	#
	rm -rf ${META_SVN_LOCAL} ${UBOOT_SVN_LOCAL} ${PROJ_SVN_LOCAL}
	for bldf in ${BLDFILES} ; do
		bldf=`basename $bldf`
		rm -rf $bldf
	done
	rm -rf .svn ${BUILD_ENV_FILE}
	#
	mkdir -p ${META_SVN_LOCAL} 
	mkdir -p ${UBOOT_SVN_LOCAL} 
	mkdir -p ${PROJ_SVN_LOCAL}
	for bd in ${BDIRS} ; do
		case "$bd" in
		*meta*)
			DEST="${META_SVN_LOCAL}"
		;;
		*proj*)
			DEST="${PROJ_SVN_LOCAL}"
		;;
		*boot*)
			DEST="${UBOOT_SVN_LOCAL}"
		;;
		*build*)
			DEST="${BUILD_SVN_LOCAL}"
		;;
		*)
			DEST=""
		;;
		esac
		[ "$DEST" != "" ] && svn co ${trunk}/$bd ${DEST}/
	done	
fi

exit 0


