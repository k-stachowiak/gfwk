/* Copyright (C) 2014 Krzysztof Stachowiak */

#include "cmp_pain.h"
#include "memory.h"

void cmp_pain_init(struct CmpPain *pain, PainType type)
{
	pain->type = type;
	pain->queue_size = 0;
}

void cmp_pain_deinit(struct CmpPain* pain)
{
	(void)pain;
}

struct CmpPain *cmp_pain_create(PainType type)
{
    struct CmpPain *result = malloc_or_die(sizeof(*result));
	cmp_pain_init(result, type);
    return result;
}

void cmp_pain_free(struct CmpPain *cmp_pain)
{
	cmp_pain_deinit(cmp_pain);
    free_or_die(cmp_pain);
}

void cmp_pain_reset(struct CmpPain *cmp_pain)
{
    cmp_pain->queue_size = 0;
}

void cmp_pain_queue_push(struct CmpPain *cmp_pain, PainType pt)
{
    int new_index = cmp_pain->queue_size % CMP_PAIN_QUEUE_MAX;
    cmp_pain->queue[new_index] = pt;
    ++cmp_pain->queue_size;
}

