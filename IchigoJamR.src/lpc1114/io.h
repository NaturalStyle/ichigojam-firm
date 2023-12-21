#ifndef __IO_H__
#define __IO_H__

#include "config.h"
#include "../vars.h"
#include "display.h"
#include "keyboard.h"

extern void IJB_pwm(int port, int plen, int len);

/*inline*/ void IJB_pwm(int port, int plen, int len);
INLINEA void pwm_off(uint8_t port);
void io_init();
INLINEA void io_set(int n);
INLINEA uint32_t analog_get(int ch);
/*INLINEA*/ int io_get();
inline void IJB_led(int st);
inline int IJB_in();
inline void IJB_clo();
inline int IJB_ana(int n);
inline int IJB_btn(int n);
inline void IJB_out(int port, int st);
inline int IJB_i2c(uint8_t writemode, uint16_t* param); // 0:success 1:io error 2:parm error

// PWM
// PWM 2,100  100kHz PWM  OUT2,3,4
// PWM 2,100,2000 で周期設定、2-4はまとめて設定、5は独立
// 	PWM n で、OUT出力からは除外される（設定してないと出力されない）
//  OUT n,0 でPWMは停止する
/*inline*/ void IJB_pwm(int port, int plen, int len) {
	int pre = 480; // 48Mhz, 480 = 100kHz : 1 = 0.01msec, 12000 = 10Hz
	if (!len) {
		len = 2000; // 20msec (test 2sec) // default
	}
	if (len < 0) { // len マイナスで prescaleを1に 1.2b13
		pre = 1;
		len = -len;
	}
	if (port == 5) {
		if (LPC_IOCON->PIO1_9 != 0xc1 || LPC_TMR16B1->PR != pre - 1 || LPC_TMR16B1->MR0 != len - plen) { // 同じ時は変えない +32byte 1.3.2b20
			// 16bit timer
			LPC_TMR16B1->PR = pre - 1; // prescale
			LPC_TMR16B1->MCR |= 0b010 << (3 * 3); // MR3R reset

			LPC_IOCON->PIO1_9 = 0xc1; // CT16B1_MAT0
			LPC_TMR16B1->MR0 = len - plen;
			LPC_TMR16B1->PWMC |= 1 << 0; // PWM MR0 on (len = MR3)
			if (LPC_TMR16B1->MR3 != len - 1) {
				LPC_TMR16B1->MR3 = len - 1;
				//			LPC_TMR16B1->TCR = 2; // reset // <beta10
			}
			LPC_TMR16B1->TCR = 2; // reset beta 11
			LPC_TMR16B1->TCR = 1; // enable
		}
	} else {
		// 3ポート分処理まとめる
		port -= 2;
		__IO uint32_t* portads[] = { &LPC_IOCON->R_PIO1_1, &LPC_IOCON->R_PIO1_2, &LPC_IOCON->SWDIO_PIO1_3 };
		__IO uint32_t* portad = portads[port];
		__IO uint32_t* mr0ad = &LPC_TMR32B1->MR0 + port;

		if (*portad != 0xc3 || LPC_TMR32B1->PR != pre - 1 || *mr0ad != len - plen) { // 同じ時は変えない +16byte 1.3.2b20
			// 32bit timer
			LPC_TMR32B1->PR = pre - 1; // prescale
			LPC_TMR32B1->MCR |= 0b010 << (3 * 3); // MR3R reset

			*portad = 0xc3;
			*mr0ad = len - plen;
			LPC_TMR32B1->PWMC |= 1 << port;



			/*
			switch (port) { // 3ポート分処理をまとめれば減らせる
			  case 2:
				LPC_IOCON->R_PIO1_1 = 0xc3; // CT32B1_MAT0
				LPC_TMR32B1->MR0 = len - plen;
				LPC_TMR32B1->PWMC |= 1 << 0; // PWM MR0 on (len = MR3)
				break;
			  case 3:
				LPC_IOCON->R_PIO1_2 = 0xc3; // CT32B1_MAT1
				LPC_TMR32B1->MR1 = len - plen;
				LPC_TMR32B1->PWMC |= 1 << 1; // PWM MR1 on (len = MR3)
				break;
			  case 4:
				LPC_IOCON->SWDIO_PIO1_3 = 0xc3; // CT32B1_MAT2
				LPC_TMR32B1->MR2 = len - plen;
				LPC_TMR32B1->PWMC |= 1 << 2; // PWM MR2 on (len = MR3)
				break;
			}
			 */
			
			if (LPC_TMR32B1->MR3 != len - 1) {
				LPC_TMR32B1->MR3 = len - 1;
				//			LPC_TMR32B1->TCR = 2; // reset < beta10
			}
			LPC_TMR32B1->TCR = 2; // reset beta 11
			LPC_TMR32B1->TCR = 1; // enable
		}
	}
}
inline void pwm_off(uint8_t port) {
	/* // 縮まらない、途中
	if (port == 5) {
		LPC_IOCON->PIO1_9 = 0xd0; // PWM5と併用
		LPC_TMR16B1->PWMC &= ~(1 << 0); // PWM MR0 off
	} else {
		port -= 2;
		__IO uint32_t* portads = &LPC_IOCON->R_PIO1_1 + port;
		if (port == 4)
			portads += 3;
		*portads = 0xd1; // PWM2と併用 // c1 -> d1 pullup ver1.1b6
		LPC_TMR32B1->PWMC &= ~(1 << port); // PWM MR0 off
	}
	*/

	/*
	// 容量変化なし
	// PWMモードからの復帰のため
	__IO uint32_t* portads = &LPC_IOCON->PIO1_9;
	if (port == 2) {
		*(portads + 3) = 0xd1; // PWM2と併用 // c1 -> d1 pullup ver1.1b6
	} else if (port == 3) {
		*(portads + 2) = 0xd1; // PWM3と併用
	} else if (port == 4) {
		*(portads + 1) = 0xd1; // PWM4と併用
	} else {
		*(portads + 0) = 0xd0; // PWM5と併用
		LPC_TMR16B1->PWMC &= ~(1 << 0); // PWM MR0 off
		return;
	}
	LPC_TMR32B1->PWMC &= ~(1 << (port - 2)); // PWM MR1 off
	*/
	/*
	// 容量変化なし
	// PWMモードからの復帰のため
	if (port == 2) {
		LPC_IOCON->R_PIO1_1 = 0xd1; // PWM2と併用 // c1 -> d1 pullup ver1.1b6
	} else if (port == 3) {
		LPC_IOCON->R_PIO1_2 = 0xd1; // PWM3と併用
	} else if (port == 4) {
		LPC_IOCON->SWDIO_PIO1_3 = 0xd1; // PWM4と併用
	} else {
		LPC_IOCON->PIO1_9 = 0xd0; // PWM5と併用
		LPC_TMR16B1->PWMC &= ~(1 << 0); // PWM MR0 off
		return;
	}
	LPC_TMR32B1->PWMC &= ~(1 << (port - 2)); // PWM MR1 off
	*/

	/* // 容量変化なし
	// PWMモードからの復帰のため
	switch (port) {
	  case 2:
		LPC_IOCON->R_PIO1_1 = 0xd1; // PWM2と併用 // c1 -> d1 pullup ver1.1b6
		break;
	  case 3:
		LPC_IOCON->R_PIO1_2 = 0xd1; // PWM3と併用
		break;
	  case 4:
		LPC_IOCON->SWDIO_PIO1_3 = 0xd1; // PWM4と併用
		break;
	  case 5:
		LPC_IOCON->PIO1_9 = 0xd0; // PWM5と併用
		LPC_TMR16B1->PWMC &= ~(1 << 0); // PWM MR0 off
		return;
	}
	LPC_TMR32B1->PWMC &= ~(1 << (port - 2)); // PWM MR1 off
	*/
	// PWMモードからの復帰のため
	switch (port) {
	  case 2:
		LPC_IOCON->R_PIO1_1 = 0xd1; // PWM2と併用 // c1 -> d1 pullup ver1.1b6
		LPC_TMR32B1->PWMC &= ~(1 << 0); // PWM MR0 off
		break;
	  case 3:
		LPC_IOCON->R_PIO1_2 = 0xd1; // PWM3と併用
		LPC_TMR32B1->PWMC &= ~(1 << 1); // PWM MR1 off
		break;
	  case 4:
		LPC_IOCON->SWDIO_PIO1_3 = 0xd1; // PWM4と併用
		LPC_TMR32B1->PWMC &= ~(1 << 2); // PWM MR1 off
		break;
	  case 5:
		LPC_IOCON->PIO1_9 = 0xd0; // PWM5と併用
		LPC_TMR16B1->PWMC &= ~(1 << 0); // PWM MR0 off
		break;
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
	*/
void io_init() {
	_g.outbuf = 0; // 1.3b7
	// IN
	// pull down
	/*
	LPC_IOCON->SWCLK_PIO0_10 = 0b00011001001; // IN1 pull down // 0xd1 pull up
	LPC_IOCON->R_PIO0_11     = 0b00011001001; // IN2 pull down ... あとでAnalog にする
	LPC_IOCON->PIO0_5        = 0b00000000000; // IN3 pull downできないので、外部でpull down必要 == I2C_DA
	LPC_IOCON->PIO0_6        = 0b00011001000; // IN4 pull down
	*/
	// pull up
	if (!displaymode) {
		LPC_IOCON->SWCLK_PIO0_10 = 0b011010001; // IN1 pull up
		//	LPC_IOCON->R_PIO0_11     = 0b011010001; // IN2 pull up ... あとでAnalog にする
		LPC_IOCON->PIO0_6        = 0b011010000; // IN4 pull up
		//LPC_GPIO0->DIR &= ~((1 << 10) | (1 < 11) | (1 << 5) | (1 << 6) | (1 << 2));
		LPC_GPIO0->DIR &= ~((1 << 10) | (1 < 11) | (1 << 5) | (1 << 6));
	} else {
		//LPC_GPIO0->DIR &= ~((1 << 5) | (1 << 2));
		LPC_GPIO0->DIR &= ~(1 << 5); // 1.3b2
	}
	LPC_IOCON->R_PIO0_11 = 0x42; // AD0 IN2
	LPC_IOCON->PIO0_5 = 0b00000000; // IN3 no pull up (pull upできないので、外部でpull up必要 == I2C_DA)
	LPC_IOCON->PIO1_4 = 0b01000001; // BTN = AD5 analog, no pull = 0x41
	//LPC_IOCON->PIO0_2 = 0b11001000; // SOUND 0xc8 pulldown // psg_init()より // 1.3b2でsound_init()へ

	//	LPC_GPIO1->DIR &= ~(1 << 4); // -> 1.2b13 なくてもok? デフォルトDIR=0 切り替えることもない

	//	LPC_IOCON->PIO0_4        = 0b000000000; // EX1 no pull up
	//	LPC_GPIO0->DIR &= ~((1 << 10) | (1 < 11) | (1 << 5) | (1 << 6) | (1 << 4)); // add PIO0_4

	// OUT
	// 1.3.2b13まで no pullup
	LPC_IOCON->R_PIO1_0     = 0b11000001; // OUT1 1.1b5 0xd1 ->0xc1; // IN5と併用
	LPC_IOCON->R_PIO1_1     = 0b11000001; // OUT2 PWM2/IN6と併用
	LPC_IOCON->R_PIO1_2     = 0b11000001; // OUT3 PWM3/IN7と併用
	LPC_IOCON->SWDIO_PIO1_3 = 0b11000001; // OUT4 PWM4/IN8と併用
	
	// pullup
	/*
	LPC_IOCON->R_PIO1_0     = 0b11010001; // OUT1 IN5と併用 1.1b5 0xd1 ->0xc1 -> 0xd1?
	LPC_IOCON->R_PIO1_1     = 0b11010001; // OUT2 PWM2/IN6と併用
	LPC_IOCON->R_PIO1_2     = 0b11010001; // OUT3 PWM3/IN7と併用
	LPC_IOCON->SWDIO_PIO1_3 = 0b11010001; // OUT4 PWM4/IN8と併用
	*/
	LPC_IOCON->PIO1_5 = 0xd0; // LED

	LPC_IOCON->PIO1_9 = 0xd0; // OUT5 0b11010000 // ir motorと一緒 c0:no pullup? d0:pullup?
	LPC_IOCON->PIO1_8 = 0xd0; // OUT6

	LPC_GPIO1->DIR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 8) | (1 << 9);
	LPC_GPIO1->MASKED_ACCESS[(1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5) | (1 << 8) | (1 << 9)] = 0; // もしかしたらいらないかも？

	// analog
	// Disable Power down bit to the ADC block
	LPC_SYSCON->PDRUNCFG &= ~(1 << 4); // IRCOUT_PD IRC_PD
	// ADC Clock ON (48MHz)
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 13) | (1 << 10) | (1 << 8); // analog, CT32B1 | CT16B1



	// Burst Start(1 << 16), 4.3MHz, AD0-5
	//	AD0CR = (1 << 16) + ((11 - 1) << 8) + 0b111111;
	// AD0 & AD5
	// //Clkdiv This time clk=4MHz (max=4.5MHz)
	// 	 LPC_ADC->CR = ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/4000000-1)<<8;
	LPC_ADC->CR = (1 << 16) + ((11 - 1) << 8) + 0b100001;

}
// ADC
inline uint32_t analog_get(int ch) {
	if (LPC_ADC->CR & (1 << ch)) {
		uint32_t data;
		while (!((data = LPC_ADC->DR[ch]) & 0x80000000));
		return (data >> 6) & 0x3ff;
	}
	return 0;
}

