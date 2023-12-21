// random

// xorshift : http://www.jstatsoft.org/v08/i14/paper。Marsaglia (July 2003). “Xorshift RNGs”
// http://hexadrive.sblo.jp/article/63660775.html

// 8x4 = 32byte

#include "config.h"

/*
unsigned long rndn[] = {
	123456789,
	362436069,
	521288629,
	88675123
};
*/
unsigned long rndn[4];


#define SAVE_RANDOM_SEED // SEED保存しないと40byteほど空く

#ifdef SAVE_RANDOM_SEED
void random_save() {
	LPC_PMU->GPREG0 = rndn[0];
	LPC_PMU->GPREG1 = rndn[1];
	LPC_PMU->GPREG2 = rndn[2];
	LPC_PMU->GPREG3 = rndn[3];
}
#else
#define random_save()
#endif

INLINEA unsigned int rnd() {
	unsigned long t = rndn[0] ^ (rndn[0] << 11);
	rndn[0] = rndn[1];
	rndn[1] = rndn[2];
	rndn[2] = rndn[3];
	return rndn[3] = (rndn[3] ^ (rndn[3] >> 19)) ^ (t ^ (t >> 8));
}

#ifdef SAVE_RANDOM_SEED
inline void random_init() {
	rndn[0] = 123456789;
	rndn[1] = 362436069;
	rndn[2] = 521288629;
	rndn[3] = 88675123;

	// 保存可能データ GPREG0 GPREG1 GPREG2 GPREG3 GPREG4(11bit〜31bitまで)
	long k = LPC_PMU->GPREG0;
	if (k) {
		rndn[0] = k;
		rndn[1] = LPC_PMU->GPREG1;
		rndn[2] = LPC_PMU->GPREG2;
		rndn[3] = LPC_PMU->GPREG3;
	}

	random_save();
}
#else
#define random_init();
/*
inline void random_init() {
	rndk = 123456789;
	rndy = 362436069;
	rndz = 521288629;
	rndw = 88675123;
}
*/
#endif

INLINEA void random_seed(int n) {
	rndn[0] = (LPC_PMU->GPREG0 & 0xffff0000) | (n & 0xffff);
	rndn[1] = LPC_PMU->GPREG1;
	rndn[2] = LPC_PMU->GPREG2;
	rndn[3] = LPC_PMU->GPREG3;
}
