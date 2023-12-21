//#define LINEBUF_SIZE 38
//#define LINEBUF_OFFSET 2
#define LINEBUF_SIZE 36
#define LINEBUF_OFFSET 1
//#define LINEBUF_SIZE 32
//#define LINEBUF_OFFSET 0
char linebuf1[LINEBUF_SIZE];
char linebuf2[LINEBUF_SIZE];
char* linebuf;
volatile uint8 vsync_flg = 0;

void video_on() {
	SCREEN_W = 32;
	SCREEN_H = 24;
	
	// TIM1 PWM CH4(PA11)
	// 15.7390kHz - 15734Hz
	GPIOA->MODER |= GPIO_MODER_AF << (2 * 11); // PA11 as alternate
//	GPIOA->OTYPERR |= 1 << 11; // 1:open drain or 0:not
//	GPIOA->OSPEEDR |= 3 << (2 * 11); // PA11 high speed
//	GPIOA->PUPDR |= 2 << (2 * 11); // PA11 pull down
//	GPIOA->PUPDR |= 1 << (2 * 11); // PA11 pull up
	GPIOA->AFRH |= 2 << (4 * (11 - 8)); // PA11 - AF2 (TIM1_CH4)
	TIM1->BDTR |= 1 << 15; // TIM1の場合、必要！
	TIM1->CR1 |= (1 << 7); // reset preload
	TIM1->CCER |= 0xb0001 << (4 * (4 - 1)); // ch4 output enable
	TIM1->CCMR2 = 0b01111100 << 8; // ch4, PWM2 default 1 111, preload enable 1, fast enable 1, output 00
//	TIM1->CCMR2 = 0b01101100 << 8; // ch4, PWM1 default 0 110, preload enable 1, fast enable 1, output 00
	TIM1->PSC = 0; // 48MHz / 1
	TIM1->ARR = 3050 - 1; // 周期 -16 から +19まで同期が合った、7インチモニター
	TIM1->DIER |= 1 << 3; // CH3 interrupt
//	TIM1->CCR3 = 440 - 1; // interrupt for LINEBUF_SIZE=38
	TIM1->CCR3 = 500 - 1; // interrupt for LINEBUF_SIZE=38 表示位置
//	TIM1->CCR4 = 225 - 1; // CH4 0: 225 or 3050
	TIM1->CCR4 = 3050 - 225 - 1; // 垂直同期からスタート
	TIM1->CR1 = 1; // enable count
	TIM1->EGR |= 1; // reset counter
	NVIC_EnableIRQ(TIM1_CC_IRQn);
	NVIC_SetPriority(TIM1_CC_IRQn, 1); // 0が最高、デフォルト、Supports 0 to 192 priority levels.
	
	/*
	// TIM3 PWM CH4(PB1)
	// 15.7990kHz - 15734Hz
	GPIOB->MODER |= 2 << (2 * 1); // PB1 as alternate
	GPIOB->AFRL |= (1 << (4 * 1)); // PB1 - AF1 (TIM3_CH4)
	TIM3->CCER |= 0xb0001 << (4 * (4 - 1)); // ch4 output enable
	TIM3->CCMR2 = 0b1110000 << 8; // ch4, PWM2 default 1
	TIM3->PSC = 0; // 48MHz / 1 = 48MHz? 24MHz?
	TIM3->ARR = 3050 - 1; // 周期
	TIM3->DIER |= 1 << 3; // CH3 interrupt
	TIM3->CCR3 = 620 - 1; // interrupt
	TIM3->CCR4 = 225 - 1; // CH4 0: 225 or 3050 - 225 - 垂直同期用
	TIM3->CR1 = 1; // enable count
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 2);
	*/
	
	// DMA
	// PB5 - AF0(MOSI)
//	GPIOB->MODER = (GPIOB->MODER & ~(3 << 2 * 5)) | (2 << (2 * 5)); // PA5 as alternate
//	GPIOB->PUPDR |= GPIO_PUPDR_PDOWN << (2 * 5); // PB5
	GPIOB->MODER |= GPIO_MODER_AF << (2 * 5); // PB5 as alternate
	GPIOB->AFRL |= (0 << (4 * 5)); // PB5 - AF0 (MOSI)
	
	// DMA_CH3 - MOSI
	DMA->CCR3 |= (0b00 << 8) | (1 << 7) | (1 << 4); // peripheral8bit, MINC(mem increment), DIR:1(from mem)
	DMA->CPAR3 = (uint32)&SPI1->DR;
	
	// SPI1
//	SPI1->CR1 = (1 << 2) | (7 << 3); // master, baud rate 0b111(f/(2<<7)) (defalut:MSB) - 1875kHz
	SPI1->CR1 = (1 << 2) | (2 << 3); // master, baud rate 0b111(f/(2<<2)) (defalut:MSB) - 6MHz
//	SPI1->CR1 = (1 << 2) | (3 << 3); // master, baud rate 0b111(f/(2<<3)) (defalut:MSB) - 3MHz
//	SPI1->CR1 = (1 << 2) | (0 << 3); // master, baud rate 0b111(f/(2<<0)) (defalut:MSB) - 24MHz
//	SPI1->CR2 = ((16 - 1) << 8) | (1 << 2) | (1 << 1); // 16bit(4-16bit) (default:SPI motorola), SSOE, TXDMAEN // 8bitとくらべて速くないし、順番変わって面倒
	SPI1->CR2 = ((8 - 1) << 8) | (1 << 2) | (1 << 1); // 8bit(4-16bit) (default:SPI motorola), SSOE, TXDMAEN
}
//uint8* pvram;
void TIM1_CC_IRQHandler(void) {
	TIM1->SR &= ~(1 << 3);
	
	// sound
	sound_tick();
	
	// display
	
	/*
	20 line : 0 - 19 - 垂直同期
	25 line : 20 - 44 blank
	1 line : 45 (=LINES_OFFSET) blank & prepare first line
	192 line : 46 - 237 vram
	23 line : 238 - 261 blank
	1 line : 262 (=LINES_FRAME-1)lines = 0: blank, vsyncflag
	--
	263 line
	*/
#define LINES_OFFSET 44
#define LINES_FRAME 262
	
	if (lines == 0) {
		pvram = vram;
	} else if (lines < 19) {
	} else if (lines <= LINES_OFFSET - 1) {
		TIM1->CCR4 = 225 - 1;
	} else if (lines <= LINES_OFFSET + 192) {
		if (lines > LINES_OFFSET) {
			// linebuf[28] &= 0x7f; // なぞ、28byte目のこのbitがonだとNG
			// なぜかSPIがhighに張り付く現象がとれず、DMAを待って無効化する方式に変更
			// DMAを使わないとデータ転送が間に合わない
			
			GPIOB->MODER = (GPIOB->MODER & ~(3 << 2 * 5)) | (GPIO_MODER_AF << (2 * 5)); // PB5
			DMA->CNDTR3 = LINEBUF_SIZE; // max:65535
			DMA->CMAR3 = (uint32)linebuf;
			DMA->CCR3 |= (1 << 0); // enable
			SPI1->CR1 |= 1 << 6; // SPI1 enable
		}
		if (lines < LINES_OFFSET + 192) { // 次の行のデータを作る
			linebuf = lines & 1 ? linebuf1 : linebuf2;
		
			int chk = (8 << _g.screen_big) - 1; // big:15, normal:7
			
			uint8 offsety = ((lines - LINES_OFFSET) & chk) >> _g.screen_big;
			const uint8* crom1 = CHAR_PATTERN + offsety;
			uint8* crom2 = screen_pcg + offsety - ((0x100 - SIZE_PCG) * 8);
			
			uint8 cx = _g.cursorx;
			uint16 invert = _g.screen_invert ? 0xffff : 0; // invert or not
			uint8 cptn = 0;
			
			if (
				(((lines - LINES_OFFSET) >> (3 + _g.screen_big)) == _g.cursory) &&
				((frames & 0x10) && _g.cursorflg)
			) {
				if (!key_flg.insert) {
					cptn = 0xf0; // ver 1.2b21 // 80 c0 e0 f0 f8 fc fe ff
				} else {
					cptn = 0xff;
				}
			}
			
			for (int i = 0; i < 32; i++) {
				int n = pvram[i];
				if (n >= 0x100 - SIZE_PCG) {
					n = crom2[n << 3];
				} else {
					n = crom1[n << 3];
				}
				if (i == cx) {
					n ^= cptn;
				}
				linebuf[i + LINEBUF_OFFSET] = (uint8)n ^ invert;
			}
			
			if (((lines - LINES_OFFSET) & chk) == chk) {
				pvram += SCREEN_W;
			}
		}
		if (lines > LINES_OFFSET) {
			while ((SPI1->SR >> 11) & 3); // FTLVL[1:0] = 00(送信するデータがなくなる)まで待ちます。
			while ((SPI1->SR >> 7) & 1); // BSY = 0 (最後のデータフレームが処理される)まで待ちます。
			SPI1->CR1 &= ~(1 << 6); // SPI1 desable
			DMA->CCR3 &= ~(1 << 0); // disable
			GPIOB->MODER = (GPIOB->MODER & ~(3 << 2 * 5)) | (GPIO_MODER_OUT << (2 * 5)); // PB5
		}
	} else if (lines < LINES_FRAME - 1) {
	} else { // lines == LINES_FRAME
		TIM1->CCR4 = 3050 - 225 - 1; // 0 - 垂直同期用
		lines = 0;
		frames++;
		vsync_flg = 1;
		return;
	}
	lines++;

	psg_tick();
}

inline void video_off(int clkdiv) {
}
inline int video_active() {
	return 1;
}

inline void IJB_lcd(int mode) {
}

inline void video_waitSync() {
	vsync_flg = 0;
	while (!vsync_flg);
}
