#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/vreg.h"

#include "dvi.h"
#include "common_dvi_pin_configs.h"
#include "sprite.h"

#include "tusb.h"
#include "myhid.h"

// for IchigoJam BASIC
#define IJB_BUILD 0 //TODO 要修正
#define IJB_VER 255 //TODO 要修正
#define NO_MEMCPY
#define uart_putc ichigojam_uart_putc
#include "config.h"
#include "IchigoJam.selected/stddef.h"
#include "IchigoJam.selected/random.h"
#include "IchigoJam.selected/lang.h"
#include "IchigoJam.selected/vars.h"
#include "IchigoJam.selected/ram.h"
#include "IchigoJam.selected/screen.h"
#include "IchigoJam.selected/error.h"
#include "IchigoJam.selected/ichigojam-jp.fnt.h"
#include "IchigoJam.selected/psg.h"
#include "IchigoJam.selected/basic.h"
//---
#include "platform_depends_functions.h"
#include "hid_app.c"
#include "keyboard.h"
#include "storage.h"
#include "display.h"

//pico
// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz
#define SCANLINE_INIT 2
#define LED_PIN 25

//IchigoJam
#define CHAR_ROWS 24
#define CHAR_COLS 32
#define FONT_SIZE 8
#define MARGIN_WIDTH (FRAME_WIDTH - CHAR_COLS * FONT_SIZE) / 2
#define MARGIN_HEIGHT (FRAME_HEIGHT - CHAR_ROWS * FONT_SIZE) / 2
// #define MARGIN_WIDTH 32
// #define MARGIN_HEIGHT 24

//pico
struct dvi_inst dvi0;
uint16_t framebuf[FRAME_WIDTH * FRAME_HEIGHT];
static repeating_timer_t out;

//IchigoJam
extern uint8* vram;
struct keyflg_def key_flg;

void core1_main() {
    multicore_lockout_victim_init();
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    dvi_start(&dvi0);
    dvi_scanbuf_main_16bpp(&dvi0);
    __builtin_unreachable();
}

//この関数の実行時間が長くなるとUSBキーボードを使う時、不具合が起こる
void core1_scanline_callback() {
    // Discard any scanline pointers passed back
    uint16_t* bufptr;
    while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr))
        ;
    // // Note first two scanlines are pushed before DVI start
    static uint scanline = SCANLINE_INIT;
    bufptr = &framebuf[FRAME_WIDTH * scanline];
    queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
    scanline = (scanline + 1) % FRAME_HEIGHT;
}

//1scanline分vramの内容をframebufに反映する
void vram_to_framebuf_scanline(uint scanline, bool visible_cursor) {
    int vram_y = (scanline - MARGIN_HEIGHT) / FONT_SIZE;
    if (0 <= vram_y && vram_y < CHAR_ROWS) {//scanlineが画面の表示範囲なら処理、そうでなければ黒のままでいいので何もしない
        int font_y = scanline % FONT_SIZE;
        uint16_t* framebuf_base = &framebuf[scanline * FRAME_WIDTH + MARGIN_WIDTH];
        uint8* c = &vram[vram_y * CHAR_COLS];
        for (int vram_x = 0; vram_x < CHAR_COLS; vram_x++) {
            unsigned char char_line = CHAR_PATTERN[*c * FONT_SIZE + font_y];
            c++;
            if (visible_cursor && _g.cursorx == vram_x && _g.cursory == vram_y) {//カーソルの位置の文字だけ反転させる
                char_line ^= key_flg.insert ? 0xff : 0xf0;//上書きモードなら文字全体を反転、挿入モードなら文字の左半分を反転
            }
            for (int x = 0; x < FONT_SIZE; x++) {
                int pixel = 0xffff * ((char_line >> (7 - x)) & 0x01);//char_lineのビットが1なら0xffff(白)、0なら0x0000(黒)に変換
                *framebuf_base = pixel;
                framebuf_base++;
            }
        }
    }
}

