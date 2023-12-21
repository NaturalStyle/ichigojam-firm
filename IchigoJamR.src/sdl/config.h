// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja


#ifndef __CONFIG_H__
#define __CONFIG_H__

// build config
//#define IJB_BETA // beta or not

#ifdef IJB_BETA
#define IJB_VER_STR 1.4
#else
#define IJB_VER_STR 1.4.3
#endif

#ifndef LANG
#define LANG LANG_ALL
#endif

#define VER_PLATFORM PLATFORM_PC

#define NO_MEMCPY
#include <string.h>

#define EXT_IOT // dummy

//#define MODE15FPS
#ifdef MODE15FPS

#define PSG_TICK_FREQ 15
#define PSG_TICK_PER_SEC 15

#else

#define PSG_TICK_FREQ 60
#define PSG_TICK_PER_SEC 60

#endif

#define VIDEO_LINES 261

#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "b" STRING2(IJB_BUILD) "ap jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC " STRING2(IJB_VER_STR) "ap jig.jp\n"
#endif

#define USR_ARMCORTEXM0
//#define USR_Z80
//#define USR_MOS6502

#include <stdio.h>

#endif	// __CONFIG_H__
