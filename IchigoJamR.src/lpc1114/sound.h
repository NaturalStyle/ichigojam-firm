#ifndef __SOUND_H__
#define __SOUND_H__

// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

#include "../vars.h"
#include "LPC11xx.h"

inline void sound_init();
void sound_switch(int on);
static inline void sound_tick();

void sound_switch(int on) {
	if (on) {
		LPC_GPIO0->DIR |= 1 << 2;
	} else {
		LPC_GPIO0->MASKED_ACCESS[1 << 2] = 0;
		LPC_GPIO0->DIR &= ~(1 << 2);
		_g.psg_sounder = 0; // 1.2b53
	}
}

inline void sound_init() {
	// sound
	LPC_GPIO0->DIR &= ~(1 << 2); // 1.3b2
//	LPC_IOCON->PIO0_2 = 0b11010000; // 0xd0 pullup
	LPC_IOCON->PIO0_2 = 0b11001000; // 0xc8 pulldown
}


static inline void sound_tick() {
	LPC_GPIO0->MASKED_ACCESS[1 << 2] = _g.psg_sounder; // 1.2b35

	/*
	// from psg_tick -> NG
	if (_g.psgtone) {
		_g.psgwaitcnt++;
		if (_g.psgwaitcnt == _g.psgtone) {
			_g.psg_sounder = ~_g.psg_sounder;
			_g.psgwaitcnt = 0;
		} else {
			__asm("nop");
			__asm("nop");
		}
	} else {
		__asm("nop");
		__asm("nop");
		__asm("nop");
		__asm("nop");
		__asm("nop");
	}
	*/
	
}

#endif	// __SOUND_H__
