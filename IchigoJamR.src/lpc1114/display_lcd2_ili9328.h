/*
240x320 液晶 M024C9328SPI ILI9328 2.4インチ
	http://www.aitendo.com/product/10944
	コントローラ：ILI9328
	http://www.aitendo.com/product/10943
	2.2インチ液晶モジュール（SPI） [M022C9328SPI]
	2.2インチ、ILI9328/SPI、3.3V/5V

SDI(SDA): VIDEO2
SCK(SCL): IN1 PIO0_10
CS0: ずっと 0 でok!? ->だめ、制御しないと動かない IN2 -> PIO0_11
	CS(SSEL): SOUND ひとつずつSELする場合使えない、BSYフラグで判定
RST: IN4 PIO0_6

D_SDO:	NC		VIDEO1(NC)
D_LED:	GND		VIDEO2(SDI) SPI必須
D_SCK:	IN1		IN1(D_SCK) SPI必須
D_SDI:	VIDEO2	IN2(D_CS) GPIOでok
D_RS:	GND		IN3(NC)
D_RST:	IN4		IN4(D_RST) GPIOでok
D_CS:	IN2		VCC(VCC)
GND:	GND		GND(GND)
VCC_IN:	VCC		OUT1(NC)

6本あれば接続可能

2.2インチ液晶モジュール（SPI） [M022C9328SPI]
	http://www.aitendo.com/product/10943
	ILI9328 コントローラ同じ、これは互換性ありそう

M-TM022-SPI ILI9340C 2.2インチ
	http://www.aitendo.com/product/7277
	コントローラが違うので、別途対応必要

M032C9341B3 3.2インチ
	http://www.aitendo.com/product/10945
	ILI9341 ... これもコントローラ違った
	3線SPI SCL,SDA,CSX(chip select) 9bit 1bite使ってdata/command選択
	4線SPI SCL,SDA,D/CX,CSX  -- DCXを使ってdataかcommandを選ぶ
	interfaec I .. 書き込みのみ
	interface II .. 読み書きする

M028C8347D8 2.8インチ
	http://www.aitendo.com/product/10942
	コントローラ：HX8347D
	8bitコントロール

1.44インチ液晶モジュール（SPI） [M014C9163SPI]
	http://www.aitendo.com/product/3857
	128x128
	シリアル4-wire SPI
	ILI9163B

*/

#ifdef BIG_SCREEN
	#define SCREEN_W 20 // 320/16
	#define SCREEN_H 15 // 240/16
#else
	#define SCREEN_W 32
	#define SCREEN_H 24
#endif

// keyboard
#ifdef KEYBOARD_USB
void key_ping();
void key_tick();
#endif

#define RST0()	LPC_GPIO0->MASKED_ACCESS[1 << 6] = 0
#define RST1()	LPC_GPIO0->MASKED_ACCESS[1 << 6] = 1 << 6

int IJB_wait(int n, int active); // if stop ret 1
inline void IJB_led(int n); // for debug

void Delay(int n) {
	IJB_wait(-n * 10, 1);
}

uint8 lcd_init = 0;

#define USE_SPI_DRIVER 1

#if USE_SPI_DRIVER

void SendDataSPI(int n) {
	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait if full
	LPC_SSP0->DR = n; // data to send SPI
//	while (!(LPC_SSP0->SR & SSPSR_TFE)); // wait until empty send buffer
}

//#define CS0()
//#define CS1()
#define CS0()	LPC_GPIO0->MASKED_ACCESS[1 << 11] = 0
//#define CS1()	while (LPC_SSP0->SR & SSPSR_BSY); LPC_GPIO0->MASKED_ACCESS[1 << 11] = 1 << 11
void CS1() {
	while (LPC_SSP0->SR & SSPSR_BSY);
	LPC_GPIO0->MASKED_ACCESS[1 << 11] = 1 << 11;
}

#else

#define SDA0()	LPC_GPIO0->MASKED_ACCESS[1 << 9] = 0
#define SDA1()	LPC_GPIO0->MASKED_ACCESS[1 << 9] = 1 << 9

#define SCL0()	LPC_GPIO0->MASKED_ACCESS[1 << 10] = 0
#define SCL1()	LPC_GPIO0->MASKED_ACCESS[1 << 10] = 1 << 10

//#define CS0()
//#define CS1()

