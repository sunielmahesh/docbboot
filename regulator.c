#include "include/doc_regulator.h"
#include "include/doc_debug_uart.h"
#include "include/doc_i2c.h"
#include "include/doc_timer.h"

/*
 * TPS65217C PMIC registers are password protected
 */

/**
 *  tps65217_reg_write() - Generic function that can write a TPS65217 PMIC
 *                         register or bit field regardless of protection
 *                         level.
 *
 *  @prot_level:           Register password protection.  Use
 *                         TPS65217_PROT_LEVEL_NONE,
 *                         TPS65217_PROT_LEVEL_1 or TPS65217_PROT_LEVEL_2
 *  @dest_reg:             Register address to write.
 *  @dest_val:             Value to write.
 *  @mask:                 Bit mask (8 bits) to be applied.  Function will only
 *                         change bits that are set in the bit mask.
 *
 *  @return:               0 for success, not 0 on failure, as per the i2c API
 */

int tps65217_reg_write(unsigned char prot_level, unsigned char dest_reg,
			unsigned char dest_val, unsigned char mask)
{
        unsigned char read_val;
        unsigned char xor_reg;
        int ret;

/*	ret = ti_i2c_regulator_read(TPS65217_CHIP_PM, TPS65217_CHIPID, 1, &read_val, 1);
	print_str("contents of TPS65217C chip id Register:");
	print_hex(read_val);
	print_nl();
*/

/*
 * If we are affecting only a bit field, read dest_reg and apply the
 * mask
 */
         
        if (mask != TPS65217_MASK_ALL_BITS) {
		ret = ti_i2c_regulator_read(TPS65217_CHIP_PM, dest_reg, 1, &read_val, 1);
                if (ret)
                        return ret;

		read_val &= (~mask);
		read_val |= (dest_val & mask);
		dest_val = read_val;
        } else {
		ret = ti_i2c_regulator_read(TPS65217_CHIP_PM, dest_reg, 1, &read_val, 1);
                if (ret)
                        return ret;
	}

	if (prot_level > 0) {
		xor_reg = dest_reg ^ TPS65217_PASSWORD_UNLOCK;
		ret = ti_i2c_regulator_write(TPS65217_CHIP_PM, TPS65217_PASSWORD, 1, &xor_reg, 1);
		if (ret)
			return ret;
		
		ret = ti_i2c_regulator_read(TPS65217_CHIP_PM, TPS65217_PASSWORD, 1, &read_val, 1);
                if (ret)
                        return ret;
	}
	
	__udelay(3000);
	ret = ti_i2c_regulator_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1);
        if (ret)
                return ret;

	if (prot_level == TPS65217_PROT_LEVEL_2) {
		__udelay(3000);
		ret = ti_i2c_regulator_write(TPS65217_CHIP_PM, TPS65217_PASSWORD, 1, &xor_reg, 1);
		if (ret)
			return ret;

		ret = ti_i2c_regulator_write(TPS65217_CHIP_PM, dest_reg, 1, &dest_val, 1);
		if (ret)
			return ret;
	}

	ret = ti_i2c_regulator_read(TPS65217_CHIP_PM, dest_reg, 1, &read_val, 1);
                if (ret)
                        return ret;
                print_str("new contents of Register:");
                print_hex(read_val);
                print_nl();
	
	return 0;
}

/**
 * tps65217_voltage_update() - Function to change a voltage level, as this
 *                             is a multi-step process.
 * @dc_cntrl_reg:              DC voltage control register to change.
 * @volt_sel:                  New value for the voltage register
 * @return:                    0 for success, not 0 on failure.
 */
int tps65217_voltage_update(unsigned char dc_cntrl_reg, unsigned char volt_sel)
{
        if ((dc_cntrl_reg != TPS65217_DEFDCDC1) &&
            (dc_cntrl_reg != TPS65217_DEFDCDC2) &&
            (dc_cntrl_reg != TPS65217_DEFDCDC3))
                return 1;

        /* set voltage level */
        if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, dc_cntrl_reg, volt_sel,
                               TPS65217_MASK_ALL_BITS))
                return 1;

	__udelay(3000);

        /* set GO bit to initiate voltage transition */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2, TPS65217_DEFSLEW,
				TPS65217_DCDC_GO, TPS65217_DCDC_GO))
		return 1;

        return 0;
}
