/*
 * here we are doing soc initialization
 */

#include "include/doc_io_barrier.h"
#include "include/doc_watchdog.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_board.h"
#include "include/doc_cpu.h"
#include "include/doc_clock.h"
#include "include/doc_davinci_rtc.h"
#include "include/doc_debug_uart.h"
#include "include/doc_i2c.h"
#include "include/doc_mux.h"
#include "include/doc_ddr3sdram.h"

static void watchdog_disable(void)
{
	hw_watchdog_disable();
}

void set_uart_mux_conf(void)
{
        enable_uart0_pin_mux();
}

static void uart_soft_reset(void)
{
        struct uart_sys *uart_base = (struct uart_sys *)UART0_BASE;
        u32 regval;

        regval = readl(&uart_base->uartsyscfg);
        regval |= UART_RESET;
        writel(regval, &uart_base->uartsyscfg);
/* while loop indicates reset is done => comes out of loop */
        while ((readl(&uart_base->uartsyssts) &
                UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK);

        /* Disable smart idle */
        regval = readl(&uart_base->uartsyscfg);
        regval |= UART_SMART_IDLE_EN;
        writel(regval, &uart_base->uartsyscfg);
}

static void rtc32k_unlock(struct davinci_rtc *rtc)
{
        /*
         * Unlock the RTC's registers.  For more details please see the
         * RTC_SS section of the TRM.  In order to unlock we need to
         * write these specific values (keys) in this order.
         */
        writel(RTC_KICK0R_WE, &rtc->kick0r);
        writel(RTC_KICK1R_WE, &rtc->kick1r);
}

static void rtc32k_enable(void)
{
        struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;

        rtc32k_unlock(rtc);

        /* Enable the RTC 32K OSC by setting bits 3 and 6. */
        writel((1 << 3) | (1 << 6), &rtc->osc);
}

void do_board_detect(void)
{
	unsigned int addr = 0x0;
	unsigned char hdr_read[12];
	unsigned char len = 12; // read the first 12 bytes of eeprom
        unsigned char byte = 2;
	unsigned int cp = 0;
	int i, j = 0;

	enable_i2c0_pin_mux();
	i2c_init();
	ti_i2c_eeprom_read(addr, CONFIG_EEPROM_CHIP_ADDRESS, byte, hdr_read, len);
	
	print_str("EEPROM header contents are 0x");
        for (i = 0; i < 4; i++ ) {
                cp = cp | (hdr_read[i] << j);
                j = j + 8;
        }
        print_hex(cp);
        print_nl();

        print_str("Board name read from EEPROM:");
        j = 48;
        for (i = 4; i < 12; i++ ) {
                cp = hdr_read[i];
                if (cp > 47 && cp < 58) {
                        cp = cp - j;
                        print_num(cp);
                } else {
                        if (cp == 65)
                                print_str("A");
                        else if (cp == 66)
                                print_str("B");
                        else if (cp == 76)
                                print_str("L");
                        else if (cp == 78)
                                print_str("N");
                        else if (cp == 84)
                                print_str("T");
                }
        }
        print_nl();
}

void early_system_init(void)
{
	watchdog_disable();
	set_uart_mux_conf();
	setup_early_clocks();
	uart_soft_reset();
	debug_uart_init();
	do_board_detect();
/* Enable RTC32K clock */
	rtc32k_enable();
}

void board_init_f(void)
{
	early_system_init();
	board_early_init_f();
	sdram_init();
}
