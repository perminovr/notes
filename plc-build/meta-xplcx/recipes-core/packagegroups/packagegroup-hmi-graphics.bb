DESCRIPTION = "Additional packages"
LICENSE = "COPYRIGHT"
PR = "-e"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup


GRAPHICS_WAYLAND = "\
	weston \
	weston-init \ 
"

GRAPHICS_RDEPENDS = "\
	glib-2.0 \
	libgbm \
	ti-sgx-ddk-km \
	ti-sgx-ddk-um \
"


RDEPENDS_${PN} = " \
	${GRAPHICS_RDEPENDS} \
	arago-gpl-notice \
	arago-feed-config \
	${@bb.utils.contains('DISTRO_FEATURES', 'wayland', "${GRAPHICS_WAYLAND}", '', d)} \  
"
