// for debug
void blink(int n) {
	for (int j = 0; j < n; j++) {
		IJB_led(1);
		for (volatile int i = 0; i < 100000; i++);
		IJB_led(0);
		for (volatile int i = 0; i < 100000; i++);
	}
	for (volatile int i = 0; i < 300000; i++);
}

#ifndef KEYLAYOUT_US
#ifndef KEYLAYOUT_JP_DVORAK
// jp-keyboard
//static const char PS2_KB[]       = "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9  ";
//static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9  ";
// 後ろ-2 長さ128->126
static const char PS2_KB[]       = "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";
// -21、後ろ-2 長さ128->105 us-keyboardが切れない
//static const char PS2_KB[]       = "q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
//static const char PS2_KB_SHIFT[] = "Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";
#else
// jp-keyboard-dvorak
//static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
//static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
#endif
#else
// us-keyboard
//static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9  ";
//static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P~  _\" {+    \n} |        \b  1 47   0.2568   +3-*9  ";
// 後ろ-2
static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9";
static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P_  _\" {+    \n} |        \b  1 47   0.2568   +3-*9";
#endif

#define IME_KEYMAP 1
#define IME_JP_KANA 2
#define IME_VN_TELEX 3

#if LANG == LANG_JP
	#define IME IME_JP_KANA
#endif

// Mongol
#if LANG == LANG_MN
	#define IME IME_KEYMAP
	static const char PS2_KB_ALT[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\xd3', '\xbe', ' ', ' ', ' ', '\xdd', '\xd9', '\xc9', '\xd4', '-', ' ', ' ', '\xc5', '\xd5', '\xc1', '\xd1', '\xa0', '"', ' ', ' ', ' ', 'c', '\xce', '\xdb', '\xc6', ':', ' ', ' ', '\xc8', '\xcc', 'x', 'a', '\xcd', '.', ' ', ' ', ' ', '\xd0', 'p', '\xc3', '_', ',', ' ', ' ', '\xda', 'o', '\xd6', '\xd2', '?', '%', ' ', ' ', '\xc2', '\xdc', '\xcb', '\xc4', '\xc7', 'e', ' ', ' ', '\\', '\xcf', ' ', '\xca', '\xd7', ' ', ' ', ' ', ' ', '\n', '\xd8', ' ', '\xd8', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\xc0', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
	static const char PS2_KB_SHIFT_ALT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\xb3', '1', ' ', ' ', ' ', '\xbd', '\xb9', '\xa9', '\xb4', '2', ' ', ' ', '\xa5', '\xb5', '\xa1', '\xb1', '4', '3', ' ', ' ', ' ', 'C', '\xae', '\xbb', '\xa6', '5', ' ', ' ', '\xa8', '\xac', 'X', 'A', '\xad', '6', ' ', ' ', ' ', '\xb0', 'P', '\xa3', '7', '8', ' ', ' ', '\xba', 'O', '\xb6', '\xb2', '0', '9', ' ', ' ', '\xa2', '\xbc', '\xab', '\xa4', '\xa7', 'E', ' ', ' ', '_', '\xaf', ' ', '\xaa', '\xb7', ' ', ' ', ' ', ' ', '\n', '\xb8', ' ', '\xb8', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
#endif


// Vietnam
#if LANG == LANG_VI
	#if 0
		#define IME IME_KEYMAP
		static const char PS2_KB_ALT[] =       "`             `      q\xc1   zsaw\xc2  cxde\xc4\xc3   vftr\xc8  nbhgy\xc9   mju\xca\xcb  ,kio\xc5\xcc  ./l;p-  \\' \xc6\xa0    \n\xc7 \\        \b  1 47   0.2568   +3-*9";
		static const char PS2_KB_SHIFT_ALT[] = "~             ~      Q\xa1   ZSAW\xa2  CXDE\xa4\xa3   VFTR\xa8  NBHGY\xa9   MJU\xaa\xab  <KIO\xa5\xac  >?L:P~  _\" \xa6\xc0    \n\xa7 |        \b  1 47   0.2568   +3-*9";
	#else
		#define IME IME_VN_TELEX
	#endif
#endif

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2) // kbhitとnkeybuf分
char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！
//#define keybuf ((char*)(ram + OFFSET_RAM_KEYBUF)) // 24560+56
//volatile uint8 keykbhit = 0; // SPC DOWN UP RIGHT LEFT // 28-32
#define keykbhit keybuf[-1]
INLINE void key_kbhit(int n) {
	keykbhit |= 1 << n;
}
INLINE void key_kbhit_rel(int n) {
	keykbhit &= ~(1 << n);
}

