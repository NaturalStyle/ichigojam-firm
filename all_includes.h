//main.c以外のファイルにもincludeしているのは、IDEに認識させるため
#ifndef __ALL_INCLUDES_H__
#define __ALL_INCLUDES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef IJB_PICO_NO_PRINT
#define printf(...)
#endif

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

#endif