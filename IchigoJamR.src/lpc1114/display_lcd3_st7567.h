#ifndef __DISPLAY_LCD3_ST7567_H__
#define __DISPLAY_LCD3_ST7567_H__

/*
参考 lcdmode1
AQM1248A - 400円（基板セット750円）
VDD    - VCC
~CS    - GND
~RESET - NC
RS     - IN4
SCLK   - IN1
SDI    - VIDEO2
GND    - GND

132x65
	128-132 = 4

FSTN液晶モジュール（SPI） [CH12864F-SPI]
	http://www.aitendo.com/product/12392
	950円
	128x64
	ST7567R
		http://aitendo3.sakura.ne.jp/aitendo_data/product_img/lcd/fstn/COG12832G-508/20080403002714_ST7567_V0.1.pdf
	4線SPI
	接続
		1
		2
		3
		4
		5
		6
		7	SCL - IN1 PIO0_10
		8	SI - VIDEO2
		9	VDD - 3.3V
		10	VSS - GND
		11	LEDA - 3.3V - backlight つけなくてもok、制御できたら楽しい
		12	/CS - GND ok
		13	/RST - 3.3V IN4 PIO0_6
		14	A0 RS D/I (1:DATA, 0:COMMAND) - IN2 PIO0_11
		15
		16
	接続
		6本、/CS=GND、LEDAはつないで制御してもよし

video4 で切替？
	LCDコマンド?
		とぐる、LCD0でオフ,LCD1でオン
ver 1.3 新コマンド
	LCD でトグル
	POS() - cursor位置
	LOCATE 三番目でON/OFF
	SCRW() ?
	SCRH() ?
	WIDTH
	HEIGHT
	LCD - LCD切替 VIDEOはOFFになる、ファンクションキーに設定？
		VIDEO1がプルダウンされていたら？
		スイッチ切替？

*/
//#define SCREEN_W 16 // 128/8 ver 1.2
//#define SCREEN_H 8 // 64 / 8
//uint SCREEN_W = 16;
//uint SCREEN_H = 8;

// 1684 -> 1400 ... 284byte
// intやuint8にすると容量増大

#define LCD_WIDTH 132
#define LCD_HEIGHT 65

#define CMD_DISPLAY_OFF 0xAE
#define CMD_DISPLAY_ON 0xAF

#define CMD_SET_START_LINE 0x40
#define CMD_SET_PAGE 0xB0

#define CMD_SET_COLUMN_UPPER 0x10
#define CMD_SET_COLUMN_LOWER 0x00

#define CMD_SET_SEG_NORMAL  0xA0
#define CMD_SET_SEG_REVERSE 0xA1

#define CMD_SET_DISP_NORMAL 0xA6
#define CMD_SET_DISP_REVERSE 0xA7

#define CMD_SET_ALLPTS_NORMAL 0xA4
#define CMD_SET_ALLPTS_ON 0xA5

#define CMD_SET_INVERT 0xA6
#define CMD_SET_INVERT_ON 0xA7
#define CMD_SET_INVERT_OFF 0xA6

#define CMD_SET_BIAS_9 0xA2 
#define CMD_SET_BIAS_7 0xA3

#define CMD_RMW 0xE0
#define CMD_RMW_END 0xEE

#define CMD_INTERNAL_RESET 0xE2

#define CMD_SET_COM_NORMAL 0xC0
#define CMD_SET_COM_REVERSE 0xC8

#define CMD_SET_POWER_CONTROL 0x28
#define CMD_SET_REGULATION_RATIO 0x20
#define CMD_SET_VOLUME_FIRST 0x81
#define CMD_SET_VOLUME_SECOND 0
#define CMD_SET_STATIC_OFF 0xAC
#define CMD_SET_STATIC_ON 0xAD
#define CMD_SET_STATIC_REG 0x0
#define CMD_SET_BOOSTER_FIRST 0xF8
#define CMD_SET_BOOSTER_234 0
#define CMD_SET_BOOSTER_5 1
#define CMD_SET_BOOSTER_6 3
#define CMD_NOP 0xE3
#define CMD_TEST 0xF0

#define ST7565_STARTBYTES 1

