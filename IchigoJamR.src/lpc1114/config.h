// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/

#ifndef __CONFIG_H__
#define __CONFIG_H__

// build config
#define IJB_BETA // beta or not
//#define KEY_LAYOUT KEY_LAYOUT_US // =2 (US) US_LAYOUT or not
//#define LCD_MODE2 // 240x320 LCD ILI9341 or not
//#define ROM_MODE // or not storage.h
//#define TV_MODE TV_MODE_PAL // =2 (PAL) uncomment if use NTSC
#define PROTECT_LAST_FLASH_FILE // for 1.4.3b27

#define NO_KBD_COMMAND

//#define IJB_SKYBERRYJAM // skyberryjam logo or not

// for debug
//#define FLASH_ONLY_ONE

/*

set us, reset lcd, rom, tvmode
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-ntsc-uskbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-ntsc-uskbd.bin

set lcd
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-lcd-uskbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-lcd-uskbd.bin

reset us
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-lcd-jpkbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-lcd-jpkbd.bin

reset lcd
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-ntsc-jpkbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-ntsc-jpkbd.bin



--



// for PAL stddef.h
set us
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-pal-uskbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-pal-uskbd.bin

reset us
./cm.sh
mv ../dest/ichigojam.hex ../release/ichigojam-pal-jpkbd.hex
mv ../dest/ichigojam.bin ../release/ichigojam-pal-jpkbd.bin

*/

#ifdef IJB_BETA
#define IJB_VER_STR 1.4
#else
#define IJB_VER_STR 1.4.3
#endif


// other config
//#define XTAL_358MHz // if use external xtal 3.58MHz

#include "../stddef.h"
#include "../lang.h"
#include "LPC11xx.h"

#define RAM_FIXED 1 // ldファイルで固定 -100byte
#define MEM_UNDER64KB
#define DIVIDE0EQUALS0 // x/0=0 にする、for draw_line screen.h
#define NO_KBD_COMMAND

// 2021.4.1
//#define WITH_MORSE // for IchigoJam MORSE
#ifdef WITH_MORSE
#define FLASH_ONLY_ONE // 4KB空けて使える -200byte
#endif

#define INLINEA static inline

#include "vectors.h"

#define VER_PLATFORM PLATFORM_LPC1114


// for debug
//#define FLASH_ONLY_ONE // 4KB空けて使える -200byte
//#define DEBUG_PRINT
#ifdef IJB_BETA
	#ifdef DEBUG_PRINT
		#define FLASH_ONLY_ONE // 4KB空けて使える
	#endif
#else
	#undef DEBUG_PRINT // DEBUG_PRINTはBetaのみ可
#endif

#define ENABLE_I2C_BPS
#define EXT_IOT // IoT sakura.io拡張

// LCD_MODE

// 1.2.3以降の通常は、LCD_MODE3でSWITCHによる切り替え動作
#ifndef LCD_MODE2
#define LCD_MODE3 // for ST7656 & NTSC // 1.2b37 800byteほど増える - 1.4標準
#endif

//

#define LEN_FLASH 4

#define IJB_USE_EXCEPTION // -500byte over, return, breakを消せばもうちょっと空く
#define USE_ILLEGAL_ARGUMENT_ERROR // +40byte
//#define CHECK_CALCSTACK // check stack memory +16byte

#if TV_MODE == TV_MODE_NTSC

#define VIDEO_LINES 261 // ver 1.2b23
#define PSG_TICK_PER_SEC 60

#elif TV_MODE == TV_MODE_PAL

#define VIDEO_LINES 312 // ver 1.2b44
#define PSG_TICK_PER_SEC 50

#endif

#define PSG_TICK_FREQ (PSG_TICK_PER_SEC * VIDEO_LINES) // VIDEO_LINES -> LINESに統合を

//#define USR_MOS6502

#if LANG == LANG_JP
#ifdef IJB_BETA
//#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) " by jig.jp\n"
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) " jig.jp\n"
#else
#ifdef IJB_SKYBERRYJAM
#define IJB_TITLE "SkyBerryJAM/IJB " STRING2(IJB_VER_STR) " by jig.jp\n"
#else
#ifdef WITH_MORSE // 2021.4.1
#define IJB_TITLE "IchigoJam BASIC morse by jig.jp\n" // morse
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) " by jig.jp\n" // normal
#endif
#endif
#endif
#endif

#if LANG == LANG_MN
#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "mn jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "mn jig.jp\n"
#endif
#endif

#if LANG == LANG_VI
#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "vn jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "vn jig.jp\n"
#endif
#endif

#if LANG == LANG_FR
#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "fr jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "fr jig.jp\n"
#endif
#endif

#if LANG == LANG_ZH
#ifdef IJB_BETA
#define IJB_TITLE "FunTick BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "\n"
#else
#define IJB_TITLE "FunTick BASIC " STRING2(IJB_VER_STR) "\n"
#endif
#endif

#if LANG == LANG_BP
#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "bp jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "bp jig.jp\n"
#endif
#endif

#ifdef DEBUG_PRINT
#include "xprintf.h"
void _printf(const char* fmt, ...) {
	va_list arp;
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
}
#endif	// DEBUG_PRINT

#endif	// __CONFIG_H__
