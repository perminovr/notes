
PR_append = ".e1"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
	file://init \
	file://0001-max-pack-size.patch \
"
