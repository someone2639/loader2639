# 0 "asm/cache.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "asm/cache.S"

# 1 "asm/asm.h" 1
# 3 "asm/cache.S" 2
# 1 "asm/registers.h" 1
# 4 "asm/cache.S" 2
# 44 "asm/cache.S"
.text
.set noreorder
.global inval_icache
inval_icache:
 blez $5, 2f
 nop
 li $11, 0x4000
 bgeu $5, $11, 3f
 nop
 move $8, $4
 addu $9, $4, $5
 bgeu $8, $9, 2f
 nop
 addiu $9, $9, -32
 andi $10, $8, (32 -1)
 subu $8, $8, $10
1:
 .set noreorder; cache (0x10|0x0), ($8); .set reorder;
    .set noreorder
 bltu $8, $9, 1b
 addiu $8, $8, 32
    .set reorder
2:
 jr $31
 #nop
3:
 li $8, 0x80000000
 addu $9, $8, $11
 addiu $9, $9, -32
4:
 .set noreorder; cache (0x0|0x0), ($8); .set reorder;
    .set noreorder
 bltu $8, $9, 4b
 addiu $8, $8, 32
    .set reorder

 jr $31
 #nop


.global inval_dcache
inval_dcache:
    blez $5, 3f
    nop
    li $11, 0x2000
    bgeu $5, $11, 4f
     nop

    move $8, $4
    addu $9, $4, $5
    bgeu $8, $9, 3f
     nop

    addiu $9, $9, -16
    andi $10, $8, (16 -1)
    beqz $10, 1f
    nop

    subu $8, $8, $10
    .set noreorder; cache (0x14|0x1), ($8); .set reorder;
    bgeu $8, $9, 3f
    # nop

    addiu $8, $8, 16
1:
    andi $10, $9, (16 -1)
    beqz $10, 2f
    # nop
    subu $9, $9, $10
    .set noreorder; cache (0x14|0x1), 0x10($9); .set reorder;
    bltu $9, $8, 3f
    # nop
2:
    .set noreorder; cache (0x10|0x1), ($8); .set reorder;
    .set noreorder
    bltu $8, $9, 2b
    addiu $8, $8, 16
    .set reorder
3:
    jr $31
     # nop
4:
    li $8, 0x80000000
    addu $9, $8, $11
    addiu $9, $9, -16
5:
    .set noreorder; cache (0x0|0x1), ($8); .set reorder;
    .set noreorder
    bltu $8, $9, 5b
    addiu $8, $8, 16
    .set reorder

    jr $31
     # nop
     # nop
