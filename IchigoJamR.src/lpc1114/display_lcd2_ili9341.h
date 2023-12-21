#ifndef __DISPLAY_LCD2_ILI9341_H__
#define __DISPLAY_LCD2_ILI9341_H__

/*
ILI9341
	SDI MOSI: VIDEO2
	SCK : IN1 - SPI_CLK
	D/C: IN2 (Data or Command)
	SS: GND  (slave select = chip select) 常時GNDでいける? → いける
	RESET　IN4（なくてもいい?） → 常時VCCでいけるけど、再起動時とかに不安定になるかも？ -> IN4

	MISO - readしなければ不要 - NC
	LED - 3.3V　(48mA?)

参考 7567
	7	SCL - IN1 PIO0_10
	8	SI - VIDEO2
	9	VDD - 3.3V
	10	VSS - GND
	11	LEDA - 3.3V - backlight つけなくてもok、制御できたら楽しい
	12	/CS - GND ok
	13	/RST - 3.3V IN4 PIO0_6
	14	A0 RS D/I (1:DATA, 0:COMMAND) - IN2 PIO0_11

IM0-3の設定は基板で固定？
	http://okiraku-camera.tokyo/blog/?p=6487
	1110 - 4 wire serial interface II - IIは読み取り時に、MISOを使うだけ、書き込みは一緒
		16bit or 18bitモードのみ
			COLMOD: Pixel Format Set (3Ah)で設定

Data Transfer Method 1
	stopコマンドまで連続して送れる

フレーム書き込み
	CASET 0x2a
	PASET 0x2b
	RAMWWR 0x2c

	2cの代わりに、Write_Memory_Continue (3Ch) にしておくと、連続転送？

aitendo
	https://www.aitendo.com/product/16038
		13 (28) VDD 1（VCC)
	8 (19,27) GND 2（GND)
	26 CS 3（CS）
	5 PORT9/RST 4（RESET)
	21 PORT8/DC 5（DC/RS)
	22 PORT7/MOSI 6（SDI/MOSI)
	25 PORT6/SCK 7(SCK)
	3 PORT5 8(LED)
	14 PORT4 9(SDO/MISO)

aitendo
	M-TM022-SPI
	ILI9340C

*/

#define USE_SPI_DRIVER
#define USE_PIN_RST // RSTを使わないと、IN4が空くけど、RESET時に初期化失敗する

// VERTICAL_SCREENの方がちょっと高速だけど、描画が自然じゃない、調整して差が縮まったので、HORIZONTAL_SCREENでいく
//#define VERTICAL_SCREEN

// keyboard
#ifdef KEYBOARD_USB
void key_ping();
void key_tick();
#endif

#define SPI_DC(n)	LPC_GPIO0->MASKED_ACCESS[1 << 11] = (n) << 11 // IN2
//#define SPI_DC(n)	LPC_GPIO0->MASKED_ACCESS[1 << 6] = (n) << 6 // IN4
//#define SPI_CS(n)	LPC_GPIO0->MASKED_ACCESS[1 << 6] = (n) << 6 // IN4
//#define SPI_RST(n)	IJB_out(1, n)

#define SPI_CS(n) // dummy
#ifdef USE_PIN_RST
#define SPI_RST(n)	LPC_GPIO0->MASKED_ACCESS[1 << 6] = (n) << 6 // IN4
#endif

//#define SPI_RST(n)	LPC_GPIO0->MASKED_ACCESS[1 << 11] = (n) << 11 // IN2
//#define SPI_CS(n)	LPC_GPIO0->MASKED_ACCESS[1 << 11] = (n) << 11 // IN2

int IJB_wait(int n, int active); // if stop ret 1
inline void IJB_led(int n); // for debug

void test() {
	for (;;) {
		IJB_led(1);
		IJB_wait(10, 1);
		IJB_led(0);
		IJB_wait(10, 1);
	}
}

void Delay(int n) {
	IJB_wait(-n * 10, 1);
}

#ifdef USE_SPI_DRIVER

#include "ssp.h"

void SendDataSPI(int n) {
	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait if full
	LPC_SSP0->DR = n; // data to send SPI
//	while (!(LPC_SSP0->SR & SSPSR_TFE)); // wait until empty send buffer
}
void SendDataSPI16(int n) {
	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait if full, Transmit FIFO Not Full. This bit is 0 if the Tx FIFO is full, 1 if not
	//LPC_SSP0->DR = n; // data to send SPI
	LPC_SSP0->DR = n >> 8;
	LPC_SSP0->DR = n & 0xff;
}
void SPI_Wait() {
	while ((LPC_SSP0->SR & SSPSR_BSY)); // wait until end to send
}

