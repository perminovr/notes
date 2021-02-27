DESCRIPTION = "Additional packages"
LICENSE = "COPYRIGHT"
PR = "-e"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup


HMI_SSH_GROUP = " \
	dropbear \
	openssh-sftp-server \
"


HMI_PYTHON_GROUP = " \
	python3 \
	python3-pyserial \
	 \
	python3-io \
	python3-shell \
	python3-netclient \
"

#python3-enum
#python3-argparse


RDEPENDS_${PN} = " \
	${HMI_SSH_GROUP} \ 
"
#${HMI_PYTHON_GROUP} 
