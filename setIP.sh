#!/bin/sh
if [ `uname -s` == "Darwin" ]; then
	IP=`ifconfig en0|grep "inet "| awk '{print $2}'`
	VPN=`ifconfig utun10|grep "inet "| awk '{print $2}'`
	MAC=`ifconfig en0|grep "ether "| awk '{print $2}' |  tr a-z A-Z`
else
	IP=`ifconfig eth0|grep "inet "| awk '{print $2}'`
	VPN=`ifconfig tun0|grep "inet "| awk '{print $2}'`
	MAC=`ifconfig eth0|grep "ether "| awk '{print $2}' |  tr a-z A-Z`
fi

AUTH=SD12-XF67AD

echo text 1 75% 1% 4% 66666 1 IP : $IP      >/wally/ip.scr
echo text 2 75% 6% 4% 66666 1 VPN : $VPN    >>/wally/ip.scr
echo text 3 75% 11% 4% 66666 1 MAC : $MAC   >>/wally/ip.scr
echo text 4 75% 16% 4% 66666 1 AUTH : $AUTH >>/wally/ip.scr