#include "ssp.h"

//extern volatile uint8 vflag; // bit

//uint8 lcd_init = 0;

//#define CS0()	LPC_GPIO1->MASKED_ACCESS[1 << 0] = 0
//#define CS1()	LPC_GPIO1->MASKED_ACCESS[1 << 0] = 1 << 0

#define SPI_DATA(n)	LPC_GPIO0->MASKED_ACCESS[1 << 9] = (n) << 9
#define SPI_CLK(n)	LPC_GPIO0->MASKED_ACCESS[1 << 10] = (n) << 10 // IN1
#define SPI_RS(n)	LPC_GPIO0->MASKED_ACCESS[1 << 11] = (n) << 11 // IN2
#define SPI_RST(n)	LPC_GPIO0->MASKED_ACCESS[1 << 6] = (n) << 6 // IN4

#define USE_SPI_DRIVER

int IJB_wait(int n, int active); // if stop ret 1

void lcd_send(uint8_t n) {
	#if defined(USE_SPI_DRIVER)
		while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait if full
	//	while (!(LPC_SSP0->SR & SSPSR_TFE)); // wait until empty?
//		CS0();
		LPC_SSP0->DR = n; // data to send SPI
		while (!(LPC_SSP0->SR & SSPSR_TFE)); // wait until empty?
//		CS1();
	#else
	
	//	CS0();
		for (int i = 7; i >= 0; i--) {
			SPI_CLK(0);
			SPI_DATA((n >> i) & 1);
			SPI_CLK(1);
	//		IJB_wait(-1, 1);
		}
	//	CS1();
	#endif
}

// uint8 lcd_reverse = 1
#define LCD_REVERSE // コメント外すと通常表示

INLINEA void lcd_show(int k, int offx) {
	//	uint8* vram = (uint8*)(ram + OFFSET_RAM_VRAM);
	
	const uint8_t* crom = CHAR_PATTERN;
	const uint8_t* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
	
	/*
	for (int k = 0; k < 6; k++) {
		LPC_GPIO0->MASKED_ACCESS[1 << 11] = 0; // RS=0
		lcd_send(0b00010000);
		lcd_send(0b00000000);
		lcd_send(0b10110000 + k);
		while (!(LPC_SSP0->SR & SSPSR_TFE));
		
		IJB_wait(-1, 1);
		
		LPC_GPIO0->MASKED_ACCESS[1 << 11] = 1 << 11; // RS=1
		for (int i = 0; i < 128 / 8; i++) {
			uint8 c = vram[i + k * SCREEN_W];
			
			for (int l = 7; l >= 0; l--) {
				int n = 0;
				for (int m = 0; m < 8; m++) {
					uint8 w = 0;
					if (c >= 0x100 - SIZE_PCG) {
						w = crom2[(c << 3) + m];
					} else {
						w = crom[(c << 3) + m];
					}
					n |= ((w >> l) & 1) << m;
				}
				lcd_send(n);
			}
		}
		lcd_send(0);
		while (!(LPC_SSP0->SR & SSPSR_TFE));
	}
	*/
	// invert or not
//	uint16_t invert = 0; // invert無効
	uint16_t invert = _g.screen_invert ? 0xff : 0; // invert or not なぜか一番上の行が表示されない -> 大丈夫? でも重いので設定で行う
	
//	int k = lines >> 4; // 元
//	int k = lines >> 5; // 1.2b64
	/*
	int k2 = lcd_reverse ? (SCREEN_H - 1) - k : k;
	int cx = lcd_reverse ? (SCREEN_W - 1) - _g.cursorx : _g.cursorx;
	int cy = lcd_reverse ? (SCREEN_H - 1) - _g.cursory : _g.cursory;
	*/
#ifdef LCD_REVERSE
	int k2 = (SCREEN_H - 1) - k;
	int cx = (SCREEN_W - 1) - _g.cursorx;
	int cy = (SCREEN_H - 1) - _g.cursory;
#else
	int k2 = k;
	int cx = _g.cursorx;
	int cy = _g.cursory;
#endif
	//	if (!(lines & 31)) {
	if (offx == -1) {
		SPI_RS(0);
		lcd_send(CMD_SET_PAGE | k);
		int col = 4; // 132px
		lcd_send(CMD_SET_COLUMN_LOWER | (col & 0xf));
		lcd_send(CMD_SET_COLUMN_UPPER | ((col >> 4) & 0xf));
	  lcd_send(CMD_RMW);		
		//		while (!(LPC_SSP0->SR & SSPSR_TFE));
		SPI_RS(1);
	} else {
//	for (int i = 0; i < SCREEN_W / 16; i++) {
		//			int x = lcd_reverse ? (SCREEN_W - 1) - i : i;
#ifdef LCD_REVERSE
		int x = (SCREEN_W - 1) - offx; // i - (lines & 31) * 4;
#else
		int x = offx; // i + (lines & 31) * 4;
#endif
		uint8_t c = vram[x + k2 * SCREEN_W];
		
		uint8_t cptn = 0;
		if ((frames & 0x10) && _g.cursorflg) {
			if (k == cy && offx == cx) {
				if (!key_flg.insert) {
					cptn = 0xf0;
				} else {
					cptn = 0xff;
				}
			}
		}
		const uint8_t* cr = 0;
		if (c >= 0x100 - SIZE_PCG) {
			cr = crom2;
		} else {
			cr = crom;
		}
		cr += c << 3;
		//			if (lcd_reverse) {
#ifdef LCD_REVERSE
		for (int l = 0; l < 8; l++) {
			int n = 0;
			for (int m = 0; m < 8; m++) {
				uint8_t w = cr[7 - m];
				w ^= cptn;
				n |= ((w >> l) & 1) << m;
			}
			lcd_send(n ^ invert);
			//lcd_send(n);
		}
#else
		for (int l = 7; l >= 0; l--) {
			int n = 0;
			for (int m = 0; m < 8; m++) {
				uint8 w = cr[m];
				w ^= cptn;
				n |= ((w >> l) & 1) << m;
			}
			lcd_send(n ^ invert);
			//lcd_send(n);
		}
#endif
//		lcd_send(0); // 1,2b64 いらない?? 後ろうめるため？
//		while (!(LPC_SSP0->SR & SSPSR_TFE));
	}
}

