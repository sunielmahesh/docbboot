#include "include/doc_types.h"
#include "include/doc_io_barrier.h"
#include "include/doc_ddr3sdram.h"
#include "include/doc_hardware_am33xx.h"
#include "include/doc_timer.h"
#include "include/doc_debug_uart.h"

/*
 * Configure DDR PHY CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd)
{
/* Base addresses for DDR PHY cmd/data regs */
	struct ddr_cmd_regs *ddr_cmd_reg = (struct ddr_cmd_regs *)DDR_PHY_CMD_ADDR;
        
	if (!cmd)
                return;

        __raw_writel(cmd->cmd0csratio, &ddr_cmd_reg->cm0csratio);
//	print_str("&ddr_cmd_reg->cm0csratio):");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm0csratio));
//	print_nl(); 
        __raw_writel(cmd->cmd0iclkout, &ddr_cmd_reg->cm0iclkout);
//	print_str("&ddr_cmd_reg->cm0iclkout:");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm0iclkout));
//	print_nl(); 

        __raw_writel(cmd->cmd1csratio, &ddr_cmd_reg->cm1csratio);
//	print_str("&ddr_cmd_reg->cm1csratio:");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm1csratio));
//	print_nl(); 
        __raw_writel(cmd->cmd1iclkout, &ddr_cmd_reg->cm1iclkout);
//	print_str("&ddr_cmd_reg->cm1iclkout:");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm1iclkout));
//	print_nl(); 

        __raw_writel(cmd->cmd2csratio, &ddr_cmd_reg->cm2csratio);
//	print_str("&ddr_cmd_reg->cm2csratio:");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm2csratio));
//	print_nl(); 
        __raw_writel(cmd->cmd2iclkout, &ddr_cmd_reg->cm2iclkout);
//	print_str("&ddr_cmd_reg->cm2iclkout:");
//	print_hex(__raw_readl(&ddr_cmd_reg->cm2iclkout));
//	print_nl(); 
}

/*
 * Configure DDR PHY DATA registers
 */
void config_ddr_data(const struct ddr_data *data)
{
/* Base addresses for DDR PHY cmd/data regs */
	struct ddr_data_regs *ddr_data_reg = (struct ddr_data_regs *)DDR_PHY_DATA_ADDR;
        int i;

        if (!data)
                return;

	__raw_writel(data->datardsratio0, &ddr_data_reg->dt0rdsratio0);
//	print_str("&ddr_data_reg->dt0rdsratio0:");
//	print_hex(__raw_readl(&ddr_data_reg->dt0rdsratio0));
//	print_nl(); 
	__raw_writel(data->datawdsratio0, &ddr_data_reg->dt0wdsratio0);
//	print_str("&ddr_data_reg->dt0wdsratio0:");
//	print_hex(__raw_readl(&ddr_data_reg->dt0wdsratio0));
//	print_nl(); 
//	__raw_writel(data->datawiratio0, &ddr_data_reg->dt0wiratio0);
//	__raw_writel(data->datagiratio0, &ddr_data_reg->dt0giratio0);
	__raw_writel(data->datafwsratio0, &ddr_data_reg->dt0fwsratio0);
//	print_str("&ddr_data_reg->dt0fwsratio0:");
//	print_hex(__raw_readl(&ddr_data_reg->dt0fwsratio0));
//	print_nl(); 
	__raw_writel(data->datawrsratio0, &ddr_data_reg->dt0wrsratio0);
//	print_str("&ddr_data_reg->dt0wrsratio0:");
//	print_hex(__raw_readl(&ddr_data_reg->dt0wrsratio0));
//	print_nl(); 
}

void config_io_ctrl(const struct ctrl_ioregs *ioregs)
{
	struct ddr_cmdtctrl *ioctrl_reg = (struct ddr_cmdtctrl *)DDR_CONTROL_BASE_ADDR;
	
        if (!ioregs)
                return;

        __raw_writel(ioregs->cm0ioctl, &ioctrl_reg->cm0ioctl);
//	print_str("&ioctrl_reg->cm0ioctl:");
//	print_hex(__raw_readl(&ioctrl_reg->cm0ioctl));
//	print_nl(); 
        __raw_writel(ioregs->cm1ioctl, &ioctrl_reg->cm1ioctl);
//	print_str("&ioctrl_reg->cm1ioctl:");
//	print_hex(__raw_readl(&ioctrl_reg->cm1ioctl));
//	print_nl(); 
        __raw_writel(ioregs->cm2ioctl, &ioctrl_reg->cm2ioctl);
//	print_str("&ioctrl_reg->cm2ioctl:");
//	print_hex(__raw_readl(&ioctrl_reg->cm2ioctl));
//	print_nl(); 
        __raw_writel(ioregs->dt0ioctl, &ioctrl_reg->dt0ioctl);
//	print_str("&ioctrl_reg->dt0ioctl:");
//	print_hex(__raw_readl(&ioctrl_reg->dt0ioctl));
//	print_nl(); 
        __raw_writel(ioregs->dt1ioctl, &ioctrl_reg->dt1ioctl);
//	print_str("&ioctrl_reg->dt1ioctl:");
//	print_hex(__raw_readl(&ioctrl_reg->dt1ioctl));
//	print_nl(); 
// IO is set to STLmode(DDR2/3)
        __raw_writel(__raw_readl(DDR_CTRL_ADDR) & (~0x10000000), DDR_CTRL_ADDR);
//	print_str("ddr_io_ctrl Register:");
//	print_hex(__raw_readl(DDR_CTRL_ADDR));
//	print_nl(); 
}

