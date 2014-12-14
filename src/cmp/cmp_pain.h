/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef CMP_PAIN_H
#define CMP_PAIN_H

#define CMP_PAIN_QUEUE_MAX 4

enum PainType {
    PT_PLAYER,
    PT_SOUL,
    PT_ARROW
};

struct CmpPain {
    enum PainType type;
    enum PainType queue[CMP_PAIN_QUEUE_MAX];
    int queue_size;
};

struct CmpPain *cmp_pain_create(enum PainType type);
void cmp_pain_free(struct CmpPain *cmp_pain);
void cmp_pain_reset(struct CmpPain *cmp_pain);
void cmp_pain_queue_push(struct CmpPain *cmp_pain, enum PainType ct);

#endif
