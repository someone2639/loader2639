#include <ultra64.h>
#include "types.h"

void strcpy(String dst, String src) {
	while (*src) *dst++ = *src++;
}

