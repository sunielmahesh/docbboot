/*
 * here we are doing soc initialization
 */

#include "include/doc_io_barrier.h"
#include "include/doc_watchdog.h"
#include "include/doc_board.h"
#include "include/doc_hardware_am33xx.h"

static void watchdog_disable(void)
{
        struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

        writel(0xAAAA, &wdtimer->wdtwspr);
        while (readl(&wdtimer->wdtwwps) != 0x0)
                ;
        writel(0x5555, &wdtimer->wdtwspr);
        while (readl(&wdtimer->wdtwwps) != 0x0)
                ;
}

void early_system_init(void)
{
	watchdog_disable();
/*        set_uart_mux_conf();
        setup_early_clocks();
        uart_soft_reset(); */
/*
 * Save the boot parameters passed from romcode.
 * We cannot delay the saving further than this,
 * to prevent overwrites.
 *
 *      save_omap_boot_params();
 */
/* Enable RTC32K clock */
//        rtc32k_enable();
}

void board_init_f(void)
{
	early_system_init();
 //       board_early_init_f();
//        sdram_init();
}
