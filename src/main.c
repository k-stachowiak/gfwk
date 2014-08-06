/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include "system.h"
#include "database.h"
#include "play.h"
#include "menu.h"

int main(void)
{
    db_init();
    sys_init();
    play_init();

    struct SysClient *client = menu_get_client();

    while (client) {
        sys_realtime_loop(client);
        client = *(client->next);
    }

    sys_deinit();
    return 0;
}
