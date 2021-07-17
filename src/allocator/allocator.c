#include <ultra64.h>

#include "allocator.h"

// pool
extern u8 _framebufferSegmentBssEnd[];

static u8 *allocator_pool;
static u8 *alloc_head;
static u32 memSize;

void allocator_setup(void) {
	allocator_pool = _framebufferSegmentBssEnd;
	memSize = osGetMemSize();
	alloc_head = 0x80000000 + memSize;
}

void allocator_reset(void) {
	alloc_head = 0x80000000 + memSize;
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
