#!/bin/bash

svn_repo="http://trac.plant.elesy.com/usvn/svn/tm7-project"
BUILD_ENV_FILE=".build_script_env"

Usage() {
	echo "\
		Prepare workspace from svn-hardcopy:
		-d | --device
			device name from svn repo (eg 'hmi01', 'elsyma-m01')
		-r | --revision REV
			revision number
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
	-r | --revision)
		REV="$2"; shift 2
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



if [ "$REV" == "" -o "$DEVICE" == "" ]; then
	Usage
fi
#
YOCTO_META_NAME="meta-elesy-${DEVICE}"
trunk="${svn_repo}/trunk/hardcopies/boards/${DEVICE}"
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
BDIRS=`grep "${REV}/$" .svnls`
BLDFILES=`grep "build" .svnls | grep "${REV}/[^$]"`
#
if [ "$BDIRS" == "" ]; then
	echo "Revision ${REV} not found in $trunk!"
	exit 1
fi
rm -rf .svnls



echo "WARNING! All files from the local repo copy will be lost!"
echo "WARNING! You should commit all changes youself!"
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
		basename $bd > ${DEST}/hardcopy
	done

rev="$REV."
DATE="`sed -n "s/.*build-\(.*\)-v.*/\1/p" ${BUILD_SVN_LOCAL}/hardcopy`"
pathsvn="$svn_repo"
trunkpath="$trunk/`cat ${BUILD_SVN_LOCAL}/version 2> /dev/null`"	# ${svn_repo}/trunk/hardcopies/boards/${DEVICE}/$VER
#
trunkp_meta=`cat ${META_SVN_LOCAL}/hardcopy 2> /dev/null` 			# hmi01-meta-elesy-$DATE-v${META_VER}.$REV
trunkp_boot=`cat ${UBOOT_SVN_LOCAL}/hardcopy 2> /dev/null`
trunkp_proj=`cat ${PROJ_SVN_LOCAL}/hardcopy 2> /dev/null`
trunkp_build=`cat ${BUILD_SVN_LOCAL}/hardcopy 2> /dev/null`
#
brname_meta_templ=`echo ${trunkp_meta} | sed -n "s/\.[0-9]*$//p"`
brname_boot_templ=`echo ${trunkp_boot} | sed -n "s/\.[0-9]*$//p"`
brname_proj_templ=`echo ${trunkp_proj} | sed -n "s/\.[0-9]*$//p"`
brname_build_templ=`echo ${trunkp_build} | sed -n "s/\.[0-9]*$//p"`

cat << EOF > ${BUILD_ENV_FILE}
rev="$rev"
DATE="$DATE"
pathsvn="$pathsvn"
trunkpath="$trunkpath"
trunkp_meta="$trunkp_meta"
trunkp_boot="$trunkp_boot"
trunkp_proj="$trunkp_proj"
trunkp_build="$trunkp_build"
brname_meta_templ="$brname_meta_templ"
brname_boot_templ="$brname_boot_templ"
brname_proj_templ="$brname_proj_templ"
brname_build_templ="$brname_build_templ"
EOF
	
fi

exit 0


