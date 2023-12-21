#ifndef __DISPLAY_NTSC_H__
#define __DISPLAY_NTSC_H__

#define HSYNC_LINES 12

// =3055/48000000 を基準として使う
//#define VSYNC 3055	// 48MHz 48000000/(262*59.97) = 3054.96 = 3055, 3055/48*262*59.97/1000000 = 1.00001224375 ・・・59.97は間違い
// -> screen.h
/*
1/(3056/48000000) = 15706.806282722513 ?
1/(3055/48000000) = 15711.947626841244 ?
3056/2=1528, 1527　で設定して、psg切り替えしてみる
*/

// 実際の周波数 262*59.97/60 = 261.86899999999997Hz
// WAIT60の待ち時間、60*1/59.97 = 1.0005002501250626 ほぼ1秒
#ifdef XTAL_358MHz
//#define FPORCH 236	// 4.7usec - 4.7/(1/50.113630) = 235.534061
//#define FPORCH 387	// 4.7usec - 4.7/(1/(82329535/1000000)) = 386.94
#define FPORCH 370	// 4.7usec - 4.7/(1/(82329535/1000000)) = 386.94
#else
#define FPORCH 225	// 4.7usec - 4.7/(1/48) = 225.6 // 安定版
#endif

#define SCREEN_H_LINES (24 * 8) // SCREEN_H == 24 or 12固定

#define SPI_16BIT  // 232byte -> 264byte, +32byte

// 32x24
#ifdef SPI_16BIT

#ifdef XTAL_358MHz

//#define SIGNAL_OFFSET 280	// 左右の表示横位置 大きいほど右にいく 220以上だとVIDEO7で揺れる
#define SIGNAL_OFFSET 480	// 左右の表示横位置 大きいほど右にいく 220以上だとVIDEO7で揺れる // for 82MHz

#define SIGNAL_OFFSET_V3 20	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V5 270 // 150	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V7 270 // 150	// 左右の表示横位置 大きいほど右にいく
//#define SPI_OFF 2666 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.1??
//#define SPI_OFF 2740 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.4b50 VIEDO1-2
#define SPI_OFF 2740 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.4b50 VIEDO1-2 // for 82MHz
#define SPI_OFF2 2760 // 2820 SPIをOFFにするタイミング調整 XTAL off? // for big screen VIDEO3-

#else

#define SIGNAL_OFFSET 220 // 260	// 左右の表示横位置 大きいほど右にいく 220以上だとVIDEO7で揺れる
#define SIGNAL_OFFSET_V3 0 // 150	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V5 270 // 150	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V7 270 // 150	// 左右の表示横位置 大きいほど右にいく
//#define SPI_OFF 2666 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.1??
#define SPI_OFF 2680 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.4b50 VIEDO1-2
#define SPI_OFF2 2760 // 2820 SPIをOFFにするタイミング調整 XTAL off? // for big screen VIDEO3-

#endif

#else
#define SIGNAL_OFFSET 220	// 左右の表示横位置 大きいほど右にいく 220以上だとVIDEO7で揺れる
#define SIGNAL_OFFSET_V3 0 // 150	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V5 270 // 150	// 左右の表示横位置 大きいほど右にいく
#define SIGNAL_OFFSET_V7 270 // 150	// 左右の表示横位置 大きいほど右にいく
//#define SPI_OFF 2666 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.1??
#define SPI_OFF 2680 // SPIをOFFにするタイミング調整 XTAL off? // 1.2.4b50 VIEDO1-2
#define SPI_OFF2 2760 // 2820 SPIをOFFにするタイミング調整 XTAL off? // for big screen VIDEO3-
#endif

// bigscreen
//#define SIGNAL_OFFSET 0	// 左右の表示横位置 大きいほど右にいく
//#define SPI_OFF 2720 // SPIをOFFにするタイミング調整 XTAL off?

#define LINES_OFFSET 32

//extern volatile uint8 vflag; // bit

/* bench mark
10 CLT:FOR I=1 TO 10000:A=A+1:NEXT:?TICK()
1841 1.2b21 ... SPI送るのを待つ場合
1379 1.2b21 ... SPI_OFF 割り込み
797 1.2b21 ... SPIO_OFF VIDEO0
1415 1.2b22
837 1.2b22 LCD
791 1.2b22 LCD版 noLCD
2155 1.1.1
1284 1.2b23 ... 261lines
// 速度、b20:1738 b19:2105 1.1.1:2154 b19->b20:20%速度アップ 1.1->b20:24%アップ
// b21:1379 1.1->36%アップ
*/

