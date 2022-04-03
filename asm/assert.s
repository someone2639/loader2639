.include "macros.inc"
.section .data
glabel _n64assert_FILE
.skip 4
glabel _n64assert_LINE
.skip 4
glabel _n64assert_MSG
.skip 4
glabel _n64assert_BUF
.fill 100, 0

.section .text

.global __n64Assert_dispatch
__n64Assert_dispatch:

sw $a0, _n64assert_FILE
sw $a1, _n64assert_LINE
sw $a2, _n64assert_MSG
syscall

