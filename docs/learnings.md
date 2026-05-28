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

