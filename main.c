#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//pico-sdk
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/flash.h"
#include "hardware/irq.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/structs/scb.h"
#include "hardware/i2c.h"

//pico-extras
#include "pico/sleep.h"
#include "hardware/rosc.h"

//PicoDVI
#include "dvi.h"
#include "common_dvi_pin_configs.h"
#include "sprite.h"

#include "tusb.h"
#include "myhid.h"

#define DVI_TIMING dvi_timing_640x480p_60hz

// for IchigoJam BASIC
#define IJB_BUILD 0 //TODO 要修正
#define IJB_VER 255 //TODO 要修正
#define NO_MEMCPY
#define uart_putc ichigojam_uart_putc //picoのものと名前衝突しているのを解決
#include "config.h"
#include "IchigoJamR.src/stddef.h"
#include "IchigoJamR.src/random.h"
#include "IchigoJamR.src/lang.h"
#include "IchigoJamR.src/vars.h"
#include "IchigoJamR.src/ram.h"
#include "IchigoJamR.src/screen.h"
#include "IchigoJamR.src/error.h"
#include "IchigoJamR.src/ichigojam-jp.fnt.h"
#include "IchigoJamR.src/psg.h"
#include "IchigoJamR.src/basic.h"
//---
#include "i2c.h"
#include "i2ceeprom.h"
#include "storage.h"
#include "keyboard.h"
#include "hid_app.c"
#include "display.h"
#include "io.h"
#include "system.h"
#include "sound.h"
#include "ext_ws.h"
#include "mainc_functions.h"
#include "IchigoJamR.src/ext_iot.h"
#include "usr.h"
#include "init.h"

//hid_app.cが複雑になってるので、もっと簡潔に処理できるなら直したい
void putc_long_press_key() {
    int save = save_and_disable_interrupts();//割り込みを止めないとなぜかtime-us-64() - lp.last_key_report_timeがオーバーフローする時がある
    int interval = lp.is_first_putc ? 500000 : 100000;//キーを長押しした時、最初の1回だけ入力の間隔を長くする
    if (time_us_64() - lp.last_key_report_time > interval) {
        lp.last_key_report_time = time_us_64();
        if (lp.last_char != 0) {
            screen_putc(lp.last_char);
            lp.is_first_putc = false;
        }
    }
    restore_interrupts(save);
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
    picodvi_init();
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
        // put_num(hid_desc.header.bLength); // bCountryCode);
        //put_num(usb_host.dev_prop.dev_desc.idVendor);
        if (_g.sleepflg) {
            _g.sleepflg = 0;
            *linebuf = 1;

            // #define BOOT_WAIT2 25	// 1.4b10 -> b11 ここに移動
            //             video_waitSync(BOOT_WAIT2);

            exec("LRUN");
        }
        screen_showCursor(1);
        IJB_random(1);
        int ch = key_getKey();
        if (ch < 0) {
            putc_long_press_key();
            continue;
        } else if (ch == 0) {
            continue;
        }
        if (_g.uartmode_txd & 4) { // 1.2b62 UART echo back
            uart_putc(ch); // 1.3b2
            //			put_chr(key);
        }
        if (ch == ESC) {
            continue;
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
		// __wfe();
    }
	__builtin_unreachable();
}