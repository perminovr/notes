
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
PR_append = "-e1"

SRC_URI += " \
           file://weston.ini \
"
