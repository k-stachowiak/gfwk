cmake_minimum_required(VERSION 2.8)

project(gfwk)

include_directories(
    ./src/kernel
    ./src/cmp
    ./src/menu
    ./src/demo1
    ./src/util
    ./src
    .
    )

set(CMAKE_C_FLAGS "-Wall -Werror -g -O0")

add_executable(gfwk
    ./src/kernel/database.c
    ./src/kernel/system.c
    ./src/kernel/resources.c
    ./src/kernel/draw.c
    ./src/kernel/play.c
    ./src/cmp/cmp_appr.c
    ./src/menu/menu.c
    ./src/menu/menu_struct.c
    ./src/demo1/demo1.c
    ./src/main.c
    )

target_link_libraries(gfwk allegro_monolith)
