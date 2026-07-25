# edc-tcp-ip

A small educational TCP/IP stack written in C.

This project builds a user-space network stack around a Linux TAP device. The
Linux kernel sees the TAP interface as if it were a real Ethernet NIC; this
program attaches to the other side of that interface as a file descriptor,
reads raw Ethernet frames, parses them, and writes crafted frames back.

The goal is not to replace the kernel network stack. The goal is to make the
wire format and state machines visible: Ethernet framing, ARP resolution, IPv4
validation and checksums, ICMP echo traffic, and the beginning of a TCP
three-way handshake and close sequence.

## What This Stack Implements

Current protocol coverage:

| Layer | Implemented | Main files |
| --- | --- | --- |
| Link | Ethernet II frame parsing and frame construction | `include/ethernet.h`, `src/ethernet.c` |
| Link/IP boundary | ARP request, ARP reply, and a fixed-size ARP cache | `include/arp.h`, `src/arp.c` |
| Network | IPv4 header parsing, destination filtering, header checksum verification, reply construction | `include/ipv4.h`, `src/ipv4.c` |
| Transport utility | ICMPv4 echo request/reply | `include/icmpv4.h`, `src/icmpv4.c` |
| Transport | Minimal TCP control-plane path for SYN, SYN-ACK, ACK, FIN, ACK close flow, and RST for closed ports | `include/tcp.h`, `src/tcp.c` |
| Device | Linux TAP allocation and one global virtual netdev | `include/tuntap_alloc.h`, `src/tuntap_alloc.c`, `include/netdev.h`, `src/netdev.c` |
| Buffering | Linux-inspired socket buffer metadata | `include/skbuff.h`, `src/skbuff.c` |

The project currently provides runnable tools instead of one monolithic
daemon. Each tool exercises one behavior, which makes protocol development and
debugging easier.

## Why TAP

The code uses a TAP device with:

```c
IFF_TAP | IFF_NO_PI
```

TAP is chosen because this stack wants to handle Ethernet frames directly. A
TUN device would expose layer-3 IP packets, skipping Ethernet and ARP. That
would hide exactly the link-layer work this repository is trying to teach.

`IFF_NO_PI` removes the extra kernel packet-information header. Reads and
writes therefore contain only the raw Ethernet frame bytes expected by
`struct eth_hdr`.

Conceptually:

```text
Linux kernel network stack
192.168.0.1 / tap0
        |
        | Ethernet frames over TAP fd
        v
User-space stack
192.168.0.254 / custom MAC
```

The TAP interface must have a Linux-side IP/MAC, while this program also needs
its own stack-side IP/MAC. Do not use the same IP address for both sides.

## Repository Layout

```text
.
|-- CMakeLists.txt              Top-level CMake project; builds tools and setup
|-- include/                    Public headers for the stack modules
|   |-- arp.h                   ARP wire structs, cache entry, constants, APIs
|   |-- ethernet.h              Ethernet header, ethertypes, frame APIs
|   |-- icmpv4.h                ICMP echo constants, header, APIs
|   |-- ipv4.h                  IPv4 header, checksum APIs, receive/reply APIs
|   |-- netdev.h                User-space netdev model and TAP-backed transmit
|   |-- skbuff.h                Socket-buffer-style packet buffer
|   |-- syshead.h               Shared system includes
|   |-- tcp.h                   TCP header, pseudo header, TCB, states, APIs
|   |-- tuntap_alloc.h          TAP allocation API
|   `-- utils.h                 Logging, IP parsing, TAP write helper
|-- src/                        Protocol and device implementations
|   |-- arp.c                   ARP cache, request, reply, receive path
|   |-- ethernet.c              Ethernet parser and reply wrapper
|   |-- icmpv4.c                ICMP echo receive/request/reply
|   |-- ipv4.c                  IPv4 receive/reply and internet checksum
|   |-- netdev.c                Global TAP-backed network device
|   |-- skbuff.c                Packet buffer allocation and pointer movement
|   |-- tcp.c                   Minimal TCP connection table and state machine
|   |-- tuntap_alloc.c          `/dev/net/tun` ioctl setup
|   `-- utils.c                 Small helpers
|-- tools/                      Runnable protocol experiments
|   |-- arp_reply.c             Reply to ARP requests for the stack IP
|   |-- arp_request.c           Send ARP requests and cache replies
|   |-- packet_dumper.c         Print basic Ethernet frame information
|   |-- ping_reply.c            Reply to ICMP echo requests
|   |-- ping_request.c          Send ICMP echo requests
|   `-- tcp_handshake.c         Listen on TCP port 8080 and handle handshake/close
|-- setup/
|   `-- tuntap_if.c             Create and persist a TAP interface
|-- scripts/
|   |-- setup_linux.sh          Persist, address, and bring up a TAP interface
|   |-- setup_mine.sh           Persist a TAP interface without full IP setup
|   `-- strip_tap.sh            Delete existing TAP interfaces
`-- docs/
    |-- learnings.md            Development notes and learning references
    `-- tcp_docs.md             TCP reference notes copied from RFC-style material
