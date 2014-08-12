/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef RANDOM_H
#define RANDOM_H

void rnd_init(void);

double rnd_uniform_d(void);
double rnd_uniform_rng_d(double low, double high);

int rnd_uniform_i(void);
int rnd_uniform_rng_i(int low, int high);

double rnd_normal(void);
double rnd_normal_param(double mu, double sigma);

#endif
