add_executable(tuntap_if "tuntap_if.c"
                        "../src/tuntap_alloc.c"
)
target_include_directories(tuntap_if
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)