```

## Build

Prerequisites:

- Linux
- C compiler with C11 support
- CMake 3.10 or newer
- Permission to open `/dev/net/tun`
- `iproute2` for `ip link`, `ip addr`, and TAP inspection

Build commands:

```sh
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug
```

Generated binaries:

```text
build/debug/setup/tuntap_if
build/debug/tools/packet_dumper
build/debug/tools/arp_reply
build/debug/tools/arp_request
build/debug/tools/ping_reply
build/debug/tools/ping_request
build/debug/tools/tcp_handshake
```

The top-level `CMakeLists.txt` currently builds `tools/` and `setup/`. It does
not build `src/` as a standalone library target; each tool links the source
files it needs directly.

## TAP Setup

Create and configure a TAP device for the Linux side:

```sh
./scripts/setup_linux.sh tap0 192.168.0.1 02:11:22:33:44:55
```

That script:

1. Creates/persists `tap0` using `build/debug/setup/tuntap_if`.
2. Sets the Linux-side MAC address.
3. Adds `192.168.0.1/24`.
4. Brings the interface up.

The user-space stack then attaches to the same TAP device, but it must be
started with its own IP and MAC:

```sh
./build/debug/tools/ping_reply tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

To remove TAP devices created during experiments:

```sh
./scripts/strip_tap.sh
```

That script deletes all interfaces returned by `ip tuntap list`, so inspect the
list first if you have other TAP devices you care about:

```sh
ip tuntap list
```

## Running The Tools

Most protocol tools use the same basic argument shape:

```text
<binary> <tap-ifname> <stack-ip> <stack-mac>
```

`packet_dumper` is the exception; it only needs the TAP interface name because
it does not initialize a stack IP or MAC.

Example stack identity:

```text
tap-ifname: tap0
stack-ip:   192.168.0.254
stack-mac:  02:aa:bb:cc:dd:ee
```

### Packet dumper

Reads frames from the TAP fd and prints Ethernet header information:

```sh
./build/debug/tools/packet_dumper tap0
```

This is useful before changing protocol logic because it confirms that frames
are reaching user space.

### ARP reply

Responds to ARP requests for the stack IP and updates the ARP cache with sender
information:

```sh
./build/debug/tools/arp_reply tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

From another terminal:

```sh
ping -c 1 192.168.0.254
```

Linux should first ARP for `192.168.0.254`; the tool should answer with the
stack MAC.

### ARP request

Sends ARP requests from the stack. The current tool is hard-coded to request
`192.168.0.1`. It is a rough development harness: it waits for a frame from the
TAP fd, sends an ARP request, then parses the received frame:

```sh
./build/debug/tools/arp_request tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

### Ping reply

Responds to ICMPv4 echo requests:

```sh
./build/debug/tools/ping_reply tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

From Linux:

```sh
ping 192.168.0.254
```

Expected flow:

1. Linux sends ARP request for `192.168.0.254`.
2. The stack sends ARP reply.
3. Linux sends ICMP echo request.
4. The stack validates IPv4 and ICMP checksums.
5. The stack sends ICMP echo reply.

### Ping request

Sends ICMPv4 echo requests from the stack. The current destination is
hard-coded to `192.168.0.1`:

```sh
./build/debug/tools/ping_request tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

If the destination MAC is not in the ARP cache yet, the first send triggers an
ARP request and asks the user to retry after the cache is populated.

### TCP handshake

Runs the stack as a minimal TCP listener on port `8080`:

```sh
./build/debug/tools/tcp_handshake tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

From Linux:

```sh
nc -vz 192.168.0.254 8080
```

The implemented path is intentionally small:

```text
LISTEN
  receive SYN
  send SYN|ACK
SYN_RECEIVED
  receive ACK
  mark ESTABLISHED
  immediately send FIN|ACK
FIN_WAIT_1
  receive ACK
FIN_WAIT_2
  receive FIN|ACK
  send ACK
  mark CLOSED
