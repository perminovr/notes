PR_append = "-e"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \	
	file://0001-user.patch \
	file://0002-hmi.h.patch \
	file://0003-version.patch \
"
