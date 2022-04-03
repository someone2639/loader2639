
#include "asm.h"
#include "registers.h"

/*
 * 'cache' instruction definitions
 */

/* Target cache */
#define	CACH_PI		0x0	/* specifies primary inst. cache */
#define	CACH_PD		0x1	/* primary data cache */
#define	CACH_SI		0x2	/* secondary instruction cache */
#define	CACH_SD		0x3	/* secondary data cache */

/* Cache operations */
#define	C_IINV		0x0	/* index invalidate (inst, 2nd inst) */
#define	C_IWBINV	0x0	/* index writeback inval (d, sd) */
#define	C_ILT		0x4	/* index load tag (all) */
#define	C_IST		0x8	/* index store tag (all) */
#define	C_CDX		0xc	/* create dirty exclusive (d, sd) */
#define	C_HINV		0x10	/* hit invalidate (all) */
#define	C_HWBINV	0x14	/* hit writeback inv. (d, sd) */
#define	C_FILL		0x14	/* fill (i) */
#define	C_HWB		0x18	/* hit writeback (i, d, sd) */
#define	C_HSV		0x1c	/* hit set virt. (si, sd) */

/*
 * Cache size definitions
 */
#define	ICACHE_SIZE		0x4000			/* 16K */
#define	ICACHE_LINESIZE		32			/* 8 words */
#define	ICACHE_LINEMASK		(ICACHE_LINESIZE-1)

#define	DCACHE_SIZE		0x2000			/* 8K */
#define	DCACHE_LINESIZE		16			/* 4 words */
#define	DCACHE_LINEMASK		(DCACHE_LINESIZE-1)

/*
 * Segment base addresses and sizes
 */
#define	KUBASE		0
#define	KUSIZE		0x80000000

.text
.set noreorder
.global inval_icache
inval_icache:
	blez a1, 2f
	nop
	li t3, ICACHE_SIZE
	bgeu a1, t3, 3f
	nop
	move t0, a0
	addu t1, a0, a1
	bgeu t0, t1, 2f
	nop
	addiu t1, t1, -ICACHE_LINESIZE
	andi t2, t0, ICACHE_LINEMASK
	subu t0, t0, t2
1:
	CACHE((C_HINV|CACH_PI), (t0))
    .set noreorder
	bltu t0, t1, 1b
	addiu t0, t0, ICACHE_LINESIZE
    .set reorder
2:
	jr ra
	#nop
3:
	li t0, KUSIZE
	addu t1, t0, t3
	addiu t1, t1, -ICACHE_LINESIZE
4:
	CACHE((C_IINV|CACH_PI), (t0))
    .set noreorder
	bltu t0, t1, 4b
	addiu t0, t0, ICACHE_LINESIZE
    .set reorder

	jr ra
	#nop


.global inval_dcache
inval_dcache:
    blez a1, 3f
    nop
    li t3, DCACHE_SIZE
    bgeu a1, t3, 4f
     nop

    move t0, a0
    addu t1, a0, a1
    bgeu t0, t1, 3f
     nop

    addiu t1, t1, -DCACHE_LINESIZE
    andi t2, t0, DCACHE_LINEMASK
    beqz t2, 1f
    nop

    subu t0, t0, t2
    CACHE((C_HWBINV|CACH_PD), (t0))
    bgeu t0, t1, 3f
    # nop

    addiu t0, t0, DCACHE_LINESIZE
1:
    andi t2, t1, DCACHE_LINEMASK
    beqz t2, 2f
    # nop
    subu t1, t1, t2
    CACHE((C_HWBINV|CACH_PD), 0x10(t1))
    bltu t1, t0, 3f
    # nop
2:
    CACHE((C_HINV|CACH_PD), (t0))
    .set noreorder
    bltu t0, t1, 2b
    addiu t0, t0, DCACHE_LINESIZE
    .set reorder
3:
    jr ra
     # nop
4:
    li t0, KUSIZE
    addu t1, t0, t3
    addiu t1, t1, -DCACHE_LINESIZE
5:
    CACHE((C_IINV|CACH_PD), (t0))
    .set noreorder
    bltu t0, t1, 5b
    addiu t0, t0, DCACHE_LINESIZE
    .set reorder

    jr ra
     # nop
     # nop



