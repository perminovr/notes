# look for files in this layer first
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

require fixrootfs.inc

PR_append = "-e16"

IMAGE_INSTALL_append = "\
	packagegroup-arago-sysvinit-boot \
	u-boot-fw-utils \
	packagegroup-hmi-base \
	packagegroup-hmi-hmi \
	kbd \
	gdbserver \
	packagegroup-hmi-graphics \
	packagegroup-arago-qte \
	iw wpa-supplicant \
"
#core-esp

#u-boot-fw-utils
#do not install into rootfs (make a single tar for update purpose only)
PACKAGE_EXCLUDE_append += " u-boot-fw-utils "


IMAGE_FEATURES += " package-management read-only-rootfs "
LINUX_KERNEL_TYPE = "preempt-rt"
KERNEL_IMAGETYPE = "zImage"

UBI_VOL1NAME ?= "${MACHINE}-workfs"

IMAGE_CMD_ubi () {
	echo \[ubifs\] > ubinize.cfg 
	echo mode=ubi >> ubinize.cfg
	echo image=${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.rootfs.ubifs >> ubinize.cfg 
	echo vol_id=0 >> ubinize.cfg 
	echo vol_type=dynamic >> ubinize.cfg 
	echo vol_name=${UBI_VOLNAME} >> ubinize.cfg 
	echo vol_size=100MiB >> ubinize.cfg
	echo >> ubinize.cfg
	echo [work-volume] >> ubinize.cfg
	echo mode=ubi >> ubinize.cfg
	echo vol_id=1 >> ubinize.cfg
	echo vol_size=10MiB >> ubinize.cfg
	echo vol_type=dynamic >> ubinize.cfg
	echo vol_name=${UBI_VOL1NAME} >> ubinize.cfg
	echo vol_flags=autoresize >> ubinize.cfg

	# del old stuff
	rm -rf ${DEPLOY_DIR_IMAGE}/*.ubi

	mkfs.ubifs -r ${IMAGE_ROOTFS} -o ${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.rootfs.ubifs ${MKUBIFS_ARGS} && ubinize -o ${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.rootfs.ubi ${UBINIZE_ARGS} ubinize.cfg

	# last compiled ubifs will be restored -> del stuff
	rm -rf ${DEPLOY_DIR_IMAGE}/*.ubifs 

	# mk link to ubi (ubifs will be created later automatically)
	ln -sf ${IMAGE_NAME}${IMAGE_NAME_SUFFIX}.ubi ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.ubi

}

addtask do_fixrootfs after do_rootfs before do_image



