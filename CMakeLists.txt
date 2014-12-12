cmake_minimum_required(VERSION 2.8)

project(gfwk)

include_directories(
    ./src/kernel
    ./src/cmp
    ./src/menu
    ./src/demo
    ./src/util
    ./src/sc
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
    ./src/cmp/cmp_ori.c
    ./src/cmp/cmp_drv.c
    ./src/cmp/cmp_ai.c
    ./src/cmp/cmp_operations.c
    ./src/menu/menu.c
    ./src/menu/menu_struct.c
    ./src/demo/demo.c
    ./src/sc/sc.c
    ./src/sc/sc_data.c
    ./src/sc/sc_hunter.c
    ./src/sc/sc_soul.c
    ./src/sc/sc_level.c
    ./src/sc/sc_graph.c
    ./src/sc/sc_collision.c
    ./src/sc/sc_pain.c
    ./src/sc/sc_platform.c
	./src/util/random.c
    ./src/util/memory.c
    ./src/main.c
    )

target_link_libraries(gfwk m allegro_monolith)
