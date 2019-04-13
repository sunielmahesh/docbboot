#include "include/doc_io_barrier.h"
#include "include/doc_ddr3sdram.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_debug_uart.h"

#define VTP_CTRL_ENABLE		(0x1 << 6)
#define VTP_CTRL_START_EN	(0x1)
#define VTP_CTRL_READY		(0x1 << 5)

#define DDR_CKE_CTRL_NORMAL     0x1

/* VTP Registers */
struct vtp_reg {
        unsigned int vtp0ctrlreg;
};

/* 
 * VTP control register (Voltage temperature process) is used to calibrate the output impedance 
 * of the DDR2 memory controller IO's with respect to voltage, temperature, and process. 
 */
void config_vtp(void)
{
	struct vtp_reg *vtpreg = (struct vtp_reg *)VTP0_CTRL_ADDR;	

	__raw_writel(__raw_readl(&vtpreg->vtp0ctrlreg) | VTP_CTRL_ENABLE, &vtpreg->vtp0ctrlreg);
	__raw_writel(__raw_readl(&vtpreg->vtp0ctrlreg) & (~VTP_CTRL_START_EN), &vtpreg->vtp0ctrlreg);
        __raw_writel(__raw_readl(&vtpreg->vtp0ctrlreg) | VTP_CTRL_START_EN, &vtpreg->vtp0ctrlreg);

        /* Poll for READY */
	while ((__raw_readl(&vtpreg->vtp0ctrlreg) & VTP_CTRL_READY) != VTP_CTRL_READY)
                ;
/*	print_str("vtp_reg");
	print_hex(__raw_readl(&vtpreg->vtp0ctrlreg));
	print_nl(); */
}

void config_ddr(unsigned int pll, const struct ctrl_ioregs *ioregs, const struct ddr_data *data, 
		const struct cmd_control *ctrl, const struct emif_regs *regs)
{
	struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;

	config_vtp();
	config_cmd_ctrl(ctrl);
	config_ddr_data(data);
	config_io_ctrl(ioregs);
	/* Set CKE to be controlled by EMIF/DDR PHY */
	__raw_writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

/*	print_str("&ddrctrl->ddrckectrl:");
	print_hex(__raw_readl(&ddrctrl->ddrckectrl));
	print_nl(); */

/* Program EMIF instance */
        config_ddr_phy(regs);
        set_sdram_timings(regs);
	config_sdram(regs);
}
