.section .init
.global _start
_start:
	call main
	fence
