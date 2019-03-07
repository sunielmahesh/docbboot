#include "include/doc_debug_uart.h"

void print_nl(void)
{
	debug_tx("\r\n");
}

void print_str(char *str)
{
	debug_tx(str);
}

void print_str_nl(char *str)
{
	debug_tx(str);
	debug_tx("\r\n");
}

void print_num(unsigned int n)
{
	int sig, i, dig;
	char num[11];

	sig = 9;
	for (i = 9; i >= 0; i--)
	{
		dig = n % 10;
		if (dig)
			sig = i;
		num[i] = '0' + dig;
		n /= 10;
	}
	num[10] = 0;
	debug_tx(num + sig);
}

void print_hex(unsigned int n)
{
	int i;
	char c;

	_debug_uart_putc('0');
	_debug_uart_putc('x');
	for (i = 7; i >= 0; i--)
	{
		c = ((n >> (i << 2)) & 0xF);
		c += ((c <= 9) ? '0' : (-10 + 'A'));
		_debug_uart_putc(c);
	}
}

unsigned char scan_char(void)
{
	return _debug_uart_getc();
}

void scan_line(char *line, int max_len)
{
	debug_rx(line, max_len);
}
