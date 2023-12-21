#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <stdio.h>

void key_send_reset() {
}

#ifdef WIN32
#include <conio.h>
#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "../stddef.h"
#include "../vars.h"
#include "../ram.h"
#include "../screen.h"

#include "system.h"

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
uint8 displaymode;

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

static inline int key_isFullBuffer() {
	return *keybuf == KEY_BUF_LEN;
}

void key_pushc(int c) {
	if (*keybuf < KEY_BUF_LEN) {
		(*keybuf)++;
		keybuf[(uint8)*keybuf] = (char)c;
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
	n -= n == 88 ? 88 - 5 : 28; // X or not
	return (keykbhit & (1 << n)) != 0;
//	return (keykbhit & (1 << (n - 28))) != 0;
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
	*keybuf = 0;
}


INLINE void uart_init() {
}

inline void IJB_uart(int16 txd, int16 rxd) {
	_g.uartmode_txd = txd;
	_g.uartmode_rxd = rxd;
}

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
	if (_g.uartmode_txd) {
		putchar(c);
	}
}

INLINE void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
}

void put_chr(char c) {
	if (_g.uartmode_txd > 0) { // 1.0.2b12 uartを先に
		uart_putc(c);
	}
	screen_putc(c); // segmentation fault
}

// basic interface
inline int stopExecute() {
	SDL_Delay(0);
//	usleep(10); // usec単位、1msec wait
	//	sched_yield(); // だめ
	return _g.key_flg_esc;
}

uint8 lastkey = 0;
int lastkeyframe;
int keyrepeatcnt;

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

const int TENKEYS[] = {
	SDLK_KP_0, SDLK_0,
	SDLK_KP_1, SDLK_1,
	SDLK_KP_2, SDLK_2,
	SDLK_KP_3, SDLK_3,
	SDLK_KP_4, SDLK_4,
	SDLK_KP_5, SDLK_5,
	SDLK_KP_6, SDLK_6,
	SDLK_KP_7, SDLK_7,
	SDLK_KP_8, SDLK_8,
	SDLK_KP_9, SDLK_9,
	SDLK_KP_7, SDLK_7,
	SDLK_KP_AT, SDLK_AT,
	SDLK_KP_BACKSPACE, SDLK_BACKSPACE,
	SDLK_KP_COMMA, SDLK_COMMA,
//	SDLK_KP_DECIMAL, SDLK_DECIMAL,
	SDLK_KP_DIVIDE, SDLK_SLASH,
	SDLK_KP_ENTER, SDLK_RETURN,
	SDLK_KP_EQUALS, SDLK_EQUALS,
	SDLK_KP_EXCLAM, SDLK_EXCLAIM,
	SDLK_KP_HASH, SDLK_HASH,
	SDLK_KP_MINUS, SDLK_MINUS,
	SDLK_KP_MULTIPLY, SDLK_ASTERISK,
	SDLK_KP_PERCENT, SDLK_PERCENT,
	SDLK_KP_PERIOD, SDLK_PERIOD,
	SDLK_KP_PLUS, SDLK_PLUS,
	SDLK_KP_SPACE, SDLK_SPACE,
	SDLK_KP_TAB, SDLK_TAB,
	0,
};
int convertTenKeys(int key) {
	for (int i = 0;; i += 2) {
		if (!TENKEYS[i])
			break;
		if (TENKEYS[i] == key)
			return TENKEYS[i + 1];
	}
	return key;
}

