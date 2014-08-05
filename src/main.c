/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include "system.h"
#include "database.h"
#include "menu.h"

int main(void)
{
    db_init();
    sys_init();
    sys_realtime_loop(menu_get_client());
    sys_deinit();
    return 0;
}