void vram_to_framebuf_all(bool visible_cursor) {
    for (int sl = 0; sl < FRAME_HEIGHT; sl++) {
        vram_to_framebuf_scanline(sl, visible_cursor);
    }
}

bool timer(repeating_timer_t* rt) {
    vram_to_framebuf_all(_g.cursorflg);
    tuh_task();
    return true;
}

void pico_init() {
    board_init();
    stdio_init_all();

    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);
#ifdef RUN_FROM_CRYSTAL
    set_sys_clock_khz(12000, true);
#else
    // Run system at TMDS bit clock
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
#endif

    setup_default_uart();

    //ラズパイが動いていることを確認するためにLEDを常時点灯させる
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // init host stack on configured roothub port
    tuh_init(BOARD_TUH_RHPORT);
    add_repeating_timer_ms(-33, timer, NULL, &out);//FPS30

    dvi0.timing = &DVI_TIMING;
    dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi0.scanline_callback = core1_scanline_callback;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

    // Once we've given core 1 the framebuffer, it will just keep on displaying
    // it without any intervention from core 0
    sprite_fill16(framebuf, 0x0000, FRAME_WIDTH * FRAME_HEIGHT);
    uint16_t* bufptr = framebuf;
    for (int i = 0; i < SCANLINE_INIT; i++) {
        queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
        bufptr += FRAME_WIDTH;
    }

    multicore_launch_core1(core1_main);
}

void ichigojam_init() {
    _g.screenw = CHAR_COLS;
    _g.screenh = CHAR_ROWS;
    screen_clear();

    for (int y = 0; y < CHAR_ROWS; ++y) {
        for (int x = 0; x < CHAR_COLS; ++x) {
            // vram[y * CHAR_COLS + x] = (y * CHAR_COLS + x) % (256 - 32) + 32;
            vram[y * CHAR_COLS + x] = 0;
        }
    }
    vram_to_framebuf_all(true);
    key_clearKey();
    key_flg.caps = true;
    key_flg.insert = 0;
    random_init();
}








STATIC void exec(char* s) {
	key_flg.bkinsert = key_flg.insert;
	key_flg.insert = 1;

	_g.screen_insertmode = 1;
	_g.key_flg_esc = 0;
	screen_showCursor(0);
	int res = basic_execute(s);
	//screen_showCursor(1); // 1.4b10
	if (_g.cursory == -1)
		_g.cursory = 0;
	if (res == BASIC_RESULT_EXECUTE && !noresmode) {// stop, exec, edit, err
		put_str("OK\n");
	}
	#ifndef IJB_USE_EXCEPTION // 例外使うときには使わないコード
//	if (res != 2) { // 1.2b36 追加、edit時(==2)以外に限定
	//if (res == BASIC_RESULT_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
	if (res == BASIC_RESULT_STOP_OR_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
		key_clearKey();
	}
	#endif

	key_flg.insert = key_flg.bkinsert;
}









