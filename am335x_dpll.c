/*
 * configure ADPLLS i.e various DPLL's 
 * CORE, MPU, PER, DDR DPLL
 * refer programming dpll doc for more info
 */

#include "include/doc_types.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_io_barrier.h"
#include "include/doc_clock.h"
#include "include/doc_debug_uart.h"
#include "include/doc_cpu.h"
#include "include/doc_timer.h"

const struct dpll_params dpll_core_1000MHz[NUM_CRYSTAL_FREQ] = {
                {625, 11, -1, -1, 10, 8, 4},    /* 19.2 MHz */
                {125, 2, -1, -1, 10, 8, 4},     /* 24 MHz */
                {40, 0, -1, -1, 10, 8, 4},      /* 25 MHz */
                {500, 12, -1, -1, 10, 8, 4}     /* 26 MHz */
//		{M,   N, m2, m3, m4, m5, m6}
};

const struct dpll_params dpll_per_192MHz[NUM_CRYSTAL_FREQ] = {
                {400, 7, 5, -1, -1, -1, -1},    /* 19.2 MHz */
                {400, 9, 5, -1, -1, -1, -1},    /* 24 MHz */
                {384, 9, 5, -1, -1, -1, -1},    /* 25 MHz */
                {480, 12, 5, -1, -1, -1, -1}    /* 26 MHz */
};

const struct dpll_params dpll_ddr3_400MHz[NUM_CRYSTAL_FREQ] = {
                {125, 5, 1, -1, -1, -1, -1}, /*19.2*/
                {50, 2, 1, -1, -1, -1, -1}, /* 24 MHz */
//                {303, 23, 1, -1, -1, -1, -1}, /* 24 MHz */
                {16, 0, 1, -1, -1, -1, -1}, /* 25 MHz */
                {200, 12, 1, -1, -1, -1, -1}  /* 26 MHz */
};

const struct dpll_params dpll_mpu_opp[NUM_CRYSTAL_FREQ][NUM_OPPS] = {
        {       /* 19.2 MHz */
                {125, 3, 2, -1, -1, -1, -1},    /* OPP 50 */
                {-1, -1, -1, -1, -1, -1, -1},   /* OPP RESERVED */
                {125, 3, 1, -1, -1, -1, -1},    /* OPP 100 */
                {150, 3, 1, -1, -1, -1, -1},    /* OPP 120 */
                {125, 2, 1, -1, -1, -1, -1},    /* OPP TB */
                {625, 11, 1, -1, -1, -1, -1}    /* OPP NT */
        },
        {       /* 24 MHz */
                {25, 0, 2, -1, -1, -1, -1},     /* OPP 50 */
                {-1, -1, -1, -1, -1, -1, -1},   /* OPP RESERVED */
                {25, 0, 1, -1, -1, -1, -1},     /* OPP 100 */
                {30, 0, 1, -1, -1, -1, -1},     /* OPP 120 */
                {100, 3, 1, -1, -1, -1, -1},    /* OPP TB */
                {125, 2, 1, -1, -1, -1, -1}     /* OPP NT */
        },
        {       /* 25 MHz */
                {24, 0, 2, -1, -1, -1, -1},     /* OPP 50 */
                {-1, -1, -1, -1, -1, -1, -1},   /* OPP RESERVED */
                {24, 0, 1, -1, -1, -1, -1},     /* OPP 100 */
                {144, 4, 1, -1, -1, -1, -1},    /* OPP 120 */
                {32, 0, 1, -1, -1, -1, -1},     /* OPP TB */
                {40, 0, 1, -1, -1, -1, -1}      /* OPP NT */
        },
        {       /* 26 MHz */
                {300, 12, 2, -1, -1, -1, -1},   /* OPP 50 */
                {-1, -1, -1, -1, -1, -1, -1},   /* OPP RESERVED */
                {300, 12, 1, -1, -1, -1, -1},   /* OPP 100 */
                {360, 12, 1, -1, -1, -1, -1},   /* OPP 120 */
                {400, 12, 1, -1, -1, -1, -1},   /* OPP TB */
                {500, 12, 1, -1, -1, -1, -1}    /* OPP NT */
        },
};

//m2, m3 are ignored since they are -1
const struct dpll_regs dpll_core_regs = {
        .cm_clkmode_dpll        = CM_WKUP + 0x90,
        .cm_idlest_dpll         = CM_WKUP + 0x5C,
        .cm_clksel_dpll         = CM_WKUP + 0x68,
        .cm_div_m4_dpll         = CM_WKUP + 0x80,
        .cm_div_m5_dpll         = CM_WKUP + 0x84,
        .cm_div_m6_dpll         = CM_WKUP + 0xD8,
};

const struct dpll_regs dpll_mpu_regs = {
        .cm_clkmode_dpll        = CM_WKUP + 0x88,
        .cm_idlest_dpll         = CM_WKUP + 0x20,
        .cm_clksel_dpll         = CM_WKUP + 0x2C,
        .cm_div_m2_dpll         = CM_WKUP + 0xA8,
};

