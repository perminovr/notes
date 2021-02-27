# look for files in this layer first
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PR_append = "-e3"


SRC_URI += " file://rcS-default \
          "
