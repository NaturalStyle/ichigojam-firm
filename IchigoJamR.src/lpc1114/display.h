#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "../screen.h"

#define DISPLAY_MODE_NTSC 0
#define DISPLAY_MODE_LCD 1

//uint8 displaymode; // = DISPLAY_MODE_NTSC; // グローバル変数にしても容量増加
#define displaymode _g.display_mode

#if defined(LCD_MODE) // LCD_MODE LCD使用時 IchigoJam pocket
#include "display_lcd1_aqm1248a.h"

#elif defined(LCD_MODE2)
//#include "display_lcd2_ili9328.h"
#include "display_lcd2_ili9341.h"

/*inline*/ void video_waitSync(uint n) { // 1.3.2b12 add param // -12byte
	for (; n > 0; n--) {
		while (!_g.vflag);
		_g.vflag = 0;
	}
}
inline void IJB_lcd(uint mode) { // 1.2b32
}

#elif defined(LCD_MODE3)

#define DISPLAY_INLINE static inline

#define SKIP_SLOWCLOCK 1

#include "display_lcd3_st7567.h"

#if TV_MODE == TV_MODE_NTSC
#include "display_ntsc.h"
#elif TV_MODE == TV_MODE_PAL
#include "display_pal.h"
#endif

void video_on(void);
void video_off(int clkdiv);
INLINE int video_active();

//volatile uint8 vflag; // bit

/*inline*/ void video_waitSync(uint n) { // 1.3.2b12 add param // -12byte
	for (; n > 0; n--) {
		while (!_g.vflag);
		_g.vflag = 0;
	}
}

static inline void sound_tick();

__attribute__((section(".text.timer16_0"))) void TIMER16_0_IRQHandler(void) {
	if (displaymode) {
		lcd3_TIMER16_0_IRQHandler();
	} else {
		ntsc_TIMER16_0_IRQHandler();
	}
}
void video_on(void) {
	frames = lines = _g.vflag = 0;
	if (displaymode) {
		lcd3_video_on();
	} else {
		ntsc_video_on();
	}
	// 1.1b4 restore clock // 1.4.0b03 集約
	if (LPC_SYSCON->SYSAHBCLKDIV != 1) {
		LPC_SYSCON->SYSAHBCLKDIV = 1;
		SystemCoreClockUpdate();
	}
}
void video_off(int clkdiv) {
	if (displaymode) {
		lcd3_video_off(clkdiv);
	} else {
		ntsc_video_off(clkdiv);
	}
	// 1.1b4 clock down // 1.4.0b03 集約 -12byte
	if (clkdiv < 1 || clkdiv > 255)
		clkdiv = 1;
	LPC_SYSCON->SYSAHBCLKDIV = clkdiv;
	SystemCoreClockUpdate();
}
INLINE int video_active() {
	if (displaymode) {
		return lcd3_video_active();
	} else {
		return ntsc_video_active();
	}
}

void io_init();

inline void IJB_lcd(uint mode) { // 1.2b32
	video_off(1);
	displaymode = mode;
	screen_clear();
	io_init();
	video_on();
//	put_str(IJB_TITLE); // 1.2b61
}

#else // 通常のNTSCテレビ出力

//uint8 displaymode = DISPLAY_MODE_NTSC;

//volatile uint8 vflag; // bit

inline void video_waitSync() {
	while (!_g.vflag);
	_g.vflag = 0;
}

#define ntsc_video_off video_off
#define ntsc_video_on video_on
#define ntsc_video_active video_active
#define ntsc_TIMER16_0_IRQHandler TIMER16_0_IRQHandler

#define DISPLAY_INLINE

#include "display_ntsc.h"

inline void IJB_lcd(int mode) {
}

#endif

#endif  // #ifndef DISPLAY_H