#include "ssp.h"

static inline void sound_tick();

INLINEA void video_sendline() {
	uint16 w;
	const uint8* vram = pvram;
	const uint8* crom1;
	const uint8* crom2;
	{
		int chk = (8 << _g.screen_big) - 1;
		uint8 offsety = ((lines - LINES_OFFSET) & chk) >> _g.screen_big;
		crom1 = CHAR_PATTERN + offsety;
		crom2 = screen_pcg + offsety - ((0x100 - SIZE_PCG) * 8);
	}
	uint8 n = 0;
	uint8 cx = _g.cursorx;
	//		cptn = offsety & 1 ? 0x55 : 0xaa; // 縞模様だと、反転で文字が読めないから却下
	
	LPC_IOCON->PIO0_9 = 0b00011100001; // MOSI0 pull down
//	LPC_IOCON->PIO0_9 = 0b00011000001; // MOSI0 no pullup 暗い？
//	LPC_IOCON->PIO0_9 = 0b00011010001; // MOSI0 pullup

	// cursor blinkの同期合わせ -> 1.3.2b17 nopを入れて不要になった -> やっぱりいる
	if (_g.screen_big < 2) {
		#ifdef SPI_16BIT
		LPC_SSP0->DR = 0; // data to send SPI
		#else
		LPC_SSP0->DR = 0; // data to send SPI
		LPC_SSP0->DR = 0; // data to send SPI
		#endif
	}
	
	uint16 invert = _g.screen_invert ? 0xffff : 0; // invert or not
	uint8 cptn = 0;
	
	if (((lines - LINES_OFFSET) >> (3 + _g.screen_big)) == _g.cursory) {
//		if ((frames & 0x10) && _g.cursorflg) {
//		if ((!!(frames & 0x10)) & _g.cursorflg) {
		if ((frames >> 4) & _g.cursorflg) {
			if (!key_flg.insert) {
		//		cptn = 0xe0; // ver 1.1
				cptn = 0xf0; // ver 1.2b21 // 80 c0 e0 f0 f8 fc fe ff
		//		cptn = offsety & 1 ? 0x40 : 0xa0;
			} else {
				cptn = 0xff;
			}
		} else {
			__asm volatile("nop");
			__asm volatile("nop");
			__asm volatile("nop");
//			__asm volatile("nop");
//			__asm volatile("nop");
//			__asm volatile("nop");
		}
	} else {
//		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
		__asm volatile("nop");
	}
	/*
	if (
		(((lines - LINES_OFFSET) >> 3) != _g.cursory) ||
		(!((frames & 0x10) && _g.cursorflg))
	) {
		cptn = 0;
	} else {
		if (key_flg.insert) {
	//		cptn = 0xe0; // ver 1.1
			cptn = 0xf0; // ver 1.2b21 // 80 c0 e0 f0 f8 fc fe ff
	//		cptn = offsety & 1 ? 0x40 : 0xa0;
		} else {
			cptn = 0xff;
		}
	}
	*/
	
	
	//while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
	// dummy send // 16bit 左端の余計なものがでないように対策
//	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full -> 1.3.2b17 nopで不要に

	if (_g.screen_big < 2) { // 左端のクロを出すため, 2以上は乱れる
		#ifdef SPI_16BIT
		LPC_SSP0->DR = 0;
		#else
		LPC_SSP0->DR = 0;
		LPC_SSP0->DR = 0;
		#endif
	}
	
	/*
	if (_g.screen_big == 3) {
		// 偶数幅のみ 16bit SPI
		// カーソル形状に難あり
		int np = 0x80;
		for (;;) {
			uint8 c;
			if (*vram >= 0x100 - SIZE_PCG) {
				c = crom2[*vram << 3];
			} else {
				c = crom1[*vram << 3];
			}
			w = c & np ? 0xff : 0;
			np >>= 1;
			if (n == cx) {
				w ^= cptn;
			}
			
			uint8 w2 = c & np ? 0xff : 0;
			np >>= 1;
			if (n == cx) {
				w2 ^= cptn;
			}
			w = (w << 8) | w2;
			while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
			LPC_SSP0->DR = w ^ invert; // 1.2b9

			if (np == 0) {
				np = 0x80;
				n++;
				vram++;
				if (n == SCREEN_W)
					break;
			}
		}
	} else {
		*/

		#ifdef SPI_16BIT // +16byte
		// 偶数幅のみ 16bit SPI
		for (;;) {
			if (*vram >= 0x100 - SIZE_PCG) {
				w = crom2[*vram << 3];
			} else {
				w = crom1[*vram << 3];
			}
			vram++;
			if (n == cx) {
				w ^= cptn;
			}
	//		w ^= (n == cx) * cptn; // サイズ増える、ブレ変わらない video3
			n++;
			w <<= 8;
			
			if (*vram >= 0x100 - SIZE_PCG) {
				w |= crom2[*vram << 3];
			} else {
				w |= crom1[*vram << 3];
			}
			vram++;
			if (n == cx) {
				w ^= cptn;
			}
	//		w ^= (n == cx) * cptn;
			n++;
			while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
			LPC_SSP0->DR = w ^ invert; // 1.2b9
	//		while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
			if (n == SCREEN_W)
				break;
		}
		#else
		// 8bit // -16byte
		for (;;) {
			if (*vram >= 0x100 - SIZE_PCG) {
				w = crom2[*vram << 3];
			} else {
				w = crom1[*vram << 3];
			}
			vram++;
			if (n == cx) {
				w ^= cptn;
			}
	//		w ^= (n == cx) * cptn;
			n++;
//			while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
			LPC_SSP0->DR = w ^ invert; // 1.2b9
			while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full
			if (n == SCREEN_W)
				break;
		}
		#endif

//	}
//	while (!(LPC_SSP0->SR & SSPSR_TNF)); // wait until not full

	if (_g.screen_big < 2) { // 2以上は乱れる
		#ifdef SPI_16BIT
		LPC_SSP0->DR = 0; // ためしにつける // 1.3.2b13 復活、画面右端の白い線を消すため
		#else
		LPC_SSP0->DR = 0; // ためしにつける // 1.3.2b13 復活、画面右端の白い線を消すため
//		LPC_SSP0->DR = 0; // ためしにつける // 1.3.2b13 復活、画面右端の白い線を消すため // いらないかも 1.4b10
		//LPC_SSP0->DR = 0; // ためしにつける // 1.3.2b13 復活、画面右端の白い線を消すため　// あるとだめ
		#endif
	}

//	sound_tick(); // 音が濁る

	// SPI送信を待つと遅い
	//while (!(LPC_SSP0->SR & SSPSR_TFE)); // wait until empty send buffer // これがないと揺れる ... なくてもいける？ これがあると遅い
	//LPC_IOCON->PIO0_9 = 0b00011001000; // POI0_9 pulldown // ver 1.2b19 上を省略できる？ ... これがないと暗い
}

