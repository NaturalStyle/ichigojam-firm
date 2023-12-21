// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define PSG_TICK_FREQ 60
#define PSG_TICK_PER_SEC 60

#define IJB_TITLE "IchigoJam BASIC 1.4.3web jig.jp\n"

#define VER_PLATFORM PLATFORM_WASM

#define IJB_DONT_LOOP

#define STATIC_ICHIGOJAM_MAIN static

#define EXPORT	__attribute__((visibility("default")))

#define FILE_SIZE (1024 * 228) // 4 -> 228 2020.5.9

#define USR_ARMCORTEXM0 // default
//#define USR_Z80
//#define USR_MOS6502 // ng

// __CONFIG_H__
#endif