/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * Do important init only if we don't start from memory!
 * Setup memory and board specific bits prior to relocation.
 * Relocate armboot to ram. Setup stack.
 *
 *************************************************************************/

	.global  reset  /* global or globl */

/* 
 * .global directive is used to make a symbol global, so that it can be used in other files
 * here none of the symbols (save_boot_params, cpu_init_cp15, cpu_init_crit, _main, save_boot_params_ret)
 * will be used in other files, so no global required
 * if a symbol is made global it will appear in .map file 
 */ 


reset:
/* Allow the board to save important registers */
	b save_boot_params

save_boot_params_ret:

/*
 * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
 * except if in HYP mode already
 * check for hypervisor mode is made here and also bootparams, refer to start.S main file
 * for more details
 * mrc implies read
 */

	mrs     r0, cpsr
        bicne   r0, r0, #0x1f           @ clear all mode bits
        orrne   r0, r0, #0x13           @ set SVC mode
        orr     r0, r0, #0xc0           @ disable FIQ and IRQ
        msr     cpsr,r0

/* Set V=0 bit in CP15 SCTLR register - for VBAR to point to vector 
   implies normal exception vectors selected, not high exception vectors */

	mrc     p15, 0, r0, c1, c0, 0   @ Read CP15 control Register
        bic     r0, #0x2000             @ V bit cleares => normal exception vectors selected, not high exception vectors
        mcr     p15, 0, r0, c1, c0, 0   @ Write CP15 SCTLR Register

/* Set vector address in CP15 VBAR register 
 * r0 is loaded with address of _start symbol 
 * and it is written in c12 which is VBAR register
 * _start is the entry point for this binary
 * in vector.S it is defined as global symbol and contains exception vectors
 */

        ldr     r0, =_start
        mcr     p15, 0, r0, c12, c0, 0  @Set VBAR

	bl      cpu_init_cp15
	bl      cpu_init_crit

	bl	_main

/*
 * here r0 might be containing some information, store it in scratch space SRAM location @ 0x4030B424
 * define OMAP_SRAM_SCRATCH_BOOT_PARAMS	(SRAM_SCRATCH_SPACE_ADDR + 0x24)
 * = 0x4030B400 + 0x24
 * save_boot_params was grabbed from arch/arm/mach-omap2/lowlevel_init.S 
 */

save_boot_params:
        ldr     r1, =0x4030B424
        str     r0, [r1]
        b       save_boot_params_ret

/*
 * cpu_init_cp15
 *
 * Setup CP15 registers (cache, MMU, TLBs). The I-cache is turned on unless
 * CONFIG_SYS_ICACHE_OFF is defined. here I-cache is turned on
 * mcr => write into registers
 */

cpu_init_cp15:
/*
 * Invalidate L1 I/D
 * research on the sequence how to do this - may be later
 */
        mov     r0, #0                  @ set up for MCR
        mcr     p15, 0, r0, c8, c7, 0   @ invalidate instruction and data TLBs
        mcr     p15, 0, r0, c7, c5, 0   @ invalidate icache
        mcr     p15, 0, r0, c7, c5, 6   @ invalidate BP array
        mcr     p15, 0, r0, c7, c10, 4  @ DSB operation
        mcr     p15, 0, r0, c7, c5, 4   @ ISB operation

/*
 * disable MMU stuff and caches
 * summary: disable MMU, data caches, use normal exception vector, decided by VBAR
 * enable instruction cache, program flow prediction and strict alignment
 */
        mrc     p15, 0, r0, c1, c0, 0   @ read c1 into r0
        bic     r0, r0, #0x00002000     @ clear bits 13 (--V-)
        bic     r0, r0, #0x00000007     @ clear bits 2:0 (-CAM)
        orr     r0, r0, #0x00000002     @ set bit 1 (--A-) Align
        orr     r0, r0, #0x00000800     @ set bit 11 (Z---) BTB
	orr     r0, r0, #0x00001000     @ set bit 12 (I) I-cache
	mcr     p15, 0, r0, c1, c0, 0   @ write r0 into c1

        mrc     p15, 0, r1, c0, c0, 0   @ r1 has Read Main ID Register (MIDR)
        mov     r3, r1, lsr #20         @ get variant field
        and     r3, r3, #0xf            @ r3 has CPU variant
        and     r4, r1, #0xf            @ r4 has CPU revision
        mov     r2, r3, lsl #4          @ shift variant field for combined value
        orr     r2, r4, r2              @ r2 has combined CPU variant + revision


	bx  lr                  @ back to my caller

/*
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 */

cpu_init_crit:
        /*
         * Jump to board specific initialization...
         * The Mask ROM will have already initialized
         * basic memory. Go here to bump up clock rate and handle
         * wake up conditions.
         */
        b       lowlevel_init           @ go setup pll,mux,memory

lowlevel_init:

/*
 * Setup a temporary stack. Global data is not available yet.
 * the code below just set's up a temporary stack and makes r9 to store global data, here
 * global data is ignored.
 * #define CONFIG_SYS_INIT_SP_ADDR NON_SECURE_SRAM_END - GENERATED_GBL_DATA_SIZE)
 * tries searching GENERATED_GBL_DATA_SIZE or how it is computed but couldnt find
 * disassembled lowlevel_init.o in spl and found out that GENERATED_GBL_DATA_SIZE = 224 bytes
 * 0:	e59fd010 	ldr	sp, [pc, #16]	; 18 <lowlevel_init+0x18>
 * 4:	e3cdd007 	bic	sp, sp, #7
 * 8:	e3a09000 	mov	r9, #0
 * c:	e92d5000 	push	{ip, lr}
 * 10:	ebfffffe 	bl	0 <lowlevel_init>
 * 14:	e8bd9000 	pop	{ip, pc}
 * 18:	4030ff20 	.word	0x4030ff20
 * ldr     sp, =CONFIG_SYS_INIT_SP_ADDR = 0x40310000 - E0 = 0x4030FF00
 */ 

	ldr     sp, =0x4030FF20
	bic     sp, sp, #7 /* 8-byte alignment for ABI compliance => last three bits should be always zero */
	mov     r9, #0 /* #ifdef CONFIG_SPL_DM */
	bx      lr /* return to caller */

_main:
	b .
