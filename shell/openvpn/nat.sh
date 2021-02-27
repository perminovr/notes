#!/bin/sh

#https://interface31.ru/tech_it/2020/02/osnovy-iptables-dlya-nachinayushhih-chast-1.html
#https://www.k-max.name/linux/netfilter-iptables-v-linux/

nat_reset() {
	echo nat_reset
	# Сбрасываем настройки брандмауэра
	iptables -F
	iptables -X
	iptables -t nat -F
	iptables -t nat -X
}

nat_setup() {
	echo nat_setup

	# transit thru vpn server
	iptables -A FORWARD -i tun0 -j ACCEPT

	# access to tcp server via tunnel
	# 10.14.0.91 - local vpn server addr
	# 20.21.0.2 - tun addr on tcp server
	iptables -A PREROUTING -p tcp -d 10.14.0.91 --dport 4444 -j DNAT --to-destination 20.21.0.2:4444 -t nat

	# internet
	iptables -A POSTROUTING -o ethd0 -j MASQUERADE -t nat

	# Разрешаем инициированные нами подключения извне
	#iptables -A INPUT -i $IFACE -m state --state ESTABLISHED,RELATED -j ACCEPT
	# Разрешаем подключения по SSH
	#iptables -A INPUT -i $IFACE -p tcp --dport 22 -j ACCEPT
	# Разрешаем подключения к OpenVPN
	#iptables -A INPUT -i $IFACE -p udp --dport 1194 -j ACCEPT
	#Запрещаем входящие извне
	#iptables -A INPUT -i $IFACE -j DROP
	# Разрешаем инициированные нами транзитные подключения извне
	#iptables -A FORWARD -i $IFACE -o tunl0 -m state --state ESTABLISHED,RELATED -j ACCEPT
	# Запрещаем транзитный трафик извне
	#iptables -A FORWARD -i $IFACE -o tunl0 -j DROP
	# Включаем маскарадинг для локальной сети
	#iptables -t nat -A POSTROUTING -o $IFACE -s 10.24.0.0/24 -j MASQUERADE
}

nat_reset
nat_setup


