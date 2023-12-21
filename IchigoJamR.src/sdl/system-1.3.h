#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <SDL.h>

#ifdef WIN32
#define USE_CONSOLE 1
//#define USE_CONSOLE 0

#if USE_CONSOLE == 1
#undef main // consoleで動かす場合はこの行を有効にする
#endif
#endif

//#define DEFAULT_UART 1
#define DEFAULT_UART 0

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "../screen.h"

#include "keyboard.h"

#define SCR_W 32
#define SCR_H 24
#define RATIO 2
#define OFFSET 8

#define SCREEN_W1 (SCR_W * 8 * RATIO)
#define SCREEN_H1 (SCR_H * 8 * RATIO)

#define SCREEN_W2 (SCREEN_W1 + OFFSET * 2)
#define SCREEN_H2 (SCREEN_H1 + OFFSET * 2)

void system_init();
void enterDeepSleep(int waitsec);
inline void deepPowerDown();
inline void IJB_sleep();
static inline void IJB_reset();
INLINE int IJB_random(int n);
INLINE int stopExecute();

STATIC void put_str(const char* s);

int getSleepFlag() {
	return 0;
}
void system_init() {
	SCREEN_W = 32;
	SCREEN_H = 24;
	screen_clear();
}
void enterDeepSleep(int waitsec) {
}
inline void deepPowerDown() {
}
int IJB_wait(int n, int active) { // if stop ret 1
	for (int i = 0; i < n; i++) {
		if (stopExecute()) {
			return 1;
		}
//		video_waitSync();
		SDL_Delay(1000 / 60); // msec
	}
	return 0;
}

void ichigojam_main(void);

inline static void init();
static void exec(char* s);
INLINE void key_enable(uint8 b);

void ichigojam_init(void) { // main
	system_init();
	init();
	{
		int sleepflg = getSleepFlag();
		
		/*
		if (!sleepflg) {
			psg_beep(10, 3);
		}
		*/
		//		if (!sleepflg2) { // 高速復帰モード
		/*
#define BOOT_WAIT 60	// 60より短いとだめなキーボードがある (AOの？)
		for (int j = 0; j < BOOT_WAIT; j++) {
//			video_waitSync();
		}
		*/
		//		if (!sleepflg) {
		for (int i = 0;; i++) {
			char c = IJB_TITLE[i];
			if (!sleepflg) {
				put_chr(c);
			}
//			video_waitSync();
			if (c == '\n') {
				break;
			}
		}
		//		}
#ifndef NONE_OK
		if (!sleepflg) {
			put_str("OK\n");
		}
#endif
		
		// resetはここにないと、キーボードが効かない
		key_enable(1); // 1.2.1b27
//		key_send_reset(); // reset // 1.2.1b27
		//		} else {
		//			key_enable(1); // 1.2.1b27
		//		}
		if (sleepflg) {
//			_g.screen_insertmode = 1;
//			screen_showCursor(0);
			exec("LRUN");
		}
	}
	
	/*
	// simple test for PC
	for (;;) {
		char buf[1024];
		for (int i = 0; i < 1023; i++) {
			int key = key_getKey();
			if (key == '\n') {
				buf[i] = 0;
				break;
			}
			buf[i] = key;
		}
		exec(buf);
	}
	 */
}

int resetflg = 0;
inline static void IJB_reset() {
	resetflg = 1;
}
inline void IJB_sleep() {
	resetflg = 2;
}

void ichigojam_loop2() {
	char* linebuf = (char*)ram + OFFSET_RAM_LINEBUF;
	for (;;) {
		IJB_random(1);
		video_waitSync(); // 消すとUART受信漏れ発生？
		
		int key = key_getKey();
		if (key < 0 || key == 27) {
		} else {
			_g.screen_insertmode = key_flg.insert;
			screen_putc(key);
			if (key == '\n') {
				uint8* s = screen_gets();
				//				put_str(s);
				if (*s == '\'') { // 1.1b14
				} else if (*s != 0) {
					uint8 i;
					for (i = 0; i < N_LINEBUF; i++) {
						linebuf[i] = s[i];
						if (!s[i])
							break;
					}
//					_g.screen_insertmode = 1; -> execへ 1.3b5
					if (s[i]) {
						put_str("Too long line\n");
					} else {
						linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
						exec(linebuf);
					}
				}
			}
		}
		if (resetflg) {
			if (resetflg == 2)
				exit(0);
			resetflg = 0;
			break;
		}
	}
}

void ichigojam_loop() {
	for (;;) {
		ichigojam_loop2();
		ichigojam_init();
	}
}

#ifdef WIN32
#include <process.h>
#define pthread_t int
void pthread_create(int* id, void* p, void* func, void* param) {
	_beginthread(func, 1024 * 1024, param);
}
#else
#include <pthread.h>
#endif

void display();
void timer();

SDL_Window* sdlw;
SDL_Renderer *sdlg;

void keyboard_key_down(SDL_KeyboardEvent* e);
void keyboard_key_up(SDL_KeyboardEvent* e);
void keyboard_mouse_down(SDL_MouseButtonEvent* e);
void keyboard_mouse_up(SDL_MouseButtonEvent* e);
void key_tick();

#ifdef MODE15FPS
#define DEFAULT_FPS 15
#else
#define DEFAULT_FPS 60
#endif


int fps = DEFAULT_FPS;

