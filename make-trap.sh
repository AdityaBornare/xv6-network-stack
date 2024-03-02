#!/bin/bash

modprobe tun
ip tuntap add tap0 mode tap
ip addr add 192.168.100.1/24 dev tap0
ip link set tap0 up

sysctl net.ipv4.ip_forward=1

iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE
iptables -I FORWARD 1 -i tap0 -j ACCEPT
iptables -I FORWARD 1 -o tap0 -m state --state RELATED,ESTABLISHED -j ACCEPT

