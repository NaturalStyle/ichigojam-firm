#include <stdio.h>

#ifdef WIN32
#include <conio.h>
#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// http://detail.chiebukuro.yahoo.co.jp/qa/question_detail/q13104330103
int kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}
#endif

struct keyflg_def key_flg;
int displaymode;

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2) // kbhitとnkeybuf分
uint8* keybuf = (uint8*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！
//#define keybuf ((char*)(ram + OFFSET_RAM_KEYBUF)) // 24560+56
//volatile uint8 keykbhit = 0; // SPC DOWN UP RIGHT LEFT // 28-32

#define keykbhit keybuf[-1]
void key_kbhit(int n) {
	keykbhit |= 1 << n;
}
void key_kbhit_rel(int n) {
	keykbhit &= ~(1 << n);
}

void key_pushc(char c) {
	if (*keybuf < KEY_BUF_LEN) {
		(*keybuf)++;
		keybuf[(uint8)*keybuf] = c;
	}
}
void key_push(char* s) { // for function keys
	for (;;) {
		char c = *s++;
		if (!c) {
			break;
		}
		key_pushc(c);
	}
}


INLINE void key_enable(uint8 b) {
//	key_enable_flg = b;
}
//void uart_checker();

extern int mousebtn;
inline int key_btn(int n) {
	//	uart_checker();
	if (n == 0) {
		return mousebtn;
	}
	return (keykbhit & (1 << (n - 28))) != 0;
}

int key_getKey() {
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	(*keybuf)--;
	for (uint8 i = 0; i < *keybuf; i++)
		keybuf[i + 1] = keybuf[i + 2];
	return res;
}
void key_clearKey() {
	//	keykbhit = 0; // ver1.2b5 clear しないように変更
	*keybuf = 0;
}


INLINE void uart_init() {
	_g.uartmode = 1;
	/*
#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
	*/
}

inline void IJB_uart(int txd, int rxd) {
//	_g.uartmode = txd;
//	_g.uartmode_rxd = rxd;
}

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
	putchar(c);
	/*
	if (_g.uartmode == 3) {
		// no buffering
		if (c == '\n') {
			uart_putc('\r');
		}
	}
	while (!(LPC_UART->LSR & LSR_THRE));
	LPC_UART->THR = c;
*/
}

inline void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
	if (n == 0) {
		n = 115200;
	} else if (n == -1) {
		n = 57600;
	} else if (n == -2) {
		n = 38400;
	}
//	UARTInit(n);
}

void put_chr(char c) {
	if (_g.uartmode > 0) { // 1.0.2b12 uartを先に
		uart_putc(c);
	}
	screen_putc(c); // segmentation fault
}

// basic interface
inline int stopExecute() {
	usleep(10); // usec単位、1msec wait
	//	sched_yield(); // だめ
	return _g.key_flg_esc;
}

uint8 lastkey = 0;
int lastkeyframe;
int keyrepeatcnt;