int keyboard_sp_down(int key) {
	//	printf("spdown: %d\n", key);
	if (key == SDLK_UP) {
		key_pushc(30);
		key_kbhit(2);
		setLastKey(30);
	} else if (key == SDLK_DOWN) {
		key_pushc(31);
		key_kbhit(3);
		setLastKey(31);
	} else if (key == SDLK_RIGHT) {
		key_pushc(29);
		key_kbhit(1);
		setLastKey(29);
	} else if (key == SDLK_LEFT) {
		key_pushc(28);
		key_kbhit(0);
		setLastKey(28);
	} else if (key == SDLK_F1) {
		key_push("\x13\x0c"); // page up, clear after cursor 1.2b13
	} else if (key == SDLK_F2) {
		key_push("\x18LOAD");
	} else if (key == SDLK_F3) {
		key_push("\x18SAVE");
	} else if (key == SDLK_F4) {
//		key_push("\x18\030LIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除 //?
		key_push("\x0cLIST\n"); // -> カーソル以降削除  1.2b37
	} else if (key == SDLK_F5) {
		key_push("\x18RUN\n"); // 4番目に文字があるとエラーになる・・・けど制限？
	} else if (key == SDLK_F6) {
		key_push("\x18?FREE()\n");
	} else if (key == SDLK_F7) {
		key_push("\x18OUT0\n"); // 0.9.4
	} else if (key == SDLK_F8) {
		key_push("\x18VIDEO1\n"); // 0.9.6
	} else if (key == SDLK_F9) {
		key_push("\x18\014FILES"); // 1.0.0b14 // \014(8進数) = 0x0c // 改行を廃止 1.2b35
	} else if (key == SDLK_F10) {
		key_pushc(0x18); // 0.9.9 = 行削除
	} else if (key == SDLK_F11) {
		key_pushc(0x0c); // 0.9.9 = カーソル以降削除
	} else if (key == SDLK_F12) {
		key_push("\x18SWITCH\n"); // 1.2b32
	} else if (key == SDLK_HOME) {
		key_pushc(0x12);
	} else if (key == SDLK_END) {
		key_pushc(0x17);
	} else if (key == SDLK_PAGEUP) {
		key_pushc(0x13);
	} else if (key == SDLK_PAGEDOWN) {
		key_pushc(0x14);
	} else if (key == SDLK_DELETE) {
		key_pushc(127); // 1.2b45 // mac なくてもok? Winないとng?
	} else {
		return 0;
	}
	return 1;
}
int keyboard_sp_up(int key) {
	//	printf("spup: %d\n", key);
	if (key == SDLK_UP) {
		key_kbhit_rel(2);
		resetLastKey(30);
	} else if (key == SDLK_DOWN) {
		key_kbhit_rel(3);
		resetLastKey(31);
	} else if (key == SDLK_RIGHT) {
		key_kbhit_rel(1);
		resetLastKey(29);
	} else if (key == SDLK_LEFT) {
		key_kbhit_rel(0);
		resetLastKey(28);
	} else {
		return 0;
	}
	return 1;
}

const char KEY_MAP_B_D[] = "1234567890[]',./=-\\;`";
const char KEY_MAP_S_D[] = "!@#$%^&*(){}\"<>?+_|:~";

//#ifdef WIN32
//const char KEY_MAP_B_Q[] =  "1234567890-^¥@[;:],./";
//const char KEY_MAP_S_Q[] =  "!\"#$%&'() =~|`{+*}<>?";
//#else
const char KEY_MAP_B_Q[] =  "1234567890-^\\@[;:],./";
const char KEY_MAP_S_Q[] =  "!\"#$%&'() =~|`{+*}<>?";
//#endif

const char* key_map_b = KEY_MAP_B_Q;
const char* key_map_s = KEY_MAP_S_Q;

int keyboard_id = 0;

void keyboard_dvorak() {
	key_map_b = KEY_MAP_B_D;
	key_map_s = KEY_MAP_S_D;
	keyboard_id = 1;
}
static inline uint key_getKeyboardID() {
	return keyboard_id;
}

//extern uint8 key_kana_buf0;
#include "../romajikana.h"

uint8 keyConvert(uint8 key, int shift, int alt) {
	if (key >= 'a' && key <= 'z') {
		if (!shift) {
			key = key - ('a' - 'A');
		}
//	} else if (key >= 'A' && key <= 'Z') {
//		key = key - ('A' - 'a');
	}
	if (shift) {
		for (int i = 0; i < strlen(key_map_b); i++) {
			if (key_map_b[i] == key) {
				key = key_map_s[i];
				break;
			}
		}
	}
	if (key == '\r') {
		key = '\n';
		if (shift) {
			key = 0x10;
		}
		key_kana_buf0 = 0;
	}
	if (key == 32) {
		if (shift) {
			key = 0xe; // Shift+Space=空白挿入
		}
	}
	if (alt) {
		//	if (mod & GLUT_ACTIVE_CTRL) { // ALTが効かないので、CTRLで代用
		//		int shift = mod & GLUT_ACTIVE_SHIFT;
		int k = key;
		if (k == '[') {
			k = '_';
		} else if (k == ']') {
			k = '\\';
		} else if (k >= '0' && k <= '9') {
	//		k = k - '0' + 0xe0;
			k = k - '0' + (shift ? 0x80 : 0xe0);
		} else if (k >= 'A' && k <= 'V') {
//			k = k - ('A' - 10) + 0xe0;
			k = k - ('A' - 10) + (shift ? 0x80 : 0xe0);
		} else if (k >= 'a' && k <= 'v') {
			k = k - ('a' - 10) + 0x80;
		} // shift 0-9 の未対応
		//		printf("alt key:%d\n", k);
		key = k;
	}
	
	return key;
}