#define ANA_THRESHOLD (1024 / 4)
inline int io_get() {
	int n = LPC_GPIO0->DATA;
	n = ((n >> 10) & 0x3) | ((n >> (5 - 2)) & 0xc);
	int m = LPC_GPIO1->DATA; // 1.1b6
	n |= (m & (1 << 9)) | ((m << 2) & (1 << 10)); // 1.1b6
	//#if !defined(LCD_MODE2) || defined(LCD_MODE3)
	if (!displaymode) {
		if (analog_get(0) > ANA_THRESHOLD) // IN2 = A0
			n |= 2;
	}
	//#endif
	if (analog_get(5) > ANA_THRESHOLD) // BTN = A5
		n |= 1 << 8; // 1.1b5 (1 << 4) -> (1 << 8)
	for (int i = 0; i < 4; i++) {
		if (!(LPC_GPIO1->DIR & (1 << i))) { // IN設定ならば
			if (analog_get(i + 1) > ANA_THRESHOLD)
				n |= 1 << (i + 4);
		}
	}
	/* // IN設定かどうかみてるから不要なはず 1.2b32
	#if defined(LCD_MODE) // IN1 と IN4 を無効化
	n &= ~((1 << 0) | (1 << 3));
	#elif defined(LCD_MODE2) || defined(LCD_MODE3) // IN1 と IN2 と IN4 を無効化
	n &= ~((1 << 0) | (1 << 1)| (1 << 3));
	#endif
	*/
	return n;
}
inline void io_set(int n) {
	/*
	OUT
		1 PIO1_0
		2 PIO1_1
		3 PIO1_2
		4 PIO1_3
		5 PIO1_9  x PIO0_2
		6 PIO1_8
		7 PIO1_5 -- LED
		8 PIO0_10 // 1.1b6 (8-11)
		9 PIO0_11
		10 PIO0_5
		11 PIO0_6
	*/
	LPC_GPIO1->MASKED_ACCESS[0b001100101111] = ((n & 0b10000) << 5) | ((n & 0b100000) << 3) | (n & 0b1111) | ((n & 0b1000000) >> 1);
	//#if defined(LCD_MODE) // IN1(0_10) と IN4(0_6) を無効化
	//	LPC_GPIO0->MASKED_ACCESS[0b100000100000] = ((n & 0b11000000000) >> 4) | ((n & 0b110000000) << 3); // 1.1b6
	if (!displaymode) {
		#ifdef USE_PIN_RST
		// IN1(0_10), IN2(0_11), IN4(0_6)を無効化
		LPC_GPIO0->MASKED_ACCESS[0b110001100000] = ((n & 0b11000000000) >> 4) | ((n & 0b110000000) << 3);
		#else
		// IN1(0_10), IN2(0_11)を無効化
		LPC_GPIO0->MASKED_ACCESS[0b110000100000] = ((n & 0b11000000000) >> 4) | ((n & 0b110000000) << 3);
		#endif
	} else {
		LPC_GPIO0->MASKED_ACCESS[0b000000100000] = ((n & 0b11000000000) >> 4) | ((n & 0b110000000) << 3); // 1.1b6
	}

	// OUT5-SOUND入れ替え
	//	LPC_GPIO1->MASKED_ACCESS[0b0100001111] = ((n & 0b100000) << 3) | (n & 0b1111);
	//	LPC_GPIO0->MASKED_ACCESS[0b0000000100] = n >> 2;
}

