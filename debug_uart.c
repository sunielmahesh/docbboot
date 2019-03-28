/* see end of the file for notes */

#include "include/doc_debug_uart.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_io_barrier.h"

struct wkup_uart0 *com_port = (struct wkup_uart0 *)UART0_BASE;

/*
 * UART0 Transmit character - single byte
 * we are in an infinite loop, checking for LSR[5], if LSR[5] = 1 => TxFIFO is empty
 * then the loop breaks.
 */
void _debug_uart_putc(unsigned char ch)
{
//        struct wkup_uart0 *com_port = (struct wkup_uart0 *)UART0_BASE;

        __raw_writel(ch, &com_port->thr_rhr_dll);
        while (!(__raw_readl(&com_port->lsr) & 0x20))
                ;
}

/*
 * UART0 Receive character - single byte
 * we are in infinite loop to receive data so
 * checking for LSR[0]:
 * 0h = No data in the Rx FIFO.
 * 1h = At least one data character in the Rx FIFO
 * if LSR[0] = 1, then we have atleast one character,
 * in Rx FIFO, implies loop breaks
 * return contents of Rx FIFO, RHR
 */
unsigned char _debug_uart_getc(void)
{
//        struct wkup_uart0 *com_port = (struct wkup_uart0 *)UART0_BASE;

        while (!(__raw_readl(&com_port->lsr) & 0x01))
                ;
        return __raw_readl(&com_port->thr_rhr_dll);
}

void debug_rx(char *str, int max_len)
{
        int i;

        for (i = 0; i < max_len - 1; i++)
        {
                str[i] = _debug_uart_getc();
                if (str[i] == '\r')
                        break;
        }
        str[i] = 0;
}

void debug_tx(char *str)
{
        while (*str)
        {
                _debug_uart_putc(*str++);
        }
}

/*
 * here we initialise the UART0 registers, already the clock for UART0 is enabled
 * at poweron, still it is configured.
 * baud_divisor is chosen and to be written in DLL and DLH reg
 * baud_divisor = clock/(16*baudrate) = 48M/(16*115200)
 * disable mdr1 and then configure, there are different modes UART0 can operate, before changing the
 * registers, disable all modes and then configure, value 7 disables all modes  
 * registers MDR1 and LCR are selected with a register bit setting of 
 * LCR[7] = 0, LCR[7] not equal to BFh, or LCR[7] = BFh. no need to configure LCR initially to select
 * mdr1. LCR might be contaning the required value 
 * next we need to program efr, for this LCR should be 0xBF. program EFR[4] = 1 to access IER[7:4], FCR[5:4], and MCR[7:5]
 * next access ier register, for this LCR should be 0. here we disable all interrupts by writing 0
 * next access DLH and DLL registers, to program baud_divisor, meaning LCR should be BF.   
 * once its is done then enable interrupts back again especially THR and RHR
 * write EFR[4] back to 0, so that those cant be accessed
 * set word length to 8 bits and normal operation in LCR
 * enable UART16x mode => basic initialization complete
 * here we are not using TLR reg. This register stores the programmable transmit and receive FIFO trigger 
 * levels used for DMA and IRQ generation. but we can set and use them
 * there are other registers, but not important for now
 */
void _debug_uart_init(void)
{
//        struct wkup_uart0 *com_port = (struct wkup_uart0 *)UART0_BASE;
        unsigned short baud_divisor = 26;

	__raw_writel(0x7, &com_port->mdr1);
	__raw_writel(0xBF, &com_port->lcr);
	__raw_writel(0x1 << 4, &com_port->efr_iir_fcr); //select efr
	__raw_writel(0x00, &com_port->lcr);
        __raw_writel(0x00, &com_port->ier_dlh); //ier
        __raw_writel(0xBF, &com_port->lcr);

	 __raw_writel((baud_divisor & 0xFF), &com_port->thr_rhr_dll); //dll
        __raw_writel(((baud_divisor >> 8) & 0xFF), &com_port->ier_dlh); //dlh
	__raw_writel(0x00, &com_port->lcr);
        __raw_writel(0x3, &com_port->ier_dlh); //ier
	__raw_writel(0xBF, &com_port->lcr);
        __raw_writel((__raw_readl(&com_port->efr_iir_fcr) & 0xEF), &com_port->efr_iir_fcr); //efr
	__raw_writel(0x03, &com_port->lcr);
        __raw_writel(0x00, &com_port->mdr1);

}

void debug_uart_init(void)
{
	char str[10];

        _debug_uart_init();
/*
 * After init we are calling getc, why ?, to give it some time after initialising the registers
 * and clear the Rx fifo. because first we hit enter key to start and this key is stored in Rx fifo
 */
	scan_char();
	print_nl();
        print_str_nl("debug port: uart0 is enabled");
        print_str_nl("test case start, printing numbers");
	print_num(56);
	print_nl();
	print_hex(256);
	print_nl();
        print_str_nl("debug port: test case end");
/*
 * this for loop is behaving like a console
 * comment when not required - used for testing whether debug is enabled properly or not
 
	for (;;)
        {
                print_str("docbboot > ");
                scan_line(str, 10);
		print_str(str);
                print_nl(); //place cursor in new line
        }
*/
}

/*
 * This is the bare metal driver for UART0 - BBB
 * the logic here is to transfer data byte by byte.
 * we are not using Tx and Rx FIFO's or DMA, hence no 
 * trigger level. trigger level implies since the FIFO
 * is 64 bytes, interrupt is generated when that trigger
 * level is reached (8bytes, 16bytes, ...), so that all 
 * bytes are read or transferred one by one.

 * so in this case only one byte

 * in baremetal enter key(from host machine because of minicom) is treated as \r.

 * \r is carriage return - generally the cursor returns to the start of the current line
 * \n is  line feed - moves cursor to newline but at that position itself not all the
 * way to left

 * so we must use \r\n combination.
 */ 
