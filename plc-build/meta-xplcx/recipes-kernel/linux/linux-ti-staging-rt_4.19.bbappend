MACHINE_KERNEL_PR_append = "-e0419"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-4.19:"

KERNEL_LOCALVERSION_append = "-0419"

KERNEL_IMAGETYPE = "zImage"

SRC_URI_append_ti33x += " \
	file://0002-ETH100.patch \
	file://0004-MDIO.patch \
	file://am335x-e.dtsi \
	file://defconfig \
	file://hmi01.dts \
	file://vs04.dts \
	file://drm_vm.patch \
"


KERNEL_DEVICETREE = " hmi01.dtb vs04.dtb "

DTS_LIST = " am335x-e.dtsi "
DTS_LIST += " hmi01.dts vs04.dts "

do_setup_dts() {
	for _dts in ${DTS_LIST}; do
		cp -rf ${WORKDIR}/${_dts} ${S}/arch/arm/boot/dts/
	done
}


do_setup_config() {
	cp -f ${WORKDIR}/defconfig	${S}/arch/arm/configs/mtax_am335x_defconfig
	cp -f ${WORKDIR}/defconfig	${B}/.config
}


do_patch[postfuncs] += " do_setup_dts do_setup_config "
