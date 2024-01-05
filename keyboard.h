#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2)

char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1));

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

static inline uint key_getKeyboardID() {
    //TODO 適切な処理をする
    return 0;//###
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

#endif