#else

#define SPI_DATA(n)	LPC_GPIO0->MASKED_ACCESS[1 << 9] = (n) << 9
#define SPI_CLK(n)	LPC_GPIO0->MASKED_ACCESS[1 << 10] = (n) << 10 // IN1

void SendDataSPI(int n) {
	for (int i = 7; i >= 0; i--) {
		SPI_DATA((n >> i) & 1);
		SPI_CLK(0);
		SPI_CLK(1);
	}
	SPI_CLK(0);
}
void SendDataSPI16(int n) {
	for (int i = 15; i >= 0; i--) {
		SPI_DATA((n >> i) & 1);
		SPI_CLK(0);
		SPI_CLK(1);
	}
	SPI_CLK(0);
}
inline void SPI_Wait() {
}

#endif

void WriteComm(unsigned int n) {
	SPI_Wait();
	SPI_CS(0);
	SPI_DC(0);
	SendDataSPI(n);
	SPI_Wait();
	SPI_DC(1);
	SPI_CS(1);
}
void WriteCommParam(int n) {
	SPI_Wait();
	SPI_CS(0);
	SPI_DC(1);
	SendDataSPI(n);
	SPI_Wait();
	SPI_DC(1);
	SPI_CS(1);
}
void WriteCommParam16(int n) {
	WriteCommParam(n >> 8);
	WriteCommParam(n & 0xff);
}

#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read
#define ILI9341_RAMWR_C 0x3C

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_IDLEMODEON 0x39 ///< Idle mode on
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set 16bit or 18bit

#define ILI9341_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC


// clang-format off
static const uint8_t initcmd[] = {
 // 何のため？なくても動く、仕様書に記述なし
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
	

	// 液晶への電源用
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0] // = 4.6V
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0] = default
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2

#ifdef VERTICAL_SCREEN
  ILI9341_MADCTL  , 1, 0b11000000, // Memory Access Control MY & MX - vertical screen
#else
  ILI9341_MADCTL  , 1, 0b11100000, // Memory Access Control MY & MX - horizontal screen
#endif
  //ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,						 // 16bits / pixel
  // ILI9341_FRMCTR1 , 2, 0x00, 0x18,				// Frame Rate Control
  ILI9341_FRMCTR1 , 2, 0x00, 0x1F,				// Frame Rate Control 60fps in Normal mode
  //ILI9341_FRMCTR2 , 2, 0x00, 0x1F,				// Frame Rate Control 60fps in Idle mode
  //ILI9341_IDLEMODEON , 0, 				// Idle mode on
  //ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control // いらない？
  //0xF2, 1, 0x00,                         // 3Gamma Function Disable
  //ILI9341_GAMMASET , 1, 0x01,            // Gamma curve selected
	// Set Gamma
  //ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  //ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};

void sendCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes) {
  WriteComm(commandByte); // Send the command byte
	for (int i = 0; i < numDataBytes; i++) {
		WriteCommParam(*dataBytes); // Send the data bytes
		dataBytes++;
  }
}
void LCD_Init() {
	// reset 必須
	
#ifdef USE_PIN_RST
	SPI_RST(0);
	Delay(100); // 50msec ??
	SPI_RST(1);
	Delay(100); // 50msec ??
#else
	Delay(100); // 5msec ??
	WriteComm(ILI9341_SWRESET); // 効かない
	Delay(100); // 5msec ??
#endif

  const uint8_t *addr = initcmd;
	for (;;) {
		uint8 cmd = *addr++;
		if (!cmd)
			break;
    uint8 x = *addr++;
    uint8 numArgs = x & 0x7f;
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;
    if (x & 0x80)
      Delay(150);
  }
}
void setAddrWindow(int x1, int y1, int w, int h) {
	uint x2 = (x1 + w - 1);
	uint y2 = (y1 + h - 1);
  WriteComm(ILI9341_CASET); // Column address set
	WriteCommParam16(x1);
	WriteCommParam16(x2);
  WriteComm(ILI9341_PASET); // Row address set
  WriteCommParam16(y1);
  WriteCommParam16(y2);
	WriteComm(ILI9341_RAMWR); // Write to RAM
}
/*
void DispColor(unsigned int color1) {
	//	int COL = 240;
	//	int ROW = 320;
	int COL = 24 * 8;
	int ROW = 32 * 8;
	BlockWrite((240 - COL) / 2, (320 - ROW) / 2, COL, ROW);

	SendDataSPI(0x72);
	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			SendDataSPI(color1);
		}
	}
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

*/

