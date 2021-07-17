.set REBOOT_STACK_SIZE, 256

.section .text
.balign 32
.global reboot_game
.type reboot_game @function 
reboot_game:
    la $sp, rebootStack + REBOOT_STACK_SIZE
    j reboot_game_internal
    nop
.balign 64
