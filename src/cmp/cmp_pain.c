/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "cmp_pain.h"
#include "memory.h"

struct CmpPain *cmp_pain_create(enum PainType type)
{
    struct CmpPain *result = malloc_or_die(sizeof(*result));
    result->type = type;
    result->queue_size = 0;
    return result;
}

void cmp_pain_free(struct CmpPain *cmp_pain)
{
    free_or_die(cmp_pain);
}

void cmp_pain_reset(struct CmpPain *cmp_pain)
{
    cmp_pain->queue_size = 0;
}

void cmp_pain_queue_push(struct CmpPain *cmp_pain, enum PainType pt)
{
    int new_index = cmp_pain->queue_size % CMP_PAIN_QUEUE_MAX;
    cmp_pain->queue[new_index] = pt;
    ++cmp_pain->queue_size;
}

