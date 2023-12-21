#ifndef __SYSTEM_H__
#define __SYSTEM_H__

//#define memset(p, n, len) clearMemory(p, len)
void clearMemory(char* p, int len) {
	for (int i = 0; i < len; i++)
		*p++ = 0;
}
void* memset(void* vp, int len, unsigned long dummy) { //??
	char* p = (char*)vp;
	for (int i = 0; i < len; i++)
		*p++ = 0;
	return vp;
}

#include "config.h"
#include "../screen.h"
#include "../basic.h"

// 1回のtickで4コマンド動かす、実機と速度を似せるため
#define CNT_EXEC_TICKS 1
#define TICK_PER_SEC (180)

static void system_init();
static void enterDeepSleep(int waitsec);
inline static void deepPowerDown();
inline static void IJB_sleep();
inline static void IJB_reset();

static int getSleepFlag() {
	uint8 buf[1024];
	if (IJB_load(0, buf, 1024, 0) != -1) {
		uint8* ad = buf;
		int sleepflg = *(uint8*)(ad + 3) == 0x40 && ((*(uint*)(ad + 4) << 8) >> 8) == 0x555241; // @ARUNかどうか
		if (sleepflg)
			return 1;
	}
	return 0;
}
static void system_init() {
}
static void enterDeepSleep(int waitsec) {
}
static inline void deepPowerDown() {
}
int nwait = 0;
static int IJB_wait(int n, int active) { // if stop ret 1
	if (n < 0)
		n = 0;
	nwait = n * CNT_EXEC_TICKS * (TICK_PER_SEC / 60);
	return 0;
}
uint8 running = 0;

inline static void init();
//void exec(char* s);
INLINE void key_enable(uint8 b);

static int checkStop() { // stop:1
	if (stopExecute()) {
		command_error(ERR_BREAK);
		nwait = 0;
		running = 0;
		key_flg.insert = key_flg.bkinsert;
		// from system_exec
		screen_showCursor(1);
		if (_g.cursory == -1)
			_g.cursory = 0;
		if (!noresmode) {// stop, exec, edit, err
			// put_str("OK\n");
		}
		return 1;
	}
	return 0;
}

uint8 inputmode = 0;
uint8 ox_input;
uint8 oy_input;

static void processInputMode() {
	if (checkStop()) {
		_g.screen_insertmode = 1;
		inputmode = 0;
		return;
	}
	int key = key_getKey();
	if (key == '\n') {
		char* line = (char*)(vram + ox_input + SCREEN_W * oy_input);
		command_input2(line);
		_g.screen_insertmode = 1;
		screen_showCursor(0);
		inputmode = 0;
	} else if (key >= 0 && key != 27 && key != 30 && key != 31 && !((key == '\b' || key == 28) && _g.cursorx == ox_input)) {
		_g.screen_insertmode = key_flg.insert;
		screen_putc(key);
	}
}

uint8 listmode = 0;
uint8 filesmode = 0;

static int system_exec(char* s) { // if contintue ret 1
	int res = basic_execute(s);
	if (res == BASIC_RESULT_CONTINUE) {
		return 1;
	} else if (res == BASIC_RESULT_INPUT) {
		inputmode = 1;
		screen_showCursor(1);
		_g.screen_insertmode = key_flg.insert;
		ox_input = _g.cursorx;
		oy_input = _g.cursory;


//		char* line = "1+1";
//		command_input2(line);
		return 1;
	} else if (res == BASIC_RESULT_LIST) {
		listmode = 1;
		return 1;
	} else if (res == BASIC_RESULT_FILES) {
		filesmode = 1;
		return 1;
	}
	screen_showCursor(1);
	if (_g.cursory == -1)
		_g.cursory = 0;
	if (res == BASIC_RESULT_EXECUTE && !noresmode) {// stop, exec, edit, err
		put_str("OK\n");
	}
	//if (res != 2) { // 1.2b36 追加
	if (res == BASIC_RESULT_STOP_OR_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
		key_clearKey();
	}
	key_flg.insert = key_flg.bkinsert;
	return 0;
}

static void ichigojam_main(void);

int resetflg = 0;
inline static void IJB_reset() {
	resetflg = 1;
}
static inline void IJB_sleep() {
	resetflg = 2;
}

int systemt = 0;

static void ichigojam_init(void) { // main
	_g.screen_big = 0;
	_g.screen_invert = 0;
	video_on();
	
	systemt = 0;

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
			_g.screen_insertmode = 0;
			screen_showCursor(0);
			running = system_exec("LRUN");
		} else {
			key_flg.insert = 0;
			screen_showCursor(1);
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

uint8* _keybuf = (uint8*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい!

uint8 initflg = 0;

static void ichigojam_tick(int waitflg) {
	if (nwait > 0) {
//		if (waitflg)
		nwait--;
		if (checkStop()) {
			inputmode = listmode = filesmode = 0;
		}
		return;
	}
	if (inputmode) {
		processInputMode();
		return;
	}
	if (listmode) {
		listmode = command_list_next();
		return;
	}
	if (filesmode) {
		filesmode = command_files_next();
		return;
	}
	if (running) {
		running = system_exec(NULL);
		return;
	}
	char* linebuf = (char*)ram + OFFSET_RAM_LINEBUF;
	
	IJB_random(1);
//	video_waitSync(); // 消すとUART受信漏れ発生?
	
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
				_g.screen_insertmode = 1;
				if (s[i]) {
					put_str("Too long line\n");
				} else {
					linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
					_g.key_flg_esc = 0;
					screen_showCursor(0);
					key_flg.bkinsert = key_flg.insert;
					key_flg.insert = 1;
					running = system_exec(linebuf);
				}
			}
		}
	}
	
	if (resetflg) {
		if (resetflg == 2) {
		}
		initflg = 0;
		resetflg = 0;
	}
}