//#define CS0_()	LPC_GPIO0->MASKED_ACCESS[1 << 2] = 0
//#define CS1_()	LPC_GPIO0->MASKED_ACCESS[1 << 2] = 1 << 2
#define CS0()	LPC_GPIO0->MASKED_ACCESS[1 << 11] = 0
#define CS1()	LPC_GPIO0->MASKED_ACCESS[1 << 11] = 1 << 11

void SendDataSPI(unsigned char dat) {
//	CS0_();
	for (int i = 0; i < 8; i++) {
		if (dat & 0x80) {
			SDA1();
		} else {
			SDA0();
		}
		dat <<= 1;
		SCL0();
		SCL1();
	}
//	CS1_();
}

#endif

void WriteComm(unsigned int i) {
	CS0();
	SendDataSPI(0x70); // device ID + RS=0
	SendDataSPI(i >> 8);
	SendDataSPI(i);
	CS1();
}
void WriteData(unsigned int i) {
	CS0();
	SendDataSPI(0x72); // device ID + RS=1
	SendDataSPI(i >> 8);
	SendDataSPI(i);
	CS1();
}
void LCD_CtrlWrite_ILI9325C(unsigned int com, unsigned int dat) {
	WriteComm(com);
	WriteData(dat);
}

static int16 INIT_DATA[] = {
	0x00E5, 0x78F0,     // set SRAM internal timing 
	0x0001, 0x0000,     // set SS and SM bit 
	0x0002, 0x0400,     // set 1 line inversion 
	
	//	0x0003, 0x1090,     // set GRAM write direction(I/D/AM=010) and BGR=1. // 縦
	//	0x0003, 0x1088,     // set GRAM write direction(I/D/AM=110) and BGR=1. // 横（左端子）
	0x0003, 0x10B8,     // set GRAM write direction(I/D/AM=110) and BGR=1. // 横 上下逆（右端子）
	
	0x0004, 0x0000,     // Resize register 
	0x0008, 0x0202,     // set the back porch and front porch 
	0x0009, 0x0000,     // set non-display area refresh cycle ISC[3:0] 
	0x000A, 0x0000,     // FMARK function 
	0x000C, 0x0000,     // RGB interface setting 
	0x000D, 0x0000,     // Frame marker Position 
	0x000F, 0x0000,     // RGB interface polarity 
	
	//*************Power On sequence **************** // 
	0x0010, 0x0000,       // SAP, BT[3:0], AP, DSTB, SLP, STB 
	0x0011, 0x0007,       // DC1[2:0], DC0[2:0], VC[2:0] 
	0x0012, 0x0000,       // VREG1OUT voltage 
	0x0013, 0x0000,       // VDV[4:0] for VCOM amplitude 
	0x0007, 0x0001, 
	-200,                             // Dis-charge capacitor power voltage 
	0x0010, 0x1690,       // SAP, BT[3:0], AP, DSTB, SLP, STB 
	0x0011, 0x0227,       // Set DC1[2:0], DC0[2:0], VC[2:0] 
	-50,                       // Delay 50ms   
	0x0012, 0x008C,       // External reference voltage= Vci; 				   
	-50,                       // Delay 50ms 
	0x0013, 0x1500,       //VDV[4:0] for VCOM amplitude  0X1000
	0x0029, 0x0004,       // VCM[5:0] for VCOMH 		 0X0009
	0x002B, 0x000D,       //Set Frame Rate 
	-50,                               // Delay 50ms 
	0x0020, 0x0000,       // GRAM horizontal Address 
	0x0021, 0x0000,       // GRAM Vertical Address  
	// ----------- Adjust the Gamma Curve ----------// 
	0x0030, 0x0000, 
	0x0031, 0x0607, 
	0x0032, 0x0305, 
	0x0035, 0x0000, 
	0x0036, 0x1604, 
	0x0037, 0x0204, 
	0x0038, 0x0001, 
	0x0039, 0x0707, 
	0x003C, 0x0000, 
	0x003D, 0x000F, 
	//------------------ Set GRAM area ---------------// 
	0x0050, 0x0000,      // Horizontal GRAM Start Address 
	0x0051, 0x00EF,      // Horizontal GRAM End Address 
	0x0052, 0x0000,      // Vertical GRAM Start Address 
	0x0053, 0x013F,      // Vertical GRAM Start Address 
	0x0060, 0xA700,      // Gate Scan Line 
	0x0061, 0x0001,      // NDL,VLE, REV 
	0x006A, 0x0000,      //  set scrolling line 
	//-------------- Partial Display Control ---------// 
	0x0080, 0x0000, 
	0x0081, 0x0000, 
	0x0082, 0x0000, 
	0x0083, 0x0000, 
	0x0084, 0x0000, 
	0x0085, 0x0000, 
	//-------------- Panel Control -------------------// 
	0x0090, 0x0010, 
	0x0092, 0x0600, 
	0x0007, 0x0133,      // 262K color and display ON
	-1
};

