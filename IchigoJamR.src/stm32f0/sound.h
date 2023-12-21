// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

// PSG_TRUE_TONE on ... beepもTIMで鳴らす
	// BEEPは172まで
// PSG_TONE_TONE off .. なんかノイジー

#ifdef PSG_TRUE_TONE


inline void sound_init() {
	// PB7 AF2 TIM17
	GPIOB->AFRL |= (2 << (4 * 7)); // PB7 - AF2 (TIM17_CH1N)
	
	// PB7 setting for TIM17_CH1N
	GPIOB->MODER = (GPIOB->MODER & ~(3 << (2 * 7))) | GPIO_MODER_AF << (2 * 7); // PB7 as alternate
	GPIOB->PUPDR |= GPIO_PUPDR_PDOWN << (2 * 7);
	
	// TIM17 PWM CH1(PB7)
	TIM17->CCER = 1 << 2; // ch1n enable
	TIM17->BDTR |= 1 << 15; // TIM17の場合、必要！
	TIM17->CCMR1 = (0b111 << 4); // ch1, OCxM ビットに“110”(PWM モード 1)や“111”(PWM モード 2)
	TIM17->PSC = 16 - 1; // 48MHz / 16
//	TIM17->ARR = 5 - 1; // 10kHz / 5 = 2kHz
//	TIM17->ARR = 50 - 1; // 10kHz / 50 = 200Hz
//	TIM17->CCR1 = TIM17->ARR / 2; // duty 50%
	TIM17->CR1 |= 1 << 7; // ARPE(ARR,CC4がバッファされる)
	
	/*
	int t = 6819; // 440Hz
	// int t = TONE_O0[calcMod(t, 12)] >> calcDiv(t, 12);
	TIM17->ARR = t - 1;
	TIM17->CCR1 = t / 2; // duty 50%
	TIM17->CR1 |= 1; // enable
	*/
}
void sound_switch(int on) {
	if (on) {
		GPIOB->MODER = (GPIOB->MODER & ~(3 << (2 * 7))) | GPIO_MODER_AF << (2 * 7); // PB7 as alternate
//		TIM17->CCMR1 |= ~(0b111 << 4); // ch1, OCxM PWMモード2
		TIM17->CR1 |= 1; // enable
	} else {
//		TIM17->CCMR1 &= ~(0b011 << 4); // ch1, OCxM 100 強制ロー // うまくいかず
		TIM17->CR1 &= ~1; // disable
		GPIOB->MODER &= (GPIOB->MODER & ~(3 << (2 * 7))) | GPIO_MODER_OUT << (2 * 7); // PB7 as output
		GPIOB->ODR &= ~(1 << 7);
	}
}
static inline void sound_tick() {
}
static inline void sound_tone(int t) {
	if (t) {
//		TIM17->CCMR1 |= ~(0b111 << 4); // ch1, OCxM PWMモード2
		TIM17->ARR = t - 1;
		TIM17->CCR1 = t / 2; // duty 50%
		TIM17->CR1 |= 1; // enable
	} else {
		TIM17->CR1 &= ~1; // disable
	}
}

#else

inline void sound_init() {
	GPIOB->MODER |= GPIO_MODER_OUT << (2 * 7);
	GPIOB->PUPDR |= GPIO_PUPDR_PDOWN << (2 * 7);
}
void sound_switch(int on) {
	if (on) {
		GPIOB->MODER |= GPIO_MODER_OUT << (2 * 7);
	} else {
		GPIOB->ODR &= ~(1 << 7);
		GPIOB->MODER &= ~(3 << (2 * 7));
		_g.psg_sounder = 0; // 1.2b53
	}
}
static inline void sound_tick() {
	GPIOB->ODR = (GPIOB->ODR & ~(1 << 7)) + (_g.psg_sounder << 7);
}


#endif
