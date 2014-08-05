/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdio.h>

#include "system.h"
#include "database.h"

int main(void)
{
    db_init();
    sys_init();
    sys_deinit();
    return 0;
}