```

If a TCP segment arrives for a port that is not in `LISTEN`, the stack builds a
temporary connection record and sends `RST|ACK`.

## Packet Flow

### Inbound frame path

Most tools share the same receive-loop shape:

```text
read(tap_fd, skb->data, 2048)
set skb->tail and skb->len
parse Ethernet ethertype
  0x0806 -> arp_recv()
  0x0800 -> ipv4_recv()
  other  -> print unsupported/corrupted message
```

`ipv4_recv()` then:

1. Reads the IPv4 header at `skb->data + ETH_HDR_LEN`.
2. Rejects non-IPv4 packets.
3. Rejects packets not addressed to the configured stack IP.
4. Verifies the IPv4 header checksum.
5. Dispatches protocol `0x01` to ICMPv4.
6. Dispatches protocol `0x06` to TCP.

### Outbound reply path

Outbound packets are built in-place in an `sk_buff`:

```text
protocol reply/request fills transport payload/header
ipv4_reply() fills IPv4 header and checksum
ipv4_reply() resolves destination MAC through ARP cache
ethernet_reply() fills Ethernet header
netdev_transmit() writes bytes to TAP fd
```

This layered path is the main reason the code has separate `*_reply()` helpers:
each layer owns the fields it understands, while lower layers wrap the payload
for transmission.

## Buffer Model

`struct sk_buff` stores packet metadata:

```c
struct sk_buff {
    uint8_t *head;
    uint8_t *data;
    uint8_t *tail;
    uint8_t *end;
    uint32_t len;
};
```

Pointer meaning:

| Pointer | Meaning |
| --- | --- |
| `head` | Start of the allocated memory region |
| `data` | First byte of currently visible packet data |
| `tail` | First byte after currently visible packet data |
| `end` | First byte after the allocation |
| `len` | Number of visible bytes |

Operations:

| Function | Purpose |
| --- | --- |
| `skbuff_alloc(size)` | Allocate metadata and backing memory |
| `skb_reserve(skb, len)` | Move `data` forward to create headroom |
| `skb_push(skb, len)` | Prepend bytes by moving `data` backward |
| `skb_pull(skb, len)` | Consume bytes by moving `data` forward |
| `skb_put(skb, len)` | Append bytes at `tail` |
| `free_skb(skb)` | Free both backing memory and metadata |

The "why" is copying avoidance. As a packet moves down a stack, each layer can
prepend its own header into existing headroom. As a packet moves up a stack,
each layer can consume its header by moving `data`. This repository only uses a
small subset of that model today, but the data structure is shaped for the same
reason Linux uses socket buffers.

## Endianness Model

Network protocols store multi-byte integers in network byte order
(big-endian). Most host CPUs used for development are little-endian. This code
therefore uses:

- `htons()` / `htonl()` before bytes go to the TAP fd
- `ntohs()` / `ntohl()` after bytes come from the TAP fd

The debug macros generally print fields in host order. Several functions
temporarily convert fields for logging and then convert them back before
dispatching or transmitting.

Important rule: only multi-byte fields need byte-order conversion. Single-byte
fields such as IPv4 version, IHL, TTL, protocol, ICMP type, and TCP flags are
not byte-swapped.

## Protocol Details

### Ethernet

Header:

```c
struct eth_hdr {
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ethertype;
    uint8_t payload[];
} __attribute__((packed));
```

Implemented ethertypes:

| Name | Value |
| --- | --- |
| ARP | `0x0806` |
| IPv4 | `0x0800` |

`__attribute__((packed))` is used so C struct layout matches the wire format.
Without it, compiler-inserted padding could move fields away from their network
byte offsets.

### ARP

The ARP implementation supports Ethernet plus IPv4:

| Constant | Value | Meaning |
| --- | --- | --- |
| `ARP_ETHERNET` | `0x0001` | Hardware type Ethernet |
| `ARP_IPV4` | `0x0800` | Protocol type IPv4 |
| `ARP_REQUEST` | `0x0001` | Request operation |
| `ARP_REPLY` | `0x0002` | Reply operation |
| `ARP_IPV4_LEN` | `4` | IPv4 address length |
| `ARP_CACHE_SIZE` | `256` | Number of cache entries |

The cache is a fixed array of `struct arp_entry *` with linear lookup. Each
entry stores:

- hardware type
- IPv4 address
- MAC address
- timestamp
- validity flag

Why the cache exists: IPv4 replies need a destination MAC before Ethernet can
transmit. If `ipv4_reply()` does not find a MAC for the destination IP, it sends
an ARP request and returns `-1`, leaving the higher-level operation to retry.

### IPv4

Header fields are represented by `struct ipv4_hdr`. The implementation
supports:

- version check for IPv4
- destination check against the configured netdev IP
- header checksum validation
- ICMPv4 dispatch
- TCP dispatch
- reply header construction

Important constants:

| Constant | Value |
| --- | --- |
| `IPV4` | `0x04` |
| `ICMPV4` | `0x01` |
| `IPV4_TCP` | `0x06` |
| `IPV4_HDR_LEN` | `sizeof(struct ipv4_hdr)` |
| Reply TTL | `64` |
| Reply ID | `0x0101` |
| Reply flags/fragment offset | `0x4000` |

Fragmentation and reassembly are not implemented. The reply path always emits a
20-byte IPv4 header (`ihl = 5`) with no options.

### Internet Checksum

`src/ipv4.c` implements the standard one's-complement checksum in two phases:

- `internet_checksum_partial()` accumulates 16-bit words.
- `internet_checksum_final()` folds carries and returns the one's complement.

The same helpers are reused by IPv4, ICMPv4, and TCP. TCP adds the pseudo
header before checksumming the TCP header and payload.

### ICMPv4

Implemented ICMP message types:

| Name | Value |
| --- | --- |
| Echo reply | `0` |
| Echo request | `8` |

`icmpv4_request()` currently builds:

- ICMP identifier `0x1234`
- sequence number `0x0001`
- 56 bytes of payload filled with `0xab`
- total ICMP bytes: 8-byte header + 56-byte payload = 64 bytes

`icmpv4_recv()` verifies the ICMP checksum before replying or printing that a
reply was received.

### TCP

The TCP code is intentionally a small state-machine experiment, not a full TCP
implementation.

Implemented constants and sizes:

| Constant | Value | Meaning |
| --- | --- | --- |
| `FIN` | `0x01` | No more data from sender |
| `SYN` | `0x02` | Synchronize sequence numbers |
| `RST` | `0x04` | Reset connection |
| `PSH` | `0x08` | Push function |
| `ACK` | `0x10` | Acknowledgment field significant |
| `URG` | `0x20` | Urgent pointer significant |
| `MAX_CONNECTIONS` | `64` | Allocated connection and port table slots |
| `TCP_SEND_BUF_SIZE` | `32` | Per-connection send buffer bytes |
| `TCP_RECV_BUF_SIZE` | `32` | Per-connection receive buffer bytes |
| `SENDER_WINDOW_LEN` | `12` | Current sender window value |
| Default listener | `8080` | Opened by `tcp_handshake` |

The connection record is a Transmission Control Block-like structure:

- source and destination IPs
- source and destination ports
- state
- send sequence variables: `snd_una`, `snd_nxt`, `snd_wnd`
- receive sequence variables: `rcv_nxt`, `rcv_wnd`
- small send and receive buffers
- send and receive lengths

The implemented state enum contains the standard TCP states:

```text
LISTEN
SYN_SENT
SYN_RECEIVED
ESTABLISHED
FIN_WAIT_1
FIN_WAIT_2
CLOSE_WAIT
LAST_ACK
TIME_WAIT
CLOSED
CLOSING
```

The active code path currently handles only the server-side path used by
`tcp_handshake`. The tables are allocated for 64 entries, but the current
listener setup hard-codes slot 0 to port 8080 and some lookup paths still need
work before the table behaves like a complete 64-connection implementation.

## Metrics, Limits, And Observable Signals

Hard-coded protocol and runtime metrics:

| Metric | Value | Where |
| --- | --- | --- |
| Netdev MTU | `1500` bytes | `netdev_init()` |
| Tool read buffer | `2048` bytes | tools receive loops |
| ARP cache entries | `256` | `src/arp.c` |
| TCP connection slots | `64` | `include/tcp.h` |
| TCP listen port slots | `64` | `include/tcp.h`, `src/tcp.c` |
| TCP send buffer | `32` bytes per connection | `include/tcp.h` |
| TCP receive buffer | `32` bytes per connection | `include/tcp.h` |
| TCP sender window | `12` | `include/tcp.h` |
| ICMP request payload | `56` bytes | `src/icmpv4.c` |
| ICMP request total | `64` bytes | `src/icmpv4.c` |
| IPv4 reply TTL | `64` | `src/ipv4.c` |
| IPv4 reply ID | `0x0101` | `src/ipv4.c` |
| IPv4 reply DF flag pattern | `0x4000` | `src/ipv4.c` |
| TCP default listening port | `8080` | `src/tcp.c`, `tools/tcp_handshake.c` |

Built-in observability:

- Ethernet debug output is enabled by `DEBUG_ETH`.
- ARP debug output is enabled by `DEBUG_ARP`.
- IPv4 debug output is enabled by `DEBUG_IPV4`.
- ICMPv4 debug output is enabled by `ICMPV4_DEBUG`.
- TCP debug output is enabled by `TCP_DBG`.

There are no in-process counters yet for packets, drops, checksum failures,
cache hits, retransmissions, or state transitions. Use the printed debug logs
and Linux interface counters while developing:

```sh
ip -s link show tap0
ip neigh show dev tap0
tcpdump -eni tap0
```

`tcpdump` is especially useful because this stack is writing real Ethernet
frames through TAP.

## Current Limitations

This repository is best understood as an educational stack in progress.

Known limitations:

- Linux only; it depends on `/dev/net/tun`, `linux/if.h`, and
  `linux/if_tun.h`.
- One global netdev (`my_dev`) is supported.
- Most tools run forever and are stopped with `Ctrl-C`.
- ARP cache lookup and insertion are linear.
- ARP cache expiration is not enforced even though entries store timestamps.
- IPv4 fragmentation, reassembly, options, routing, and forwarding are not
  implemented.
- IPv4 only accepts packets addressed to the configured stack IP.
- ICMP support is limited to echo request/reply.
- TCP supports a narrow server-side handshake and close experiment.
- TCP port opening currently ignores its argument and opens port `8080` in slot
  0.
- TCP connection lookup currently needs correction before multiple active
  connection slots can be trusted.
- TCP data transfer, retransmission, congestion control, window updates,
  timers, simultaneous open, full reset handling, and complete close semantics
  are not implemented.
- Several tools have hard-coded destination IPs such as `192.168.0.1`.
- There are no automated tests yet.
- Error handling and memory ownership are still rough in places.

These limits are useful development boundaries: each one maps to a clear future
learning milestone.

## Debugging Guidance

Good first checks:

```sh
ip link show tap0
ip addr show tap0
ip tuntap list
ip neigh show dev tap0
```

If no frames arrive:

- Confirm the TAP interface exists.
- Confirm it is up.
- Confirm Linux and stack IPs are in the same subnet.
- Confirm the tool attached to the same interface name.
- Confirm permissions for `/dev/net/tun`.

If ARP works but ping does not:

- Check that `ping_reply` is running, not only `arp_reply`.
- Check IPv4 destination address logs.
- Check checksum failure messages.
- Capture traffic with `tcpdump -eni tap0`.

If TCP does not connect:

- Confirm `tcp_handshake` is running.
- Confirm the destination port is `8080`.
- Watch for "Port unavailable." and TCP debug logs.
- Remember that successful establishment is immediately followed by a FIN path.

## Development Notes

The source code deliberately keeps protocol structs close to wire format. This
has two consequences:

1. Structs are packed.
2. Byte order must be handled explicitly.

The code also favors direct protocol flow over abstraction. That makes it
easier to see which layer owns each field, and it keeps the project suitable for
learning with GDB, packet captures, and RFCs open side by side.

Useful references already used by the project:

- ARP: RFC 826
- IPv4: RFC 791
- TCP: RFC 793
- TUN/TAP: Linux TUN/TAP interface tutorials and kernel documentation

## Suggested Next Milestones

High-value improvements:

1. Add a shared `stack` library target in CMake and link tools against it.
2. Add argument validation to every tool.
3. Fix ownership leaks and run Valgrind cleanly for each tool.
4. Add packet counters for RX, TX, drops, checksum failures, ARP cache hits, and
   TCP state transitions.
5. Add unit tests for checksum, ARP cache, and TCP state transitions.
6. Replace hard-coded destination IPs with command-line arguments.
7. Add ARP cache expiration or LRU replacement.
8. Add IPv4 payload length validation.
9. Implement TCP data transfer after `ESTABLISHED`.
10. Add retransmission timers and more complete TCP close/reset handling.

## Quick Start

```sh
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug

./scripts/setup_linux.sh tap0 192.168.0.1 02:11:22:33:44:55

./build/debug/tools/ping_reply tap0 192.168.0.254 02:aa:bb:cc:dd:ee
```

In another terminal:

```sh
ping 192.168.0.254
```

That round trip exercises TAP, Ethernet, ARP, IPv4, ICMPv4 checksum handling,
and Ethernet transmission back to Linux.
