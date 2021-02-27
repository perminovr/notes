#!/bin/bash

# BUILD PROPERTIES

# buildable device name
DEVICE="plc"
# suffix after dir name (must not start from '-')
# ex: elsyma-m01-meta-companytag_cs16
build_suffix=""
meta_suffix=""
proj_suffix=""
boot_suffix=""

Usage() {
	echo "\
		[-m | --meta DIR]
			choose meta-working directory
			def: ${DEVICE}-ti
		[-u | --uboot DIR]
			choose meta-u-boot directory
			def: ${DEVICE}-u-boot
		[-p | --project DIR]
			choose meta-project directory
			def: ${DEVICE}-project
		[-b | --branches DATA REV]
			choose svn branches revision and data
			def: create branches
		[--u_XPLCX DIR]
			choose meta-u-boot-XPLCX directory
			def: XPLCX-u-boot
	"
	exit 1
}


# SCRIPT

# all variables with init-prefix '[ "$var" == "" ] &&' could be found here:
if [ -f .build_script_env ]; then
	. .build_script_env
fi



# build version
vbuild=$(cat version)
# yocto meta receipes name
YOCTO_META_NAME="meta-${DEVICE}"
REALDATE=`date "+%Y-%m-%d"`
#
[ "$DATE" == "" ] && DATE=$REALDATE
[ "$pathsvn" == "" ] && pathsvn="http://example.com/usvn/svn/project"
[ "$trunkpath" == "" ] && trunkpath="$pathsvn/trunk/boards/${DEVICE}"
[ "$trunkp_meta" == "" ] && trunkp_meta="${DEVICE}-meta-companytag${meta_suffix}"
[ "$trunkp_boot" == "" ] && trunkp_boot="${DEVICE}-u-boot${boot_suffix}"
[ "$trunkp_proj" == "" ] && trunkp_proj="${DEVICE}-project${proj_suffix}"
[ "$trunkp_build" == "" ] && trunkp_build="${DEVICE}-build${build_suffix}"
[ "$trunkp_bootXPLCX" == "" ] && trunkp_bootXPLCX="XPLCX-u-boot"
#
realrev=`svn info $pathsvn | grep ^Revision | sed "s#Revision: ##"`
rev="$rev$realrev" # 'rev' could be already set
#
msg="$REALDATE release dir $vbuild.$rev was made automatically"
branchdir_dev="$pathsvn/branches/boards/${DEVICE}"
branchdir="$branchdir_dev/$vbuild"



svn ls "$branchdir_dev"
if [ "$?" != "0" ]; then
	echo
	echo "Error! Need to create $branchdir_dev"
	echo
	exit 1
fi



# main build directories
branch="1"
#
meta_devdir=""
syswork_devdir="${DEVICE}-ti"
uboot_devdir="${DEVICE}-u-boot"
proj_devdir="${DEVICE}-project"
uboot_XPLCX="XPLCX-u-boot"
#
while true; do
	case "$1" in
	-m | --meta)
		syswork_devdir="$2"; shift 2
	;;
	-u | --uboot)
		uboot_devdir="$2"; shift 2
	;;
	--u_XPLCX)
		uboot_XPLCX="$2"; shift 2
	;;
	-p | --project)
		proj_devdir="$2"; shift 2
	;;
	-b | --branches)
		DATE="$2"
		rev="$3"
		branch="0"
		shift 3
	;;
	--help)
		Usage
		break
	;;
	* )
		break
	;;
	esac
done
meta_devdir="${syswork_devdir}/sources/${YOCTO_META_NAME}"

# Установка прав для общих файлов и директорий
umask 0002



# работа с репозиториями
# 1 - trunk path (source)
# 2 - local folder
get_sources () {
	if [ -d ${2} ]; then
		svn up ${2}
		svn st ${2} | grep ^M
		if [ "`svn st ${2} | grep ^M`" != "" ]; then
			echo "WARNING! You have changes in ${2}!"
			read -n 1 -p "commit them? [y/n] (do not press 'enter'): " ucmd
			[ "$ucmd" == "y" ] && svn ci -m "$msg" ${2}
		fi
	else
		svn co ${1} ${2}
	fi	
}

