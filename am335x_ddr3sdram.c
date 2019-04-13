/*
 * SDRAM initialization:
 * SDRAM initialization has two parts:
 * 1. Configuring the SDRAM device
 * 2. Update the AC timings related parameters in the EMIF module
 * (1) should be done only once and should not be done while we are
 * running from SDRAM.
 * (2) can and should be done more than once if OPP changes.
 * Particularly, this may be needed when we boot without SPL and
 * and using Configuration Header(CH). ROM code supports only at 50% OPP
 * at boot (low power boot). So u-boot has to switch to OPP100 and update
 * the frequency. So,
 * Doing (1) and (2) makes sense - first time initialization
 * Doing (2) and not (1) makes sense - OPP change (when using CH)
 * Doing (1) and not (2) doen't make sense
 * See do_sdram_init() for the details
 */

#include "include/doc_ddr3sdram.h"

const struct cmd_control ddr3_beagleblack_cmd_ctrl_data = {
        .cmd0csratio = MT41K256M16HA125E_RATIO,
        .cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

        .cmd1csratio = MT41K256M16HA125E_RATIO,
        .cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

        .cmd2csratio = MT41K256M16HA125E_RATIO,
        .cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

const struct ddr_data ddr3_beagleblack_data = {
        .datardsratio0 = MT41K256M16HA125E_RD_DQS,
        .datawdsratio0 = MT41K256M16HA125E_WR_DQS,
        .datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
        .datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

struct ctrl_ioregs ioregs_bonelt = {
        .cm0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
        .cm1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
        .cm2ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
        .dt0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
        .dt1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
};

struct emif_regs ddr3_beagleblack_emif_reg_data = {
        .sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
        .ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
        .sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
        .sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
        .sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
        .ocp_config = EMIF_OCP_CONFIG_BEAGLEBONE_BLACK,
        .zq_config = MT41K256M16HA125E_ZQ_CFG,
        .emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

/* 
 * 400MHz is the frequency output from the DDR PLL 
 * 400MHz is passed via div by 2 circuit and 200MHz serves as input clock for EMIF 
 * DDR3L SDRAM chips are driven @ 400MHz
 */
void sdram_init(void)
{
	config_ddr(400, &ioregs_bonelt, &ddr3_beagleblack_data, &ddr3_beagleblack_cmd_ctrl_data,
			&ddr3_beagleblack_emif_reg_data);
}
