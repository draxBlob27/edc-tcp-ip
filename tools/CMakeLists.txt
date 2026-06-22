add_executable(packet_dumper "packet_dumper.c"
                            "../src/tuntap_alloc.c"
                            "../src/ethernet.c"
                            "../src/skbuff.c"
                            "../src/utils.c"   
                            "../src/netdev.c"             
)

add_executable(arp_reply "arp_reply.c"
                        "../src/tuntap_alloc.c"
                        "../src/ethernet.c"
                        "../src/skbuff.c"
                        "../src/utils.c"   
                        "../src/netdev.c" 
                        "../src/arp.c"
)

add_executable(arp_request "arp_request.c"
                        "../src/tuntap_alloc.c"
                        "../src/ethernet.c"
                        "../src/skbuff.c"
                        "../src/utils.c"   
                        "../src/netdev.c" 
                        "../src/arp.c"
)

add_executable(ping_reply "ping_reply.c"
                        "../src/tuntap_alloc.c"
                        "../src/ethernet.c"
                        "../src/skbuff.c"
                        "../src/utils.c"   
                        "../src/netdev.c" 
                        "../src/arp.c"
                        "../src/ipv4.c"
                        "../src/icmpv4.c"
                        "../src/tcp.c"
)

add_executable(ping_request "ping_request.c"
                        "../src/tuntap_alloc.c"
                        "../src/ethernet.c"
                        "../src/skbuff.c"
                        "../src/utils.c"   
                        "../src/netdev.c" 
                        "../src/arp.c"
                        "../src/ipv4.c"
                        "../src/icmpv4.c"
                        "../src/tcp.c"
)

add_executable(tcp_handshake "tcp_handshake.c"
                        "../src/tuntap_alloc.c"
                        "../src/ethernet.c"
                        "../src/skbuff.c"
                        "../src/utils.c"   
                        "../src/netdev.c" 
                        "../src/arp.c"
                        "../src/ipv4.c"
                        "../src/icmpv4.c"
                        "../src/tcp.c"
)


# target_include_directories(packet_dumper
#     PUBLIC
#         ${PROJECT_SOURCE_DIR}/include
# )

# add_executable(packet_resp "packet_resp.c")
# target_include_directories(packet_resp
#     PUBLIC
#         ${PROJECT_SOURCE_DIR}/include
# )
