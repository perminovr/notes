#!/bin/sh

if [ ! -f start.sh.init  ]; then

touch start.sh.init

insmod /lib/modules/4.19.79-rt28e-g5baf382c8f-0419/kernel/drivers/net/tun.ko

ip a del 10.8.1.81/20 dev vlan2
ip a add 10.24.1.55/16 dev vlan2

echo "telnet stream tcp nowait root telnetd" > /var/run/inetd.conf_op ; echo "ftp stream tcp nowait root ftpd ftpd -w /home/root" >> /var/run/inetd.conf_op ; if [ -f /etc/init.d/inetdctl.sh ]; then /etc/init.d/inetdctl.sh ; else start-stop-daemon -K -x /usr/sbin/inetd ; start-stop-daemon -S -b -x /usr/sbin/inetd -- /var/run/inetd.conf_op ; fi

fi

ip ro del default
openvpn --config client.conf &

sleep 5
ip ro add default dev tun0

