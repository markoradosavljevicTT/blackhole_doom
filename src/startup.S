.section .init
.global _start
_start:
	j 1f
.org 0x20
1:	li sp, 0x180000
	call main
	fence
