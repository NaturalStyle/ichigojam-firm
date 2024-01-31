#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2)

char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1));
struct keyflg_def key_flg;

static inline uint key_getKeyboardID() {
    //TODO 適切な処理をする
    return 0;//###
}

INLINE void IJB_kbd(uint mode) {
    //TODO 実装する
}

//TODO いつ呼ばれるか確認する
static inline void uart_putc(char c) {
    printf("uart_putc");
    putchar(c);
}

STATIC void put_chr(char c) {
    if (_g.uartmode_txd > 0) { // 1.0.2b12 uartを先に
        uart_putc(c);
    }
    screen_putc(c);
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

INLINE int stopExecute() {
    return _g.key_flg_esc;
}

#endif