/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef DATABASE_H
#define DATABASE_H

void db_init(void);
long db_integer(char *key);
double db_real(char *key);

#endif
