
PR_append = "-e"

pkg_postinst_${PN} () {
	:
	exit 0
}


do_install_append() {
	# remove display from system output tty
	sed -i "/tty1$/d" ${D}${sysconfdir}/inittab
}
