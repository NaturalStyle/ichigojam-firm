#define SCREEN_W 16 // 128/8 ver 1.2
#define SCREEN_H 6 // 48 / 8

uint8 lcd_init = 0;

int IJB_wait(int n, int active); // if stop ret 1

void lcd_send(int n) {
	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait if full
	LPC_SSP0->DR = n; // data to send SPI
}

// uint8 lcd_reverse = 1
#define LCD_REVERSE // コメント外すと通常表示

inline void lcd_show() {
	//	uint8* vram = (uint8*)(ram + OFFSET_RAM_VRAM);
	
	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
	
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
	uint16 invert = 0; // _g.screen_invert ? 0xff : 0; // invert or not なぜか一番上の行が表示されない
	//	int invert = 0; // _g.screen_invert ? 0xff : 0; // invert or not なぜか一番上の行が表示されない
	
	int k = (lines >> 4) - 1;
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
	if (!(lines & 8)) {
		LPC_GPIO0->MASKED_ACCESS[1 << 6] = 0; // RS=0
		lcd_send(0b00010000);
		lcd_send(0b00000000);
		lcd_send(0b10110000 + k);
		//		while (!(LPC_SSP0->SR & SSPSR_TFE));
	} else {
		LPC_GPIO0->MASKED_ACCESS[1 << 6] = 1 << 6; // RS=1
		for (int i = 0; i < SCREEN_W; i++) {
			//			int x = lcd_reverse ? (SCREEN_W - 1) - i : i;
#ifdef LCD_REVERSE
			int x = (SCREEN_W - 1) - i;
#else
			int x = i;
#endif
			uint8 c = vram[x + k2 * SCREEN_W];
			
			uint8 cptn = 0;
			if ((frames & 0x10) && _g.cursorflg) {
				if (k == cy && i == cx) {
					if (key_flg.insert) {
						cptn = 0xf0;
					} else {
						cptn = 0xff;
					}
				}
			}
			const uint8* cr = 0;
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
					uint8 w = cr[7 - m];
					w ^= cptn;
					n |= ((w >> l) & 1) << m;
				}
				lcd_send(n ^ invert);
			}
					//			} else {
					#else			
					for (int l = 7; l >= 0; l--) {
				int n = 0;
				for (int m = 0; m < 8; m++) {
					uint8 w = cr[m];
					w ^= cptn;
					n |= ((w >> l) & 1) << m;
				}
				lcd_send(n ^ invert);
			}
					//			}
					#endif			
				}
				lcd_send(0);
		//		while (!(LPC_SSP0->SR & SSPSR_TFE));
	}
}

// test beep20,20
void TIMER16_0_IRQHandler(void) {
	uint ir = LPC_TMR16B0->IR;
	LPC_TMR16B0->IR = ir;
	
	LPC_GPIO0->MASKED_ACCESS[1 << 2] = _g.psg_sounder;
	lines++;
	if (lines <= 13 * 8 && (lines & 7) == 0) {
		if (lcd_init && LPC_TMR16B0->MR0)
		lcd_show();
	} else if (lines >= LINES - 1) { // 1.2.0 bug??
		lines = -1;
		frames++;
		vflag = 1;
	}
	linecnt++;
	psg_tick();
}

static char LCD_INIT[] = { 0xAE, 0xA0, 0xC8, 0xA3, 0x2C, 0, 0x2E, 0, 0x2F, 0x23, 0x81, 0x1C, 0xA4, 0x40, 0xA6, 0xAF };

/*
AQM1248A - 400円（基板セット750円）
VDD    - VCC
~CS    - GND
~RESET - NC
RS     - IN4
SCLK   - IN1
SDI    - VIDEO2
GND    - GND
*/

void video_on() { // LCD_MODE vide_on
	lcd_init = 0;
	
	// signal SPI0
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11) | (1 << 7); // SPI0, CT16B0(timer)
	LPC_SYSCON->PRESETCTRL |= 1 << 0; // reset SSP0
	
	// sync by 16bit timer0
	LPC_TMR16B0->EMR = 0;
	LPC_TMR16B0->PWMC = 0; // PIO0_8 PWMC off
	LPC_TMR16B0->MCR = (0b001 << 3) | (0b010 << 6); // MR0 noop(MAT0), MR1 interrupt, MR2 reset
	LPC_TMR16B0->PR = 0;
	
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	NVIC_SetPriority(TIMER_16_0_IRQn, 1);
	
	//	LPC_TMR16B0->MR0 = FPORCH;
	LPC_TMR16B0->MR0 = 1; // video enable
	LPC_TMR16B0->MR1 = 0; //SIGNAL_OFFSET;
	LPC_TMR16B0->MR2 = VSYNC;
	LPC_TMR16B0->TCR = 1;
	
	// SPI
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6); // VIDEO2:SDI, IN1:SCLK, IN4:RS (VIDEO1は使えない？）
	LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに
	
	LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
	LPC_IOCON->PIO0_9 =     0b00011000001; // MOSI0 no pullup
	LPC_IOCON->SWCLK_PIO0_10 = 0b11000010; // IN1 SCK0 no pullup (IOCON_SCK_LOC はデフォルトで0_10）
	//	LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> RS, PIO0_11 no pullup
	LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up
	
	LPC_SYSCON->SSP0CLKDIV = 25; // clock divider (1-255), main clock / 18 (2.7MHz) // 15(3.2MHz)が限界、それ以上小さくすると映らなくなる
	
	//	LPC_SSP0->CR0 = 0b011111; // Frame Format:TI(01) Data Size: 16bit(1111) // for video
	LPC_SSP0->CR0 = 0b11000111; // Frame Format:CPHA(1) CPOL(1) CSPI(00) Data Size: 8bit(0111)
	
	LPC_SSP0->CPSR = 2; // clock prescale (2-254) 
	LPC_SSP0->CR1 = SSPCR1_SSE; // SPI Enable
	
	// test
	/*
	for (;;) {
		IJB_led(1);
		IJB_wait(60, 1);
		IJB_led(0);
		IJB_wait(60, 1);
	}
	*/
	
	LPC_GPIO0->MASKED_ACCESS[1 << 6] = 0; // RS=0
	for (int i = 0; i < sizeof(LCD_INIT); i++) {
		int n = LCD_INIT[i];
		if (!n) {
			IJB_wait(-33, 1); // 2mesc wait = 1/8 line = 262/816msec
		} else {
			lcd_send(n);
		}
	}
	while (!(LPC_SSP0->SR & SSPSR_TFE));
	
	lcd_init = 1;
	
	// 1.1b4 restore clock
	if (LPC_SYSCON->SYSAHBCLKDIV != 1) {
		LPC_SYSCON->SYSAHBCLKDIV = 1;
		SystemCoreClockUpdate();
	}
}
void video_off(int clkdiv) {
	lcd_init = 0;
	
	// video off
	LPC_TMR16B0->MR0 = 0; // video disable
	
	// 1.1b4 clock down
	if (clkdiv < 1 || clkdiv > 255)
	clkdiv = 1;
	LPC_SYSCON->SYSAHBCLKDIV = clkdiv;
	SystemCoreClockUpdate();
}
