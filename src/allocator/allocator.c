#include <PR/ultratypes.h>

#include "allocator.h"

// 1mb pool
static u8 allocator_pool[0x100000];

static u8 *alloc_head;

void allocator_setup(void) {
	alloc_head = &allocator_pool[0x100000];
}

void allocator_reset(void) {
	allocator_setup();
}

void *allocator_malloc(u32 size) {
	alloc_head -= size;
	return alloc_head;
}


void *allocator_malloc_align(u32 size, u32 align) {
	alloc_head -= size;
	while (!((u32) alloc_head & ~align)) {
		alloc_head--;
	}
	return alloc_head;
}

void *allocator_malloc_dl(u32 size) {
	return allocator_malloc_align(size, 16);
}
