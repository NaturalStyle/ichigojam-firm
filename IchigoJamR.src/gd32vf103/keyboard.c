//* 
//* created by yrm (c) 2016
//* 
// #include "ichigolatte.h"
#include <sys/types.h>

void jmp_fromint(int);
void key_setESC();

extern const uint8_t g_scancode_us[];
extern const uint8_t g_scancodeS_us[];
extern const uint8_t g_scancodeE_us[];

extern const uint8_t g_scancode_jp[];
extern const uint8_t g_scancodeS_jp[];
extern const uint8_t g_scancodeE_jp[];

#define KBD_JP

#if defined(KBD_JP)
uint8_t* g_scancode = g_scancode_jp;
uint8_t* g_scancodeS = g_scancodeS_jp;
uint8_t* g_scancodeE = g_scancodeE_jp;
/*
#define g_scancode g_scancode_jp
#define g_scancodeS g_scancodeS_jp
#define g_scancodeE g_scancodeE_jp
*/
#else
uint8_t* g_scancode = g_scancode_us;
uint8_t* g_scancodeS = g_scancodeS_us;
uint8_t* g_scancodeE = g_scancodeE_us;
/*
#define g_scancode g_scancode_us
#define g_scancodeS g_scancodeS_us
#define g_scancodeE g_scancodeE_us
*/
#endif

uint8_t g_keymap = 0;

void keyboard_setKeymap(int n) {
    if (n == 0) {
        g_keymap = 0;
        g_scancode = g_scancode_us;
        g_scancodeS = g_scancodeS_us;
        g_scancodeE = g_scancodeE_us;
    } else {
        n = 1;
        g_keymap = n;
        g_scancode = g_scancode_jp;
        g_scancodeS = g_scancodeS_jp;
        g_scancodeE = g_scancodeE_jp;
    }
}
int keyboard_getKeymap() {
    return g_keymap;
}

/*
// defined by ctype.h
uint8_t tolower(uint8_t c) {
	if (c >= 'A' && c <= 'Z') {
		//return c & 0b1011111; // 速度一緒 なぜか小文字のほうが速い
		return c - ('A' - 'a');
	}
	return c;
}
uint8_t toupper(uint8_t c) {
	if (c >= 'a' && c <= 'z') {
		return c & 0b1011111; // 速度一緒 なぜか小文字のほうが速い
//		return c + ('A' - 'a');
	}
	return c;
}
*/

extern struct{
    uint8_t  bit;
    uint16_t data;
} g_frames[];

extern struct{
    uint8_t r;
    uint8_t w;
    uint8_t waiting_for_r;
}g_rw;

#define RW_INC(rw) (rw = (rw+1)&0b11111)
#define WFR_NO   0
#define WFR_FULL 1
#define WFR_ERR  2



enum {
    KB_SL   = 0b00000001,
    KB_CL   = 0b00000010,
    KB_AL   = 0b00000100,
    KB_OL   = 0b00001000,
    KB_SR   = 0b00010000,
    KB_CR   = 0b00100000,
    KB_AR   = 0b01000000,
    KB_OR   = 0b10000000,
};

enum {
    KB_NK   = 0b00000001,
    KB_CK   = 0b00000010,
    KB_SK   = 0b00000100,
};

static uint8_t g_exkey = 0;
static uint8_t g_lkkey = KB_CK; // caps lock on

//static uint8_t g_keybuf[32] = {};    // if change size also modify RW_INC
static uint8_t g_keybuf[200] = {};    // if change size also modify RW_INC

static struct {
    uint8_t r;
    uint8_t w;
    uint8_t waiting_for_r;
} g_kbrw = { 0, 0, 0 };

#define KBRW_INC(rw) (rw = (rw+1)&0b11111)
#define KBWFR_NO   0
#define KBWFR_FULL 1

volatile uint8_t g_jmp_code = 0x00;
volatile uint8_t g_jmp_mute = 0;