const struct dpll_regs dpll_per_regs = {
        .cm_clkmode_dpll        = CM_WKUP + 0x8C,
        .cm_idlest_dpll         = CM_WKUP + 0x70,
        .cm_clksel_dpll         = CM_WKUP + 0x9C,
        .cm_div_m2_dpll         = CM_WKUP + 0xAC,
};

const struct dpll_regs dpll_ddr_regs = {
        .cm_clkmode_dpll        = CM_WKUP + 0x94,
        .cm_idlest_dpll         = CM_WKUP + 0x34,
        .cm_clksel_dpll         = CM_WKUP + 0x40,
        .cm_div_m2_dpll         = CM_WKUP + 0xA0,
};

void setup_post_dividers(const struct dpll_regs *dpll_regs, const struct dpll_params *params)
{
	unsigned int rval = 0;
        /* Setup post-dividers */
	
        if (params->m2 >= 0) {
		rval = *((volatile unsigned int *)(0x44E00400 + 0xA0));
		rval = rval & ~0x1F;
		rval = rval | params->m2;
/*		rval = __raw_readl(dpll_regs->cm_div_m2_dpll);
		print_hex(params->m2);
		print_nl();
		print_hex(params->m2);
		print_nl();
		__raw_writel(params->m2, dpll_regs->cm_div_m2_dpll);*/
		*((volatile unsigned int *)(0x44E00400 + 0xA0)) = rval;
/*		print_str("addr cm_div_m2_dpll reg:");
		print_hex(dpll_regs->cm_div_m2_dpll);
		print_nl();
		print_str("checking cm_div_m2_dpll reg:"); */
		rval = *((volatile unsigned int *)(0x44E00400 + 0xA0));
//		print_hex(rval);
//		print_nl();
	}
        if (params->m3 >= 0)
                __raw_writel(params->m3, dpll_regs->cm_div_m3_dpll);
        if (params->m4 >= 0) {
                __raw_writel(params->m4, dpll_regs->cm_div_m4_dpll);
/*		print_str("checking cm_div_m4_dpll reg:");
		print_hex(__raw_readl(dpll_regs->cm_div_m4_dpll));
		print_nl(); */
	}
        if (params->m5 >= 0) {
		__raw_writel(params->m5, dpll_regs->cm_div_m5_dpll);
/*		print_str("checking cm_div_m5_dpll reg:");
		print_hex(__raw_readl(dpll_regs->cm_div_m5_dpll));
		print_nl(); */
	}
        if (params->m6 >= 0) {
		__raw_writel(params->m6, dpll_regs->cm_div_m6_dpll);
/*		print_str("checking cm_div_m6_dpll reg:");
		print_hex(__raw_readl(dpll_regs->cm_div_m6_dpll));
		print_nl(); */
	}
}

/*
 * after setting parameters in respective registers we need to lock DPLL  
 * enable DPLL in lock mode => bits(2:0) of CM_CLKMODE_DPLL_CORE Register are 111
 */
static inline void do_lock_dpll(const struct dpll_regs *dpll_regs)
{
        clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
                        CM_CLKMODE_DPLL_DPLL_EN_MASK,
                        DPLL_EN_LOCK << CM_CLKMODE_DPLL_EN_SHIFT);
/*	print_str("checking cm_clockmode_dpll reg for lock:");
	print_hex(__raw_readl(dpll_regs->cm_clkmode_dpll));
	print_nl(); */
}

/*
 * check the CM_IDLEST_DPLL_CORE register, the above step is setting this register i think
 * if DPLL is locked then bit(0) = 1
 */