//void ps2_process();

/*
extern uint8 ps2_data;
extern char ps2_nbit;
extern char ps2_cnt; // PS2信号の1の数を数える
*/

//static inline void ntsc_TIMER16_0_IRQHandler(void) {
DISPLAY_INLINE void ntsc_TIMER16_0_IRQHandler(void) {
	uint ir = LPC_TMR16B0->IR;
	LPC_TMR16B0->IR = ir; // 必須
	if (ir == 0b1000) { // MR3
		if (LPC_TMR16B0->PWMC) {
			LPC_IOCON->PIO0_9 = 0b00011001000; // POI0_9 pulldown // ver 1.2b19 上を省略できる？ ... これがないと暗い
			return;
		} else {
			LPC_TMR16B0->MR1++;
			if (LPC_TMR16B0->MR1 < PSG_RATIO) {
				sound_tick();
				psg_tick();
				return;
			}
			LPC_TMR16B0->MR1 = 0;
		}
	}
	sound_tick();
	/*
	if (ir == 0b0100) { // MR2 for VIDEO0
		psg_tick();
		_g.linecnt++;
		lines++;
		if (lines >= LINES) {
			lines = 0;
			frames++;
			_g.vflag = 1;
		}
		return;
	}
	*/
	
	lines++;
	if (lines < LINES_OFFSET) {
	} else if (lines < LINES_OFFSET + SCREEN_H_LINES) {
		if (LPC_TMR16B0->PWMC) {
			video_sendline();
		}
		int chk = (8 << _g.screen_big) - 1; // big:15, normal:7
		if (((lines - LINES_OFFSET) & chk) == chk) {
			pvram += SCREEN_W;
		}
	} else if (lines == LINES_OFFSET + SCREEN_H_LINES) {
		frames++;
		pvram = vram;
		_g.vflag = 1;
	} else if (lines == LINES - HSYNC_LINES) {
		LPC_TMR16B0->MR0 = VSYNC - FPORCH / 2; // 安定版
#if 0 // #ifdef XTAL_358MHz
	} else if (lines >= LINES + (frames & 1)) { // 交互にやって 525本実現版
#else
	} else if (lines >= LINES) {
#endif
		lines = 0;
		LPC_TMR16B0->MR0 = FPORCH;
	}

	_g.linecnt++;

#if 0
//	LPC_GPIO0->MASKED_ACCESS[1 << 2] = _g.psg_sounder; // 1.2b34まで
	if (lines >= LINES - 1 - HSYNC_LINES) {
		LPC_TMR16B0->MR0 = VSYNC - FPORCH / 2; // 安定版
		//LPC_TMR16B0->MR3 = FPORCH + VSYNC / 2; // for sound
	} else {
		LPC_TMR16B0->MR0 = FPORCH;
		//LPC_TMR16B0->MR3 = SPI_OFF; // 1.3.2b16
	}
	
	lines++;
//	if (lines >= LINES - 1) { // 261回？？
	if (lines >= LINES) { // 1.2b22 ... 261じゃないとずれるテレビあり
		lines = 0;
		//		cursorblink = _g.cursorflg && (frames & 0x10);
/*		
	} else if (lines == LINES_OFFSET - 1) {
		#define PS2KB_CLK 7 // ノイズ防止になるけど、キーリピートが効かなくなる
		LPC_SYSCON->STARTERP0 &= ~(1 << PS2KB_CLK); // disable
		LPC_GPIO0->DIR |= 1 << PS2KB_CLK; // キーボード通信禁止
		LPC_GPIO0->MASKED_ACCESS[1 << PS2KB_CLK] = 0;
		*/
	} else if (lines >= LINES_OFFSET && lines < LINES_OFFSET + SCREEN_H_LINES) {
		//		if (videoenable)
		if (LPC_TMR16B0->PWMC) {
			video_sendline();
		}
		int chk = (8 << _g.screen_big) - 1; // big:15, normal:7
		if (((lines - LINES_OFFSET) & chk) == chk) {
			pvram += SCREEN_W;
		}
	} else if (lines == LINES_OFFSET + SCREEN_H_LINES) {
		frames++;
		pvram = vram;
		_g.vflag = 1;
/*
		LPC_GPIO0->DIR &= ~(1 << PS2KB_CLK); // キーボード通信許可 // 1.2b10
		ps2_data = ps2_nbit = ps2_cnt = 0;
		LPC_SYSCON->STARTERP0 |= 1 << PS2KB_CLK; // enable
		*/
	}
	_g.linecnt++;
#endif
	//
	psg_tick();
	
#ifdef KEYBOARD_USB
	// for USB keyboard
	if ((linecnt & 0x1f) == 0x1f) {
		//		key_ping();
		key_tick(); // 処理間に合っていないので分割必要
	} else if (linecnt == 0 || linecnt == 10 || linecnt == 20 || linecnt == 30) {
//		key_tick(); // 処理間に合っていないので分割必要
	}
#endif
}

