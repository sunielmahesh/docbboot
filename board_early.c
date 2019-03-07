#include "include/doc_hardware_am33xx.h"
#include "include/doc_board.h"
#include "include/doc_debug_uart.h"
#include "include/doc_io_barrier.h"
#include "include/doc_clock.h"

#define PACKAGE_TYPE_MASK	(3 << 16)
#define PACKAGE_TYPE_SHIFT	16
#define DEVICE_ID_MASK		0x1FFF

/* Package Type */
#define PACKAGE_TYPE_UNDEFINED          0x0
#define PACKAGE_TYPE_ZCZ                0x1
#define PACKAGE_TYPE_ZCE                0x2
#define PACKAGE_TYPE_RESERVED           0x3

/* MPU max frequencies */
#define AM335X_ZCZ_300                  0x1FEF
#define AM335X_ZCZ_600                  0x1FAF
#define AM335X_ZCZ_720                  0x1F2F
#define AM335X_ZCZ_800                  0x1E2F
#define AM335X_ZCZ_1000                 0x1C2F
#define AM335X_ZCE_300                  0x1FDF
#define AM335X_ZCE_600                  0x1F9F

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

int am335x_get_efuse_mpu_max_freq(struct ctrl_dev *cdev)
{
        int sil_rev;
	int efuse_arm_mpu_max_freq, package_type;

	sil_rev = __raw_readl(&cdev->deviceid) >> 28;

        if (sil_rev == 0) {
                /* No efuse in PG 1.0. Use max speed */
                return MPUPLL_M_720;
        } else if (sil_rev >= 1) {
                /* Check what the efuse says our max speed is. */
                efuse_arm_mpu_max_freq = __raw_readl(&cdev->efuse_sma);
                package_type = (efuse_arm_mpu_max_freq & PACKAGE_TYPE_MASK) >>
                                PACKAGE_TYPE_SHIFT;

                /* PG 2.0, efuse may not be set. */
                if (package_type == PACKAGE_TYPE_UNDEFINED || package_type ==
                    PACKAGE_TYPE_RESERVED)
                        return MPUPLL_M_800;

                switch ((efuse_arm_mpu_max_freq & DEVICE_ID_MASK)) {
                case AM335X_ZCZ_1000:
                        return MPUPLL_M_1000;
                case AM335X_ZCZ_800:
                        return MPUPLL_M_800;
                case AM335X_ZCZ_720:
                        return MPUPLL_M_720;
                case AM335X_ZCZ_600:
                case AM335X_ZCE_600:
                        return MPUPLL_M_600;
                case AM335X_ZCZ_300:
                case AM335X_ZCE_300:
                        return MPUPLL_M_300;
                }
       }

        /* unknown, use the PG1.0 max */
        return MPUPLL_M_720;
}


void scale_vcores(void)
{
        int freq;

        freq = am335x_get_efuse_mpu_max_freq(cdev);
	print_str("Maximum Frequency supported by the device: ");
	print_num(freq);
	print_str("Hz");
        print_nl();
//	scale_vcores_bone(freq);
}

void prcm_init(void)
{
        scale_vcores();
//        setup_dplls();
}

int board_early_init_f(void)
{
        prcm_init();
//	set_mux_conf_regs();
        return 0;
}
