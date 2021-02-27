FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

PR_append = "-e"

SRC_URI += " \
	file://sysctl.conf \
	file://volatiles.e \
	file://hmistartup \
	file://userapp \
	file://setupuserapp \
"

do_install_append () {
	install -d ${D}${sysconfdir}/modules-load.d
	install -d ${D}${sysconfdir}/profile.d

	install -m 0640    ${WORKDIR}/sysctl.conf    	${D}${sysconfdir}/sysctl.conf
	install -m 0640    ${WORKDIR}/volatiles.e	${D}${sysconfdir}/default
	
	install -m 0750    ${WORKDIR}/hmistartup    	${D}${sysconfdir}/init.d
	install -m 0750    ${WORKDIR}/userapp    	${D}${sysconfdir}/init.d
	install -m 0750    ${WORKDIR}/setupuserapp    	${D}${sysconfdir}/init.d

	update-rc.d -r ${D} hmistartup start 08 S .
	update-rc.d -r ${D} setupuserapp start 99 S .
}
