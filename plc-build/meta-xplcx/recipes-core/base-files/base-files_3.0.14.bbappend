# look for files in this layer first
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PR_append = "-e4"

dirs755 += "\
/media/ram \
"
