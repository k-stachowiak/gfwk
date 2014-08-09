/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef DATABASE_H
#define DATABASE_H

void db_init(void);
void db_deinit(void);
long db_integer(char *key);
double db_real(char *key);

#endif