// for JavaScript
EXPORT uint8* getRAM() {
	return ram;
}
EXPORT int getSizeRAM() {
	return SIZE_RAM;
}
EXPORT const uint8* getCharPattern() {
	return CHAR_PATTERN;
}
EXPORT int getCharPatternSize() {
	return 256 * 8;
}

extern uint8 file[];
extern uint file_update_flg;

EXPORT uint8* getStorage() {
	return file;
}
EXPORT int getStorageSize() {
	return FILE_SIZE;
}
EXPORT int checkFileUpdate() {
	if (!file_update_flg) {
		return -1;
	}
	file_update_flg = 0;
	return _g.lastfile;
}

/*
#define SIZE_SCRBUF (32 * 8) * (24 * 8) * 4
uint8 scrbuf[SIZE_SCRBUF];
int systemt;

void drawVRAM() {
#define SCRBUF_H 24
#define SCRBUF_W 32
	
	for (int i = 0; i < SCRBUF_H; i++) {
		for (int j = 0; j < SCRBUF_W; j++) {
			uint8 c = ram[OFFSET_RAM_VRAM + (j + i * SCRBUF_W)];
			for (int k = 0; k < 8; k++) {
				uint8 p = CHAR_PATTERN[c * 8 + k];
				for (int l = 0; l < 8; l++) {
					int b = (p & (0x80 >> l)) == 0 ? 0 : 255;
					int idx = (j * 8 + l + (i * 8 + k) * (SCRBUF_W * 8)) * 4;
					scrbuf[idx++] = b;
					scrbuf[idx++] = b;
					scrbuf[idx++] = b;
					scrbuf[idx++] = 0xff; // alpha: 0=transparent
				}
			}
		}
	}

}
uint8* getScreenBuffer() {
	return scrbuf;
}
int getSizeScreenBuffer() {
	return SIZE_SCRBUF;
}
*/

EXPORT
int tick() {
	static int tickcnt = 0;
	if (!initflg) {
		initflg = 1;
		frames = 0;
		tickcnt = 0;
		ichigojam_init();
	}
	tickcnt++;
	int waitflg = 0;
	//if (tickcnt % 3 == 0) {
	if ((tickcnt & 3) == 0) {
		frames++;
		waitflg = 1;
		psg_tick();
	}
	for (int i = 0; i < CNT_EXEC_TICKS; i++) {
		ichigojam_tick(waitflg);
	}
//	drawVRAM();
	return 0;
}
uint32 outport = 0;
EXPORT
int getOutPort() { // LED: 1<<7
	return outport;
}

int pwmvalue[12];
int pwmlen[12];

EXPORT
int getPWMValue(int port) {
	if (port >= 0 && port < 12) {
		return pwmvalue[port];
	}
	return 0;
}
EXPORT
int getPWMLen(int port) {
	if (port >= 0 && port < 12) {
		return pwmlen[port];
	}
	return 0;
}
/*
uint32 getCursor() {
	return ((key_flg.insert & 1) << 17) | ((_g.cursorflg & 1) << 16) | ((_g.cursory << 8) & 0xff) | (_g.cursorx & 0xff);
}
*/
EXPORT
int getCursorX() {
	return _g.cursorx;
}
EXPORT
int getCursorY() {
	return _g.cursory;
}
EXPORT
int getCursorFlag() {
	return _g.cursorflg;
}
EXPORT
int getCursorInsert() {
	return key_flg.insert & 1;
//	return _g.screen_insertmode;
}
//
EXPORT
int getScreenInvert() {
	return _g.screen_invert;
}
EXPORT
int getScreenBig() {
	return _g.screen_big;
}
EXPORT
int getFreq() {
	if (!_g.psgtone)
		return 0;
	return 60 * 261 / _g.psgtone;
}

//

extern int instate[12];
extern int btnstate;

EXPORT void setStateIN(int n, int m) {
	if (n >= 0 && n <= 11) {
		if (n == 0 || n == 9) {
			instate[9] = m;
			instate[0] = m;
			btnstate = m;
		} else {
			instate[n] = m;
		}
	}
}

EXPORT void setButtonState(int n) {
	btnstate = n;
	instate[0] = instate[9] = n != 0;
}

#define UARTOUTPUT_SIZE 256
uint8 uartoutput[UARTOUTPUT_SIZE];

EXPORT uint8* getUARTOutput() {
	return uartoutput;
}
EXPORT int getUARTOutputSize() {
	return UARTOUTPUT_SIZE;
}

// __SYSTEM_H__
#endif