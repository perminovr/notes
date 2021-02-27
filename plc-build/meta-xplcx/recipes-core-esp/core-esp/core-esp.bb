
DESCRIPTION = "core-esp firmware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${THISDIR}/${PN}/MIT;md5=661e33d8509253d28aa96ab2d85be447"

ESP_GIT_URI = "git://github.com/espressif/ESP8266_NONOS_SDK"
ESP_GIT_PROTOCOL = "git"
BRANCH = "master"
SRCREV = "7a31cb7e48aff8f7d96fa5bccc2ba535b9c446a1"
SRC_URI += "${ESP_GIT_URI};protocol=${ESP_GIT_PROTOCOL};branch=${BRANCH}"
SRC_URI[md5sum] = "2c91913749082291d473657856eb8295"
SRC_URI[sha256sum] = "db9080eaa687e154deff57b369fb821a2a526ec632326c211bdad77c65673d8f"
PR = "r01"

S = "${WORKDIR}/git"

PACKAGES = "${PN}"

FILES_${PN} = " \
	/boot/esp-firmware \
"

do_configure () {
	cp -rf ${S}/examples/at ${S}/at
	#ln -sf ${S}/examples/at ${S}/at
}

do_compile () {
	export PATH=/opt/xtensa-lx106-elf/bin:$PATH	
	cd ${S}/at
	make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=5
}

do_install () {
	install -d ${D}/boot/esp-firmware
	install -m 0750    ${S}/bin/upgrade/user1.2048.new.5.bin  	${D}/boot/esp-firmware/user1.2048.new.5.bin
	install -m 0750    ${S}/bin/blank.bin     			${D}/boot/esp-firmware/blank.bin 
	install -m 0750    ${S}/bin/boot_v1.7.bin    			${D}/boot/esp-firmware/boot_v1.7.bin
	install -m 0750    ${S}/bin/esp_init_data_default_v08.bin   	${D}/boot/esp-firmware/esp_init_data_default_v08.bin
}