static inline void sound_tick();

// LINES = 262
// 16 * 8 + 8 = 132

DISPLAY_INLINE void lcd3_TIMER16_0_IRQHandler(void) {
	uint32_t ir = LPC_TMR16B0->IR;
	LPC_TMR16B0->IR = ir;
	if (ir == 0b1000) {
		LPC_TMR16B0->MR1++;
		if (LPC_TMR16B0->MR1 < PSG_RATIO) {
			sound_tick();
			psg_tick();
			return;
		}
		LPC_TMR16B0->MR1 = 0;
	}
	sound_tick();

	lines++;
//	if (lines <= 144 && (lines & 7) == 0) {
	if (lines < 256) {
		int col = lines & 31;
		if (col < 17) {
			if (_g.lcd_init && LPC_TMR16B0->MR0) {
				lcd_show(lines >> 5, col - 1);
			}
		}
	} else if (lines >= LINES - 1) { // 1.2.0 bug??
		lines = -1;
		frames++;
		_g.vflag = 1;
	}
	_g.linecnt++;
	
	psg_tick();
}

static uint8_t LCD_INIT[] = {
	CMD_SET_BIAS_7,
	//	CMD_SET_SEG_NORMAL,
	CMD_SET_SEG_REVERSE,
	CMD_SET_COM_NORMAL,
	CMD_SET_START_LINE | 0, //(8 * 6), // disp start line = 0
	CMD_SET_POWER_CONTROL | 0x4,
	0, // >50msec
	CMD_SET_POWER_CONTROL | 0x7,
	0, // >10msec
	// 1.3.2b12 可変に
//	CMD_SET_REGULATION_RATIO | 5, // regulation ratio 小さいディスプレイichigoboymini（1-3:薄くてみえない, 4:ちょっと薄い、薄くて見えない、5:ちょうどいい, 6:濃い, 7:見えない）
	CMD_SET_REGULATION_RATIO | 4, // CMD_SET_REGULATION_RATIOは、設定しないと表示されない
	CMD_DISPLAY_ON,

	CMD_SET_ALLPTS_NORMAL,
//	CMD_SET_ALLPTS_ON,

//	CMD_SET_INVERT_ON, // 画面白黒反転

//	CMD_SET_VOLUME_FIRST, CMD_SET_VOLUME_SECOND | 14, // brightness // 最小:1-最大:0x3f regulation_ratio == 4 -> 5: 薄い, 10:ちょっと薄め、16なかなか、32濃くてみえない
	//CMD_SET_VOLUME_FIRST, CMD_SET_VOLUME_SECOND | 8, // brightness // 最小:1-最大:0x3f   regulation_ratio == 5 -> 1: ちょっと薄い, 10:いい感じ、16濃くて見えない
	
	//	CMD_SET_BOOSTER_FIRST, 2, // set booster  // 効果ない

};

