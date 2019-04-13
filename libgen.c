#include "include/doc_ddr3sdram.h"
#include "include/doc_debug_uart.h"

#define BITS_PER_LONG 32

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 */
long get_ram_size(long *base, long maxsize)
{
	volatile long *addr;
        long           save[BITS_PER_LONG - 1];
        long           save_base;
        long           cnt;
        long           val;
        long           size;
        int            i = 0, j;

//cnt = 2^27 = 134217728; cnt > 0; cnt = cnt >> 1
//	for (cnt = 134217728; cnt > 0; cnt >>= 1) {
//this loop is saving the exisiting values @ addr A0000000, 90000000, 88000000 .... 80000000
//into save[31] and writing new values
	for (cnt = (maxsize / sizeof(long)) >> 1; cnt > 0; cnt >>= 1) {
                addr = base + cnt;      /* pointer arith! */
                save[i++] = *addr;
                *addr = ~cnt; // 0x7FFFFFF 0xEFFFFFF
        }

	addr = base;
        save_base = *addr; 
        *addr = 0;

	if ((val = *addr) != 0) {
                /* Restore the original data before leaving the function. */
                *base = save_base;
                for (cnt = 1; cnt < maxsize / sizeof(long); cnt <<= 1) {
                        addr  = base + cnt;
                        *addr = save[--i];
                }
//		print_str_nl("in val = *addr condition");
                return (0);
        }

//maxsize / sizeof(long) = 2^28 = 268435456 = 0x10000000
//	for (cnt = 1; cnt < 0x10000000; cnt <<= 1) {
	for (cnt = 1; cnt < maxsize / sizeof(long); cnt <<= 1) {
                addr = base + cnt;      /* pointer arith! */
                val = *addr;
//		print_hex((unsigned int)val);
//		print_nl();
                *addr = save[--i];
//		print_hex((unsigned int)cnt);
//		print_nl();
                if (val != ~cnt) {
                        size = cnt * sizeof(long);
//			print_hex((unsigned int)size);
//			print_nl();
                        /*
                         * Restore the original data
                         * before leaving the function.
                         */
                        for (cnt <<= 1; cnt < 0x10000000; cnt <<= 1) {
                                addr  = base + cnt;
                                *addr = save[--i];
                        }
                        /* warning: don't restore save_base in this case,
                         * it is already done in the loop because
                         * base and base+size share the same physical memory
                         * and *base is saved after *(base+size) modification
                         * in first loop
                         */
                        return (size);
                }
        }
	*base = save_base;

        return (maxsize);
}
