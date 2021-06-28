#include <ultra64.h>
#include <PR/gs2dex.h>

u64 progStack[0x2000];

#include "filesystem/ff.h"

void main(void) {
    allocator_setup();

    FIL f;

    f_open(&f, "My Hacks/smb64.z64", FA_READ);

    u8 *space = allocator_malloc(0x100000);

    u32 bread;
    f_read(&f, space, 0x100000, &bread);
    while (1);
}

