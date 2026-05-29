#!/bin/bash
echo "Use: ./setup_tap.sh <ifname> <ipaddr> <macaddr>" &&
echo "Interface name: $1" &&

echo "Created persistant virtual interface: $1" &&
sudo ./build/debug/src/tuntap_if $1 && 

sudo ip link set dev $1 address $3
sudo ip addr add $2/24 dev $1 &&
sudo ip link set up dev $1 && 

echo "Attaching packet dumper to interface $1" &&
./build/debug/tools/packet_dumper $1 &&

echo "$1 up and attached"