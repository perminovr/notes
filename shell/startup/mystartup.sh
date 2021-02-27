#!/bin/sh

#/etc/systemd/system/mystartup.service
#systemctl enable mystartup.service

start() {
}

stop() {
}

case $1 in
	start|stop) "$1" ;;
esac

exit 0