static void key_pushc(char c) {
	/*
	if (key_nbuf < PS2_BUF_LEN) { // バッファ一杯ならいれない
		key_buf[key_nbuf++] = c;
	}
	*/
	if (*keybuf < KEY_BUF_LEN) {
		(*keybuf)++;
		keybuf[(uint8)*keybuf] = c;
	}
}
static void key_push(char* s) { // for function keys
	for (;;) {
		char c = *s++;
		if (!c)
			break;
		key_pushc(c);
	}
}

// uart
/*
#define UART_BUF_SIZE 4
uint8 uart_buf[UART_BUF_SIZE];
int16 uart_write = 0;
int16 uart_read = 0;
*/

void USART1_IRQHandler(void) {
	// if (USART1->ISR & USART_ISR_RXNE)  // omit, only recevie
	uint8 c = (uint8)(USART1->RDR);
	if (c == 27) {
		_g.key_flg_esc = 1;
	}
	key_pushc(c);
}

void uart_put(int c) {
	while (!(USART1->ISR & USART_ISR_TXE)); // wait for sent
	USART1->TDR = c;
}

struct keyflg_def key_flg;
uint8 displaymode;


// PS/2 keyboard

uint8 key_kana_buf[2]; // xtsu/ltsu 対応する場合増やす必要あり
char key_kana_nbuf = 0;

//#define PS2_TICK systick
#define PS2_TICK() screen_tick()
#define PS2_TICK_DIF 1

uint16 key_bkt = 0;
uint8 key_data = 0;
char key_nbit = 0;
char key_cnt = 0; // PS2信号の1の数を数える
char key_enable_flg = 0; // varに入れたほうが大きくなる

