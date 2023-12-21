#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

// 参考
// https://tokoik.github.io/opengl/libglut.html#3.3


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
inline static void IJB_reset();

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
		video_waitSync();
	}
	return 0;
}

void ichigojam_main(void);

inline static void init();
void exec(char* s);
INLINE void key_enable(uint8 b);


void ichigojam_init(void) { // main
	system_init();
	init();
	{
		int sleepflg = getSleepFlag();
		
		//		if (!sleepflg2) { // 高速復帰モード
		if (!sleepflg) {
			psg_beep(10, 3);
		}
#define BOOT_WAIT 60	// 60より短いとだめなキーボードがある (AOの？)
		for (int j = 0; j < BOOT_WAIT; j++) {
//			video_waitSync();
		}
		//		if (!sleepflg) {
		for (int i = 0;; i++) {
			char c = IJB_TITLE[i];
			if (!sleepflg)
			put_chr(c);
//			video_waitSync();
			if (c == '\n')
			break;
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
			_g.screen_insertmode = 0;
			screen_showCursor(0);
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
	char* linebuf = ram + OFFSET_RAM_LINEBUF;
	for (;;) {
		IJB_random(1);
		video_waitSync(); // 消すとUART受信漏れ発生？
		
		int key = key_getKey();
		if (key < 0 || key == 27) {
		} else {
			_g.screen_insertmode = key_flg.insert;
			screen_putc(key);
			if (key == '\n') {
				char* s = screen_gets();
				//				put_str(s);
				if (*s == '\'') { // 1.1b14
				} else if (*s != 0) {
					uint8 i;
					for (i = 0; i < N_LINEBUF; i++) {
						linebuf[i] = s[i];
						if (!s[i])
						break;
					}
					_g.screen_insertmode = 0;
					if (s[i]) {
						put_str("Too long line\n");
					} else {
						linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
						_g.key_flg_esc = 0;
						screen_showCursor(0);
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

int main(int argc, char** argv) {
	glutInitWindowSize(SCREEN_W2, SCREEN_H2);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutCreateWindow("IchigoJam");
	
	ichigojam_init();
	
	glutDisplayFunc(display);
	//	glutIdleFunc(glutPostRedisplay); // 空き時間に再描画
	glutTimerFunc(1000 / 60, timer, 0); // タイマーで60fps
	
	pthread_t t;
	pthread_create(&t, NULL, (void*)ichigojam_loop, (void*)1);
	
	glutMainLoop();
	return 0;
}