uint8 keyConvert(uint8 key, int mod) {
	if (key >= 'a' && key <= 'z') {
		key = key - ('a' - 'A');
	} else if (key >= 'A' && key <= 'Z') {
		key = key - ('A' - 'a');
	}
	if (key == '\r') {
		key = '\n';
		if (mod & GLUT_ACTIVE_SHIFT) {
			key = 0x10;
		}
	}
	
	/*
	GLUT_ACTIVE_SHIFT : Shift キーが押されている
	GLUT_ACTIVE_CTRL : Ctrl キーが押されている
	GLUT_ACTIVE_ALT : Alt キーが押されている
	*/
	//	printf("mod:%d\n", (mod & GLUT_ACTIVE_ALT));
	if (mod & GLUT_ACTIVE_ALT) {
		//	if (mod & GLUT_ACTIVE_CTRL) { // ALTが効かないので、CTRLで代用
		//		int shift = mod & GLUT_ACTIVE_SHIFT;
		int k = key;
		if (k >= '0' && k <= '9') {
			k = k - '0' + 0xe0;
			//			k = k - '0' + (shift ? 0x80 : 0xe0);
		} else if (k >= 'A' && k <= 'V') {
			k = k - ('A' - 10) + 0xe0;
		} else if (k >= 'a' && k <= 'v') {
			k = k - ('a' - 10) + 0x80;
		} // shift 0-9 の未対応
		//		printf("alt key:%d\n", k);
		key = k;
	}
	
	return key;
}
void keyboard(unsigned char key, int x, int y) {
//	printf("key:%d mod:%d %d\n", key, glutGetModifiers(), GLUT_ACTIVE_ALT);
	int mod = glutGetModifiers();
	if (mod & GLUT_ACTIVE_CTRL) {
		return;
	}
	key = keyConvert(key, mod);
	if (key == '\033') { // esc
		//		exit(0);
		_g.key_flg_esc = 1;
	}
	if (key == 32) {
		key_kbhit(4);
	}
	key_pushc(key);
	lastkey = key;
	lastkeyframe = frames;
	keyrepeatcnt = 0;
}
void keyboardup(unsigned char key, int x, int y) {
	int mod = glutGetModifiers();
	if (mod & GLUT_ACTIVE_CTRL) {
		return;
	}
	
	key = keyConvert(key, mod);
	if (key == 32) {
		key_kbhit_rel(4);
	}
	if (key == lastkey) {
		lastkey = 0;
	}
}
void key_tick() {
	if (lastkey) {
		if (keyrepeatcnt == 0) {
			if (frames - lastkeyframe > 40) {
				key_pushc(lastkey);
				lastkeyframe = frames;
				keyrepeatcnt++;
			}
		} else {
			if (frames - lastkeyframe > 2) {
				key_pushc(lastkey);
				lastkeyframe = frames;
				keyrepeatcnt++;
			}
		}
	}
	
	if (kbhit())
		key_pushc(getchar());

}
void setLastKey(int key) {
	lastkey = key;
	lastkeyframe = frames;
	keyrepeatcnt = 0;
}
void resetLastKey(int key) {
	if (lastkey == key) {
		lastkey = 0;
	}
}
void keyboardsp(int key, int x, int y) {
//	printf("spdown: %d\n", key);
	if (key == GLUT_KEY_UP) {
		key_pushc(30);
		key_kbhit(2);
		setLastKey(30);
	} else if (key == GLUT_KEY_DOWN) {
		key_pushc(31);
		key_kbhit(3);
		setLastKey(31);
	} else if (key == GLUT_KEY_RIGHT) {
		key_pushc(29);
		key_kbhit(1);
		setLastKey(29);
	} else if (key == GLUT_KEY_LEFT) {
		key_pushc(28);
		key_kbhit(0);
		setLastKey(28);
	} else if (key == GLUT_KEY_F1) {
		key_push("\x13\x0c"); // page up, clear after cursor 1.2b13
	} else if (key == GLUT_KEY_F2) {
		key_push("\x18LOAD");
	} else if (key == GLUT_KEY_F3) {
		key_push("\x18SAVE");
	} else if (key == GLUT_KEY_F4) {
		key_push("\x18\030LIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
	} else if (key == GLUT_KEY_F5) {
		key_push("\x18RUN\n");
	} else if (key == GLUT_KEY_F6) {
		key_push("\x18?FREE()\n");
	} else if (key == GLUT_KEY_F7) {
		key_push("\x18OUT0\n"); // 0.9.4
	} else if (key == GLUT_KEY_F8) {
		key_push("\x18VIDEO1\n"); // 0.9.6
	} else if (key == GLUT_KEY_F9) {
		key_push("\x18\014FILES\n"); // 1.0.0b14 // \014(8進数) = 0x0c
	} else if (key == GLUT_KEY_F10) {
		key_pushc(0x18); // 0.9.9
	} else if (key == GLUT_KEY_F11) {
		key_pushc(0x0c); // 0.9.9
	} else if (key == GLUT_KEY_F12) {
		key_push("\x18SWITCH\n"); // 1.2b32
	}
}
void keyboardspup(int key, int x, int y) {
//	printf("spup: %d\n", key);
	if (key == GLUT_KEY_UP) {
		key_kbhit_rel(3);
		resetLastKey(30);
	} else if (key == GLUT_KEY_DOWN) {
		key_kbhit_rel(2);
		resetLastKey(31);
	} else if (key == GLUT_KEY_RIGHT) {
		key_kbhit_rel(1);
		resetLastKey(29);
	} else if (key == GLUT_KEY_LEFT) {
		key_kbhit_rel(0);
		resetLastKey(28);
	}
}
int mousebtn;
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		mousebtn = state == GLUT_DOWN;
	}
}

inline static void key_init() {
	key_flg.insert = 1;
	glutIgnoreKeyRepeat(GL_TRUE);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutSpecialFunc(keyboardsp);
	glutSpecialUpFunc(keyboardspup);
	glutMouseFunc(mouse);
}

