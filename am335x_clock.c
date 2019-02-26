#include "include/doc_io_barrier.h"
#include "include/doc_clock.h"
#include "include/doc_cpu.h"
#include "include/doc_hardware_am33xx.h"

#define TIMER_LOAD_VAL		0
#define LDELAY			1000000

struct cm_perpll *const cmper = (struct cm_perpll *)CM_PER;
struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;
struct cm_rtc *const cmrtc = (struct cm_rtc *)CM_RTC;

static struct gptimer *timer_base = (struct gptimer *)0x48040000;

void setup_clocks_for_console(void)
{
/* Start a software forced wake-up transition on the domain 
 look at the 12th bit as well and 2nd bit */
        clrsetbits_le32(&cmwkup->wkclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
                        CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
                        CD_CLKCTRL_CLKTRCTRL_SHIFT);

/* Start a software forced wake-up transition on the PER domain 
  wake up in the sense may be reset */
        clrsetbits_le32(&cmper->l4hsclkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
                        CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
                        CD_CLKCTRL_CLKTRCTRL_SHIFT);
/* enable clock to UART0, but the module looks disabled because of 16,17 bit */
        clrsetbits_le32(&cmwkup->wkup_uart0ctrl,
                        MODULE_CLKCTRL_MODULEMODE_MASK,
                        MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
                        MODULE_CLKCTRL_MODULEMODE_SHIFT);
}

static inline void enable_clock_domain(u32 *const clkctrl_reg, u32 enable_mode)
{
        clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
                        enable_mode << CD_CLKCTRL_CLKTRCTRL_SHIFT);
//        debug("Enable clock domain - %p\n", clkctrl_reg);
}

/* this looks like some kind of a delay or making sure value is written in reg 
   here delay is soft delay => decrementing some number */
static inline void wait_for_clk_enable(u32 *clkctrl_addr)
{
        u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_DISABLED;
        u32 bound = LDELAY;

        while ((idlest == MODULE_CLKCTRL_IDLEST_DISABLED) ||
                (idlest == MODULE_CLKCTRL_IDLEST_TRANSITIONING)) {
                clkctrl = readl(clkctrl_addr);
                idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
                         MODULE_CLKCTRL_IDLEST_SHIFT;
                if (--bound == 0) {
//                        printf("Clock enable failed for 0x%p idlest 0x%x\n",
//                               clkctrl_addr, clkctrl);
                        return;
                }
        }
}

static inline void enable_clock_module(u32 *const clkctrl_addr, u32 enable_mode,
                                       u32 wait_for_enable)
{
        clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
                        enable_mode << MODULE_CLKCTRL_MODULEMODE_SHIFT);
//        debug("Enable clock module - %p\n", clkctrl_addr);
        if (wait_for_enable)
                wait_for_clk_enable(clkctrl_addr);
}

void do_enable_clocks(u32 *const *clk_domains,
                      u32 *const *clk_modules_explicit_en, u8 wait_for_enable)
{
        u32 i, max = 100;

        /* Put the clock domains in SW_WKUP mode */
        for (i = 0; (i < max) && clk_domains[i]; i++) {
                enable_clock_domain(clk_domains[i],
                                    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
        }

        /* Clock modules that need to be put in SW_EXPLICIT_EN mode */
        for (i = 0; (i < max) && clk_modules_explicit_en[i]; i++) {
                enable_clock_module(clk_modules_explicit_en[i],
                                    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
                                    wait_for_enable);
        };
}

void enable_basic_clocks(void)
{
        u32 *const clk_domains[] = {
                &cmper->l3clkstctrl,
                &cmper->l4fwclkstctrl,
                &cmper->l3sclkstctrl,
                &cmper->l4lsclkstctrl,
                &cmwkup->wkclkstctrl,
                &cmper->emiffwclkctrl,
                &cmrtc->clkstctrl,
                0
        };

        u32 *const clk_modules_explicit_en[] = {
                &cmper->l3clkctrl,
                &cmper->l4lsclkctrl,
                &cmper->l4fwclkctrl,
                &cmwkup->wkl4wkclkctrl,
                &cmper->l3instrclkctrl,
                &cmper->l4hsclkctrl,
                &cmwkup->wkgpio0clkctrl,
                &cmwkup->wkctrlclkctrl,
                &cmper->timer2clkctrl,
                &cmper->gpmcclkctrl,
                &cmper->elmclkctrl,
                &cmper->mmc0clkctrl,
                &cmper->mmc1clkctrl,
                &cmwkup->wkup_i2c0ctrl,
                &cmper->gpio1clkctrl,
                &cmper->gpio2clkctrl,
                &cmper->gpio3clkctrl,
                &cmper->i2c1clkctrl,
                &cmper->cpgmac0clkctrl,
                &cmper->spi0clkctrl,
                &cmrtc->rtcclkctrl,
                &cmper->usb0clkctrl,
		&cmper->emiffwclkctrl,
                &cmper->emifclkctrl,
                0
        };

        do_enable_clocks(clk_domains, clk_modules_explicit_en, 1);

        /* Select the Master osc 24 MHZ as Timer2 clock source */
        writel(0x1, &cmdpll->clktimer2clk);
}

int timer_init(void)
{
        /* start the counter ticking up, reload value on overflow */
        writel(TIMER_LOAD_VAL, &timer_base->tldr);
        /* enable timer */
//        writel((CONFIG_SYS_PTV << 2) | TCLR_PRE | TCLR_AR | TCLR_ST, &timer_base->tclr);
        writel((2 << 2) | TCLR_PRE | TCLR_AR | TCLR_ST, &timer_base->tclr);

        return 0;
}

void setup_early_clocks(void)
{
        setup_clocks_for_console();
	enable_basic_clocks();
	timer_init();
}