// int clear_color = 0;
void clearAll(int clear_color) {
	int x = 0;
	int y = 0;
	int w = 320;
	int h = 240;
#ifdef VERTICAL_SCREEN
	setAddrWindow(x, y, h, w); // Clipped area - vertical
#else
	setAddrWindow(x, y, w, h); // Clipped area - horizontal
#endif
	SPI_CS(0);
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			SendDataSPI16(clear_color);
		}
	}
	//clear_color++;
	SPI_Wait();
	SPI_CS(1);
}


int n_lcd_show; // 0 - 256 * 4

#ifdef VERTICAL_SCREEN

static inline void lcd_show() {
	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);

	#define ORG_SCREEN_H 24
	#define ORG_SCREEN_W 32

	// 横分割 50sec
	// video1 256 * 4, h/4ずつ描画 = 6文字分
	// video2 256 * 4, h/4ずつ描画 = 3文字分
	int j = n_lcd_show & 0xff;
	int offy = n_lcd_show >> 8;
	if (j == 0) {
		setAddrWindow((240 - ORG_SCREEN_H * 8) / 2 + offy * (ORG_SCREEN_H / 4 * 8), (320 - ORG_SCREEN_W * 8) / 2, ORG_SCREEN_H * (8 / 4), ORG_SCREEN_W * 8);
	}
	int h4 = SCREEN_H >> 2;
	offy *= h4;
	//int cw = 1 << _g.screen_big;
	
	/*
	// 縦分割 // 56sec、video2で76秒?
	if (n_lcd_show == 0) {
		setAddrWindow((240 - ORG_SCREEN_H * 8) / 2, (320 - ORG_SCREEN_W * 8) / 2, ORG_SCREEN_H * (8 / 4), ORG_SCREEN_W * 8);
	}
	int j = n_lcd_show >> 2;
	int offy = (n_lcd_show & 3) * (SCREEN_H >> 2);
	int cw = 1 << _g.screen_big;
	*/

	uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not
	//for (int j = 0; j < SCREEN_W * 8; j++) {

		int x = j >> (3 + _g.screen_big);
		int cflg = (frames & 0x10) && _g.cursorflg && x == _g.cursorx;
		int x2 = (j >> _g.screen_big) & 7;
		for (int i = 0; i < h4; i++) { // for 横分割
			int y = offy + i;
			uint8 cptn = invert;
			if (cflg && y == _g.cursory) {
				if (!key_flg.insert) {
					cptn ^= 0xf0;
				} else {
					cptn ^= 0xff;
				}
			}

			uint8 c = vram[x + y * SCREEN_W];
			const uint8* cr = crom;
			if (c >= 0x100 - SIZE_PCG) {
				cr = crom2;
			}
			cr += c << 3;

			if (!_g.screen_big) { // ループ内分岐すると処理落ちする
				for (int k = 0; k < 8; k++) {
					int w = cr[k] ^ cptn;
					int b = w & (0x80 >> x2);
					int color = b ? 0xffff : 0;
					LPC_SSP0->DR = color >> 8;
					LPC_SSP0->DR = color;
				}
			} else {
				for (int k = 0; k < 16; k++) {
					int w = cr[k >> 1] ^ cptn;
					int b = w & (0x80 >> x2);
					int color = b ? 0xffff : 0;
					LPC_SSP0->DR = color >> 8;
					LPC_SSP0->DR = color;
				}
			}
		}
	//}
	n_lcd_show++;
	n_lcd_show &= 0x3ff;
	/*
	if (n_lcd_show == 256 * 4) {
		n_lcd_show = 0;
	}
	*/
}

#else // horizontal