# 1 - from (trunk)
# 2 - to (branch)
# 3 - revision for externals
do_branch () {
	# new branch
	svn cp --parents -m "$msg" ${1} ${2}
	# externals HEAD -> REV
	if [ "$3" != "" ]; then
		svn pg svn:externals ${2} > props
		if [ "`cat props`" != "" ]; then
			sed -e "s#\(^.*\) #\1@${3} #" props > newprops
			svnmucc propsetf -m "$msg" svn:externals newprops ${2}
			rm -rf newprops props
		fi
	fi
}

# получение скрипта распаковки рецептов yocto
if ! [ -d oe-layersetup/ ]; then
	git clone git://example.com/sdk/oe-layersetup.git
fi

# распаковка рецептов yocto
oe-layersetup/oe-layertool-setup.sh -f oe-layersetup/configs/config.txt -d /opt/downloads -b ${syswork_devdir} -a ${YOCTO_META_NAME}

# Получение исходников из репозитория
get_sources "${trunkpath}/${trunkp_meta}/" "${meta_devdir}"
get_sources "${trunkpath}/${trunkp_boot}/" "${uboot_devdir}"
get_sources "${trunkpath}/${trunkp_proj}/" "${proj_devdir}"
get_sources "${trunkpath}/${trunkp_bootXPLCX}/" "${uboot_XPLCX}"

vmeta=$(cat ${meta_devdir}/base_version)
vboot=$(cat ${uboot_devdir}/base_version)
vproj=$(cat ${proj_devdir}/base_version)
vbootXPLCX=$(cat ${uboot_XPLCX}/base_version)
# подготовка branch в репозитории
if [ "$branch" == "1" ]; then
	svn mkdir -m "$msg" "$branchdir"
	#
	[ "$brname_meta_templ" == "" ] && brname_meta_templ="${trunkp_meta}-${DATE}-v$vmeta"
	[ "$brname_boot_templ" == "" ] && brname_boot_templ="${trunkp_boot}-${DATE}-v$vboot"
	[ "$brname_proj_templ" == "" ] && brname_proj_templ="${trunkp_proj}-${DATE}-v$vproj"
	[ "$brname_build_templ" == "" ] && brname_build_templ="${trunkp_build}-${DATE}-v$vbuild"
	[ "$brname_boot_templXPLCX" == "" ] && brname_boot_templXPLCX="${trunkp_bootXPLCX}-${DATE}-v$vbootXPLCX"
	#
	do_branch "${trunkpath}/${trunkp_meta}/" "$branchdir/${brname_meta_templ}.$rev" "$realrev"
	do_branch "${trunkpath}/${trunkp_boot}/" "$branchdir/${brname_boot_templ}.$rev" "$realrev"
	do_branch "${trunkpath}/${trunkp_proj}/" "$branchdir/${brname_proj_templ}.$rev" "$realrev"
	do_branch "${trunkpath}/${trunkp_build}/" "$branchdir/${brname_build_templ}.$rev" "$realrev"
	do_branch "${trunkpath}/${trunkp_bootXPLCX}/" "$branchdir/${brname_boot_templXPLCX}.$rev" "$realrev"
fi

main_dir=$(pwd)
deploy_img="${syswork_devdir}/build/arago-tmp-external-arm-toolchain/deploy/images/mtax_am335x"


# подготовка среды для сборки
# версии
corepath="${main_dir}/${meta_devdir}/recipes-codesys/companytag-core-arm"
echo "BRDATE = \"$DATE\"" > $corepath/companytagversion.inc
echo "COMPANYTAG_BASEREV = \"$rev\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_REALREV = \"$realrev\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_PROJVER = \"$vproj\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_BOOTVER = \"$vboot\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_METAVER = \"$vmeta\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_BUILDVER = \"$vbuild\"" >> $corepath/companytagversion.inc
echo "COMPANYTAG_BOOTVER_XPLCX = \"$vbootXPLCX\"" >> $corepath/companytagversion.inc