void lcd3_video_on() { // LCD_MODE vide_on
//	_g.lcd_init = 0;
	if (_g.lcd_init) // 1.4.0b03
		return;

	SCREEN_W = 16;
	SCREEN_H = 8;
	
	// signal SPI0
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11) | (1 << 7); // SPI0, CT16B0(timer)
	LPC_SYSCON->PRESETCTRL |= 1 << 0; // reset SSP0
	
	// sync by 16bit timer0
	LPC_TMR16B0->EMR = 0;
	LPC_TMR16B0->PWMC = 0; // PIO0_8 PWMC off
//	LPC_TMR16B0->MCR = (0x000 << 0) | (0b001 << 3) | (0b010 << 6) | (0b000 << 9); // MR0 none, MR1 interrupt, MR2 reset, MR3 none
//	LPC_TMR16B0->MCR = (0x000 << 0) | (0b001 << 3) | (0b011 << 6) | (0b000 << 9); // MR0 none, MR1 interrupt, MR2 interrrupt & reset, MR3 none // for sound x2
	LPC_TMR16B0->MCR = (0x000 << 0) | (0b000 << 3) | (0b011 << 6) | (0b000 << 9); // MR0 none, MR1 none, MR2 interrrupt & reset, MR3 none
	//LPC_TMR16B0->MCR = (0b000 << 0) | (0b001 << 3) | (0b010 << 6) | (0b000 << 9); // MR0 noop(MAT0), MR1 interrupt, MR2 reset, MR3 interrupt
	LPC_TMR16B0->PR = 0;

	// 単独で使う場合必要！
//	NVIC_EnableIRQ(TIMER_16_0_IRQn);
//	NVIC_SetPriority(TIMER_16_0_IRQn, 1);
	
	/*
	//	LPC_TMR16B0->MR0 = FPORCH;
	LPC_TMR16B0->MR0 = 1; // video enable
	LPC_TMR16B0->MR1 = 0; //SIGNAL_OFFSET;
	LPC_TMR16B0->MR2 = VSYNC;
	LPC_TMR16B0->TCR = 1;
	*/
	LPC_TMR16B0->MR0 = 1; // video enable
	LPC_TMR16B0->MR2 = VSYNC;
	LPC_TMR16B0->TCR = 1;
	_g.psgratio = 1;

	#if defined(USE_SPI_DRIVER)
		// use SPI
		LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6) | (1 << 11);
		LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに

		LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
		LPC_IOCON->PIO0_9 =     0b00011000001; // MOSI0 no pullup
		LPC_IOCON->SWCLK_PIO0_10 = 0b11000010; // IN1 SCK0 no pullup (IOCON_SCK_LOC はデフォルトで0_10）
		LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> RS, PIO0_11 no pullup
		LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up

		LPC_SYSCON->SSP0CLKDIV = 1; // clock divider (1-255), main clock / 1 48MHzでもok
		
		//	LPC_SSP0->CR0 = 0b011111; // Frame Format:TI(01) Data Size: 16bit(1111) // for video
		LPC_SSP0->CR0 = 0b11000111; // Frame Format:CPHA(1) CPOL(1) CSPI(00) Data Size: 8bit(0111)

		LPC_SSP0->CPSR = 2; // clock prescale (2-254) 
		LPC_SSP0->CR1 = SSPCR1_SSE; // SPI Enable
	#else
		// use GPIO
		LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6) | (1 << 11);
		LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに

		LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
		LPC_IOCON->PIO0_9 =     0b00011000000; // GPIO no pullup
		LPC_IOCON->SWCLK_PIO0_10 = 0b11000001; // GPIO IN1 -> OUT
		LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> RS, PIO0_11 no pullup
		LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up
	#endif
	
	/*
	IJB_led(1);
	IJB_wait(1, 1);
	IJB_led(0);
	IJB_wait(-10, 1);
	IJB_led(1);
	IJB_wait(1, 1);
	*/
	SPI_RST(0);
	IJB_wait(1, 1);
	SPI_RST(1);
	IJB_wait(1, 1);
	
