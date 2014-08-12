/* Copyright (C) 2014 Krzysztof Stachowiak */

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>

bool pending_normal;
double normal_buffer;

void rnd_init(void)
{
    srand(time(NULL));
}

double rnd_uniform_d(void)
{
    return (double)rand() / RAND_MAX;
}

double rnd_uniform_rng_d(double low, double high)
{
    return low + rnd_uniform_d() * (high - low);
}

int rnd_uniform_i(void)
{
    return (int)rnd_uniform_rng_d(0, INT_MAX + 0.999);
}

int rnd_uniform_rng_i(int low, int high)
{
    return low + rnd_uniform_d() * (high - low + 1);
}

double rnd_normal(void)
{
    double u, v, x, y;

    if (pending_normal) {
        pending_normal = false;
        return normal_buffer;
    }

    u = rnd_uniform_d();
    v = rnd_uniform_d();
    x = sqrt(-2.0 * log(u)) * cos(2 * 3.1415 * v);
    y = sqrt(-2.0 * log(u)) * sin(2 * 3.1415 * v);

    pending_normal = true;
    normal_buffer = y;
    return x;
}

double rnd_normal_param(double mu, double sigma)
{
    return rnd_normal() * sigma + mu;
}

