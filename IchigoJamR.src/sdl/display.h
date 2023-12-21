volatile uint8 vflag; // bit

inline void video_waitSync(uint n) {
	for (int i = 0; i < n; i++) {
		while (!vflag) {
			SDL_Delay(2); // msec
		}
		vflag = 0;
	}
}


#define SCRW 320
#define SCRH 240
#define SW 320
#define SH 240

extern int led;

int videoactive = 1;


// 負荷18% -> VIDEO2でも負荷あまり変わらず、安定
//uint32 sdlvram[32 * 8 * 24 * 8];
uint8 sdlvram[32 * 8 * 24 * 8];
void display(void) {
	SDL_SetRenderDrawBlendMode(sdlg, SDL_BLENDMODE_NONE);
//	SDL_SetRenderTarget(sdlg, sdlbuf);
	SDL_SetRenderDrawColor(sdlg, 0, 0, 0, 0);
	SDL_RenderClear(sdlg);
	
	SDL_SetRenderDrawColor(sdlg, 255, 255, 255, 0);
	if (videoactive) {
		const uint8* crom = CHAR_PATTERN;
		const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
		uint8 cy = _g.cursory;
		
		uint8 cptn = 0xff;
		if (!key_flg.insert) {
			cptn = 0xe0;
		}
		uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not
//		uint32* psdlvram = sdlvram;
		uint8* psdlvram = sdlvram;
		for (int i = 0; i < SCREEN_H * 8; i++) {
			uint8 cx = _g.cursorx;
			if ((i >> 3) != cy) {
				cx = 255;
			}
			if (!((frames & 0x10) && _g.cursorflg)) {
				cx = 255;
			}
			for (int j = 0; j < SCREEN_W * 8; j++) {
				unsigned char c = vram[(j >> 3) + SCREEN_W * (i >> 3)];
				const unsigned char* rom = crom;
				if (c >= 0x100 - SIZE_PCG) {
					rom = crom2;
				}
				unsigned char p = rom[c * 8 + (i & 7)] ^ invert;
				if ((j >> 3) == cx) {
					p ^= cptn;
				}
				int white = (p & (1 << (7 - (j & 7)))) != 0;
//				*psdlvram = white ? 0xffffffff : 0;
				*psdlvram = white ? 0xff : 0;
				psdlvram++;
			}
		}
	}
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(sdlvram, SCREEN_W * 8, SCREEN_H * 8, 8, SCREEN_W * 8, 0x7 << 5, 0x7 << 2, 0x3, 0);
//	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(sdlvram, 32 * 8, 24 * 8, 32, 32 * 8 * 4, 0xff << 24, 0xff << 16, 0xff << 8, 0);
	SDL_Texture* sdlbuf = SDL_CreateTextureFromSurface(sdlg, surface);
	
	vflag = 1;
	
	int scrw, scrh;
	SDL_GL_GetDrawableSize(sdlw, &scrw, &scrh);
	int bratio = scrh / (SCR_H * 8);
	int bratio2 = scrw / (SCR_W * 8);
	if (bratio2 < bratio) {
		bratio = bratio2;
	}
	if (bratio == 0) {
		bratio = 1;
	}
	int offx = (scrw - bratio * (SCR_W * 8)) / 2;
	int offy = (scrh - bratio * (SCR_H * 8)) / 2;
	if (offx < 0) {
		offx = 0;
	}
	if (offy < 0) {
		offy = 0;
	}
	//	int ratio = bratio << _g.screen_big;
	int ratio = bratio;
	
	if (led) {
		// SDL_SetRenderDrawColor(sdlg, 255, 255, 255, 255);
		SDL_SetRenderDrawColor(sdlg, 220, 55, 55, 255);
	} else {
		SDL_SetRenderDrawColor(sdlg, 0, 0, 0, 255);
	}
	SDL_RenderClear(sdlg);
	
	SDL_Rect rectdst = { offx, offy, SCR_W * 8 * ratio, SCR_H * 8 * ratio };
	SDL_RenderCopy(sdlg, sdlbuf, NULL, &rectdst);
//	displayFlush();

	if (show_splash) {
		SDL_Rect rectdst = { offx, offy, SCR_W * 8 * ratio, SCR_H * 8 * ratio };
		SDL_RenderCopy(sdlg, show_splash == 1 ? splash1 : splash2, NULL, &rectdst);
	}

	SDL_RenderPresent(sdlg); // Show render on window
	
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(sdlbuf);
}


