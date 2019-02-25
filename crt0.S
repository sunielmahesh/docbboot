/*
 * crt0 - C runtime startup Code for ARM docBoot
 */

	.global  _main

_main:
	bl	board_init_f
	b	.
