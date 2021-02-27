
PR_append = ".e1"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
	file://110-enable-zlib.patch \
"
