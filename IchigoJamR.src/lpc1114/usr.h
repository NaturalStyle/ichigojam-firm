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

#define USE_AEBI_DIVU


#ifdef USE_AEBI_DIVU

//extern int __aeabi_idiv(int numerator, int denominator); // 0x0000000000004b10
//extern int __aeabi_idivmod(int numerator, int denominator); // 0x0000000000004cdc idiv + 460
//extern int __aeabi_uidiv(int numerator, int denominator); // numerator / deminator = + 268 で __aeabi_uidivmod
//extern int __aeabi_idivmod(int numerator, int denominator); // 0x0000000000004cdc idiv + 460

//  +280byte
extern uint64 __aeabi_uidiv(uint numerator, uint denominator); // numerator / deminator = + 268 で __aeabi_uidivmod // 1.3.2b12 int -> uint -> uint64 R0,R1
//#define calcDivU(n, m) __aeabi_uidiv((n), (m))
#define FUNC_DIVU __aeabi_uidiv

/*
const uint8_t ASM_DIVU[] = {
	0x0A,0x46,0x01,0x46,0x00,0x20,0x01,0x23,
	0x91,0x42,0x0E,0xD3,0x12,0x42,0x0C,0xD0,
	0x04,0xD4,0x5B,0x00,0x52,0x00,0x01,0xD4,
	0x8A,0x42,0xFA,0xD3,0x91,0x42,0x01,0xD3,
	0x89,0x1A,0xC0,0x18,0x52,0x08,0x5B,0x08,
	0xF8,0xD1,0x70,0x47
};
*/
//int *(__aeabi_uidiv)(uint n, uint m) = NULL;
//uint *(__aeabi_uidiv)(uint numerator, uint denominator) = (void*)ASM_DIVU; // error
/*
uint __aeabi_uidiv(uint n, uint m) { // linker error
	return n - m;
}
*/

#else

// 44byte
const uint8_t ASM_DIVU[] = {
	0x0A,0x46,0x01,0x46,0x00,0x20,0x01,0x23,
	0x91,0x42,0x0E,0xD3,0x12,0x42,0x0C,0xD0,
	0x04,0xD4,0x5B,0x00,0x52,0x00,0x01,0xD4,
	0x8A,0x42,0xFA,0xD3,0x91,0x42,0x01,0xD3,
	0x89,0x1A,0xC0,0x18,0x52,0x08,0x5B,0x08,
	0xF8,0xD1,0x70,0x47
};
//uint (*calcDivU)(uint, uint) = (void*)(ASM_DIVU + 1);
#define FUNC_DIVU (ASM_DIVU + 1)

#endif


inline int IJB_usr(int ad, int n) {
	if (ad >= OFFSET_RAMROM && ad < SIZE_RAM + OFFSET_RAMROM) { // b10 limit:0x800 -> 0x1000
		//		int16 (*f)(int16) = screen_pcg + ad - (0x100 - SIZE_PCG) * 8;
		//		return (int16)(f(n));
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 + 1)); // ver 1.0.1 まで
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 - 1)); // ver 1.0.2 から
		//int (*f)(int16, void*, void*, void*) = (void*)(ram + ad - (OFFSET_RAMROM - 1)); // ver 1.1b6 から // +1 したアドレスを呼び出す必要あり Thumb2
		int (*f)(int, void*, void*, void*) = (void*)(ram + ad - (OFFSET_RAMROM - 1)); // ver 1.3.2b12 から int16 -> 1nt
		
		//		_printf("usr %x %x %x\n", screen_pcg, f, f2);
		//		int (*f2)(int) = testasm;
		//		char* ff = (char*)f2;
		//		_printf("usr [#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x]\n", ff[-1], ff[0], ff[1], ff[2], ff[3], ff[4], ff[5], ff[6], ff[7], ff[8], ff[9], ff[10]);
		void* mem = (void*)((uint)ram - OFFSET_RAMROM); // R1
		void* rom = (void*)CHAR_PATTERN; // R2
//		void* fdiv = (void*)__aeabi_uidiv; // R3 1.2.2b31
		void* fdiv = (void*)FUNC_DIVU; // R3
//		//		return f(mem, n);
		_g.err = ERR_SEGMENTATION_FAULT;
		n = f(n, mem, rom, fdiv); // 1.2.2b31
		_g.err = 0;
	} else {
		command_error(ERR_ILLEGAL_ARGUMENT);
	}
	return n;
}
