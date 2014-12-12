#ifndef MEMORY_H
#define MEMORY_H

void *malloc_or_die(size_t size);
void *calloc_or_die(size_t count, size_t size);
void *realloc_or_die(void *old, size_t size);
void free_or_die(void *ptr);

#endif