static inline void key_process(uint8 key) {
	if (key == 0xe1) { // pause (e1, 14, 77, e1, f0, 14, f0, 77) -> F12(=0x07)相当
		key = 0x07;
	}
	if (key > 0xe0 && key != 0xf0) // 制御コード無視、もっと広くてもいいかも
		return;
	uint8 k = 0;
	
	//		xprintf("%02x ", key);
	if (key == 0xe0) {
		key_flg.e0 = 1;
	} else if (key == 0xf0) {
		key_flg.release = 1;
	} else {
		if (key_flg.release) {
			if (key_flg.e0) {
				if (key == 0x75) { // up
					//						keykbhit &= ~(1 << 2);
					key_kbhit_rel(2);
				} else if (key == 0x72) { // down
//						keykbhit &= ~(1 << 3);
					key_kbhit_rel(3);
				} else if (key == 0x6b) { // left
//						keykbhit &= ~(1 << 0);
					key_kbhit_rel(0);
				} else if (key == 0x74) { // right
//						keykbhit &= ~(1 << 1);
					key_kbhit_rel(1);
				} else if (key == 0x11) { // right ALT
					key_flg.alt_r = 0;
				} else if (key == 0x14) { // right CTRL
					key_flg.ctrl_r = 0;
//					} else if (key == 0x1f) { // left WIN == left ALT
//						key_flg.alt_l = 0;
				}
				key_flg.e0 = 0;
			} else {
				if (key == 0x12) { // left SHIFT
					key_flg.shift_l = 0;
				} else if (key == 0x59) { // right SHIFT
					key_flg.shift_r = 0;
				} else if (key == 0x11 || key == 0x67) { // left ALT and  == 無変換 1.2.4b50
					key_flg.alt_l = 0;
				} else if (key == 0x14) { // left CTRL
					key_flg.ctrl_l = 0;
				} else if (key == 0x29) {
//						keykbhit &= ~(1 << 4);
					key_kbhit_rel(4);
				}
			}
			key_flg.release = 0;
		} else {
			if (key_flg.e0) {
				if (key == 0xf0) {
					key_flg.release = 1;
				} else {
					if (key == 0x75) { // up
						k = 30;
						//							keykbhit |= 1 << 2;
						key_kbhit(2);
					} else if (key == 0x72) { // down
						k = 31;
//							keykbhit |= 1 << 3;
						key_kbhit(3);
					} else if (key == 0x6b) { // left
						k = 28;
//							keykbhit |= 1 << 0;
						key_kbhit(0);
					} else if (key == 0x74) { // right
						k = 29;
//							keykbhit |= 1 << 1;
						key_kbhit(1);
					} else if (key == 0x4A) {
						k = '/';
					} else if (key == 0x5A) {
						k = key_flg.shift_r || key_flg.shift_l ? 0x10 : '\n'; // ver1.0.1
					} else if (key == 0x71) {
						k = 0x7f;
					} else if (key == 0x6c) { // home
						k = 0x12;
					} else if (key == 0x69) { // end
						k = 0x17;
					} else if (key == 0x7d) { // page up
						k = 0x13;
					} else if (key == 0x7a) { // page down
						k = 0x14;
					} else if (key == 0x70) { // insert key
						//key_flg.insert = !key_flg.insert;
						k = 0x11;
					} else if (key == 0x11) { // right ALT = kana -> same as left ALT -> 1.2b52 kana廃止
						/*
						if (key_flg.shift_r || key_flg.shift_l) { // SHIFT + right ALT = ins
							key_flg.insert = !key_flg.insert;
						} else {
							key_flg.kana = !key_flg.kana; // right ALT = kana
							key_kana_nbuf = 0;
						}
						*/
						key_flg.alt_r = 1;
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // INS
							key_flg.insert = !key_flg.insert;
							//k = 0x11;
						/* // right ALT = kana 廃止 1.2b52
						} else {
							key_flg.kana = !key_flg.kana;
							key_kana_nbuf = 0;
						*/
						}
					} else if (key == 0x14) { // right CTRL
						key_flg.ctrl_r = 1;
//						} else if (key == 0x1f) { // left WIN == left ALT ver1.2.4b50 x
//							key_flg.alt_l = 1;
					}
					key_flg.e0 = 0;
				}
			} else {
				if (key == 0x12) { // left SHIFT
					key_flg.shift_l = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						key_flg.kana = !key_flg.kana;
						key_kana_nbuf = 0;
					}
				} else if (key == 0x59) { // right SHIFT
					key_flg.shift_r = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						key_flg.kana = !key_flg.kana;
						key_kana_nbuf = 0;
					}
				} else if (key == 0x58) { // caps
					if (key_flg.kana) {
						key_flg.kana = 0; // 0.9.4
						key_kana_nbuf = 0;
					} else {
						key_flg.caps = !key_flg.caps;
//							key_send_caps(key_flg.caps); // 割り込みの関係で微妙
					}
				} else if (key == 0x13) { // kana
					key_flg.kana = !key_flg.kana;
					key_kana_nbuf = 0;
				} else if (key == 0x11 || key == 0x67) { // left ALT and == 無変換 ver1.2.4b50
					key_flg.alt_l = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // INS
						//	key_flg.insert = !key_flg.insert;
						k = 0x11;
					}
				} else if (key == 0x14) { // left CTRL
					key_flg.ctrl_l = 1;
				} else if (key == 0x29) { // space
//						k = 0x20;
					k = key_flg.shift_r || key_flg.shift_l ? 0xe : 0x20; // Shift+Space=空白挿入
//						keykbhit |= 1 << 4;
					key_kbhit(4);
				} else if (key == 0x5A) { // enter
					//k = '\n';
					k = key_flg.shift_r || key_flg.shift_l ? 0x10 : '\n'; // Shift+Enter=行分割 ver1.0.0b14
					key_kana_nbuf = 0; // 1.2.1
				} else if (key == 0x05) { // F1
//						key_push("\030CLS\n"); // 022(8進数) = 0x12
					key_push("\x13\x0c"); // page up, clear after cursor 1.2b13
				} else if (key == 0x06) { // F2
					key_push("\x18LOAD");
				} else if (key == 0x04) { // F3
					key_push("\x18SAVE");
				} else if (key == 0x0c) { // F4
//						key_push("\x18\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
//						key_push("\x18\030LIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
//						key_push("\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除 ->0x0cのみに?
					key_push("\x18\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> 先頭へ、カーソル以降削除 v1.2b42
				} else if (key == 0x03) { // F5
					key_push("\x18RUN\n");
				} else if (key == 0x0B) { // F6
					key_push("\x18?FREE()\n");
				} else if (key == 0x83) { // F7
					key_push("\x18OUT0\n"); // 0.9.4
				} else if (key == 0x0A) { // F8
					key_push("\x18VIDEO1\n"); // 0.9.6
				} else if (key == 0x01) { // F9
					key_push("\x18\014FILES"); // 1.0.0b14 // \014(8進数) = 0x0c // 1.2b35 \nなくした
				} else if (key == 0x09) { // F10
					key_push("\x18SWITCH\n"); // 1.2b32
//						key_pushc(0x18); // 0.9.9
				} else if (key == 0x78) { // F11
					key_pushc(0x0c); // 0.9.9 // カーソル以下クリア
				} else if (key == 0x07) { // F12 // numlockの0xe1ついか ver1.2b42
//						key_pushc(0x10); // 1.0.0b14 // shift+enter
					// F7-F12:  83 0A 01 09 78 07
				} else if (key == 0x76) { // ESC
					k = 0x1b;
					key_kana_nbuf = 0; // 1.2.1
				} else if (key == 0x0d) { // TAB
					key_pushc('\t');
				} else if (key == 0x7e) { // Scroll Lcok 無視、小型キーボードで起動時に送られてくる
				} else if (key == 0x77) { // num lock 無視
				} else if (key == 0x14) { // 左CTRL 無視
#ifndef KEYLAYOUT_US
				} else if (key == 0x0e) { // 半角/全角 無視
#endif
				} else if (key == 0x64) { // 変換 無視
				} else if (key == 0x67) { // 無変換 無視
				} else {
					int shift = key_flg.shift_r || key_flg.shift_l;
//						if (key < 0x80) {
					if (key < 0x80 - 2) { // 1.2b6
//						if (key >= 21 && key < 0x80 - 2) { // 1.2b6 x us非対応
						if (key_flg.alt_l || key_flg.alt_r) { // 1.2b52
//							if (key_flg.alt_l) { // やっぱり左だけにする 1.1b7 -> 1.2b52 両ALT一緒に
							// shift でかえる 0-F + G-V で32キャラ x 2
							// W-Zを0-3と同様とする ver 1.2.4b50
							k = PS2_KB[key];
							//			_printf("%x %d\n", key, k);
							if (k >= '0' && k <= '9') {
								k = k - '0' + (shift ? 0x80 : 0xe0);
							} else if (k >= 'w' && k <= 'z') { // wxyz ver1.2.4b50
								k = k - 'w' + (shift ? 0x80 : 0xe0);
							} else if (k >= 'a' && k <= 'v') {
								k = k - ('a' - 10) + (shift ? 0x80 : 0xe0);
							} else if (k == '[') { // ver 1.2b5
								k = '_';
							} else if (k == ']') { // ver 1.2b5
								k = key_flg.kana ? 0xa0 : '\\';
							} else {
								k = 0;
							}
						} else if (!key_flg.kana) {
							if (shift) {
								k = PS2_KB_SHIFT[key];
								if (!key_flg.caps) {
									if (k >= 'A' && k <= 'Z') {
										k += 'a' - 'A';
									}
								}
							} else {
								k = PS2_KB[key];
								if (!key_flg.caps) {
									if (k >= 'a' && k <= 'z') {
										k += 'A' - 'a';
									}
								}
							}
						} else { // kana
#if IME == IME_KEYMAP
							if (shift) {
								k = PS2_KB_SHIFT_ALT[key];
								/*
								if (!key_flg.caps) {
									if (k >= 'A' && k <= 'Z') {
										k += 'a' - 'A';
									}
								}
								*/
							} else {
								k = PS2_KB_ALT[key];
								/*
								if (!key_flg.caps) {
									if (k >= 'a' && k <= 'z') {
										k += 'A' - 'a';
									}
								}
								*/
							}
#elif IME == IME_VN_TELEX
							if (shift) {
								k = PS2_KB_SHIFT[key];
							} else {
								k = PS2_KB[key];
							}
							if (key_kana_nbuf == 1) {
								int bk = key_kana_buf[0];
								// 小文字の場合
								if (bk == 'a' || bk == 'i' || bk == 'u' || bk == 'e' || bk == 'o' || bk == 'y' || (bk >= 0xc1 && bk <= 0xc7 && bk != 0xc5) ||
									bk == 'A' || bk == 'I' || bk == 'U' || bk == 'E' || bk == 'O' || bk == 'Y' || (bk >= 0xa1 && bk <= 0xa7 && bk != 0xa5)) { // 母音
									if (k == 's' || k == 'S') { // '
										k = 0xcb;
									} else if (k == 'f' || k == 'F') { // `
										k = 0xc8;
									} else if (k == 'r' || k == 'R') { // ?
										k = 0xc9;
									} else if (k == 'x' || k == 'X') { // ~
										k = 0xca;
									} else if (k == 'j' || k == 'J') { // .
										k = 0xcc;
									} else if (bk == 'a' && k == 'w') { // au
										key_pushc(0x08); // backspace
										k = 0xc1;
									} else if (bk == 'a' && k == 'a') { // a^
										key_pushc(0x08); // backspace
										k = 0xc2;
									} else if (bk == 'e' && k == 'e') { // e^
										key_pushc(0x08); // backspace
										k = 0xc3;
									} else if (bk == 'u' && k == 'w') { // u'
										key_pushc(0x08); // backspace
										k = 0xc6;
									} else if (bk == 'o' && k == 'w') { // o'
										key_pushc(0x08); // backspace
										k = 0xc7;
									} else if (bk == 'A' && (k == 'w' || k == 'W')) { // Au
										key_pushc(0x08); // backspace
										k = 0xa1;
									} else if (bk == 'A' && (k == 'a' || k == 'A')) { // A^
										key_pushc(0x08); // backspace
										k = 0xa2;
									} else if (bk == 'E' && (k == 'e' || k == 'E')) { // E^
										key_pushc(0x08); // backspace
										k = 0xa3;
									} else if (bk == 'U' && (k == 'w' || k == 'W')) { // U'
										key_pushc(0x08); // backspace
										k = 0xa6;
									} else if (bk == 'O' && (k == 'w' || k == 'W')) { // O'
										key_pushc(0x08); // backspace
										k = 0xa7;
									}
								} else if (bk == 'd' && k == 'd') { // dd
									key_pushc(0x08); // backspace
									k = 0xc5;
								} else if (bk == 'D' && (k == 'd' || k == 'D')) { // dd
									key_pushc(0x08); // backspace
									k = 0xa5;
								}
							}
							// http://www.vntyping.com/
							key_kana_buf[0] = k;
							key_kana_nbuf = 1;
							
#elif IME == IME_JP_KANA
							if (shift) {
								k = PS2_KB_SHIFT[key];
								if (!key_flg.caps) {
									if (k >= 'A' && k <= 'Z') {
										k += 'a' - 'A';
									}
								}
							} else {
								k = PS2_KB[key];
								int m = -1;
								// 0xb1 a
								if (k == 'a') {
									m = 0;
								} else if (k == 'i') {
									m = 1;
								} else if (k == 'u') {
									m = 2;
								} else if (k == 'e') {
									m = 3;
								} else if (k == 'o') {
									m = 4;
								} else if (k == '.') {
									k = 0xa1;
								} else if (k == ',') {
									k = 0xa4;
								} else if (k == '-') {
									k = 0xb0;
								} else if (k == '[') {
									k = 0xa2;
								} else if (k == ']') {
									k = 0xa3;
								} else if (k == '/') {
									k = 0xa5;
								} else if (k == '\\') {
									k = 0xa0; // yen mark
								} else if (k >= 'a' && k <= 'z') {
									if (key_kana_nbuf == 2) {
										//k = key_kana_buf[0];
										key_kana_buf[0] = key_kana_buf[1];
										key_kana_buf[1] = k;
										k = 0;
										/*
										key_kana_nbuf = 0;
										k = 0;
										*/
									} else if (key_kana_nbuf == 1) {
										/*if (key_kana_buf[0] == 'n') { // ん の簡易入力 復活 1.2.1 -> bug
											if (k == 'n') {
												key_kana_nbuf = 0;
											} else {
												key_kana_buf[0] = k;
											}
											k = 0xdd;
										} else */if (key_kana_buf[0] == k) {
											//												k = 0xaf;
											if (k == 'n') {
												k = 0xdd;
												key_kana_nbuf = 0;
											} else {
												k = 0xaf;
											}
										} else {
											key_kana_buf[(int)key_kana_nbuf] = k;
											key_kana_nbuf++;
											k = 0;
										}
									} else {
										key_kana_buf[(int)key_kana_nbuf] = k;
										key_kana_nbuf++;
										k = 0;
									}
								}
								if (m >= 0) {
									if (key_kana_nbuf == 0) {
										k = 0xb1 + m;
									} else if (key_kana_nbuf == 1) {
										switch (key_kana_buf[0]) {
										  case 'k':
											k = 0xb6 + m;
											break;
										  case 's':
											k = 0xbb + m;
											break;
										  case 't':
											k = 0xc0 + m;
											break;
										  case 'n':
											k = 0xc5 + m;
											break;
										  case 'h':
											k = 0xca + m;
											break;
										  case 'm':
											k = 0xcf + m;
											break;
										  case 'y':
											if (m == 1) {
												k = 0xb2;
											} else if (m == 3) {
												k = 0xb4;
											} else {
												k = 0xd4 + m / 2;
											}
											break;
										  case 'r':
											k = 0xd7 + m;
											break;
										  case 'w':
											if (m == 0) {
												k = 0xdc;
											} else if (m == 1) {
												key_pushc(0xb3);
												k = 0xa8;
											} else if (m == 2) {
												k = 0xb3;
											} else if (m == 3) {
												key_pushc(0xb3);
												k = 0xaa;
											} else if (m == 4) {
												k = 0xa6;
											}
											break;
										  case 'l':
										  case 'x':
											k = 0xa7 + m;
											break;
										  case 'g':
											key_pushc(0xb6 + m);
											k = 0xde;
											break;
										  case 'z':
											key_pushc(0xbb + m);
											k = 0xde;
											break;
										  case 'j': // じゃじじゅじぇじょ
											key_pushc(0xbc);
											key_pushc(0xde);
											if (m == 0) {
												k = 0xac;
											} else if (m == 1) {
												k = 0;
											} else if (m == 2) {
												k = 0xad;
											} else if (m == 3) {
												k = 0xaa;
											} else if (m == 4) {
												k = 0xae;
											}
											break;
										  case 'f': // ふぁふぃふふぇふぉ
											key_pushc(0xcc);
											if (m == 2) {
												k = 0;
											} else {
												k = 0xa7 + m;
											}
											break;
										  case 'v': // ヴぁヴぃヴヴぇヴぉ
											key_pushc(0xb3);
											key_pushc(0xde); // 濁点 1.2.1
											if (m == 2) {
												k = 0;
											} else {
												k = 0xa7 + m;
											}
											break;
										  case 'd':
											key_pushc(0xc0 + m);
											k = 0xde;
											break;
										  case 'b':
											key_pushc(0xca + m);
											k = 0xde;
											break;
										  case 'p':
											key_pushc(0xca + m);
											k = 0xdf;
											break;
										}
										key_kana_nbuf = 0;
									} else if (key_kana_nbuf == 2) {
										char k0 = key_kana_buf[0];
										char k1 = key_kana_buf[1];
										if ((k0 == 'c' || k0 == 's') && k1 == 'h') { // ちゃちちゅちぇちょ、しゃししゅしぇしょ(1.2.1)
											key_pushc(k0 == 'c' ? 0xc1 : 0xbc);
											if (m == 0) {
												k = 0xac;
											} else if (m == 1) {
												k = 0;
											} else if (m == 2) {
												k = 0xad;
											} else if (m == 3) {
												k = 0xaa;
											} else if (m == 4) {
												k = 0xae;
											}
										} else if (k0 == 't' && k1 == 's' && m == 2) {
											k = 0xc2;
										} else if ((k0 == 'l' || k0 == 'x') && k1 == 't' && m == 2) { // ltu xtu っ
											k = 0xaf;
										} else if ((k0 == 't' || k0 == 'd') && k1 == 'h') {
											key_pushc(0xc3);
											if (k0 == 'd')
												key_pushc(0xde); // 濁点
											if (!(m & 1)) {
												k = 0xac + m / 2;
											} else {
												k = 0xa7 + m;
											}
										} else if (k1 == 'y') {
											switch (k0) {
											  case 'k': // きゃきゅきょ
												key_pushc(0xb6 + 1);
												break;
											  case 's':
												key_pushc(0xbb + 1);
												break;
											  case 't':
												key_pushc(0xc0 + 1);
												break;
											  case 'n':
												key_pushc(0xc5 + 1);
												break;
											  case 'h':
												key_pushc(0xca + 1);
												break;
											  case 'f': // 1.2.1 ふゃふゅふょ
												key_pushc(0xca + 2);
												break;
											  case 'j': // 1.2.1 じゃじゅじょ
												key_pushc(0xbc);
												key_pushc(0xde);
												break;
											  case 'm':
												key_pushc(0xcf + 1);
												break;
											  case 'r':
												key_pushc(0xd7 + 1);
												break;
											  case 'g':
												key_pushc(0xb6 + 1);
												key_pushc(0xde);
												break;
											  case 'z':
												key_pushc(0xbb + 1);
												key_pushc(0xde);
												break;
											  case 'd':
												key_pushc(0xc0 + 1);
												key_pushc(0xde);
												break;
											  case 'b':
												key_pushc(0xca + 1);
												key_pushc(0xde);
												break;
											  case 'p':
												key_pushc(0xca + 1);
												key_pushc(0xdf);
												break;
											}
											if (!(m & 1)) {
												k = 0xac + m / 2;
											} else {
												k = 0xa7 + m;
											}
										}
										key_kana_nbuf = 0;
									}
								}
							}
#endif
						}
					}
				}
			}
			if (k) {
				key_pushc(k);
				if (k == 27) {
					_g.key_flg_esc = 1;
				}
			}
		}
	}
}

