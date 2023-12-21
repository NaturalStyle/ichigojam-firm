// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

#include <SDL.h>

inline void sound_init();
void sound_on();
void sound_off();
static inline void sound_tick();

int soundflg; // on: 1, off: 0

int soundlen;
SDL_AudioSpec audiospec;

/*
double wave(double t) {
	//  return t - sin(t) < 0.5 ? 1 : -1;
	//    return sin(t);
	//    return (sin(t) + sin(t * 2)) / 2;
	//    return (sin(t) + sin(t * 2) + sin(t *3)) / 3;
	//    return (sin(t) + sin(t * 2) + sin(t *3) + sin(t * 4)) / 4;
	//    return (sin(t) + sin(t * 2) + sin(t *3) + sin(t * 4) + sin(t * 5)) / 5;
//	return (sin(t / 3) + sin(t / 2) + sin(t) + sin(t * 2) + sin(t *3) + sin(t * 4) + sin(t * 5)) / 7;
	return sin(t) < 0.0 ? 1 : -1;
	//    return 0;
}
*/

uint step = 0;

#define PI 3.1415926535

void callback(void* unused, Uint8* stream, int len) {
	if (_g.psgtone) {
		soundlen = (double)audiospec.freq * _g.psgtone / (60 * VIDEO_LINES);
	} else {
		soundlen = 0;
	}
//	printf("sound_tick: %d \n", _g.psgtone);
//	printf("sound_tick: %d len: %d freq:%f\n", _g.psgtone, _g.psglen, (double)audiospec.freq);
	
	//	memset(stream, 0, len);
	// 16bit
	/*
	short *frames = (short*)stream;
	int framesize = len / 2;
	double c = audiofreq / audiospec.freq * (2 * PI);
	for (int i = 0; i < framesize; i++, step++) {
		frames[i] = (short)(wave(c * step) * 3000);
	}
	*/
	if (!soundlen) {
		memset(stream, 0, len);
		return;
	}
	signed char* frames = (signed char*)stream;
	int framesize = len;
	for (int i = 0; i < framesize; i++, step++) {
		 frames[i] = step % soundlen > soundlen / 2 ? 20 : -20;
	}
}

void sound_switch(int on) {
	if (on) {
	//	audiofreq = (double)(60 * VIDEO_LINES) / _g.psgtone;
		//	printf("sound_on: %d len: %d freq:%f\n", _g.psgtone, _g.psglen, audiofreq);
//		soundlen = (double)audiospec.freq * _g.psgtone / (60 * VIDEO_LINES);
//		printf("sound_on: %d len: %d freq:%f\n", _g.psgtone, _g.psglen, (double)audiospec.freq);
	} else {
		soundlen = 0;
//		printf("sound_off\n");
	}
}
int flg_sound_init = 0;
inline void sound_init() {
	_g.psgratio = 1;
	
	if (flg_sound_init)
		return;
	flg_sound_init = 1;

	soundlen = 0;
	if (!soundflg) {
		return; // no sound
	}
	/*
	//	if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)) { 
	if ((SDL_Init(SDL_INIT_AUDIO) == -1)) { 
		fprintf(stderr, "can't open audio\n");
//		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		return;
	}
	*/
	
	SDL_AudioSpec want;
	
	SDL_memset(&want, 0, sizeof(want)); /* または SDL_zero(want); */
	//    want.freq = 8000;
	want.freq = 16000;
	//    want.freq = 22050; //44100; //22050; /* Sampling rate: 22050Hz */
	//    want.freq = 44100; //44100; //22050; /* Sampling rate: 22050Hz */
	//	want.format = AUDIO_S16LSB; // 16-bit signed audio
	want.format = AUDIO_S8; // 8-bit signed audio
	want.channels = 1; // 1:Mono 2:stereo
	//	want.samples = want.freq / 100; // 0.01sec
	want.samples = want.freq / 50; // 0.05sec
	want.callback = callback;
	want.userdata = NULL;
	
	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &audiospec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (dev == 0) {
		fprintf(stderr, "can't open audio\n");
//		fprintf(stderr, "can't open audio: %s\n", SDL_GetError());
		return;
	}
	SDL_PauseAudioDevice(dev, 0);
//	SDL_CloseAudioDevice(dev);
//	SDL_Quit();
}
static inline void sound_tick() {
}
