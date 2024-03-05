#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2)

#define DEFAULT_UARTMODE_TXD 2	// txd 0:disable, 1:only text, 2:with ctrl, +4:echo back mode, +8:画面表示オフ(PRINTやSyntax errorなどのメッセージがオフになり、キーボード入力は表示されたまま)
#define DEFAULT_UARTMODE_RXD 1	// rxd 0:disable, 1:enable, 2:ignore esc mode, 4:CR mode, 6:ignore esc & CR mode (auto comment mode??) // -> keyboard_ps2.h
#define UART_ID uart0
#define UART_IRQ UART0_IRQ

char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1));
struct keyflg_def key_flg;
extern uint8_t keycode2ascii[128][4];
//TODO? フラッシュに書き込んで保存する？
bool kbd_mode = 0;

static inline uint key_getKeyboardID() {
    return kbd_mode;
}

void set_keymap(uint8_t keymap[][4]) {
    memcpy(keycode2ascii, keymap, sizeof(keycode2ascii));
}

INLINE void IJB_kbd(uint mode) {
    if (mode == 0) { //US
        set_keymap(keycode_to_ascii_us);
    } else { //JA
        mode = 1;
        set_keymap(keycode_to_ascii_ja);
    }
    kbd_mode = mode;
}

INLINE void uart_init_IJ() {
    // #if DEFAULT_UARTMODE_TXD != 0
    _g.uartmode_txd = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
    // #endif
    _g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
    irq_set_enabled(UART_IRQ, _g.uartmode_rxd & 1);
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
    _g.uartmode_txd = txd;
    _g.uartmode_rxd = rxd;
}

//TODO キャリッジリターンをする状態になっているが、大丈夫か確認する(実機ではキャリッジリターンしていない)
static inline void uart_putc(char c) {
    if ((_g.uartmode_txd & 3) == 3) { // 1.3b2
        if (c == '\n') {
            uart_putc('\r');
        }
    }
    putchar(c);
}

STATIC void put_chr(char c) {
    if (_g.uartmode_txd & 3) { // 1.3b2
        uart_putc(c);
    }
    if (!(_g.uartmode_txd & 8)) { // 1.3b11
        screen_putc(c);
    }
}

void key_pushc(char c) {
    /*
    if (key_nbuf < key_BUF_LEN) { // バッファ一杯ならいれない
        key_buf[key_nbuf++] = c;
    }
    */
    if (*keybuf < KEY_BUF_LEN) {
        (*keybuf)++;
        keybuf[(uint8)*keybuf] = c;
    }
}
void key_push(char* s) { // for function keys
    for (;;) {
        char c = *s++;
        if (!c)
            break;
        key_pushc(c);
    }
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
    //	key_kbhit = 0; // ver1.2b5 clear しないように変更
    *keybuf = 0;
}

void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
    if (n == 0) {
        n = 115200;
    } else if (n == -1) {
        n = 57600;
    } else if (n == -2) {
        n = 38400;
    }
    //	UARTInit(n);
}

INLINE int stopExecute() {
    return _g.key_flg_esc;
}

#endif