//	IJB_wait(120, 1);
	SPI_RS(0);
	for (int i = 0; i < sizeof(LCD_INIT); i++) {
		uint8_t n = LCD_INIT[i];
		if (!n) {
			IJB_wait(3, 1); // 50msec
		} else {
			lcd_send(n);
		}
//		IJB_wait(1, 1);
	}
	// 1.3.2b12
	uint contrast = _g.display_mode - 1;
	if (contrast == 0 || contrast > 0x3f)
		contrast = 14;
	lcd_send(CMD_SET_VOLUME_FIRST);
	lcd_send(CMD_SET_VOLUME_SECOND | contrast); // brightness // 最小:1-最大:0x3f regulation_ratio == 4 -> 5: 薄い, 10:ちょっと薄め、16なかなか、32濃くてみえない

	// 1.4.0b03
//	lcd_send(CMD_SET_INVERT + _g.screen_invert); // この実装の方が軽い
	
//	lcd_send(CMD_SET_REGULATION_RATIO | contrast); // regulation ratio 小さいディスプレイichigoboymini（1-3:薄くてみえない, 4:ちょっと薄い、5:ちょうどいい, 6-7:濃い見えないデフォ?）
//	while (!(LPC_SSP0->SR & SSPSR_TFE));
	
	// test
	/*
	for (int i = 0; i < 9; i++) {
		LPC_GPIO0->MASKED_ACCESS[1 << 6] = 0 << 6; // RS=0
		lcd_send(CMD_SET_PAGE | i);
		int col = 0;
		lcd_send(CMD_RMW_END);
		lcd_send(CMD_SET_COLUMN_LOWER | (col & 0xf));
		lcd_send(CMD_SET_COLUMN_UPPER | ((col >> 4) & 0xf));
		lcd_send(CMD_RMW);
		
		LPC_GPIO0->MASKED_ACCESS[1 << 6] = 1 << 6; // RS=1
		for (int i = 0; i < LCD_HEIGHT; i++) {
			lcd_send(0);
		}
	}
	*/
	
	lines = -1; // 1.2b64
	_g.lcd_init = 1;
	
#ifndef SKIP_SLOWCLOCK
	// 1.1b4 restore clock
	if (LPC_SYSCON->SYSAHBCLKDIV != 1) {
		LPC_SYSCON->SYSAHBCLKDIV = 1;
		SystemCoreClockUpdate();
	}
#endif
}
INLINE void lcd3_video_off(int clkdiv) {
	_g.lcd_init = 0;
	SPI_RST(0);

	LPC_TMR16B0->MCR = (0x000 << 0) | (0b000 << 3) | (0b000 << 6) | (0b011 << 9); // MR0 none, MR1 interrupt, MR2 interrrupt & reset, MR3 interrupt // for sound x2
	LPC_TMR16B0->MR0 = 0; // video disable
	LPC_TMR16B0->MR1 = 0; // as counter
	LPC_TMR16B0->MR3 = VSYNC_0; // for sound
	_g.psgratio = PSG_RATIO;

	
#ifndef SKIP_SLOWCLOCK
	// 1.1b4 clock down
	if (clkdiv < 1 || clkdiv > 255)
		clkdiv = 1;
	LPC_SYSCON->SYSAHBCLKDIV = clkdiv;
	SystemCoreClockUpdate();
#endif
}
INLINE int lcd3_video_active() {
	return _g.lcd_init;
}

#endif
