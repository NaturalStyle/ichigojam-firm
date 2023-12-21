// volatile uint8 vflag; // bit

#ifdef WIN32
#include <windows.h>
void usleep(int n) {
	Sleep(n / 1000);
}

#else
#include <unistd.h>
#endif

inline void video_waitSync() {
	while (!vflag) {
		usleep(100); // usec単位、1msec wait
	}
	vflag = 0;
}

void clearBackground(int white) {
	if (white) {
		glClearColor(1.0, 1.0, 1.0, 1.0);
	} else {
		glClearColor(0.0, 0.0, 0.0, 1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT);
}

void setPixel(int x, int y, int white, int ratio) {
	if (white) {
		glColor3d(1.0, 1.0, 1.0);
	} else {
		glColor3d(0.0, 0.0, 0.0);
	}
	double dx = (double)(x * ratio + OFFSET) / SCREEN_W2 * 2.0 - 1.0;
	double dy = -(double)(y * ratio + OFFSET) / SCREEN_H2 * 2.0 + 1.0;
	double dx2 = (double)((x + 1) * ratio + OFFSET) / SCREEN_W2 * 2.0 - 1.0;
	double dy2 = -(double)((y + 1) * ratio + OFFSET) / SCREEN_H2 * 2.0 + 1.0;
	glBegin(GL_POLYGON);
	glVertex2d(dx, dy);
	glVertex2d(dx2, dy);
	glVertex2d(dx2, dy2);
	glVertex2d(dx, dy2);
	glEnd();
}
void displayFlush() {
	glFlush();
}

#define SCRW 320
#define SCRH 240
#define SW 320
#define SH 240

extern int led;

int videoactive = 1;

void display(void) {
	clearBackground(led);
	if (videoactive) {
		int ratio = RATIO << _g.screen_big;
		const uint8* crom = CHAR_PATTERN;
		const uint8* crom2 = screen_pcg - ((0x100 - SIZE_PCG) * 8);
		uint8 cy = _g.cursory;
		
		uint8 cptn = 0xff;
		if (key_flg.insert) {
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
				setPixel(j + ((SCRW - SW) / 2), i + (SCRH - SH) / 2, white ? 1 : 0, ratio);
			}
		}
	}
	vflag = 1;
	displayFlush();
}

void key_tick();
void psg_tick();

void timer(int value) {
	frames++;
	psg_tick();
	key_tick();
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, timer, 0);
}

void video_on() {
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

inline void IJB_lcd(int mode) {
}