static inline void lcd_show() {
	const uint8* crom = CHAR_PATTERN;
	const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);

	#define ORG_SCREEN_H 24
	#define ORG_SCREEN_W 32
	if (n_lcd_show == 0) {
		setAddrWindow((320 - ORG_SCREEN_W * 8) / 2, (240 - ORG_SCREEN_H * 8) / 2, ORG_SCREEN_W * 8, ORG_SCREEN_H * 8);
	}
	int x = (n_lcd_show & 7) * (SCREEN_W >> 3);
	int y2 = (n_lcd_show >> (3 + _g.screen_big)) & 7;
	int y = n_lcd_show >> (3 + 3 + _g.screen_big);
	//int cw = 1 << _g.screen_big;
	
	uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not
	int cflg = (frames & 0x10) && _g.cursorflg && y == _g.cursory;
	for (int i = SCREEN_W / 8 - 1; i >= 0; i--, x++) {
		uint8 cptn = invert;
		if (cflg && x == _g.cursorx) {
			if (!key_flg.insert) {
				cptn ^= 0xf0;
			} else {
				cptn ^= 0xff;
			}
		}

		uint8 c = vram[x + y * SCREEN_W];
		const uint8* cr = crom;
		if (c >= 0x100 - SIZE_PCG) {
			cr = crom2;
		}
		cr += c << 3;
		c = cr[y2] ^ cptn;

		if (!_g.screen_big) {
			for (int k = 7; k >= 0; k--) {
				uint8 b = c & (1 << k);
				int color = b ? 0xffff : 0x0000;
				LPC_SSP0->DR = color >> 8;
				LPC_SSP0->DR = color;
			}
		} else {
			for (int k = (8 << _g.screen_big) - 1; k >= 0; k--) {
				uint8 b = c & (1 << (k >> _g.screen_big));
				int color = b ? 0xffff : 0x0000;
				LPC_SSP0->DR = color >> 8;
				LPC_SSP0->DR = color;
			}
		}
	}
	n_lcd_show++;
	if (n_lcd_show == 24 * 8 * 8)
		n_lcd_show = 0;
}

#endif

void TIMER16_0_IRQHandler(void) {
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
	psg_tick();

#ifdef VERTICAL_SCREEN
	if (lines < 192 && _g.lcd_init) { // 128 -> 20sec, 192 -> 27sec:  (128+64)/(256*4)*60 = 11.25fps
		lcd_show();
	}
#else
	if (lines < 256 && _g.lcd_init) { // 256 -> 27sec (video3 26sec), 256/(192*8)*60 = 10fps
		lcd_show();
	}
#endif
	lines++;

	if (lines >= LINES) { // LINES = 261 define at screen.h
		lines = 0;
		frames++;
		_g.vflag = 1;
	}
	_g.linecnt++;

#ifdef KEYBOARD_USB
	// for USB keyboard
	if ((_g.linecnt & 0x3) == 0x3) { // 描画に120msecかかっている、その間にキーボードの接続が切れる
		key_ping();
	} else if (_g.linecnt == 0) {
		key_tick(); // 処理間に合っていないので分割必要
	}
#endif
}

uint8 bk_screen_big;
uint8 bk_lcd_init;

void video_on() { // LCD_MODE vide_on
#ifdef VERTICAL_SCREEN
	if (_g.screen_big > 1) {
		_g.screen_big = 1;
	}
#else
	if (_g.screen_big > 2) {
		_g.screen_big = 2;
	}
#endif
	if (_g.lcd_init && _g.screen_big == bk_screen_big) {
		return;
	}

	_g.lcd_init = 0;
	SPI_Wait();

	// 1.1b4 restore clock
	if (LPC_SYSCON->SYSAHBCLKDIV != 1) {
		LPC_SYSCON->SYSAHBCLKDIV = 1;
		SystemCoreClockUpdate();
	}

	SCREEN_W = 32 >> _g.screen_big;
	SCREEN_H = 24 >> _g.screen_big;

	displaymode = 1; // LCD_MODE

	// 単独で使う場合必要！
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	NVIC_SetPriority(TIMER_16_0_IRQn, 1);

	#ifdef USE_SPI_DRIVER
	// signal SPI0
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 11) | (1 << 7); // SPI0, CT16B0(timer)
	LPC_SYSCON->PRESETCTRL |= 1 << 0; // reset SSP0

#ifdef USE_PIN_RST
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6) | (1 << 11); // VIDEO2:SDI, IN1:SCLK, IN4:RS (VIDEO1は使えない？）, IN2
#else
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (0 << 6) | (1 << 11); // VIDEO2:SDI, IN1:SCLK, IN4:RS (VIDEO1は使えない？）, IN2
#endif
	LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに

	LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
	LPC_IOCON->PIO0_9 =     0b00011000001; // MOSI0 no pullup
	LPC_IOCON->SWCLK_PIO0_10 = 0b11000010; // IN1 SCK0 no pullup (IOCON_SCK_LOC はデフォルトで0_10）
	LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> RS, PIO0_11 no pullup
#ifdef USE_PIN_RST
	LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up