int main() {
    pico_init();
    ichigojam_init();

	char* linebuf = (char*)ram + OFFSET_RAM_LINEBUF;
	if (*linebuf) {
		//		put_str(ERR_MESSAGES[ERR_STACK_OVERFLOW - 1]);
		//		put_chr('\n');
		if (_g.cursory == -1) { // 1.4.1 前にもってくる
			_g.cursory = 0;
		}
		if (!noresmode) {
			if (!_g.err) {
				_g.err = ERR_COMPLEX_EXPRESSION;
			}
			basic_printError();
			//			put_str(ERR_MESSAGES[_g.err - 1]);
			//			put_chr('\n');
		}
		// from exec
//		screen_showCursor(1);
	//	if (res != 2) { // 1.2b36 追加、edit時(==2)以外に限定
		//if (res == BASIC_RESULT_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
		key_clearKey(); // 1.3b4 エラー停止の時だけ、キークリア

		key_flg.insert = key_flg.bkinsert;
    }
    while (1) {
        static uint64 cursor_time = 0;
        if (time_us_64() - cursor_time > 250000) {
            screen_showCursor(!_g.cursorflg);
            cursor_time = time_us_64();
        }
        IJB_random(1);
        while (1) {
            int ch = key_getKey();
            if (ch == -1) {
                break;
            } else if (ch == 0) {
                continue;//今は通らない？
            }
            _g.screen_insertmode = key_flg.insert;
            screen_putc(ch);
            if (ch == RETURN) {
                uint8* s = screen_gets();

                //		put_str(s);
                if (*s == '\'') { // 1.1b14
                } else if (*s != 0) {
                    uint8 i;
                    for (i = 0; i < N_LINEBUF; i++) {
                        linebuf[i] = s[i];
                        if (!s[i])
                            break;
                    }
                    //				_g.screen_insertmode = 1;
                    if (s[i]) {
                        //					put_str("Too long line\n");
                        put_str("Too long\n"); // 1.2b45
                    } else {
                        linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
                        exec(linebuf);
                    }
                }
            }
        }
        // vram_to_framebuf_all(true); //割り込み処理の中でframebufを更新するので不要
		// __wfe();
	}









    // from IchigoJamR.src
// 	char* linebuf = (char*)ram + OFFSET_RAM_LINEBUF;
// 	if (*linebuf) {
// //		put_str(ERR_MESSAGES[ERR_STACK_OVERFLOW - 1]);
// //		put_chr('\n');
// 		if (_g.cursory == -1) { // 1.4.1 前にもってくる
// 			_g.cursory = 0;
// 		}
// 		if (!noresmode) {
// 			if (!_g.err) {
// 				_g.err = ERR_COMPLEX_EXPRESSION;
// 			}
// 			basic_printError();
// //			put_str(ERR_MESSAGES[_g.err - 1]);
// //			put_chr('\n');
// 		}
// 		// from exec
// //		screen_showCursor(1);
// 	//	if (res != 2) { // 1.2b36 追加、edit時(==2)以外に限定
// 		//if (res == BASIC_RESULT_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
// 		key_clearKey(); // 1.3b4 エラー停止の時だけ、キークリア

// 		key_flg.insert = key_flg.bkinsert;
// 	}
// 	for (;;) {
// 		tuh_task();     //MOTTEKITA



//     	// put_num(hid_desc.header.bLength); // bCountryCode);
// 		//put_num(usb_host.dev_prop.dev_desc.idVendor);
// 		if (_g.sleepflg) {
// 			_g.sleepflg = 0;
// 			*linebuf = 1;

// 			#define BOOT_WAIT2 25	// 1.4b10 -> b11 ここに移動
// 			video_waitSync(BOOT_WAIT2);

// 			exec("LRUN");
// 		}
// 		screen_showCursor(1); // 1.4b10
// 		IJB_random(1);
// 		video_waitSync(1); // 消すとUART受信漏れ発生?

// 		int key = key_getKey();
// 		if (key < 0)
// 			continue;
// 		if (_g.uartmode_txd & 4) { // 1.2b62 UART echo back
// 			uart_putc(key); // 1.3b2
// //			put_chr(key);
// 		}
// 		if (key == 27)
// 			continue;
		
// 		_g.screen_insertmode = key_flg.insert;

// 		screen_putc(key);

// 		if (key == '\n') {
// 			uint8* s = screen_gets();

// 	//		put_str(s);
// 			if (*s == '\'') { // 1.1b14
// 			} else if (*s != 0) {
// 				uint8 i;
// 				for (i = 0; i < N_LINEBUF; i++) {
// 					linebuf[i] = s[i];
// 					if (!s[i])
// 						break;
// 				}
// //				_g.screen_insertmode = 1;
// 				if (s[i]) {
// //					put_str("Too long line\n");
// 					put_str("Too long\n"); // 1.2b45
// 				} else {
// 					linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
// 					exec(linebuf);
// 				}
// 			}
// 		}
// 	}










	__builtin_unreachable();
}