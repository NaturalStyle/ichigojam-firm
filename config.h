// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja
#ifndef __CONFIG_H__
#define __CONFIG_H__
#define ICHIGOJAM

//#define IJB_BETA // beta or not

// #define PSG_TRUE_TONE_UNIT (109 * 1)
// #define PSG_TICK_PER_SEC 60

#define PLATFORM_RP2040 8               //### ここに書くべきか、それが問題だ
#define VER_PLATFORM PLATFORM_RP2040

#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC 1.5b rv jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC 0.5.0 RP2040\n"
#endif

#define ENABLE_I2C_BPS

#define N_FLASH_STORAGE 100

#define VERSION15

#define EXT_IOT
// #define KEY_INIT_DELAY
#endif
