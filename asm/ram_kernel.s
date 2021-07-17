.set noreorder
.set gp=64

.section .text

.globl clear_ram_and_boot
clear_ram_and_boot: # clear_ram_and_boot(entrypoint)

	la $t0, 0x80100000

1:
		sw $zero, ($t0)
		sw $zero, 4($t0)
		sw $zero, 8($t0)
		sw $zero, 0xC($t0)
ble $t0, 0x803FFFF0, 1b
addiu $t0, 16;

	jr $a0
	nop

