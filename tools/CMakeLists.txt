add_executable(packet_dumper "packet_dumper.c")
target_include_directories(packet_dumper
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(packet_resp "packet_resp.c")
target_include_directories(packet_resp
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
