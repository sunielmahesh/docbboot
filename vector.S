/*
 * SPL doesnt use exception vectors, if an exception occurs then it goes into
 * dead loop as written below
 */	
	.macro ARM_VECTORS
        
	b       reset
        ldr     pc, _undefined_instruction
        ldr     pc, _software_interrupt
        ldr     pc, _prefetch_abort
        ldr     pc, _data_abort
        ldr     pc, _not_used
        ldr     pc, _irq
        ldr     pc, _fiq
        
	.endm

.globl _start
	
/*
 * in linker script one of the input sections was *(.vectors) for the output section .text
 * define it here
 * Vectors have their own section so linker script can map them easily
 */

.section ".vectors", "ax"

_start:
	ARM_VECTORS

        .globl  reset
        .globl  _undefined_instruction
        .globl  _software_interrupt
        .globl  _prefetch_abort
        .globl  _data_abort
        .globl  _not_used
        .globl  _irq
        .globl  _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:    .word software_interrupt
_prefetch_abort:        .word prefetch_abort
_data_abort:            .word data_abort
_not_used:              .word not_used
_irq:                   .word irq
_fiq:                   .word fiq

        .balignl 16,0xdeadbeef
/* align the location counter on a 16 byte boundary, multiple of 16. pad the empty bytes with 0xdeadbeef */
/* reason for above is not clear */
/*
 * exception handling, in SPL just hang, no code
 * .align  x meaning next byte should be multiple of 2^x
 * if .align  4, meaning 2^4 = 16 byte, so next address would be multiple of 16 byte 
 * code or data is filled at bytes which are multiple of 16.
 * here .align 5, 2^5 = 32 byte
 * so exception vectors are place on a 32 byte boundary, for some reason here exception
 * vectors are placed on 32 byte boundary,  
 */
 
.align  5 
undefined_instruction:
software_interrupt:
prefetch_abort:
data_abort:
not_used:
irq:
fiq:
1:
	b	1b	
/* hang and never return meaning, b 1b implies branch to label 1 backwards, so infinite loop */
	
