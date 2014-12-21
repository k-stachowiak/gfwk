cmake_minimum_required(VERSION 2.8)

project(gfwk)

include_directories(
    ./src/kernel
    ./src/cmp
    ./src/menu
    ./src/demo
    ./src/util
    ./src/sc
    ./src/sc/engines
    ./src/sc/objects
    ./src
    .
    )

set(CMAKE_C_FLAGS "-Wall -Werror -g -O0")

add_executable(gfwk
	./src/util/random.c
	./src/util/memory.c
	./src/cmp/cmp_drv.c
	./src/cmp/cmp_operations.c
	./src/cmp/cmp_ai_soul.c
	./src/cmp/cmp_appr.c
	./src/cmp/cmp_ori.c
	./src/cmp/cmp_pain.c
	./src/demo/demo.c
	./src/kernel/play.c
	./src/kernel/system.c
	./src/kernel/draw.c
	./src/kernel/database.c
	./src/kernel/resources.c
	./src/main.c
	./src/sc/engines/sc_pain.c
	./src/sc/engines/sc_platform.c
	./src/sc/engines/sc_collision.c
	./src/sc/engines/sc_tick.c
	./src/sc/engines/sc_draw.c
	./src/sc/engines/sc_arms.c
	./src/sc/sc.c
	./src/sc/objects/sc_graph.c
	./src/sc/objects/sc_soul.c
	./src/sc/objects/sc_hunter.c
	./src/sc/objects/sc_arrow.c
	./src/sc/objects/sc_level.c
	./src/sc/sc_data.c
	./src/menu/menu_struct.c
	./src/menu/menu.c
)

target_link_libraries(gfwk m allegro_monolith)