void sdlMainLoop(void) {
#ifdef BOOT_CHECK
char chk = 0;
for (int i = 0; i < 6; i++) {
	chk ^= IJB_TITLE[i];
}
printf("%d\n", chk);
if (chk != BOOT_CHECK) {
	//for (;;);
}
#endif
	
	SDL_Event event;
	
	uint tick = SDL_GetTicks();
	
#ifdef MODE15FPS
	uint wait_timer = 1000 / 15;
#else
	uint wait_timer = 1000 / 60;
#endif
	uint next_frame_timer = tick + wait_timer;
	
	uint wait_display = 1000 / fps;
	uint next_frame_display = tick + wait_display;
	
	for (;;) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return;
			}
			if (event.type == SDL_KEYDOWN) {
				keyboard_key_down(&event.key);
			}
			if (event.type == SDL_KEYUP) {
				keyboard_key_up(&event.key);
			/*
				printf("key: %d\n", event.key.keysym.sym);
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return;
				}
				*/
			}
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				keyboard_mouse_down(&event.button);
			}
			if (event.type == SDL_MOUSEBUTTONUP) {
				keyboard_mouse_up(&event.button);
			}
		}
		
		tick = SDL_GetTicks();
		if (tick >= next_frame_timer) {
			timer();
			next_frame_timer += wait_timer;
		}
		if (tick >= next_frame_display) {
			display();
			next_frame_display += wait_display;
			SDL_Delay(0);
		} else {
			SDL_Delay(2);
		}
		
		/*
			if (SDL_GetTicks() < next_frame) {
				display();
			}
//			printf("%f\n", SDL_GetTicks() - next_frame);
//			next_frame = SDL_GetTicks() + wait;
			SDL_Delay(0);
		} else {
			SDL_Delay(2);
		}
		*/
	}
}

void keyboard_dvorak();

#include <string.h>

void storage_setDirectory(char* path);
void storage_setHostNameInclude(int b);
void storage_setHostNameHexEncoding(int b);

void setCharmap(const uint8* map) {
	for (int i = 0; i < 256 * 8; i++) {
		CHAR_PATTERN[i] = map[i];
	}
}

extern int soundflg; // on: 1, off: 0

/*
#ifdef WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	int argc = 1;
	char* argv[] = {
		"ichigojam-ap.exe",
		"--uart0"
	};
#else
int main(int argc, char** argv) {
#endif
*/

int main(int argc, char** argv) {
	// change current dir
	char* datapath = argv[0];
	
	setCharmap(CHAR_PATTERN_JP);
	// option
	soundflg = 1;
	fps = DEFAULT_FPS;
	int uart = DEFAULT_UART;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--help") == 0) {
				printf("--help         show this help\n");
				printf("--keymapus     change the keymap to US\n");
				printf("--mongolian    change the charmap as Mongol\n");
				printf("--vietnamese   change the charmap as Vietnam\n");
				printf("--chinese      change the charmap as Chinese\n");
				printf("--uart1        starts with console output\n");
				printf("--nosound      no sound mode (CPU low load mode)\n");
				printf("--fps30        30 fps mode (CPU low load mode)\n");
				printf("--fps15        15 fps mode (CPU low load mode)\n");
				printf("--withhostname include the computer host name to the directory to save\n");
				printf("--enchostname  hex encoding computer host name\n");
				printf("-D[path]       change the directory to put data files (must ends with '\\')\n");
				return 0;
			} else if (strcmp(argv[i], "--keymapus") == 0) {
				keyboard_dvorak();
			} else if (strcmp(argv[i], "--mongolian") == 0) {
				setCharmap(CHAR_PATTERN_MN);
			} else if (strcmp(argv[i], "--vietnamese") == 0) {
				setCharmap(CHAR_PATTERN_VI);
			} else if (strcmp(argv[i], "--chinese") == 0) {
				setCharmap(CHAR_PATTERN_ZH);
			} else if (strcmp(argv[i], "--nosound") == 0) {
				soundflg = 0;
			} else if (strcmp(argv[i], "--fps30") == 0) {
				fps = 30;
			} else if (strcmp(argv[i], "--fps15") == 0) {
				fps = 15;
			} else if (strcmp(argv[i], "--uart1") == 0) {
				uart = 1;
			} else if (strcmp(argv[i], "--withhostname") == 0) {
				storage_setHostNameInclude(1);
			} else if (strcmp(argv[i], "--enchostname") == 0) {
				storage_setHostNameHexEncoding(1);
			} else if (strncmp(argv[i], "-D", 2) == 0) {
				datapath = argv[i] + 2;
			}
		}
	}
//	printf("%d\n", fps);
	storage_setDirectory(datapath);
		
	_g.uartmode = uart;
	
	ichigojam_init();
	
//	_g.uartmode = uart;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "couldn't init SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	// add SDL_HINT_VIDEO_HIGHDPI_DISABLED, SDL_WINDOW_ALLOW_HIGHDPI 2019.1.3
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	if (SDL_CreateWindowAndRenderer(SCREEN_W2, SCREEN_H2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI, &sdlw, &sdlg) < 0) {
		fprintf(stderr, "couldn't open window: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	SDL_SetWindowTitle(sdlw, "IchigoJam");
	
	pthread_t t;
#ifdef WIN32
	pthread_create(&t, NULL, (void * (*)(void *))ichigojam_loop, (void*)1);
#else
	pthread_create(&t, NULL, (void *_Nullable (*)(void *))ichigojam_loop, (void*)1);
#endif
	
	sdlMainLoop();

	SDL_DestroyWindow(sdlw);
	SDL_Quit();
	return 0;
}


#endif	// __SYSTEM_H__