#endif

	LPC_SYSCON->SSP0CLKDIV = 1; // clock divider (1-255), main clock / 1 (48MHz) 最速でもok!

	//LPC_SSP0->CR0 = 0b11000111; // Frame Format:CPHA(1) CPOL(1) CSPI(00) Data Size:  8bit(0111) アイドル時SLK1、0->1で送信
	LPC_SSP0->CR0 = 0b00000111; // Frame Format:CPHA(0) CPOL(0) CSPI(00) Data Size:  8bit(0111) アイドル時SLK0、0->1で送信
	//LPC_SSP0->CR0 = 0b10000111; // Frame Format:CPHA(1) CPOL(0) CSPI(00) Data Size:  8bit(0111) アイドル時SLK1、1->0で送信
	//LPC_SSP0->CR0 = 0b01000111; // Frame Format:CPHA(0) CPOL(1) CSPI(00) Data Size:  8bit(0111) アイドル時SLK0、1->0で送信
	// LPC_SSP0->CR0 =   0b11001111; // Frame Format:CPHA(1) CPOL(1) CSPI(00) Data Size: 16bit(1111) アイドル時SLK1、0->1で送信

	LPC_SSP0->CPSR = 2; // clock prescale (2-254)
	LPC_SSP0->CR1 = SSPCR1_SSE; // SPI Enable
	
	#else
	// signal
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7); // SPI0, CT16B0(timer)

	// use GPIO
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 10) | (1 << 6) | (1 << 11);
	LPC_GPIO0->DIR &= ~(1 << 8); // VIDEO1はIN=ハイインピーダンスに

	LPC_IOCON->PIO0_8 =        0b11000000; // VIDEO1 PIO0_8 no pullup, IN(=ハイインピーダンス?)
	LPC_IOCON->PIO0_9 =     0b00011000000; // GPIO no pullup
	LPC_IOCON->SWCLK_PIO0_10 = 0b11000001; // GPIO IN1 -> OUT
	LPC_IOCON->R_PIO0_11 =     0b11000001; // IN2 -> RS, PIO0_11 no pullup
#ifdef USE_PIN_RST
	LPC_IOCON->PIO0_6        = 0b11000000; // IN4 -> RS, PIO0_6 no pull up
#endif
	SPI_CLK(0);
	#endif

	// sync by 16bit timer0
	LPC_TMR16B0->EMR = 0;
	LPC_TMR16B0->PWMC = 0; // PIO0_8 PWMC off
	// LPC_TMR16B0->MCR = (0b001 << 3) | (0b010 << 6); // MR0 noop(MAT0), MR1 interrupt, MR2 reset
	LPC_TMR16B0->MCR = (0x000 << 0) | (0b000 << 3) | (0b011 << 6) | (0b000 << 9); // MR0 none, MR1 none, MR2 interrrupt & reset, MR3 none
	LPC_TMR16B0->PR = 0;

	//	LPC_TMR16B0->MR0 = FPORCH;
	LPC_TMR16B0->MR0 = 1; // video enable
	LPC_TMR16B0->MR2 = VSYNC;
	LPC_TMR16B0->TCR = 1;
	_g.psgratio = 1;

	
	if (!bk_lcd_init) {
		LCD_Init();
	}

	/*
	for (;;) {
		clearAll();
		//WriteComm(0xaa);
		Delay(1);
	}
	test();
	*/
	if (!bk_lcd_init || _g.screen_big != bk_screen_big) {
		clearAll(_g.screen_invert ? 0xffff : 0);
	}
	lines = 0;
	n_lcd_show = 0;
	bk_screen_big = _g.screen_big;

	_g.lcd_init = bk_lcd_init = 1;
}
void video_off(int clkdiv) {
	_g.lcd_init = 0;

	// video off
	// LPC_TMR16B0->MR0 = 0; // video disable

	// video off
	LPC_TMR16B0->TCR = 0;

	LPC_TMR16B0->EMR = 0;
	LPC_TMR16B0->PWMC = 0; // PWMC off

	LPC_IOCON->PIO0_8 = 0xd0;
	LPC_IOCON->PIO0_9 = 0xd0;
	LPC_GPIO0->DIR &= ~((1 << 8) | (1 << 9));

	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 11); // SPI0 off

	// video off and sound enhanced
	LPC_TMR16B0->MCR = (0b000 << 0) | (0b000 << 3) | (0b000 << 6) | (0b011 << 9); // MR0 off, MR1 off, MR2 off, MR3 reset & interrupt
	LPC_TMR16B0->MR1 = 0; // as counter
	LPC_TMR16B0->MR3 = VSYNC_0; // sound x8
	_g.psgratio = PSG_RATIO;

	LPC_TMR16B0->TCR = 1;

	// 1.1b4 clock down
	if (clkdiv < 1 || clkdiv > 255)
		clkdiv = 1;
	LPC_SYSCON->SYSAHBCLKDIV = clkdiv;
	SystemCoreClockUpdate();
}

INLINE int video_active() {
	return _g.lcd_init;
}

#endif