static inline void wait_for_lock(const struct dpll_regs *dpll_regs)
{
        if (!wait_on_value(ST_DPLL_CLK_MASK, ST_DPLL_CLK_MASK,
                           (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
//                print_str_nl("DPLL locking failed");
                while(1); // hang();
        }
/*	print_str("checking cm_idlest_dpll reg for lock:");
	print_hex(__raw_readl(dpll_regs->cm_idlest_dpll));
	print_nl(); */
}

u32 get_sys_clk_index(void)
{
        struct ctrl_stat *ctrl = (struct ctrl_stat *)CTRL_BASE;
	u32 ind = __raw_readl(&ctrl->statusreg);

	if ((ind >> 22) == 1)
		print_str_nl("control status reg, crystal freq is 24MHz");
	return ((ind & CTRL_SYSBOOT_15_14_MASK) >> CTRL_SYSBOOT_15_14_SHIFT);
}

/*
 * we are accessing CM_CLKMODE_DPLL_CORE Register and put this
 * reg in MN bypass mode => bits(2,0)=100
*/
static inline void do_bypass_dpll(const struct dpll_regs *dpll_regs)
{
        clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
                        CM_CLKMODE_DPLL_DPLL_EN_MASK,
                        DPLL_EN_MN_BYPASS << CM_CLKMODE_DPLL_EN_SHIFT);
}

/*
 * wait_on_value() - common routine to allow waiting for changes in
 * volatile regs.
 */
u32 wait_on_value(u32 read_bit_mask, u32 match_value, void *read_addr,
                  u32 bound)
{
        u32 i = 0, val;
        do {
                ++i;
                val = __raw_readl((u32)read_addr) & read_bit_mask;
                if (val == match_value)
                        return 1;
                if (i == bound)
                        return 0;
        } while (1); //this breaks when in MN bypass mode
}

static inline void wait_for_bypass(const struct dpll_regs *dpll_regs)
{
        if (!wait_on_value(ST_DPLL_CLK_MASK, 0,
                           (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
                print_str_nl("Bypassing DPLL failed");
        }
}

void bypass_dpll(const struct dpll_regs *dpll_regs)
{
        do_bypass_dpll(dpll_regs);
        wait_for_bypass(dpll_regs);
/*	print_str("checking cm_clockmode_dpll mode:");
	print_hex(__raw_readl(dpll_regs->cm_clkmode_dpll));
	print_nl();
	print_str("checking cm_idlest_dpll reg for bypass:");
	print_hex(__raw_readl(dpll_regs->cm_idlest_dpll));
	print_nl(); */
}

void do_setup_dpll(const struct dpll_regs *dpll_regs, const struct dpll_params *params)
{
        u32 temp;

        if (!params)
                return;

/* reading multiplier and dividor factor from clksel_dpll core register */
        temp = __raw_readl(dpll_regs->cm_clksel_dpll);
/*	print_str("contents of cm_clksel_dpll:");
	print_hex(temp);
	print_nl(); */

        bypass_dpll(dpll_regs);

        /* Set M & N */
        temp &= ~CM_CLKSEL_DPLL_M_MASK;//clear bits(18:8)
//write 125 which is M in bits(18:8) which is 7D00 in clksel_dpll core register
        temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

        temp &= ~CM_CLKSEL_DPLL_N_MASK;//clear bits(6:0)
//write 2 which is N in bits(6:0) which is 02 in clksel_dpll core register
        temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

        __raw_writel(temp, dpll_regs->cm_clksel_dpll);
/*	print_str("checking cm_clksel_dpll dividers M, N:");
	print_hex(__raw_readl(dpll_regs->cm_clksel_dpll));
	print_nl(); */

        setup_post_dividers(dpll_regs, params);

        /* Wait till the DPLL locks */
        do_lock_dpll(dpll_regs);
        wait_for_lock(dpll_regs);
}

const struct dpll_params *get_dpll_core_params(void)
{
        int ind = get_sys_clk_index();

//	print_str_nl("CORE dpll config start");
        return &dpll_core_1000MHz[ind];
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	int ind = get_sys_clk_index();
        int freq = am335x_get_efuse_mpu_max_freq();
	
//	print_str_nl("MPU dpll config start");
	if(freq == MPUPLL_M_1000)
		return &dpll_mpu_opp[ind][5];
}

const struct dpll_params *get_dpll_per_params(void)
{
        int ind = get_sys_clk_index();

//	print_str_nl("PER dpll config start");
	return &dpll_per_192MHz[ind];
}

const struct dpll_params *get_dpll_ddr_params(void)
{
        int ind = get_sys_clk_index();

//	print_str_nl("DDR dpll config start");
	return &dpll_ddr3_400MHz[ind];
}

void setup_dplls(void)
{
	struct cm_wkuppll *const cmwkup = (struct cm_wkuppll *)CM_WKUP;
        const struct dpll_params *params;
/*
 * Programming Core PLL is described in 8.1.6.7.1 Core PLL Configuration
 * of the TRM
 */
        params = get_dpll_core_params();
        do_setup_dpll(&dpll_core_regs, params);
//	print_str_nl("CORE dpll config done");

        params = get_dpll_mpu_params();
        do_setup_dpll(&dpll_mpu_regs, params);
//	print_str_nl("MPU dpll config done");

/* Programming PER DPLL not required as it is already done by bootrom
 * but still tried to program, unable to write into its registers
 * dont know why, so ignoring.... 	
	params = get_dpll_per_params();
	do_setup_dpll1(&dpll_per_regs, params);
	print_str_nl("PER dpll config done");
*/

/*
 * CM_CLKDCOLDO_DPLL_PER is the register that provides controls over the digitally controlled oscillator output 
 * of the PER DPLL. Enable this clock and force it to stay enabled.
 */
        __raw_writel(0x300, &cmwkup->clkdcoldodpllper);

        params = get_dpll_ddr_params();
        do_setup_dpll(&dpll_ddr_regs, params);
//	print_str_nl("DDR dpll config done");
}
