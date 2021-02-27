
PR_append = ".e1"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://disable-shortcuts.patch \
"