#define PS2KB_CLK	4 // LPC0_7 wakeup KBD1
#define PS2KB_DATA	3 // LPC0_3 KBD2

#define PS2KB_CLK_VAL (GPIOB->IDR & (1 << PS2KB_CLK))
#define PS2KB_DATA_VAL (GPIOB->IDR & (1 << PS2KB_DATA))

#define SET_PS2KB_CLK_VAL() (GPIOB->ODR |= (1 << PS2KB_CLK))
#define SET_PS2KB_DATA_VAL() (GPIOB->ODR |= (1 << PS2KB_DATA))
#define RESET_PS2KB_CLK_VAL() (GPIOB->ODR &= ~(1 << PS2KB_CLK))
#define RESET_PS2KB_DATA_VAL() (GPIOB->ODR &= ~(1 << PS2KB_DATA))

#define INPUT_PS2KB_CLK() (GPIOB->MODER &= ~(1 << (2 * PS2KB_CLK)))
#define INPUT_PS2KB_DATA() (GPIOB->MODER &= ~(1 << (2 * PS2KB_DATA)))
#define OUTPUT_PS2KB_CLK() (GPIOB->MODER |= (1 << (2 * PS2KB_CLK)))
#define OUTPUT_PS2KB_DATA() (GPIOB->MODER |= (1 << (2 * PS2KB_DATA)))

