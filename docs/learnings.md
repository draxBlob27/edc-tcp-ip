*** How to run for now
./scripts/setup_tap.sh <ifname> <ipaddr>
tap0 <ipaddr> = 192.168.0.1, <macaddr>
tap1 <ipaddr> = 192.168.0.2, <macaddr>


1. Understanding TUN/TAP was not easy. 
    -> Best resource, slighlty hard to ingest but very good and thorugh.(https://backreference.org/2010/03/26/tuntap-interface-tutorial/)

2. Any standard is good to understand the prortocol fields. RFCs are also gud.
3. In newer kernel, tun/tap either needs to be made persistant or needs to be connected to a client for keep alive session. 
4. any interface is down by deafult -> use shell script to setup after persist using c code/ or can create
interface in script as well.
5. only multi byte fileds get affected by endianness(hton, ntoh).
6. best tutotrial to understand pointers in C. enough to read open source c codes with a lot of ptrs
    jensen ptr tutorial github v3
7. whenever ping is done, first ARP is aplied to find mac of pinged ip addr.
8. The usual case is that you know the IP address of some service in your LAN, but to establish actual communications, also the hardware address (MAC) needs to be known. Hence, ARP is used to broadcast and query the network, asking the owner of the IP address to report its hardware address.
9. Get a deep understanding of padding and memory alignment in compilers. => __attribute__((packed))
10. ARP - mapping of L3(ip) address to a L2(mac) address.(https://www.youtube.com/watch?v=QPi5Nvxaosw)
11. RFC of ARP - (https://datatracker.ietf.org/doc/html/rfc826)
12. To send a ARP reply message: what to fill in src mac: [Since this is a virtual interface, pick any locally-administered MAC, EXAMPLE: 0x52, 0x54, 0x00, 0x12, 0x34, 0x56].
A locally administered MAC address (LAA) is a MAC address where the second least significant bit of the first byte is set to 1.
13. To establish arp req-rep, i created 2 tap interfaces(namely tap0 and tap1).
    tap0 send arp-request to tap1
    tap1 sedn arp-reply to tap0 and vice versa
14. I am implementing a cache of size 256, with linear search, can use linked list with LRU eviction.
    will think upon this later, currently linaer array with naive search.
15. nread returned from reading fd, is useful when writing to the fd.
16. Unitialzed pointer variable is not a modifiable lvalue.
17. Always use mecpy when copying arrays using ptrs.