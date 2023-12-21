// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja
#define ICHIGOJAM

//#define IJB_BETA // beta or not

#define NO_MEMCPY
#include <string.h>
#include "../stddef.h"
#include "../lang.h"

#define PSG_TRUE_TONE
//#define PSG_TRUE_TONE_UNIT 381
//#define PSG_TRUE_TONE_UNIT 125 //? 100
#define PSG_TRUE_TONE_UNIT (109 * 1)
#define PSG_TICK_PER_SEC 60

#define VER_PLATFORM PLATFORM_GD32VF103

#ifdef IJB_BETA
#define IJB_TITLE "IchigoJam BASIC 1.5b rv jig.jp\n"
#else
#define IJB_TITLE "IchigoJam BASIC 1.5.0 by jig.jp\n"
#endif

#define N_FLASH_STORAGE 15

#define EXT_IOT
#define KEY_INIT_DELAY