inline void IJB_led(int st) {
	IJB_out(7, st != 0);
}
inline int IJB_in() {
	return io_get();
}
// out buffer
//uint16_t outbuf = 0;
inline void IJB_out(int port, int st) {
//	static uint16_t outbuf = 0; // sttaicから外すと32byte空いた
	if (port) {
//		pwm_off(port); // いらないのでは？ 1.3.2b12 -80byte
		if (st < 0) { // OUT n(1-4),-1 でIN(ANA)へ切り替え、ANALOGオープンへ（-1:open、-2:pullup(OUT1-6まで有効)
			if (port >= 1 && port <= 4) {
				LPC_GPIO1->DIR &= ~(1 << (port - 1));
				int set = 0x42; // open
				if (st == -2)
					set |= 0x10; // -1:open, -2:pullup
				if (port == 1) {
					LPC_IOCON->R_PIO1_0 = set; // AD1 IN5と併用 // out1
				} else if (port == 2) {
					LPC_IOCON->R_PIO1_1 = set; // AD2 PWM2/IN6と併用
				} else if (port == 3) {
					LPC_IOCON->R_PIO1_2 = set; // AD3 PWM3/IN7と併用
				} else if (port == 4) {
					LPC_IOCON->SWDIO_PIO1_3 = set; // AD4 PWM4/IN8と併用
				}
				LPC_ADC->CR |= (1 << port);
			} else if (port == 5) { // 1.1b6
				LPC_GPIO1->DIR &= ~(1 << 9);
				LPC_IOCON->PIO1_9 = st == -1 ? 0xc0 : 0xd0;
			} else if (port == 6) {
				LPC_GPIO1->DIR &= ~(1 << 8);
				LPC_IOCON->PIO1_8 = st == -1 ? 0xc0 : 0xd0;
			} else if (port == 8) {
				LPC_GPIO0->DIR &= ~(1 << 10); // IN1
				LPC_IOCON->SWCLK_PIO0_10 = st == -1 ? 0xc1 : 0xd1;
			} else if (port == 9) {
				LPC_GPIO0->DIR &= ~(1 << 11); // IN2
				LPC_IOCON->R_PIO0_11 = st == -1 ? 0x42 : 0x52;
			} else if (port == 10) {
				LPC_GPIO0->DIR &= ~(1 << 5); // IN3 open drain
				LPC_IOCON->PIO0_5 = 0;
			} else if (port == 11) {
				LPC_GPIO0->DIR &= ~(1 << 6); // IN4
				LPC_IOCON->PIO0_6 = st == -1 ? 0xc0 : 0xd0;
			}
		} else {
			if (port >= 1 && port <= 4) {
				LPC_GPIO1->DIR |= 1 << (port - 1);
				if (port == 1) {
					LPC_IOCON->R_PIO1_0 = 0xc1; // IN5と併用
				} else if (port == 2) {
					LPC_IOCON->R_PIO1_1 = 0xc1; // PWM2/IN6と併用
				} else if (port == 3) {
					LPC_IOCON->R_PIO1_2 = 0xc1; // PWM3/IN7と併用
				} else if (port == 4) {
					LPC_IOCON->SWDIO_PIO1_3 = 0xc1; // PWM4/IN8と併用
				}
				LPC_ADC->CR &= ~(1 << port);
			} else if (port == 5) { // 1.1b6
				LPC_GPIO1->DIR |= (1 << 9);
				LPC_IOCON->PIO1_9 = 0xc0;
			} else if (port == 6) {
				LPC_GPIO1->DIR |= (1 << 8);
				LPC_IOCON->PIO1_8 = 0xc0;
			} else if (port == 8) {
				LPC_GPIO0->DIR |= (1 << 10); // IN1
				LPC_IOCON->SWCLK_PIO0_10 = 0xc1;
			} else if (port == 9) {
				LPC_GPIO0->DIR |= (1 << 11); // IN2
				LPC_IOCON->R_PIO0_11 = 0xc1;
			} else if (port == 10) {
				LPC_GPIO0->DIR |= (1 << 5); // IN3
				LPC_IOCON->PIO0_5 = 0;
			} else if (port == 11) {
				LPC_GPIO0->DIR |= (1 << 6); // IN4
				LPC_IOCON->PIO0_6 = 0xc0;
			}
			if (st) {
				_g.outbuf |= 1 << (port - 1);
			} else {
				_g.outbuf &= ~(1 << (port - 1));
			}
		}
	} else {
		_g.outbuf = st;
	}
	io_set(_g.outbuf);
}
inline int IJB_btn(int n) {
	//	return button_get();
	if (n == 0) {
		return analog_get(5) < ANA_THRESHOLD;
	} else if (n < 0) { // 1.3.2b17
		return keykbhit;
	}
	return key_btn(n);
}
inline int IJB_ana(int n) {
	if (n == 0 || n == 9) { // 5 -> 9
		n = 5;
	} else if (n == 2) {
		n = 0;
	} else if (n >= 5 && n <= 8) {
		n -= 4;
	} else {
		return 0;
	}
	return analog_get(n);
}
inline void IJB_clo() {
	io_init();
}

