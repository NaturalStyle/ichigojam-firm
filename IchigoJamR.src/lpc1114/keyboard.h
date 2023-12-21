#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

//#define CONFIG_PS2_10_11

// uart

//extern volatile uint8_t UARTBuffer[]; // uart.h 0x40 -> 16byte -> 10byte (recv buffer)
//extern volatile uint8_t UARTCount;

#include "LPC11xx.h"
#include "../vars.h"
#include "../screen.h"

#ifdef WITH_MORSE
#define DEFAULT_UARTMODE_TXD (2+4)	// txd 0:disable, 1:only text, 2:with ctrl, +4:echo back mode
#else
#define DEFAULT_UARTMODE_TXD 2	// txd 0:disable, 1:only text, 2:with ctrl, +4:echo back mode
#endif
#define DEFAULT_UARTMODE_RXD 1	// rxd 0:disable, 1:enable, 2:ignore esc mode, 4:CR mode, 6:ignore esc & CR mode (auto comment mode??) // -> keyboard_ps2.h

//volatile uint8 _g.key_flg_esc = 0;

// keyboard
#ifndef KEYBOARD_USB
#include "keyboard_ps2.h"
#else
#include "keyboard_usb.h"
#endif

INLINE void uart_init() {
#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode_txd = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
	_g.uartmode_txd = txd;
	_g.uartmode_rxd = rxd;
}

#define UART_MODE_BUF // 入らない 24632 - 24576 ... 56byte! 一時的にHELPを抜いた入れた

#ifdef UART_MODE_BUF
//#define UART_BUF_LEN 16
//char uartbuf[UART_BUF_LEN];
//int8 nuartbuf = 0;

#ifdef WITH_MORSE
#include "../ext_morse.h"
#endif

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
	if ((_g.uartmode_txd & 3) == 3) { // 1.3b2
		/*
		if (c != '\n') {
			uartbuf[nuartbuf++] = c;
			if (nuartbuf < UART_BUF_LEN)
				return;
		} else {
			uartbuf[nuartbuf++] = '\r';
			uartbuf[nuartbuf++] = '\n';
		}
		UARTSend(uartbuf, nuartbuf);
		nuartbuf = 0;
		*/
		
		// no buffering
		if (c == '\n') {
			uart_putc('\r');
		}
	}
	while (!(LPC_UART->LSR & LSR_THRE));
	LPC_UART->THR = c;

	#ifdef WITH_MORSE
	morse_out(c);
	#endif
}
#else
inline void uart_putc(char c) {
	UARTSend((unsigned char*)&c, 1);
	//	while (!(LPC_UART->LSR & LSR_THRE));
	//	LPC_UART->THR = c;
}
#endif

INLINE void uart_bps(int n) { // 0:115200, -1:57600, -2:38400, -100以下で x -100bpsで設定
	if (n == 0) {
		n = 115200;
	} else if (n == -1) {
		n = 57600;
	} else if (n == -2) {
		n = 38400;
	} else if (n <= -100) {
		n = -n * 100; // 230400; // ver 1.2.4
	}
	UARTInit(n);
}

void put_chr(char c) {
	#ifdef WITH_MORSE // morse modeのときはscreen先
	if (!(_g.uartmode_txd & 8)) { // 1.3b11
		screen_putc(c);
	}
	if (_g.uartmode_txd & 3) { // 1.3b2
		uart_putc(c);
	}
	#else
//	if (_g.uartmode > 0) { // 1.0.2b12 uartを先に
	if (_g.uartmode_txd & 3) { // 1.3b2
		uart_putc(c);
	}
	if (!(_g.uartmode_txd & 8)) { // 1.3b11
		screen_putc(c);
	}
	#endif
}

// basic interface
inline int stopExecute() {
	return _g.key_flg_esc;
}

#endif
