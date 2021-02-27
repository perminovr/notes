FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PR_append = "-e1"

SRC_URI += " \
	file://pam.d/common-auth \
	file://pam_userfile-src.patch \
	file://pam_userfile-build.patch \
"

RDEPENDS_${PN}-runtime += " pam-plugin-userfile-${libpam_suffix} "