/*
void IJB_motor(int ch, int n) {
#ifdef SUPPORT_IR
	LPC_TMR16B0->TCR = 0; // 一端ビデオ止める
	systick_init(38000 * 2); // for IR
	irLegoSingleOutput(ch >> 1, ch & 1, n);
	systick_stop();
	LPC_TMR16B0->TCR = 1; // 再開
#endif
}
*/

#include "iic.h"

// writemode: I2C_READ, I2C_WRITE
inline int IJB_i2c(uint8_t writemode, uint16* param) { // 0:success 1:io error 2:parm error
	// i2cad, int adcmd, int lencmd, int addata, int lendata
	int adcmd = param[1];
	int lencmd = param[2]; // 1-4byte それ以上はNG! -> 緩和
	int addata = param[3];
	int lendata = param[4];
	adcmd -= OFFSET_RAMROM;
	addata -= OFFSET_RAMROM;
	if (lencmd > 0 && (adcmd < 0 || adcmd + lencmd > SIZE_RAM)) { // 長さ0でアドレスチェックしない 1.2b42
		return 2; // address error
	}
	if (lendata > 0 && (addata < 0 || addata + lendata > SIZE_RAM)) { // 長さ0でアドレスチェックしない 1.2b42
		return 2; // address error
	}
//	xprintf("i2c %d: #%x %d %d %d %d\n", writemode, param[0], adcmd, lencmd, addata, lendata);
	if (i2c0_init()) {
		return 1;
	}
	//	I2CCTRL I2cCtrl;
	I2cCtrl.sla = param[0]; //i2cad;
	I2cCtrl.dir = writemode; // ? I2C_WRITE : I2C_READ;
	I2cCtrl.ncmd = lencmd;
	I2cCtrl.cmd = (uint8_t*)(ram + adcmd);
	I2cCtrl.ndata = lendata;
	I2cCtrl.data = (uint8_t*)(ram + addata);
	//	I2cCtrl.eotfunc = 0; // i2c_eot2;
	//if (!i2c0_start(&I2cCtrl)) {
	if (!i2c0_start()) {
		return 1; // 1.2b40
	}
	//while (!I2cCtrl.stat) {
	while (!_g.i2c_stat) {
		//		xprintf("%d\n", I2cCtrl.stat);
		if (stopExecute()) { // 1.2b38
			i2c0_abort(); // 1.2b40
			break;
		}
	}
	/*
	IOCON_PIO0_5 = 0x0; // 戻さなくても大丈夫そう?
	GPIO0DIR &= ~(1<<5);
	*/
	//return I2cCtrl.stat != I2C_SUCCEEDED;
	return _g.i2c_stat != I2C_SUCCEEDED;
}

#endif	//__IO_H__