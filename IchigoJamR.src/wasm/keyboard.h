#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "config.h"
#include "../ram.h"
#include "../screen.h"

INLINE void IJB_kbd(uint mode) {
}

struct keyflg_def key_flg;
uint8 displaymode;

inline static void key_init() {
}

static inline uint key_getKeyboardID() {
	//return PS2_KB[-2];
	//return *(uint8_t*)(0x5700);
	return 0;
}

INLINE void key_enable(uint8 b) {
//	key_enable_flg = b;
}
//void uart_checker();

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2) // kbhitとnkeybuf分
uint8* keybuf = (uint8*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい!
#define keykbhit *(uint8*)(ram + OFFSET_RAM_KEYBUF)
//#define keykbhit keybuf[-1]

static void key_kbhit(int n) {
	keykbhit |= 1 << n;
}
static void key_kbhit_rel(int n) {
	keykbhit &= ~(1 << n);
}

inline int key_btn(int n) {
	//	uart_checker();
	n -= n == 88 ? 88 - 5 : 28; // X or not
	return (keykbhit & (1 << n)) != 0;
//	return 0;
}

static void key_send_reset() {
}

//extern uint8 key_kana_buf0; // なぜ extern？
uint8 key_kana_buf0; // なぜ extern？
static inline int romajikana_input(int ch);

int kanamode = 0;

EXPORT
void key_putc(int ch) {
	if (ch == 0xf) {
		kanamode = !kanamode;
		key_kana_buf0 = 0;
	}
	if (kanamode) {
		ch = romajikana_input(ch);
		if (!ch)
			return;
	}

	if (ram[OFFSET_RAM_KEYBUF + 1] < 126) {
		ram[OFFSET_RAM_KEYBUF + 1]++;
		ram[OFFSET_RAM_KEYBUF + 1 + ram[OFFSET_RAM_KEYBUF + 1]] = (uint8)ch;
	}
	if (ch == 27) {
		_g.key_flg_esc = 1;
	}
}

void key_pushc(int ch) {
	if (ram[OFFSET_RAM_KEYBUF + 1] < 126) {
		ram[OFFSET_RAM_KEYBUF + 1]++;
		ram[OFFSET_RAM_KEYBUF + 1 + ram[OFFSET_RAM_KEYBUF + 1]] = (uint8)ch;
	}
}

#include "../romajikana.h"

int key_getKey() {
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	(*keybuf)--;
	for (uint8 i = 0; i < *keybuf; i++)
		keybuf[i + 1] = keybuf[i + 2];
	return res;
}
void key_clearKey() {
	//	keykbhit = 0; // ver1.2b5 clear しないように変更
	*keybuf = 0;
}


INLINE void uart_init() {
	_g.uartmode_txd = 1;
	/*
#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
	*/
}

uint8 uartbuf[UARTOUTPUT_SIZE];
int uartbuf_idx = 0;

INLINE void IJB_uart(int16 txd, int16 rxd) {
	_g.uartmode_txd = txd;
	_g.uartmode_rxd = rxd;
}

static void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる?
	if (c == '\n') {
		uartbuf[uartbuf_idx] = 0;
		memcpy(uartoutput, uartbuf, uartbuf_idx + 1);
		uartbuf_idx = 0;
	} else {
		if (uartbuf_idx < UARTOUTPUT_SIZE - 2) {
			uartbuf[uartbuf_idx++] = c;
		}
	}
//	putchar(c);
	/*
	if (_g.uartmode_txd == 3) {
		// no buffering
		if (c == '\n') {
			uart_putc('\r');
		}
	}
	while (!(LPC_UART->LSR & LSR_THRE));
	LPC_UART->THR = c;
*/
}

void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
	if (n == 0) {
		n = 115200;
	} else if (n == -1) {
		n = 57600;
	} else if (n == -2) {
		n = 38400;
	}
//	UARTInit(n);
}

void put_chr(char c) {
	if (_g.uartmode_txd > 0) { // 1.0.2b12 uartを先に
		uart_putc(c);
	}
	screen_putc(c); // segmentation fault
}

// basic interface
inline int stopExecute() {
	for (uint8 i = 0; i < *keybuf; i++) {
		if (keybuf[i + 1] == 27) {
			_g.key_flg_esc = 1;
		}
	}
	return _g.key_flg_esc;
}

// __KEYBOARD_H__
#endif