#!/bin/bash
sudo ip addr add 192.168.0.1/24 dev tap0 &&
sudo ip link set up dev tap0 && 
echo "tap0 setup done"