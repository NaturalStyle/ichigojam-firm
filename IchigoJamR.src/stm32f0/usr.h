inline int IJB_usr(int ad, int n);

/*
// 845
110 a=usr(#700,a)
110 a=(a>>8)|(a<<8)

(i>>8)|(i<<8)

int16 testasm(int16 n) { // 2bc0
	//	asm("mov r0, #3");
	//	return __REV16(n);
	asm("  rev16 r0, r0"); // #40,#ba,#70,#47
	return n;
	//	return n*2; // #40,#0,#0,#b2,#70,#47
	//	return 100; // // #64 #20 #70 #47 return 100
	}
	*/

//extern int __aeabi_idiv(int numerator, int denominator); // 0x0000000000004b10
//extern int __aeabi_idivmod(int numerator, int denominator); // 0x0000000000004cdc idiv + 460
extern int __aeabi_uidiv(int numerator, int denominator); // numerator / deminator = + 268 で __aeabi_uidivmod
//extern int __aeabi_idivmod(int numerator, int denominator); // 0x0000000000004cdc idiv + 460

inline int IJB_usr(int ad, int n) {
	if (ad >= OFFSET_RAMROM && ad < SIZE_RAM + OFFSET_RAMROM) { // b10 limit:0x800 -> 0x1000
		//		int16 (*f)(int16) = screen_pcg + ad - (0x100 - SIZE_PCG) * 8;
		//		return (int16)(f(n));
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 + 1)); // ver 1.0.1 まで
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 - 1)); // ver 1.0.2 から
		int (*f)(int16, void*, void*, void*) = (void*)(ram + ad - (OFFSET_RAMROM - 1)); // ver 1.1b6 から // +1 したアドレスを呼び出す必要あり Thumb2
		
		//		_printf("usr %x %x %x\n", screen_pcg, f, f2);
		//		int (*f2)(int) = testasm;
		//		char* ff = (char*)f2;
		//		_printf("usr [#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x]\n", ff[-1], ff[0], ff[1], ff[2], ff[3], ff[4], ff[5], ff[6], ff[7], ff[8], ff[9], ff[10]);
		void* mem = (void*)((uint)ram - OFFSET_RAMROM); // R1
		void* rom = (void*)CHAR_PATTERN; // R2
		void* fdiv = (void*)(__aeabi_uidiv); // R3 1.2.2b31
//		void* fdiv = (void*)(__aeabi_uidiv + 1); // R3 1.2.2b45 +1
		//		return f(mem, n);
		return f(n, mem, rom, fdiv); // 1.2.2b31
	}
	return 0;
}
