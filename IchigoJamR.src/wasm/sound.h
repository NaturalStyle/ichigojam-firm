// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

#ifndef __SOUND_H__
#define __SOUND_H__

static inline void sound_init();
static void sound_switch(int on);
static inline void sound_tick();

static void sound_switch(int on) {
}
static inline void sound_init() {
	_g.psgratio = 1;
}
static inline void sound_tick() {
}

// __SOUND_H__
#endif