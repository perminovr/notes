# look for files in this layer first
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PR_append = "-e9"

SRC_URI += " \
	file://dhcpc.patch \
	file://ftpd.patch \
	file://ifupdown.patch \
"

DEPENDS += " libpam "
RDEPENDS_${PN} += " libpam "

#SRC_URI += " \
#	file://inetd \
#	file://inetd.conf \
#"
#
