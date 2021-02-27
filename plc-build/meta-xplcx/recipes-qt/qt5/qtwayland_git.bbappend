FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
PR_append = "-e"

SRC_URI += " \
    file://del-warnings.patch \
"
