#!/bin/bash

ip link add name $1 type bridge
ip addr add $2 dev $1
ip link set $1 up
# dnsmasq --interface=$1 --bind-interfaces --dhcp-range=172.20.1.2,172.20.1.254

modprobe tun

if [ ! -d /etc/qemu ]
then
    mkdir /etc/qemu
fi

echo allow all > /etc/qemu/bridge.conf

sysctl net.ipv4.ip_forward=1
sysctl net.ipv6.conf.default.forwarding=1
sysctl net.ipv6.conf.all.forwarding=1

iptables -t nat -A POSTROUTING -o wlp1s0 -j MASQUERADE
iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $1 -o wlp1s0 -j ACCEPT
