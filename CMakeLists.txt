cmake_minimum_required(VERSION 2.8)

project(gfwk)

include_directories(
    ./src/kernel
    ./src/util
    ./src
    .
    )

set(CMAKE_C_FLAGS "-Wall -Werror -g -O0")

add_executable(gfwk
    ./src/kernel/database.c
    ./src/kernel/system.c
    ./src/main.c
    )

target_link_libraries(gfwk allegro_monolith)