void ntsc_video_on() { // TV-mode
	// 0(1-2):32x24, 1(3-4):16x12, 2(5-6):8x6, 3(7-8):4x3
//	_g.screen_big = 3;
	
	SCREEN_W = 32 >> _g.screen_big; // 1.2big
	SCREEN_H = 24 >> _g.screen_big;

//	videoenable = 1; //  == LPC_TMR16B0->PWMC
	pvram = vram;
	
	LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 7) | (1 << 11); // CT16B0 SPI0
	LPC_GPIO0->DIR |= (1 << 9) | (1 << 8);
	
	// sync by 16bit timer0
	LPC_IOCON->PIO0_8 = 0b00011000010; // TIMER0 no pullup CT16B0_MAT0
//	LPC_IOCON->PIO0_8 = 0b00011001010; // TIMER0 pull down CT16B0_MAT0 変化なし
//	LPC_IOCON->PIO0_8 = 0b00011010010; // TIMER0 pull up CT16B0_MAT0 変化なし

	LPC_TMR16B0->TCR = 0;

	LPC_TMR16B0->EMR = (1 << 0) | (0b01 << 4); // enable MAT0, zero if hit
	LPC_TMR16B0->PWMC = 0b0001; // PWM Enable MAT0
	LPC_TMR16B0->MCR = (0b000 << 0) | (0b001 << 3) | (0b010 << 6) | (0b001 << 9); // MR0 noop(MAT0), MR1 interrupt, MR2 reset, MR3 interrupt
    //LPC_TMR16B0->MCR = (0b000 << 0) | (0b001 << 3) | (0b010 << 6) | (0b000 << 9); // MR0 noop(MAT0), MR1 interrupt, MR2 reset, MR3 OFF
    LPC_TMR16B0->PR = 0;
	
	NVIC_EnableIRQ(TIMER_16_0_IRQn);
	NVIC_SetPriority(TIMER_16_0_IRQn, 1);
	
	LPC_TMR16B0->MR0 = FPORCH;
	LPC_TMR16B0->MR1 = SIGNAL_OFFSET;
	LPC_TMR16B0->MR2 = VSYNC;
	LPC_TMR16B0->MR3 = SPI_OFF;
	_g.psgratio = 1;

	//LPC_TMR16B0->MR3 = SIGNAL_OFFSET + VSYNC / 2; // for sound
	// _g.screen_big 0:video1,2, 1:video3,4: 2:video5,6, 3:video7,8
	if (_g.screen_big > 0) {
		LPC_TMR16B0->MR3 = SPI_OFF2;
		if (_g.screen_big == 1) {
			LPC_TMR16B0->MR1 = SIGNAL_OFFSET_V3; //10; //8; //#define SIGNAL_OFFSET 0	// 左右の表示横位置 大きいほど右にいく 8未満だとVIDEO3でドットがぶれる、要調整
		}
		if (_g.screen_big == 2) {
			LPC_TMR16B0->MR1 = SIGNAL_OFFSET_V5; //10; //8; //#define SIGNAL_OFFSET 0	// 左右の表示横位置 大きいほど右にいく 8未満だとVIDEO3でドットがぶれる、要調整
		}
		if (_g.screen_big == 2) {
			LPC_TMR16B0->MR1 = SIGNAL_OFFSET_V7; //10; //8; //#define SIGNAL_OFFSET 0	// 左右の表示横位置 大きいほど右にいく 8未満だとVIDEO3でドットがぶれる、要調整
		}
	}
	
	LPC_TMR16B0->TCR = 1;
	
	// signal SPI0
	