void LCD_Init() {
	RST1();  
	Delay(10); // 1mesc
	RST0();
	Delay(100); // 10msec
	RST1();
	Delay(500); // 50mesc
	CS0();
	for (int i = 0;; i++) {
		int n = INIT_DATA[i];
		if (n == -1) {
			break;
		} else if (n < 0) {
			Delay(-n);
		} else {
			int m = INIT_DATA[++i];
			LCD_CtrlWrite_ILI9325C(n, m);
		}
	}
}


void BlockWrite(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	WriteComm(0x0050);
	WriteData(x); 
	WriteComm(0x0051);
	WriteData(x + w - 1);	
	WriteComm(0x0052);
	WriteData(y);
	WriteComm(0x0053);
	WriteData(y + h - 1);
	
	WriteComm(0x0020);
	WriteData(x);
	WriteComm(0x0021);
	WriteData(y);
	WriteComm(0x0022);
}

/*
Internal 172,800 bytes graphic RAM
 262,144 colors 

RS、使わない場合、GNDに
	A register select signal.
	 Low: select an index or status register
	 High: select a control register
	 Fix to either IOVcc or GND level when not in use. 
SDI 立ち上がりに読む
*/

inline void clearAll() {
	#define F_COL 240
	#define F_ROW 320
	BlockWrite(0, 0, F_COL, F_ROW);
	
	CS0();
	SendDataSPI(0x72);
	for (int i = 0; i < F_ROW * F_COL * 2; i++) {
		SendDataSPI(0);
	}
	CS1();
}

void DispColor(unsigned int color1) {
	//	int COL = 240;
	//	int ROW = 320;
	int COL = 24 * 8;
	int ROW = 32 * 8;
	BlockWrite((240 - COL) / 2, (320 - ROW) / 2, COL, ROW);
	
	CS0();
	SendDataSPI(0x72);
	for (int i = 0; i < ROW; i++) {
		for (int j = 0;j < COL; j++) {
			SendDataSPI(color1 >> 8);
			SendDataSPI(color1);
		}
	}
	CS1();
	/*
	for (int i = 0; i < ROW; i++) {
		for (int j = 0;j < COL; j++) {
			WriteData(color1);
		}
	}
	*/
}
void test() {
	for (;;) {
		DispColor(0xf800);	   //RED
		Delay(100);
		
		DispColor(0x07e0);	 //GREEN
		Delay(100);
		
		DispColor(0x001f);	//BLUE
		Delay(100);
		
		DispColor(0xffff);	 //WHITE		
		Delay(100);
		
		DispColor(0x0);	 //BLACK
		Delay(100);
	}
}

// uint8 lcd_reverse = 1
#define LCD_REVERSE // コメント外すと通常表示

inline void lcd_show() {
	
	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
	
	int cx = _g.cursorx;
	int cy = _g.cursory;
	
#ifdef BIG_SCREEN
	BlockWrite(0, 0, 240, 320);
	CS0();
	SendDataSPI(0x72);
	
	for (int i = 0; i < SCREEN_H * 8; i++) {
		int y = i >> 3;
		for (int i2 = 0; i2 < 2; i2++) {
			for (int j = 0; j < SCREEN_W; j++) {
				uint8 cptn = 0;
				if ((frames & 0x10) && _g.cursorflg) {
					if (y == cy && j == cx) {
						if (key_flg.insert) {
							cptn = 0xf0;
						} else {
							cptn = 0xff;
						}
					}
				}
				
				uint8 c = vram[j + y * SCREEN_W];
				const uint8* cr = crom;
				if (c >= 0x100 - SIZE_PCG) {
					cr = crom2;
				}
				cr += c << 3;
				uint8 w = cr[i & 7];
				w ^= cptn;
				for (int k = 0; k < 8; k++) {
					int b = w & (0x80 >> k);
					int color1 = b ? 0xffff : 0;
					SendDataSPI(color1 >> 8);
					SendDataSPI(color1);
					SendDataSPI(color1 >> 8);
					SendDataSPI(color1);
				}
			}
		}
	}
	CS1();
	
#else
	BlockWrite((240 - SCREEN_H * 8) / 2, (320 - SCREEN_W * 8) / 2, SCREEN_H * 8, SCREEN_W * 8);
	CS0();
	SendDataSPI(0x72);
	
	for (int i = 0; i < SCREEN_H * 8; i++) {
		int y = i >> 3;
		for (int j = 0; j < SCREEN_W; j++) {
			uint8 cptn = 0;
			if ((frames & 0x10) && _g.cursorflg) {
				if (y == cy && j == cx) {
					if (key_flg.insert) {
						cptn = 0xf0;
					} else {
						cptn = 0xff;
					}
				}
			}
			
			uint8 c = vram[j + y * SCREEN_W];
			const uint8* cr = crom;
			if (c >= 0x100 - SIZE_PCG) {
				cr = crom2;
			}
			cr += c << 3;
			uint8 w = cr[i & 7];
			w ^= cptn;
			for (int k = 0; k < 8; k++) {
				int b = w & (0x80 >> k);
				int color1 = b ? 0xffff : 0;
				SendDataSPI(color1 >> 8);
				SendDataSPI(color1);
			}
		}
	}
	CS1();
	
#endif
}