initpath="${main_dir}/${meta_devdir}/recipes-core/initscripts/initscripts"
echo "$vbuild.$realrev" > $initpath/companytag_version

# u-boot fw_util
#cp -f $main_dir/${uboot_devdir}/recipes-bsp/u-boot/u-boot/*.patch \
#		${main_dir}/${meta_devdir}/recipes-bsp/u-boot/u-boot-fw-utils/
#cp -f $main_dir/${uboot_devdir}/recipes-bsp/u-boot/u-boot/*.h \
#		${main_dir}/${meta_devdir}/recipes-bsp/u-boot/u-boot-fw-utils/
#uboot_recipe="$main_dir/${uboot_devdir}/recipes-bsp/u-boot/u-boot-am33x_2013.01.01.bbappend"
#uboot_fw_inc="${main_dir}/${meta_devdir}/recipes-bsp/u-boot/companytag-u-boot.inc"
#uboot_fw_script="${main_dir}/${meta_devdir}/getUbootPatches.py"
#python ${uboot_fw_script} -f $uboot_recipe > $uboot_fw_inc
#echo "COMPANYTAG_BASEREV = \"$rev\"" >> $uboot_fw_inc
#echo "COMPANYTAG_REALREV = \"$realrev\"" >> $uboot_fw_inc
#echo "COMPANYTAG_BOOTVER = \"$vboot\"" >> $uboot_fw_inc


# сборка системы
cd ${main_dir}/${syswork_devdir}/build
echo
echo " ### Prepared for image building ### "
echo
. conf/setenv
MACHINE=mtax_am335x ARAGO_RT_ENABLE=1 ARAGO_SYSVINIT=1 bitbake -k core-image-minimal
if [ "$?" != "0" ]; then
	echo "Build FAIL"
	exit 1
fi


# копирование утилит обновления устройства
# signverify
signverify_bin="${main_dir}/${meta_devdir}/signverify"
if [ -f $signverify_bin ]; then
	cp -f ${signverify_bin} ${main_dir}/${deploy_img}/
fi
# signverify1.1
signverify_bin="${main_dir}/${meta_devdir}/signverify1.1"
if [ -f $signverify_bin ]; then
	cp -f ${signverify_bin} ${main_dir}/${deploy_img}/
fi
# update.sh
update_sh="${main_dir}/${meta_devdir}/recipes-core/initscripts/initscripts/update.sh"
if [ -f $update_sh ]; then
	cp -f ${update_sh} ${main_dir}/${deploy_img}/
fi


# запрос паролей
echo "$USER@buildserver's password:"
read -s passw
echo "Enter pass phrase for pem key: "
read -s pempassw


# создание образа обновления
bash ${main_dir}/${meta_devdir}/make-image.sh $pempassw $main_dir/keys
if [ "$?" != "0" ]; then
	echo "Make image FAIL"
	exit 1
fi


device_boot="${DEVICE}-u-boot_v${vboot}"
device_bootXPLCX="XPLCX-u-boot_v${vbootXPLCX}"
serv="ftp://$USER:$passw@buildserver/$main_dir"
pathimage="$serv/${deploy_img}"
release_dir="${DEVICE}_v${vbuild}.${rev}"

# сброс маски
umask 0022

