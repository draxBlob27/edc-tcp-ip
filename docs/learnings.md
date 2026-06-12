*** How to run for now
./scripts/setup_tap.sh <ifname> <ipaddr>
tap0 <ipaddr> = 192.168.0.1, <macaddr>
tap1 <ipaddr> = 192.168.0.2, <macaddr>

./scripts/setup_linux.sh tap0 192.168.0.1 02:11:22:33:44:55
./scripts/setup_mine.sh tap0 192.168.0.254 02:AA:BB:CC:DD:EE


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
18. First time understood what do we actually mean by user space stack, and what
    tun/tap interfaces are.
    '''
            Linux
        192.168.0.254
              ^
              |
             tap0
              |
              v
            My Stack
          192.168.0.1
    '''
    The ip link commands, set linux side ip addr and mac addr.
    tun_alloc and my cl args, set the structure of my stacks ip addr and hwaddr.
19. A tap interface acts as a bridge bw them, giving my program a fd, to read raw bytes, and make linux
assume that it is communicating with another machine(on the same LAN).
20. Eralier i was allocating same (ip, mac) to both. And then my prgraming was runnig but didnt gave correct output.
21. Endianness is real bug creator, always need to pay attention, always print in host order for debugging.
By this i meant, before sending data to wire keep it in host order -> print -> change to netwrok order -> send.
22. Alsways keep in mind the above figure.
23. Valgrind shows a hell lot of memory leaks, which i expected. Will need to fix those. 100KB of 
memory leaked while running stack for < 10secs
24. ipv4 header info(https://datatracker.ietf.org/doc/html/rfc791#section-3.1)

25. While writing ping repsonse. I found the need to eliminate frequent and redundant copying of buffer
data to different structs. From there i got to learn about socket buffers used in linux.
26. Implementing a socket buffer DS was my next goal.
27. About sk_buff :     +-----------------------------------------+
                        |  Headroom  |    Packet Data    |Tailroom|
                        +-----------------------------------------+
                        ^            ^                   ^        ^
                        |            |                   |        |
          skb->head ----+            |                   |        |
          skb->data -----------------+                   |        |
          skb->tail -------------------------------------+        |
          skb->end -----------------------------------------------+

-> suppose data is travelling down the stack. when it reachesip layer, ip layer is going to add its header
info, instead of copying the buffer, the headroom is shrunked and data pointer is moved backwards.
-> and so on for link layer
-> while travelling up the stack, the required header is read and headroom is expanded. Eliminating the need
of copying data every time.

28. while refactoring my packet dumper, i was freeing only the sb_buff structure, which aused memory as 
underlying buffer was not getting freed. soln was to free(skb->head) as well.
?? why not skb->data, because these pointers move while traversing the stack. head remains where it started.

29. It is easier for development if every part of stack assumes data it recevied is in network order or host order, not an intermix of two.

30. GDB is a great tool to find endianness mistakes, memory allocations issues. Basically a very good 
debugger.

31. TCP RFC (https://datatracker.ietf.org/doc/html/rfc793)
32. It says about sockets -> To allow for many processes within a single Host to use TCP
    communication facilities simultaneously, the TCP provides a set of
    addresses or ports within each host.  Concatenated with the network
    and host addresses from the internet communication layer, this forms
    a socket.  A pair of sockets uniquely identifies each connection.
    That is, a socket may be simultaneously used in multiple
    connections.

33. TCP header format (https://datatracker.ietf.org/doc/html/rfc793#section-3.1)

34. Till now i was doing this, reading from buffer, mutating it in place, writing it to fd as reply.
35. But in tcp this approach wont work. 