// random

// xorshift : http://www.jstatsoft.org/v08/i14/paper
//	Marsaglia (July 2003). “Xorshift RNGs”
// http://hexadrive.sblo.jp/article/63660775.html

// 8x4 = 32byte

unsigned long rndn[] = {
	123456789,
	362436069,
	521288629,
	88675123
};
const unsigned long RNDN[] = {
	123456789,
	362436069,
	521288629,
	88675123
};

#define random_save()

INLINE unsigned int rnd() {
	unsigned long t = rndn[0] ^ (rndn[0] << 11);
	rndn[0] = rndn[1];
	rndn[1] = rndn[2];
	rndn[2] = rndn[3];
	return rndn[3] = (rndn[3] ^ (rndn[3] >> 19)) ^ (t ^ (t >> 8));
}
INLINE void random_init() {
}

INLINE void random_seed(int n) {
	rndn[0] = (RNDN[0] & 0xffff0000) | (n & 0xffff);
	rndn[1] = RNDN[1];
	rndn[2] = RNDN[2];
	rndn[3] = RNDN[3];
}