int keyboard_enqKeyBuf2(uint8_t k) { // to check codes
    //return;
    if (k >= 0x10) {
        keyboard_enqKeyBuf2(k & 0xf);
        k >>= 4;
    }
    k = k < 10 ? k + '0' : k - 10 + 'A';

    if (!g_kbrw.waiting_for_r) {
        g_keybuf[g_kbrw.w] = k;
        KBRW_INC(g_kbrw.w);
        g_kbrw.waiting_for_r = (g_kbrw.w == g_kbrw.r) ? KBWFR_FULL : KBWFR_NO;
        return 1;
    }
    return 0;
}

int keyboard_enqKeyBufWithoutKana(uint8_t k) {
    if (k && !g_kbrw.waiting_for_r) {
        g_keybuf[g_kbrw.w] = k;
        KBRW_INC(g_kbrw.w);
        g_kbrw.waiting_for_r = (g_kbrw.w == g_kbrw.r) ? KBWFR_FULL : KBWFR_NO;
        return 1;
    }
    return 0;
}

int keyboard_enqKeyBuf(uint8_t k);

void keyboard_clearQueue() {
    g_kbrw.r = g_kbrw.w = g_kbrw.waiting_for_r = 0;
}
void keyboard_enqChars(const char* s) {
    for (;;) {
        uint8_t c = (uint8_t)*s++;
        if (!c) {
            break;
        }
        keyboard_enqKeyBuf(c);
    }
}

void keyboard_enqFunction(uint8_t k) {
    if (k == 0xF1) {
        keyboard_enqChars("\x13\x0c");
    } else if (k == 0xF2) {
        keyboard_enqChars("\x18LOAD");
    } else if (k == 0xF3) {
        keyboard_enqChars("\x18SAVE");
    } else if (k == 0xF4) {
        keyboard_enqChars("\x18\x0cLIST\n");
    } else if (k == 0xF5) {
        keyboard_enqChars("\x18RUN\n");
    } else if (k == 0xF6) {
        keyboard_enqChars("\x18?FREE()\n");
    } else if (k == 0xF7) {
        keyboard_enqChars("\x18OUT0\n");
    } else if (k == 0xF8) {
        keyboard_enqChars("\x18VIDEO1\n");
    } else if (k == 0xF9) {
        keyboard_enqChars("\x18\014FILES\n");
    } else if (k == 0xFA) {
        keyboard_enqChars("\x18SWITCH\n");
    } else if (k == 0xFB) {
        //keyboard_enqChars("SWITCH\n");
    } else if (k == 0xFC) {
        //keyboard_enqChars("SWITCH\n");
    }
}

uint8_t _g_key_kana = 0;

uint8_t _g_key_kana_buf0;
uint8_t _g_key_kana_buf1;
#define key_kana_buf0 _g_key_kana_buf0 // _g.key_kana_buf_0
#define key_kana_buf1 _g_key_kana_buf1 // _g.key_kana_buf_0

#include "../romajikana.h"

int keyboard_enqKeyBuf(uint8_t k) {
    if (k == 0) {
        return 0;
    }
    // for IchigoJam
    if (k == 0x1b) {
        key_setESC();
    }
	// if (key_flg.kana) { // 1.3b2
    if (_g_key_kana) {
		k = romajikana_input(k);
	}
    if (k && !g_kbrw.waiting_for_r) {
        g_keybuf[g_kbrw.w] = k;
        KBRW_INC(g_kbrw.w);
        g_kbrw.waiting_for_r = (g_kbrw.w == g_kbrw.r) ? KBWFR_FULL : KBWFR_NO;
        return 1;
    }
    return 0;
}

uint8_t keyboard_deqKeyBuf(void) {
    if (g_kbrw.r == g_kbrw.w && g_kbrw.waiting_for_r == KBWFR_NO) {
        return 0x00;
    }
    
    uint8_t k = g_keybuf[g_kbrw.r];
    
    KBRW_INC(g_kbrw.r);
    g_kbrw.waiting_for_r = KBWFR_NO;
    
    return k;
}

void key_setBtn(int n, int m); // n:28-32 // add by @taisukef

void put_num(int n); // for debug

