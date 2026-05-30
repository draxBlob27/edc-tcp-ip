add_executable(arp "arp.c")
target_include_directories(arp
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(ethernet "ethernet.c")
target_include_directories(ethernet
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(netdev "netdev.c")
target_include_directories(netdev
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(tuntap_alloc "tuntap_alloc.c")
target_include_directories(tuntap_alloc
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(utils "utils.c")
target_include_directories(utils
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)