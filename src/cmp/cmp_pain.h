/* Copyright (C) 2014,2015 Krzysztof Stachowiak */

#ifndef CMP_PAIN_H
#define CMP_PAIN_H

#define CMP_PAIN_QUEUE_MAX 4

typedef int PainType;

struct CmpPain {
    PainType type;
    PainType queue[CMP_PAIN_QUEUE_MAX];
    int queue_size;
};

void cmp_pain_init(struct CmpPain *pain, PainType type);
void cmp_pain_deinit(struct CmpPain* pain);

void cmp_pain_reset(struct CmpPain *cmp_pain);
void cmp_pain_queue_push(struct CmpPain *cmp_pain, PainType ct);

#endif
