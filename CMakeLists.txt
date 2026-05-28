cmake_minimum_required(VERSION 3.10)
project(tcp-ip)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_EXTENSIONS OFF)

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_RELEASE "-Ofast -DNDEBUG -march=native")

add_subdirectory(tools)
add_subdirectory(src)