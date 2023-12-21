
static inline uint key_getKeyboardID() {
	return 0;
}
void key_send_reset() {
}

struct keyflg_def key_flg;
int displaymode;

inline static void key_init() {
}


INLINE void key_enable(uint8 b) {
//	key_enable_flg = b;
}
//void uart_checker();

inline int key_btn(int n) {
	//	uart_checker();
	//	return (keykbhit & (1 << (n - 28))) != 0;
	return 0;
}

char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！

int key_getKey() {
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	(*keybuf)--;
	for (uint8 i = 0; i < *keybuf; i++)
		keybuf[i + 1] = keybuf[i + 2];
	return res;
	return 0;
}
void key_clearKey() {
	//	keykbhit = 0; // ver1.2b5 clear しないように変更
	*keybuf = 0;
}


INLINE void uart_init() {
	/*
#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
	*/
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
//	_g.uartmode = txd;
//	_g.uartmode_rxd = rxd;
}

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
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

/*inline*/ void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
	if (n == 0) {
		n = 115200;
	} else if (n == -1) {
		n = 57600;
	} else if (n == -2) {
		n = 38400;
	}
//	UARTInit(n);
}

STATIC void put_chr(char c) {
	/*
	if (_g.uartmode > 0) { // 1.0.2b12 uartを先に
		uart_putc(c);
	}
	*/
	screen_putc(c);
}

// basic interface
inline int stopExecute() {
	return _g.key_flg_esc;
}
