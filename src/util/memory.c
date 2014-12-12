#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define MEM_TRACE(op, addr) printf(op " %p\n", addr)

void *malloc_or_die(size_t size)
{
	void *result = malloc(size);
	if (!result) {
		fprintf(stderr, "Allocation failure.");
		exit(1);
	}
	MEM_TRACE("malloc", result);
	return result;
}

void *calloc_or_die(size_t count, size_t size)
{
	void *result = calloc(count, size);
	if (!result) {
		fprintf(stderr, "Callocation failure.");
		exit(1);
	}
	MEM_TRACE("calloc", result);
	return result;
}

void *realloc_or_die(void *old, size_t size)
{
	void *result = realloc(old, size);
	if (!result) {
		fprintf(stderr, "Reallocation failure.");
		exit(1);
	}
	MEM_TRACE("realloc", result);
	return result;
}

void free_or_die(void *ptr)
{
	MEM_TRACE("free", ptr);
	free(ptr);
}