void putn(int n) {
	int a = 1000000;
	for (int i = 0; i < 7; i++) {
		int m = n / a;
		uart_putc(m + '0');
		n %= a;
		a /= 10;
	}
	uart_putc('\n');
}

void EXTI4_15_IRQHandler(void) {
//	if (!(EXTI->PR & (1 << 4)))
//		return;
	EXTI->PR |= 1 << 4; // 割り込みペンディングリセット
	
	int ps2data = PS2KB_DATA_VAL;
	
	int tick = PS2_TICK();
	if (tick - key_bkt > PS2_TICK_DIF) {
		key_data = key_nbit = key_cnt = 0;
	}
	key_bkt = tick;
	
	if (!key_enable_flg)
		return;
	
	if (ps2data) {
		if (key_nbit > 0 && key_nbit < 9) {
			key_data |= 1 << (key_nbit - 1);
		}
		key_cnt++;
	}
	key_nbit++;
	
	if (key_nbit == 11) {
		if (!(key_cnt & 1)) {
			key_process((uint8)key_data);
		}
		key_data = 0;
		key_nbit = 0;
		key_cnt = 0;
	}
}


inline static void key_init() {
	// setting
	GPIOB->PUPDR |= GPIO_PUPDR_PUP << (2 * PS2KB_CLK); // PB4 == KBD1 PS2_CLK
	GPIOB->PUPDR |= GPIO_PUPDR_PUP << (2 * PS2KB_DATA); // PB3 = KBD2 PS2_DATA
	
	for (volatile int i = 0; i < 5 * 1000000; i++);
	
	// PB4 割り込み
	SYSCFG->EXTICR2 = 1; // EXTI4 = PB
	EXTI->RTSR |= 1 << PS2KB_CLK; // 立ち上がり
	EXTI->IMR = 1 << PS2KB_CLK; // enable interrupt
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_SetPriority(EXTI4_15_IRQn, 0);
}


