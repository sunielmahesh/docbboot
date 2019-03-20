#include "include/doc_io_barrier.h"
#include "include/doc_timer.h"
#include "include/doc_i2c.h"
#include "include/doc_debug_uart.h"

void i2c_init(void)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;
	unsigned long internal_clk = 0, fclk;
        unsigned int speed, prescaler;
	unsigned int scll, sclh;
/*
 * Enable the wakeup module - has to be the first one before accessing any I2C
 * already done in am335x_clock.c
 * carry out I2C Pin mux Settings
 */
/* we are using I2C in standard mode => communication speed = 100KHz */
	speed = 100;
/*
 * The I2C module is operated with a functional clock (SCLK) frequency that can be in a range of 12-100
 * MHz, here it is 48MHz. according to I2C mode that must be used (an internal ~24 MHz clock (ICLK) is 
 * recommended in case of Fast/Standard operation mode). Note that the frequency of the functional clock 
 * influences directly the I2C bus performance and timings. but in how to program I2C it is said to
 * configure to 12MHz, so configuring to 12MHz
 */
	internal_clk = 12000; /* (in KHz) */
///	internal_clk = 24000; /* (in KHz) */
/* the main clk to i2c module is 48MHz, convert to KHz; fclk = 48000000/1000 */
	fclk = 48000;
/* prescaler = fclk / internal_clk = 48000/12000, from data sheet */
	prescaler = 4;
	prescaler = prescaler - 1; /* because a value 1 in register represents number 2 */
/* prescaler = fclk / internal_clk = 48000/24000, from data sheet */
//	prescaler = 2;
//	prescaler = prescaler - 1; /* because a value 1 in register represents number 2 */
/*
 * Set up I2C clock i.e SCL for 100Kbps or 400Kbps
 * SCL should run at a particular frequency depending on the mode we want (standard, fast ....)
 * since we want to run in standard mode, maximum speed is 100KHz.
 * now we want to generate a 100KHz clock on SCL line. time period t = 1/100KHz
 * t is divided into tLow and tHigh => t = tLow + tHigh
 * since its a clock, duty cycle is 50% => tLow = t/2 and tHigh = t/2
 * if we want to generate a 100KHz clock on SCL line, we need to program SCLL and SCLH as per the
 * below equations
 * tLow = (SCLL + 7) * 1/ICLK
 * tHigh = (SCLH + 5) * 1/ICLK
 * tLow = t/2 = (SCLL + 7) * 1/ICLK
 * 1/200K = (SCLL + 7) * 1/12M => SCLL = 12M/200K - 7 = 60-7 = 53
 * similarly tHigh = 1/200K = (SCLH + 5) * 1/ICLK
 * 1/200K = (SCLH + 5) * 1/12M => SCLH = 12M/200K - 5 = 60-5 = 55
 * finally this should be satisfied => F/S bit rate = INTERNAL_CLK / (SCLL + 7 + SCLH + 5)
 * F/S bit rate = 12M/120 = 100KHz = 100kbps
 */
        scll = 53;
        sclh = 55;
//        scll = 23;
//        sclh = 25;
	
        __raw_writel(prescaler, &i2c_base->psc);
        __raw_writel(scll, &i2c_base->scll);
        __raw_writel(sclh, &i2c_base->sclh);
	__raw_writel(0 << 12 | 0 << 7, &i2c_base->con);
        __raw_writel(__raw_readl(&i2c_base->con) | I2C_CON_EN, &i2c_base->con);

	__raw_writel(__raw_readl(&i2c_base->ie) | 1 << 4 | 1 << 3 | 1 << 2, &i2c_base->ie);
}

int send_start(void)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;

        while (__raw_readl(&i2c_base->stat) & I2C_STAT_BB) // Wait for bus not busy
                ;
	
	__raw_writel(__raw_readl(&i2c_base->con) | I2C_CON_STT, &i2c_base->con); // Trigger start
	
	while (__raw_readl(&i2c_base->con) & I2C_CON_STT) // Wait for start complete
                ;

        return 0;
}

