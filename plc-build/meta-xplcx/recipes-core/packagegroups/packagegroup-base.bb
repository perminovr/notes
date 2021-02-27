DESCRIPTION = "Additional packages"
LICENSE = "COPYRIGHT"
PR = "-e"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

HMI_BASE_GROUP = " \
	kernel-base opkg opkg-collateral libgcc \
	ethtool \
	libstdc++ libbz2 \
	libnl libnl-cli libnl-route libnl-nf libnl-genl libnl-idiag \
	module-init-tools-depmod \
	libpcap tcpdump \
	libcrypto libssl \
	libcli libudev \
	libarchive \
	libxml2 \
	libpcre \
	udev-extraconf \
	mtd-utils \
"


RDEPENDS_${PN} = " \
	${HMI_BASE_GROUP} \ 
"
