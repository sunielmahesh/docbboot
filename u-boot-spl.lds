/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *	Aneesh V <aneesh@ti.com>
 *
 *
 * MEMORY { .sram : ORIGIN = CONFIG_SPL_TEXT_BASE,\
 *		LENGTH = CONFIG_SPL_MAX_SIZE }
 * MEMORY { .sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
 *		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }
 *
 * The linker’s default configuration permits allocation of all available memory.
 * MEMORY is a command. using this command we can tell linker which memory region
 * to use and which memory region to avoid. we can also assign sections (text, data)
 * to particular memory regions. syntax:
 * MEMORY
 * {
 *   name [(attr)] : ORIGIN = origin, LENGTH = len
 * }
 * in the above case:
 * MEMORY
 * {
 *   .sram : ORIGIN = origin, LENGTH = len
 * }
 *
 *
 *
 * OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
 * the above are the following output formats defined, all are elf32 little indian
 * if we want we can have big endian as well, refer this section in gnu_linker_syntax.pdf
 * OUTPUT_ARCH(arm)
 * ENTRY(_start) - the entry point for the binary is _start, this is defined in vector.S 
 * SECTIONS - all sections of the binary are defined here
 * {
 * SECTIONS are divided into input and output sections
 * eg: 
 * .text : { *(.text)}
 * meaning .text is an output section, which is formed by input sections
 * this ouput section is made of various text sections of different files *(.text)
 * so various text sections of different files serve as input sections
 *	.text      :
 *	{
 *		__start = .; assigning '.' location counter which is the first downloadable address to __start - 0x402F0400
 *		*(.vectors) - first code will be vector.S
 *		arch/arm/cpu/armv7/start.o	(.text*)
 *		*(.text*) - matching any text section which has characters after text, i dontknow may be some
 * files haave text sections ending with .text**** and so on
 *	} >.sram - put this section at .sram, which is beginning 0x402F0400
 *
 *	. = ALIGN(4);  - align the current location on a 4 byte boundary
 * each instruction on 32 bit arm is 4 byte wide
 * 
 * 
 *	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram
 *
 * SORT_BY_ALIGNMENT is very similar to SORT_BY_NAME. The difference is SORT_BY_ALIGNMENT will sort
 * sections into descending order by alignment before placing them in the output file. Larger alignments are
 * placed before smaller alignments in order to reduce the amount of padding necessary.
 *
 * rodata, data, contains may be data structures and so on, so alignments may big or small
 * bss also contain data structures but i dont know why SORT_BY_ALIGNMENT is not used
 *
 *	. = ALIGN(4); - align the current location on a 4 byte boundary
 *	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram
 *
 *	. = ALIGN(4);
 *	.u_boot_list : {
 *		KEEP(*(SORT(.u_boot_list*)));
 *	} >.sram
 *
 *	. = ALIGN(4);
 *	__image_copy_end = .; - end location pointer is assigned to __image_copy_end
 *
 *	.end :  
 *	{
 *		*(.__end)   - all ends of various files are accumulated here, may be every file has an end section.
 *	}
 *
 *  every elf source file might have __start and end sections i think
 *
 *	_image_binary_end = .;  -end location pointer is assigned to __image_binary_end
 *
 * __image_copy_end and __image_binary_end are may be two symbols, binary end implies end of binary
 *
 *	.bss :
 *	{
 *		. = ALIGN(4);
 *		__bss_start = .;
 *		*(.bss*)
 *		. = ALIGN(4);
 *		__bss_end = .;
 *	} >.sdram
 *
 * bss is located in SDRAM at location  0x80A00000
 * align is used twice only for this section
 * SORT_BY_ALIGNMENT not used here
 * }
 ------------------------------------------- 
--------------------------------------------
--------------------------------------------
----------start of docbboot------------------*/

/*
 * The MEMORY command describes the location and size of blocks of memory in the target
 * describes which memory regions may be used by the linker, and which memory regions it 
 * must avoid
 */
MEMORY { .sram : ORIGIN = 0x402F0400, LENGTH = 0x1B000 }
MEMORY { .sdram : ORIGIN = 0x80A00000, LENGTH = 0x80000 }

/*
 * OUTPUT_FORMAT()
 * When ld is configured to support multiple object code formats, you can use this command 
 * to specify a particular output format
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")

/*
 * OUTPUT_ARCH()
 * Specify a particular output machine architecture
 */
OUTPUT_ARCH(arm)

/*
 * The linker command language includes a command specifically for defining the first executable 
 * instruction in an output file (its entry point). Its argument is a symbol name:
 * ENTRY(symbol)
 */ 
ENTRY(_start)

SECTIONS
{
        .text      :
        {
                __start = .;
                *(.vectors)
                start.o      (.text*)
                *(.text*)
        } >.sram

/*
 * ARM requires that the instructions be present in 32-bit aligned memory locations.
 * ALIGN(4) means that the address pointed by location pointer is always divisible by 4
 */
        . = ALIGN(4);
        .rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram

        . = ALIGN(4);
        .data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram

        . = ALIGN(4);
        .u_boot_list : {
                KEEP(*(SORT(.u_boot_list*)));
        } >.sram

        . = ALIGN(4);
        __image_copy_end = .;

        .end :
        {
                *(.__end)
        }

        _image_binary_end = .;

        .bss :
        {
                . = ALIGN(4);
                __bss_start = .;
                *(.bss*)
                . = ALIGN(4);
                __bss_end = .;
        } >.sdram
}