int send_data(unsigned int data)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;

        while (!(__raw_readl(&i2c_base->stat) & I2C_STAT_XRDY)) // Wait for ready for accepting data for Tx
		;
	__raw_writel(data, &i2c_base->data); // Send data
// Clear ready for accepting data for Tx
	__raw_writel(__raw_readl(&i2c_base->stat) | I2C_STAT_XRDY, &i2c_base->stat);
        
	return 0;
}

int send_restart(void)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;

	__raw_writel(__raw_readl(&i2c_base->con) | I2C_CON_STT, &i2c_base->con); // Trigger start again
        while (__raw_readl(&i2c_base->con) & I2C_CON_STT) // Wait for start complete
                ;
        return 0;
}

int recv_data(unsigned char *data)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;

        while (!(__raw_readl(&i2c_base->stat) & I2C_STAT_RRDY)) // Wait for ready for reading data from Rx
                ;
        *data = __raw_readl(&i2c_base->data); // Read data
	__raw_writel(__raw_readl(&i2c_base->stat) | I2C_STAT_RRDY, &i2c_base->stat); // Clear ready for reading data from Rx
//	print_hex(*data);
//	print_nl();
//	print_str_nl("in recv_data");
        return 0;
}

void send_stop(void)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;

	__raw_writel(__raw_readl(&i2c_base->con) | I2C_CON_STP, &i2c_base->con); // Trigger stop
        while (__raw_readl(&i2c_base->con) & I2C_CON_STP) // Wait for stop complete
		;
}

int i2c_master_tx_rx(unsigned int addr, unsigned int *tx_data, unsigned char tx_len,
			unsigned char *rx_data, unsigned char rx_len)
{
	struct i2c *i2c_base = (struct i2c *)I2C_BASE0;
        int i;

	__raw_writel(addr, &i2c_base->sa);
	__raw_writel(tx_len & 0xFF, &i2c_base->cnt);
	__raw_writel(__raw_readl(&i2c_base->con) | I2C_CON_TRX | I2C_CON_MST, &i2c_base->con);
        
	if(send_start()) { // Trigger by sending Start
		send_stop(); 
		return -1;
	}
        
	for (i = 0; i < tx_len; i++) {// Send Data
		if(send_data(tx_data[i])) {
			send_stop(); 
			print_str_nl("returning -1");
			return -1;
		}
	}
        
        while (!(__raw_readl(&i2c_base->stat) & I2C_STAT_ARDY)) // Wait for ready for accessing registers after the tx complete
                ;
//clear the bit because it came out of while loop => access ready
	__raw_writel(__raw_readl(&i2c_base->stat) | I2C_STAT_ARDY, &i2c_base->stat);
        __raw_writel(rx_len & 0xFF, &i2c_base->cnt); // Set the rx data count
	__raw_writel(__raw_readl(&i2c_base->con) & ~(1 << 9), &i2c_base->con); // Set the Master Rx mode - note master is already set
//	print_hex(__raw_readl(&i2c_base->cnt));
//	print_nl();
	
	if(send_restart()) { // Trigger by sending Start again
		send_stop();
		return -1;
	}
        
	for (i = 0; i < rx_len; i++) {// Receive Data
//		print_num(rx_len);
//		print_nl();
//		print_hex((unsigned int)rx_data[i]);
//		print_nl();
		if(recv_data(&rx_data[i])) {
			send_stop();
			return -1;
		}
	}
        
	send_stop(); // Done, so Stop
        return 0;
}

int ti_i2c_eeprom_read(unsigned int addr, unsigned int chip, unsigned char byte,
			unsigned char *hdr_read, unsigned char len)
{
        unsigned int eeprom_addr = addr;
        unsigned int eeprom_data[2] = { ((eeprom_addr >> 8) & 0xFF), (unsigned int)(eeprom_addr & 0xFF)};

        i2c_master_tx_rx(chip, eeprom_data, byte, hdr_read, len);
	return 0;
}

