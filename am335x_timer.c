#include "include/doc_io_barrier.h"
#include "include/doc_timer.h"
#include "include/doc_debug_uart.h"

#define CONFIG_SYS_TIMERBASE	0x48040000
#define CONFIG_SYS_PTV		2
#define TIMER_LOAD_VAL          0

#define V_SCLK			24000000
#define TIMER_CLOCK             (V_SCLK / (2 << CONFIG_SYS_PTV))
#define TIMER_OVERFLOW_VAL      0xffffffff


int timer_init(void)
{
	struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;

        /* start the counter ticking up, reload value on overflow */
        __raw_writel(TIMER_LOAD_VAL, &timer_base->tldr);
        /* enable timer */
        __raw_writel((CONFIG_SYS_PTV << 2) | TCLR_PRE | TCLR_AR | TCLR_ST, &timer_base->tclr);

/*        print_nl();
	print_str("TCRR reg: ");
        print_num(__raw_readl(&timer_base->tcrr));
        print_nl();
	print_str("TLDR reg: ");
        print_num(__raw_readl(&timer_base->tldr));
        print_nl();
	print_str("TCLR reg: ");
        print_num(__raw_readl(&timer_base->tclr));
        print_nl(); */

        return 0;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	struct gptimer *timer_base = (struct gptimer *)CONFIG_SYS_TIMERBASE;
        long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
        unsigned long now, last = __raw_readl(&timer_base->tcrr);

/*	print_num(tmo);
	print_nl();
	print_num(last);
	print_nl(); */
        while (tmo > 0) {
                now = __raw_readl(&timer_base->tcrr);
/*		print_num(now);
		print_nl(); */
                if (last > now) {/*  count up timer overflow */
                        tmo -= TIMER_OVERFLOW_VAL - last + now + 1;
/*			print_num(tmo);
			print_nl(); */
		}
                else {
                        tmo -= now - last;
/*			print_num(tmo);
			print_nl();*/
		}
                last = now;
	}
}
