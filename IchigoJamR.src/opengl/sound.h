// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

inline void sound_init();
void sound_on();
void sound_off();
static inline void sound_tick();

void sound_on() {
	printf("sound_on: %d len: %d\n", _g.psgtone, _g.psglen);
}
void sound_off() {
	printf("sound_off\n");
}
inline void sound_init() {
}
static inline void sound_tick() {
}