//	LPC_IOCON->PIO0_9 = 0b00011000000; // POI0_9 no pullup
//	LPC_IOCON->PIO0_9 = 0b00011000001; // MOSI0 no pullup
//	LPC_IOCON->PIO0_9 = 0b00011001000; // POI0_9 pulldown // ver 1.2b19 上を省略できる？
//	LPC_GPIO0->MASKED_ACCESS[1 << 9] = 0;
	
	LPC_SYSCON->PRESETCTRL |= 1 << 0; // reset SSP0
	LPC_SYSCON->SSP0CLKDIV = 1; // clock divider (1-255), main clock / 1 (48MHz)
	
	#ifdef SPI_16BIT
	LPC_SSP0->CR0 = 0b011111; // Frame Format:TI Data Size: 16bit
	#else
	LPC_SSP0->CR0 = 0b010111; // Frame Format:TI Data Size: 8bit
	#endif

	// 偶数のみ？
	
	//	LPC_SSP0->CPSR = 9; // clock prescale (2-254) 表示数減る
	LPC_SSP0->CPSR = 8 << _g.screen_big; // clock prescale (2-254) default  --- ちょうどいい 6MHz
//	if (_g.screen_big == 3) // 容量不足で失敗
//		LPC_SSP0->CPSR = 8;
	//	LPC_SSP0->CPSR = 7; // clock prescale (2-254)
//	LPC_SSP0->CPSR = 6; // clock prescale (2-254) 横にたくさん表示できる
//	LPC_SSP0->CPSR = 5; // clock prescale (2-254) 5以下だと描画処理が間に合わない
	LPC_SSP0->CR1 = SSPCR1_SSE; // SPI Enable
	
#ifndef SKIP_SLOWCLOCK
	// 1.1b4 restore clock
	if (LPC_SYSCON->SYSAHBCLKDIV != 1) {
		LPC_SYSCON->SYSAHBCLKDIV = 1;
		SystemCoreClockUpdate();
	}
#endif
}
INLINEA void ntsc_video_off(int clkdiv) {
//	videoenable = 0;
	
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

#ifndef SKIP_SLOWCLOCK
	// 1.1b4 clock down
	if (clkdiv < 1 || clkdiv > 255)
		clkdiv = 1;
	LPC_SYSCON->SYSAHBCLKDIV = clkdiv;
	SystemCoreClockUpdate();
#endif
}
inline int ntsc_video_active() {
	return LPC_TMR16B0->PWMC != 0;
}

#endif  // __DISPLAY_NTSC_H__
