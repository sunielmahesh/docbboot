#include "include/doc_watchdog.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_io_barrier.h"

#define PTV                     0       /* prescale */
#define WDT_WCLR_PRE		1<<5
#define WDT_WCLR_PTV_OFF        2
/* Hardware timeout in seconds */
#define WDT_HW_TIMEOUT 60
/*
 * watchdog timer load register WDT_WLDR, holds the load value 
 * prescaler ratios PTV are chosen as 0 
 * for 60s(OVF_Rate) what value we need to load
 * OVF_Rate = (FFFF FFFFh – WDT_WLDR + 1) × (wd-functional clock period) × PS
 * where wd-functional clock period = 1/(wd-functional clock frequency) and PS = 2^PTV = 1<<PTV
 * therefore WDT_WLDR = 0xFFFF FFFF - ((OVF_RATE*32768)/2^PTV) + 1
 */
#define GET_WLDR_VAL(secs)      (0xffffffff - ((secs) * (32768/(1<<PTV))) + 1)

static unsigned int wdt_trgr_pattern = 0x1234;

void hw_watchdog_reset(void)
{
        struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;

        /* wait for posted write to complete */
        while ((__raw_readl(&wdt->wdtwwps)) & (1 << 3))
                ;

        wdt_trgr_pattern = ~wdt_trgr_pattern;
        __raw_writel(wdt_trgr_pattern, &wdt->wdtwtgr); //Watchdog Trigger Register
// when that value is reached 

        /* wait for posted write to complete */
        while ((__raw_readl(&wdt->wdtwwps) & (1 << 3)))
                ;
}

static int am335_wdt_set_timeout(unsigned int timeout)
{
        struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;
        unsigned int pre_margin = GET_WLDR_VAL(timeout);

        while (__raw_readl(&wdt->wdtwwps) & (1 << 2))//polling
                ;

        __raw_writel(pre_margin, &wdt->wdtwldr); //writing value in WLDR
        while (__raw_readl(&wdt->wdtwwps) & (1 << 2)) //polling
                ;

        return 0;
}

void hw_watchdog_init(void)
{
        struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;

/* 
 * initialize prescaler 
 * wdtwwps anded with 0, implies there is no write pending in writeable functional registers
 * WCLR, WCRR, WLDR, WTGR, WSPR, WDLY
 */
        while (__raw_readl(&wdt->wdtwwps) & 0)
                ;

        __raw_writel(WDT_WCLR_PRE, &wdt->wdtwclr); //prescaler enabled
        while (__raw_readl(&wdt->wdtwwps) & 0) //making sure above write is done => polling
                ;

        am335_wdt_set_timeout(WDT_HW_TIMEOUT);

        /* Sequence to enable the watchdog */
        __raw_writel(0xBBBB, &wdt->wdtwspr);
        while ((__raw_readl(&wdt->wdtwwps)) & (1 << 4))
                ;

        __raw_writel(0x4444, &wdt->wdtwspr);
        while ((__raw_readl(&wdt->wdtwwps)) & (1 << 4))
                ;
}

void hw_watchdog_disable(void)
{
        struct wd_timer *wdt = (struct wd_timer *)WDT_BASE;
        /*
         * Disable watchdog
         */
	__raw_writel(0xAAAA, &wdt->wdtwspr);
        while (__raw_readl(&wdt->wdtwwps) != 0x0)
                ;
        __raw_writel(0x5555, &wdt->wdtwspr);
        while (__raw_readl(&wdt->wdtwwps) != 0x0)
                ;
}

