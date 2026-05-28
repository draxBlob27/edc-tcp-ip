#!/bin/bash
for i in $(ip tuntap list | awk -F: '{print $1}'); do
    sudo ip link delete "$i" && echo "Deleted $i interface"
done