# 8. Выполним команды на удаленном сервере
ssh $USER@buildserver2 << EOF
	cd /home/hcs/tftp

	mkdir ${release_dir}
	install -d -m 0700 ${release_dir}/workdir
	cd ${release_dir}
	
	# Установка имен файлов сборки
	echo 	_spl=\"${DEVICE}.spl\"		>> workdir/build.names.inc
	echo 	_mlo=\"${DEVICE}.mlo\"		>> workdir/build.names.inc
	echo 	_uboot=\"${DEVICE}.img\"	>> workdir/build.names.inc
	echo 	_dtbs=\"XPLCX-1.dtb XPLCX-2.dtb\" >> workdir/build.names.inc
	echo 	_krnl=\"${DEVICE}.bin\"		>> workdir/build.names.inc
	echo 	_ubi=\"${DEVICE}.ubi\"		>> workdir/build.names.inc
	echo 	_fs_tar=\"${DEVICE}.tar.gz\"	>> workdir/build.names.inc
	
	. workdir/build.names.inc
	_dtbs_a=(\${_dtbs})
	
	# Копируем файлы u-boot из локальной папки
	cp ../$device_boot/"\$(readlink "../$device_boot/u-boot-spl.bin")" 			\${_spl}
	cp ../$device_boot/"\$(readlink "../$device_boot/MLO")" 					\${_mlo}
	cp ../$device_boot/"\$(readlink "../$device_boot/u-boot.img")" 				\${_uboot}
	cp ../$device_bootXPLCX/"\$(readlink "../$device_bootXPLCX/u-boot-spl.bin")" 	XPLCX.spl
	cp ../$device_bootXPLCX/"\$(readlink "../$device_bootXPLCX/MLO")" 				XPLCX.mlo
	cp ../$device_bootXPLCX/"\$(readlink "../$device_bootXPLCX/u-boot.img")" 		XPLCX.img
	
	# Копируем файлы системы из сервера сборки
	wget -q --retr-symlinks -P . -np -nd -m -nH $pathimage/XPLCX-1.dtb 			-O \${_dtbs_a[0]}
	wget -q --retr-symlinks -P . -np -nd -m -nH $pathimage/XPLCX-2.dtb 			-O \${_dtbs_a[1]}
	wget -q --retr-symlinks -P . -np -nd -m -nH $pathimage/zImage 								-O \${_krnl}
	wget -q --retr-symlinks -P . -np -nd -m -nH $pathimage/core-image-minimal-mtax_am335x.ubi 		-O \${_ubi}
	wget -q --retr-symlinks -P . -np -nd -m -nH $pathimage/core-image-minimal-mtax_am335x.tar.xz 	-O \${_fs_tar}
	wget -q --retr-symlinks=no -P . -np -nd -m -nH $serv/cmd-line.sh
	
	# Распаковываем файл release и changelog.txt
	tar -xvf \${_fs_tar} ./boot/release --strip=2
	tar -xvf \${_fs_tar} ./boot/changelog.txt --strip=2

	# Копируем скрипт для работы tftp
	rm -rf /home/hcs/tftp/scripts/tftp_${DEVICE}.sh
	wget -q --retr-symlinks=no -P . -np -nd -m -nH $serv/tftp_${DEVICE}.sh
	
	# Копируем файлы для обновления
	wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $serv/keys
	wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $serv/make-release.sh
	wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $serv/${syswork_devdir}/build/k.img.update
	#wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $pathimage/companytag-utils.tar
	#wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $pathimage/signverify
	#wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $pathimage/signverify1.1
	wget -q --retr-symlinks=no -P workdir -np -nd -m -nH $pathimage/update.sh
	cp \${_spl} 		workdir
	cp \${_mlo} 		workdir
	cp \${_uboot} 		workdir
	cp XPLCX.spl 		workdir
	cp XPLCX.mlo  		workdir
	cp XPLCX.img 		workdir
	cp \${_dtbs} 		workdir
	cp \${_krnl} 		workdir
	cp \${_ubi} 		workdir
	cp release 			workdir
	cp changelog.txt 	workdir
	cp cmd-line.sh 		workdir
	
	# Создаем архив системы
	bash workdir/make-release.sh "workdir" $pempassw "$vbuild.$realrev" "/home/hcs/releases"
	
	# Удаляем временную директорию
	rm -rf workdir

	# Опубликуем систему на ФТП
	mv tftp_${DEVICE}.sh /home/hcs/tftp/scripts/tftp_${DEVICE}.sh
	cd /home/hcs/tftp
	bash scripts/tftp_${DEVICE}.sh "/home/hcs/tftp" "${release_dir}"
	bash scripts/g.sh
EOF

echo "------------------------------------------"
echo "Build ${DEVICE}: ${DEVICE}_v$vbuild.$rev successfully"
echo "------------------------------------------"