INLINE void key_enable(uint8 b) {
	key_enable_flg = b;
}
//void uart_checker();

inline int key_btn(int n) {
	//	uart_checker();
	return (keykbhit & (1 << (n - 28))) != 0;
}

//char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！

int key_getKey() {
//	return uart_get();
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

/*
0xDE 0x07 {bit0, bit1, bit2} キーボードのLEDつける

参考
http://ioiodesu.web.fc2.com/PS2/PS2.HTML

キーボードはCLKを10mS以下の間隔で監視しシステムからのコマンドに応答する。
キーボードはシステムからのコマンドに20mS以内に応答する。
システムはキーボードの応答が無効またはエラーなら再送信する
*/
#define PS2_TIMEOUT_BIT1 14 // LPC1114:10 こっちはすぐ？
#define PS2_TIMEOUT_BIT2 20 // LPC1114:15, 32000あれば十分だろう

static void key_send(int data) {
	int timeout;
	
	EXTI->IMR &= ~(1 << PS2KB_CLK); // disable interrupt
	
	// wait 100usec
//	for (volatile int i = 0; i < 5 * 100000; i++);
	
	// 両方 pull upでhigh
	OUTPUT_PS2KB_CLK(); // キーボード通信禁止
	RESET_PS2KB_CLK_VAL();
	
//	blink(1);
	// wait 100usec
	for (volatile int i = 0; i < 5 * 100000; i++);
	
//	blink(1);
//	for (int i = 0; i < 10; i++) video_waitSync(); // 100usec wait
	
	OUTPUT_PS2KB_DATA();
	RESET_PS2KB_DATA_VAL(); // start bit = 0
	INPUT_PS2KB_CLK(); // キーボード通信許可
	
	for (timeout = 0; !PS2KB_CLK_VAL; timeout++) {
		if (timeout >> PS2_TIMEOUT_BIT1) {
			return;
		}
	}
	for (timeout = 0; PS2KB_CLK_VAL; timeout++) {
		if (timeout >> PS2_TIMEOUT_BIT2) { // clock によってかえるべき？
//			xprintf("timeout!\n");
			blink(2);
			return;
		}
	}
//	t3 = timeout; // 最大で7200くらい
	
	int cnt = 0;
	for (int i = 0; i < 8; i++) {
		int bit = data & (1 << i);
		if (bit) {
			SET_PS2KB_DATA_VAL();
			cnt++;
		} else {
			RESET_PS2KB_DATA_VAL();
		}
		while (!PS2KB_CLK_VAL);
		while (PS2KB_CLK_VAL);
	}
	if (cnt & 1) {
		RESET_PS2KB_DATA_VAL();
	} else {
		SET_PS2KB_DATA_VAL();
	}
	while (!PS2KB_CLK_VAL);
	while (PS2KB_CLK_VAL);
	
	SET_PS2KB_DATA_VAL(); // stop bit = 1
	while (!PS2KB_CLK_VAL);
	while (PS2KB_CLK_VAL);
	
	INPUT_PS2KB_DATA();
	while (!PS2KB_DATA_VAL); // wait data
	while (!PS2KB_CLK_VAL); // wait clk
	
	// 開放待ち
	while (PS2KB_DATA_VAL); // wait data
	while (PS2KB_CLK_VAL); // wait clk
	
	EXTI->IMR |= 1 << PS2KB_CLK; // enable interrupt
}

static inline void key_send_reset() {
	key_send(0xff); // reset
	INPUT_PS2KB_CLK();
	INPUT_PS2KB_DATA();
	EXTI->IMR = 1 << PS2KB_CLK; // enable interrupt
}

INLINE void uart_init() {
	_g.uartmode = 2;
	/*
#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
	*/
	
	// USART1 PA9/PA10
	GPIOA->AFRH |= (1 << (4 * (9 - 8))) | (1 << (4 * (10 - 8))); // PA9 and PA10 as AF1 == USART
	GPIOA->MODER |= (2 << (2 * 9)) | (2 << (2 * 10)); // PA9, PA10 as alternatefunction(AF)
	
	// USART1 PA14/PA15
//	GPIOA->AFRH |= (1 << (4 * (14 - 8))) | (1 << (4 * (15 - 8))); // PA14 and PA15 as AF1 == USART
//	GPIOA->MODER |= (2 << (2 * 14)) | (2 << (2 * 15)); // PA14, PA15 as alternatefunction(AF)
	
	USART1->BRR = 48000000 / 115200; // 48MHz, 115200bps
//	USART1->BRR = 48000000 / 9600; // 48MHz, 9600bps
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 1);
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
}

inline void IJB_uart(int txd, int rxd) {
//	_g.uartmode = txd;
//	_g.uartmode_rxd = rxd;
}

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
	uart_put(c);
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
	screen_putc(c);
}

// basic interface
inline int stopExecute() {
	return _g.key_flg_esc;
}