void keyboard_scan(void) {
    if (g_rw.waiting_for_r == WFR_ERR) {
        // reset frames
        while(g_rw.r != (g_rw.w)){
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
        }
        
        g_frames[g_rw.w].bit = g_frames[g_rw.w].data = 0;
        g_rw.waiting_for_r = WFR_NO;
        return;
    }
    
    if(g_rw.r == g_rw.w && g_rw.waiting_for_r == WFR_NO) return;
    
    if(g_jmp_mute) return;

    register uint8_t r = g_rw.r;
    register uint8_t c = (g_frames[r].data>>1)&0xff;
    //keyboard_enqKeyBuf2(c); // check keboard scan code

    if(c == 0xE1) {
        if(RW_INC(r) == g_rw.w) return;
        if(RW_INC(r) == g_rw.w) return;
        
        // discard data
        g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
        RW_INC(g_rw.r);
        g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
        RW_INC(g_rw.r);
        g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
        RW_INC(g_rw.r);
    } else if (c == 0xE0) {
        if (RW_INC(r) == g_rw.w) return;
        c = (g_frames[r].data>>1)&0xff;
        
        if (c == 0xF0) {  // break code
            if(RW_INC(r) == g_rw.w) return;
            c = (g_frames[r].data>>1)&0xff;

            // for IchigoJam
            if (c == 107) {
                key_setBtn(28, 0);
            } else if (c == 116) {
                key_setBtn(29, 0);
            } else if (c == 117) {
                key_setBtn(30, 0);
            } else if (c == 114) {
                key_setBtn(31, 0);
            }
            
            if(c == 0x14){
                g_exkey &= ~KB_CR;
            }else
            if(c == 0x11){
                g_exkey &= ~KB_AR;
            }
            
            // discard data
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
        } else {
            // for IchigoJam
            if (c == 107) {
                key_setBtn(28, 1);
            } else if (c == 116) {
                key_setBtn(29, 1);
            } else if (c == 117) {
                key_setBtn(30, 1);
            } else if (c == 114) {
                key_setBtn(31, 1);
            }

            if(c == 0x14){
                g_exkey |= KB_CR;
            }else
            if (c == 0x11) {
                g_exkey |= KB_AR;
                if (g_exkey & (KB_CL | KB_CR)) {
                    keyboard_enqKeyBuf(0x11); // inesrt
                }
            } else {
                // keyboard_enqKeyBuf2(c);
                
                register uint8_t k = g_scancodeE[c&0x7f];
                
                // if(g_exkey&(KB_CL|KB_CR) && g_exkey&(KB_AL|KB_AR) && k==0x7f) _reset();
                
                if(g_jmp_code && (k == g_jmp_code)){
                    jmp_fromint(1);
                } else {   
                    keyboard_enqKeyBuf(k);
                }
            }
            
            // discard data
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
        }
    } else {
        if (c == 0xF0) {  // break code
            if(RW_INC(r) == g_rw.w) return;
            c = (g_frames[r].data>>1)&0xff;

            //put_num(c);
            // for IchigoJam
            if (c == 41) {
                key_setBtn(32, 0);
            } else if (c == 34) { // X
                key_setBtn(33, 0);
            }

            if(c == 0x12){
                g_exkey &= ~KB_SL;
            }else
            if(c == 0x59){
                g_exkey &= ~KB_SR;
            }else
            if(c == 0x14){
                g_exkey &= ~KB_CL;
            }else
            if(c == 0x11){
                g_exkey &= ~KB_AL;
            }
            
            // discard data
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
        } else {
            int ctrl = g_exkey & (KB_CL | KB_CR);
            // for IchigoJam
            if (c == 41) {
                key_setBtn(32, 1);
                if (ctrl) {
                    _g_key_kana = !_g_key_kana;
                    _g_key_kana_buf0 = 0;
                }
            } else if (c == 34) { // X
                key_setBtn(33, 1);
            } else if (c == 19) { // kana
                _g_key_kana = !_g_key_kana;
                _g_key_kana_buf0 = 0;
            } else if (c == 103) { // 無変換
                _g_key_kana = 0;
                _g_key_kana_buf0 = 0;
            }

            if (c == 0x12) {
                g_exkey |= KB_SL;
                if (ctrl) {
                    _g_key_kana = !_g_key_kana;
                    _g_key_kana_buf0 = 0;
                }
            } else if (c == 0x59) {
                g_exkey |= KB_SR;
                if (ctrl) {
                    _g_key_kana = !_g_key_kana;
                    _g_key_kana_buf0 = 0;
                }
            } else if (c == 0x14) {
                g_exkey |= KB_CL;
            } else if (c == 0x11) {
                g_exkey |= KB_AL;
                if (ctrl) {
                    keyboard_enqKeyBuf(0x11); // inesrt
                }
            } else if (c == 0x58) { // caps
                if (_g_key_kana) {
                    _g_key_kana = 0;
                    _g_key_kana_buf0 = 0;
                } else {
                    g_lkkey ^= KB_CK;
                }
            }else
            /*if(c == 0x7E){
                // ntsc_onoff(1);

                // extern uint8_t g_ntsc_addl;
                // g_ntsc_addl = (~g_ntsc_addl & 0b1);
            }else*/
            if (c < 0x90) {
                // keyboard_enqKeyBuf2(c);
                //register uint8_t k = ((g_exkey&(KB_SL|KB_SR)) ? g_scancodeS : g_scancode)[c&0x7f];
                register uint8_t k = ((g_exkey & (KB_SL | KB_SR)) ? g_scancodeS : g_scancode)[c];
                register uint8_t k0 = g_scancode[c];
                if (k >= 0x80) {
                    keyboard_enqFunction(k);
                } else if (g_exkey & (KB_CL | KB_CR)) {
                    // ctrl+? は何もしない
                } else {
                    //k -= (g_exkey&(KB_CL|KB_CR)) ? ( (0x60<=k) ? 0x60 : (0x40<=k) ? 0x40 : 0 ) : 0;
                    //k += (g_exkey&(KB_AL|KB_AR)) ? 0x80 : 0;
                    int shift = g_exkey & (KB_SL | KB_SR);
                    if (k0 < 0x80 && (g_exkey & (KB_AL | KB_AR))) {
                        int kana = 0; // key_flg.kana;
                        k = tolower(k0);
                        if (k >= '0' && k <= '9') {
                            k = k - '0' + (shift ? 0x80 : 0xe0);
                        } else if (k >= 'w' && k <= 'z') { // wxyz ver1.2.4b50
                            k = k - 'w' + (shift ? 0x80 : 0xe0);
                        } else if (k >= 'a' && k <= 'v') {
                            k = k - ('a' - 10) + (shift ? 0x80 : 0xe0);
                        } else if (k == '[') { // ver 1.2b5
                            k = '_';
                        } else if (k == ']') { // ver 1.2b5
                            k = kana ? 0xa0 : '\\';
                        } else {
                            k = 0;
                        }
                    } else {
                        k += (g_lkkey & KB_CK) ? ( ('A'<=k && k<='Z') ? +0x20 : ('a'<=k && k<='z') ? -0x20 : 0 ) : 0;
                    }
                    if (k0 == 32 && shift) { // shift + space
                        k = 0xe;
                    }
                    if (k0 == 10 && shift) { // shift + enter
                        k = 0x10;
                    }
                    if (g_jmp_code && (k == g_jmp_code)) {
                        jmp_fromint(1);
                    } else {
                        keyboard_enqKeyBuf(k);
                    }
                }
            }
            
            // discard data
            g_frames[g_rw.r].bit = g_frames[g_rw.r].data = 0;
            RW_INC(g_rw.r);
        }
    }
    
    g_rw.waiting_for_r = WFR_NO;
}

/*
void DUMP(){
    register void* sp asm("sp");

    printf("sp: %p\n", sp);

    uint8_t i = 0;
    while (i < 22){
        printf("[%0x]\n", *(int*)(sp+4*i));
        ++i;
    }
}
*/