// test beep20,20
void TIMER16_0_IRQHandler(void) {
	uint ir = LPC_TMR16B0->IR;
	LPC_TMR16B0->IR = ir;
	
	LPC_GPIO0->MASKED_ACCESS[1 << 2] = _g.psg_sounder; // sound無効
	lines++;
	/*
	if (lines <= 13 * 8 && (lines & 7) == 0) {
		if (lcd_init && LPC_TMR16B0->MR0)
		lcd_show();
	*/
	if (lines == 0) {
		if (lcd_init) {
#ifdef BIG_SCREEN
			if ((frames & 0x3) == 0)
				lcd_show(); // 8/60秒に1回描画
#else
			if ((frames & 0x3) == 0)
				lcd_show(); // 4/60秒に1回描画
#endif
		}
	} else if (lines >= LINES - 1) { // 1.2.0 bug??
		lines = -1;
		frames++;
		vflag = 1;
	}
	linecnt++;
	psg_tick();
	
#ifdef KEYBOARD_USB
	// for USB keyboard
	if ((linecnt & 0x3) == 0x3) { // 描画に120msecかかっている、その間にキーボードの接続が切れる
		key_ping();
	} else if (linecnt == 0) {
		key_tick(); // 処理間に合っていないので分割必要
	}
#endif
}

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
	
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6) | (1 << 11); // VIDEO2:SDI, IN1:SCLK, IN4:RS (VIDEO1は使えない？）, IN2
	LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに
	
	LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up
	LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
	
#if USE_SPI_DRIVER
	// SPI
	LPC_IOCON->PIO0_9 =     0b00011000001; // MOSI0 no pullup
	LPC_IOCON->SWCLK_PIO0_10 = 0b11000010; // IN1 SCK0 no pullup (IOCON_SCK_LOC はデフォルトで0_10）
	LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> CS, PIO0_11 no pullup
	
	//	LPC_SYSCON->SSP0CLKDIV = 25; // clock divider (1-255), main clock / 18 (2.7MHz) // 15(3.2MHz)が限界、それ以上小さくすると映らなくなる
	LPC_SYSCON->SSP0CLKDIV = 1; // clock divider (1-255), main clock / 1 (48MHz) 最速でもok!
	
	LPC_SSP0->CR0 = 0b11000111; // Frame Format:CPHA(1) CPOL(1) CSPI(00) Data Size: 8bit(0111) アイドル時SLK1、0->1で送信
	
	LPC_SSP0->CPSR = 2; // clock prescale (2-254) 
	LPC_SSP0->CR1 = SSPCR1_SSE; // SPI Enable
	
#else
	LPC_GPIO0->DIR |= (1 << 2); // SOUNDピンをout
	LPC_IOCON->PIO0_9 =     0b00011000000; // GPIO, no pullup
	LPC_IOCON->SWCLK_PIO0_10 = 0b11000001; // GPIO, no pullup
	LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> CS, PIO0_11 no pullup
	
//	LPC_IOCON->R_PIO1_0 =     0b00011000001; // GPIO, no pullup // for CS
	
#endif
	
	LCD_Init();
	
	clearAll();
//	test();
	/*
	// test
	for (;;) {
		IJB_led(1);
		IJB_wait(60, 1);
		IJB_led(0);
		IJB_wait(60, 1);
	}
	*/
	
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
