/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include "database.h"
#include "system.h"
#include "play.h"
#include "menu.h"
#include "resources.h"

int main(void)
{
	struct SysClient *client;

    db_init();
    sys_init();
    play_init();

    client = menu_get_client();

    while (client) {
        sys_realtime_loop(client);
        client->deinit();
        client = *(client->next);
    }

    res_deinit();
    play_deinit();
    sys_deinit();
    db_deinit();

    return 0;
}