//void keyboard_input_kana(int k);

int flg_keyboard_ctrl = 0; // 1.4.1

void keyboard_key_down(SDL_KeyboardEvent* e) {
//	printf("keydown: %d %d\n", e->keysym.sym);
//	printf("keydown: key:%d scancode:%d\n", e->keysym.sym, e->keysym.scancode);
	//	int key = e->keysym.scancode;
	int key = e->keysym.sym;
	int scancode = e->keysym.scancode;
	int shift = (e->keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
	int altl = (e->keysym.mod & KMOD_LALT) != 0;
	int altr = (e->keysym.mod & KMOD_RALT) != 0;
	int alt = altl || altr;
	int ctrl =  (e->keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0;

	if (key == SDLK_RCTRL || key == SDLK_LCTRL) // 1.4.1 for autorun
		flg_keyboard_ctrl = 1;
	
	//if (key == SDLK_RALT || (shift && (key == SDLK_RCTRL || key == SDLK_LCTRL))) {
	if ((key == SDLK_RSHIFT || key == SDLK_LSHIFT) && ctrl) {
		key_flg.kana = !key_flg.kana;
		key_kana_buf0 = 0;
	}
	if ((key == SDLK_RALT || key == SDLK_LALT) && ctrl) {
		key_flg.insert = !key_flg.insert;
	}
//	printf("%x %d %d\n", key, SDLK_MINUS, '-');
	key = convertTenKeys(key);
	if (keyboard_sp_down(key)) {
		return;
	}
//	printf("%x %d %d\n", key, SDLK_MINUS, '-');
	if (key >= 0x100) {
		return;
	}
	if (key == 165) { // 特殊？
		// 135, 137 円マーク区別ができない問題
		key = shift ? (scancode == 135 ? '_' : '|') :'\\'; // windows用特殊対応
	}
	if (key == SDLK_SPACE) {
		key_kbhit(4);
	}
	key = keyConvert(key, shift, altl);
	if (key == 27) { // esc
		_g.key_flg_esc = 1;
		key_kana_buf0 = 0;
	}
	if (key_flg.kana) { // 1.3b2
	//if (key_flg.kana && !shift) {
		key = romajikana_input(key);
	}
	if (key) {
		key_pushc(key);
	}
//	printf("keydown: %d -> %d\n", e->keysym.sym, key);
}
void keyboard_key_up(SDL_KeyboardEvent* e) {
//	printf("keyup:");
	int key = e->keysym.sym;
	int ctrl =  (e->keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0;

	if (key == SDLK_RCTRL || key == SDLK_LCTRL) // 1.4.1 for autorun
		flg_keyboard_ctrl = 0;
	
	key = convertTenKeys(key);
	if (keyboard_sp_up(key)) {
		return;
	}
	if (key == SDLK_SPACE) {
		key_kbhit_rel(4);
	}
	if (key >= 0x100) {
		return;
	}
//	printf("keyup: %d\n", e->keysym.sym);
}
int mousebtn;
void keyboard_mouse_down(SDL_MouseButtonEvent* e) {
	if (e->button & SDL_BUTTON_LEFT) {
		mousebtn = 1;
	}
}
void keyboard_mouse_up(SDL_MouseButtonEvent* e) {
	if (e->button & SDL_BUTTON_LEFT) {
		mousebtn = 0;
	}
}

/*
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
*/
void key_tick() {
	/*
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
	*/
	if (!key_isFullBuffer()) {
		if (kbhit()) {
#ifdef WIN32
			int ch = getch();
#else
			int ch = getchar();
#endif
			if (ch == '\r') {
				ch = '\n';
			} else if (ch == 0x1b) {
				_g.key_flg_esc = 1;
			}
//			printf("ch: %d %c\n", ch, (char)ch);
			key_pushc(ch);
		}
	}
}

inline static void key_init() {
	key_flg.insert = 0;
	/*
	glutIgnoreKeyRepeat(GL_TRUE);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutSpecialFunc(keyboardsp);
	glutSpecialUpFunc(keyboardspup);
	glutMouseFunc(mouse);
	*/
}

#endif // __KEYBOARD_H__
