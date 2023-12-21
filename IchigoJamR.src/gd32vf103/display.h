void video_on() {
	SCREEN_W = 32;
	SCREEN_H = 24;
	ntsc_init();
	g_ntsc_on = 1;
}

inline void video_off(int clkdiv) {
	g_ntsc_on = 0;
}

inline int video_active() {
	return g_ntsc_on;
}

inline void IJB_lcd(uint mode) {
}

/// wait for `n` lines
inline void video_waitSync(uint n) {
	for (; n > 0; n--) {
		while (!_g.vflag);
		_g.vflag = 0;
	}
}