/*
static inline void clearBackground(int white) {
	if (white) {
		SDL_SetRenderDrawColor(sdlg, 255, 255, 255, 255);
	} else {
		SDL_SetRenderDrawColor(sdlg, 0, 0, 0, 255);
	}
	SDL_RenderClear(sdlg);
}

void displayFlush() {
	SDL_RenderPresent(sdlg); // Show render on window
}

SDL_Texture* sdlbuf = NULL;
// 負荷 48% -> 12%、ただしVIDEO2で38%まで上がる
void display(void) {
	if (sdlbuf == NULL) {
		sdlbuf = SDL_CreateTexture(sdlg, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_TARGET, SCR_W * 8, SCR_H * 8);
//		sdlbuf = SDL_CreateTexture(sdlg, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, SCR_W * 8, SCR_H * 8);
//		sdlbuf = SDL_CreateTexture(sdlg, SDL_PIXELFORMAT_INDEX1LSB, SDL_TEXTUREACCESS_TARGET, SCR_W * 8, SCR_H * 8);
	}
	
	SDL_SetRenderDrawBlendMode(sdlg, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(sdlg, sdlbuf);
	SDL_SetRenderDrawColor(sdlg, 0, 0, 0, 0);
	SDL_RenderClear(sdlg);
	
	SDL_SetRenderDrawColor(sdlg, 255, 255, 255, 0);
	if (videoactive) {
		const uint8* crom = CHAR_PATTERN;
		const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
		uint8 cy = _g.cursory;
		
		uint8 cptn = 0xff;
		if (!key_flg.insert) {
			cptn = 0xe0;
		}
		uint8 invert = _g.screen_invert ? 0xff : 0; // invert or not
		for (int i = 0; i < SCREEN_H * 8; i++) {
			uint8 cx = _g.cursorx;
			if ((i >> 3) != cy) {
				cx = 255;
			}
			if (!((frames & 0x10) && _g.cursorflg)) {
				cx = 255;
			}
			for (int j = 0; j < SCREEN_W * 8; j++) {
				unsigned char c = vram[(j >> 3) + SCREEN_W * (i >> 3)];
				const unsigned char* rom = crom;
				if (c >= 0x100 - SIZE_PCG) {
					rom = crom2;
				}
				unsigned char p = rom[c * 8 + (i & 7)] ^ invert;
				if ((j >> 3) == cx) {
					p ^= cptn;
				}
				int white = (p & (1 << (7 - (j & 7)))) != 0;
				if (white) {
					SDL_RenderDrawPoint(sdlg, j, i);
				}
			}
		}
	}
	vflag = 1;
	
	int scrw, scrh;
	SDL_GL_GetDrawableSize(sdlw, &scrw, &scrh);
	int bratio = scrh / (SCR_H * 8);
	int bratio2 = scrw / (SCR_W * 8);
	if (bratio2 < bratio) {
		bratio = bratio2;
	}
	if (bratio == 0) {
		bratio = 1;
	}
	int offx = (scrw - bratio * (SCR_W * 8)) / 2;
	int offy = (scrh - bratio * (SCR_H * 8)) / 2;
	if (offx < 0) {
		offx = 0;
	}
	if (offy < 0) {
		offy = 0;
	}
	int ratio = bratio << _g.screen_big;
	
	SDL_SetRenderTarget(sdlg, NULL);
	clearBackground(led);
	SDL_Rect rectdst = { offx, offy, SCR_W * 8 * ratio, SCR_H * 8 * ratio };
	SDL_RenderCopy(sdlg, sdlbuf, NULL, &rectdst);
	displayFlush();
}
*/

//void key_tick();
void psg_tick();

void timer() {
#ifdef MODE15FPS
	frames += 4;
#else
	frames++;
#endif
	psg_tick();
	key_tick();
//	glutPostRedisplay();
//	glutTimerFunc(1000 / 60, timer, 0);
}

void video_on() {
	if (_g.screen_big > 4)
		_g.screen_big = 4;
	SCREEN_W = 32 >> _g.screen_big; // 1.2big
	SCREEN_H = 24 >> _g.screen_big;
	videoactive = 1;
}
inline void video_off(int clkdiv) {
	videoactive = 0;
}
inline int video_active() {
	return videoactive;
}

inline void IJB_lcd(uint mode) {
}

