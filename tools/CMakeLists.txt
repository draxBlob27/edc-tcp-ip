add_executable(packet_dumper "packet_dumper.c")
target_include_directories(packet_dumper
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
