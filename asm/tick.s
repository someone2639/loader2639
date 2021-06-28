.set noat
.set noreorder
.section .text, "ax"

.global TICKS_READ
TICKS_READ:
    jr $ra
    mfc0 $v0, $9;