/*
 * Configure DDR PHY
 * Disable initialization and refreshes for now until we finish
 * programming EMIF regs and set time between rising edge of
 * DDR_RESET to rising edge of DDR_CKE to > 500us per memory spec.
 * We currently hardcode a value based on a max expected frequency
 * of 400MHz.
 */
void config_ddr_phy(const struct emif_regs *regs)
{
	struct emif_reg_struct *emif_reg = (struct emif_reg_struct *)EMIF4_0_CFG_BASE;

	__raw_writel(EMIF_REG_INITREF_DIS_MASK | 0x3100, &emif_reg->emif_sdram_ref_ctrl);
	__raw_writel(regs->emif_ddr_phy_ctlr_1, &emif_reg->emif_ddr_phy_ctrl1);
	__raw_writel(regs->emif_ddr_phy_ctlr_1, &emif_reg->emif_ddr_phy_ctrl1_shdw);
//	print_str("emif_ddr_phy_ctrl1:");
//	print_hex(__raw_readl(&emif_reg->emif_ddr_phy_ctrl1));
//	print_nl(); 
}

/*
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs)
{
	struct emif_reg_struct *emif_reg = (struct emif_reg_struct *)EMIF4_0_CFG_BASE;

	__raw_writel(regs->sdram_tim1, &emif_reg->emif_sdram_tim_1);
	__raw_writel(regs->sdram_tim1, &emif_reg->emif_sdram_tim_1_shdw);
	__raw_writel(regs->sdram_tim2, &emif_reg->emif_sdram_tim_2);
	__raw_writel(regs->sdram_tim2, &emif_reg->emif_sdram_tim_2_shdw);
	__raw_writel(regs->sdram_tim3, &emif_reg->emif_sdram_tim_3);
	__raw_writel(regs->sdram_tim3, &emif_reg->emif_sdram_tim_3_shdw);
//	print_str("emif_sdram_tim_1:");
//	print_hex(__raw_readl(&emif_reg->emif_sdram_tim_1));
//	print_nl();
//	print_str("emif_sdram_tim_2:");
//	print_hex(__raw_readl(&emif_reg->emif_sdram_tim_2));
//	print_nl();
//	print_str("emif_sdram_tim_3:");
//	print_hex(__raw_readl(&emif_reg->emif_sdram_tim_3));
//	print_nl();
}

/*
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs)
{
	struct emif_reg_struct *emif_reg = (struct emif_reg_struct *)EMIF4_0_CFG_BASE;
	struct ctrl_stat *cstat = (struct ctrl_stat *)CTRL_BASE;
	long ram_size;
	int i;
	
	if (regs->zq_config) {
                /* Trigger initialization, 3100 is written in refresh rate just to start with */
		__raw_writel(0x00003100, &emif_reg->emif_sdram_ref_ctrl);
                /* Wait 1ms because of L3 timeout error */
		__udelay(3000); //but given 3ms
		/* Here compute and Write proper sdram_ref_cref_ctrl value */
		__raw_writel(regs->ref_ctrl, &emif_reg->emif_sdram_ref_ctrl);
		__raw_writel(regs->ref_ctrl, &emif_reg->emif_sdram_ref_ctrl_shdw);
//		print_str("emif_sdram_ref_ctrl:");
//		print_hex(__raw_readl(&emif_reg->emif_sdram_ref_ctrl));
//		print_nl();
		__raw_writel(regs->zq_config, &emif_reg->emif_zq_config);
//		print_str("emif_reg->emif_zq_config:");
//		print_hex(__raw_readl(&emif_reg->emif_zq_config));
//		print_nl();
		__raw_writel(regs->sdram_config, &cstat->secure_emif_sdram_config);
		__raw_writel(regs->sdram_config, &emif_reg->emif_sdram_config);
//		print_str("emif_sdram_config:");
//		print_hex(__raw_readl(&emif_reg->emif_sdram_config));
//		print_nl();
        }

        /* Write REG_COS_COUNT_1, REG_COS_COUNT_2, and REG_PR_OLD_COUNT. 
        if (regs->ocp_config)
		__raw_writel(regs->ocp_config, &emif_reg->emif_ocp_config);*/

	
/*	print_str("CM_PER_EMIF_CLKCTRL:");
	print_hex(*(volatile unsigned int *)(0x44E00028));
	print_nl();
	print_str("CM_PER_L3_CLKCTRL:");
	print_hex(*(volatile unsigned int *)(0x44E0000C));
	print_nl(); */
	
	ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, CONFIG_MAX_RAM_BANK_SIZE);
	print_str("size of on-board Physical Memory:");
	ram_size = (ram_size/1024)/1024;
	print_num((unsigned int)ram_size);
	print_str("MiB");
	print_nl();
/*	print_str("size of long:");
	print_num(sizeof(long));
	print_nl();
	*((char *)0x80000000) = 0xA;
	print_str("contents of PHYS MEM:");
	print_hex((unsigned int)__raw_readl(0x80000000));
	print_nl();*/
	print_str_nl("Initilizing SDRAM done...");
        print_str_nl("Testing SDRAM...");

        print_str_nl("Writing into sdram...");
        for(i=0; i < 20; i++) {
                *((char *)0x80000000+i) = 40+i;
        }
        *((char *)0x80000000+21) = '\0';

        print_str_nl("Reading from sdram...");
        print_str_nl((char *)0x80000000);
}
