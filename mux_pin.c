#include "include/doc_mux.h"

static struct module_pin_mux uart0_pin_mux[] = {
        {OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},  /* UART0_RXD */
        {OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},              /* UART0_TXD */
        {-1},
};

/* this was pinmux according to sysplay
static struct module_pin_mux i2c0_pin_mux[] = {
        {OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
                        PULLUDEN | PULLUP_EN)},  //I2C_DATA
        {OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
                        PULLUDEN | PULLUP_EN)},  //I2C_SCLK
        {-1},
};
*/

static struct module_pin_mux i2c0_pin_mux[] = {
        {OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
                        PULLUDEN | SLEWCTRL)},  //I2C_DATA
        {OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
                        PULLUDEN | SLEWCTRL)},  //I2C_SCLK
        {-1},
};

void configure_module_pin_mux(struct module_pin_mux *mod_pin_mux)
{
        int i;

        if (!mod_pin_mux)
                return;

        for (i = 0; mod_pin_mux[i].reg_offset != -1; i++)
                MUX_CFG(mod_pin_mux[i].val, mod_pin_mux[i].reg_offset);
}

void enable_uart0_pin_mux(void)
{
        configure_module_pin_mux(uart0_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
        configure_module_pin_mux(i2c0_pin_mux);
}
