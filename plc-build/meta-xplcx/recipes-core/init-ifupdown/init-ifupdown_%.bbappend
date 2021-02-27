FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
PR_append = "-e"

SRC_URI_append = " \
           file://init \
"
