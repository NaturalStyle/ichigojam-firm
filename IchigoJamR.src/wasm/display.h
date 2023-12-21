#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "../screen.h"

void video_on() {
//	SCREEN_W = 32;
//	SCREEN_H = 24;
	SCREEN_W = 32 >> _g.screen_big; // 1.2big
	SCREEN_H = 24 >> _g.screen_big;
}
inline void video_off(int clkdiv) {
}
inline int video_active() {
	return 0;
}

INLINE void IJB_lcd(uint mode) {
}

inline void video_waitSync() {
}

// __DISPLAY_H__
#endif