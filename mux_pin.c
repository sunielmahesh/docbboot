#include "include/doc_mux.h"

static struct module_pin_mux uart0_pin_mux[] = {
        {OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},  /* UART0_RXD */
        {OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},              /* UART0_TXD */
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
