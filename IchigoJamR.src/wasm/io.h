#ifndef __IO_H__
#define __IO_H__

#include "../stddef.h"
#include "system.h"
#include "keyboard.h"

/* INLINE */ void IJB_pwm(int port, int plen, int len);
INLINE void pwm_off(int port);
static void io_init();
INLINE void io_set(int n);
INLINE int analog_get(int ch);
INLINE int io_get();
INLINE void IJB_led(int st);
int IJB_in();
INLINE void IJB_clo();
INLINE int IJB_ana(int n);
/*INLINE*/ int IJB_btn(int n);
void IJB_out(int port, int st);
INLINE int IJB_i2c(uint8 writemode, uint16* param); // 0:success 1:io error 2:parm error

static int i2c0_init() {
	return 0;
}


// PWM
// PWM 2,100  100kHz PWM  OUT2,3,4
// PWM 2,100,2000 で周期設定、2-4はまとめて設定、5は独立
// 	PWM n で、OUT出力からは除外される (設定してないと出力されない)
//  OUT n,0 でPWMは停止する
/* INLINE */

void IJB_pwm(int port, int plen, int len) {
	if (len == 0) {
		len = 2000;
	}
	if (port >= 0 && port < 12) {
		pwmvalue[port] = plen;
		pwmlen[port] = len;
	}
}
INLINE void pwm_off(int port) {
	if (port >= 0 && port < 12) {
		pwmvalue[port] = 0;
		pwmlen[port] = 0;
	}
}
// I/O

/*
IN
	1 PIO0_10
	2 PIO0_11
	3 PIO0_5 (OD)
	4 PIO0_6
	5 PIO1_0 OUT1と共有
	6 PIO1_1 OUT2と共有
	7 PIO1_2 OUT3と共有
	8 PIO1_3 OUT4と共有
OUT
	1 PIO1_0 IN5と共有
	2 PIO1_1 IN6と共有
	3 PIO1_2 IN7と共有
	4 PIO1_3 IN8と共有
	5 PIO1_9
	6 PIO1_8
	7 LED
	8 IN1
	9 IN2
	10 IN3
	11 IN4
	*/ 
int instate[12];
int btnstate = 0;
static void io_init() {
	for (int i = 0; i < 12; i++)
		instate[i] = 0;
	btnstate = 0;
	outport = 0;
}
// ADC
INLINE int analog_get(int ch) {
	if (ch == 0 || ch == 2 || (ch >= 5 && ch <= 9))
		return instate[ch];
	return 0;
}

#define ANA_THRESHOLD (1024 / 4)
INLINE int io_get() {
	int st = 0;
	for (int i = 1; i < 12; i++)
		st |= (instate[i] != 0) << (i - 1);
	return st;
}
/*
INLINE void io_set(int n) {
	
}
*/

INLINE void IJB_led(int st) {
	IJB_out(7, st != 0);
}
int IJB_in() {
	return io_get();
}
// out buffer
void IJB_out(int port, int st) {
	if (port == 0) {
		outport = st;
	} else if (port >= 1 && port <= 11) {
		if (st) {
			outport |= 1 << (port - 1);
		} else {
			outport &= ~(1 << (port - 1));
		}
	}
}


/*INLINE*/ int IJB_btn(int n) {
	
	if (n == 0) { // 1.4
		return btnstate;
	} else if (n < 0) { // 1.3.2b17
		return keykbhit;
	}
	//return key_btn(n); // WebAssembly なぜかこれだとエラー!?
	n -= n == 88 ? 88 - 5 : 28; // X or not
	return (keykbhit & (1 << n)) != 0; // ok
//	return 0;
}
INLINE int IJB_ana(int n) {
	return analog_get(n);
}
INLINE void IJB_clo() {
	io_init();
}

INLINE int IJB_i2c(uint8 writemode, uint16* param) { // 0:success 1:io error 2:parm error
	return 1;
}

// __IO_H__
#endif