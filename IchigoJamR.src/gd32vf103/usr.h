inline int IJB_usr(int ad, int n);


/*
int kamakura(int n, void* p1, void* p2, void* param) {
	return n * n;
}
*/

uint64 funcDivU(uint n, uint m) {
	uint n1 = n / m;
	uint n2 = n % m;
	return ((uint64)n2 << 32) | n1;
}
/*
const uint8_t ASM_RET[] = {
	0x82, 0x80,
};
*/

inline int IJB_usr(int ad, int n) {
	if (ad >= OFFSET_RAMROM && ad < SIZE_RAM + OFFSET_RAMROM) { // b10 limit:0x800 -> 0x1000
		//		int16 (*f)(int16) = screen_pcg + ad - (0x100 - SIZE_PCG) * 8;
		//		return (int16)(f(n));
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 + 1)); // ver 1.0.1 まで
		//		int (*f)(int16) = (void*)(screen_pcg + ad - ((0x100 - SIZE_PCG) * 8 - 1)); // ver 1.0.2 から
		//int (*f)(int16, void*, void*, void*) = (void*)(ram + ad - (OFFSET_RAMROM - 1)); // ver 1.1b6 から // +1 したアドレスを呼び出す必要あり Thumb2
		//int (*f)(int, void*, void*, void*) = (void*)(ram + ad - (OFFSET_RAMROM - 1)); // ver 1.3.2b12 から int16 -> 1nt // Armは+1
//		int (*f)(int, void*, void*, void*) = (void*)(ram + ad - OFFSET_RAMROM); // ver 1.3.2b12 から int16 -> 1nt
		int (*f)(int, void*, void*, void*) = (void*)(ram + ad - OFFSET_RAMROM); // ver 1.5.0b1
		//int (*f)(int, void*, void*, void*) = (void*)ASM_RET;
		//int (*f)(int, void*, void*, void*) = (void*)kamakura;
		
		//		_printf("usr %x %x %x\n", screen_pcg, f, f2);
		//		int (*f2)(int) = testasm;
		//		char* ff = (char*)f2;
		//		_printf("usr [#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x,#%x]\n", ff[-1], ff[0], ff[1], ff[2], ff[3], ff[4], ff[5], ff[6], ff[7], ff[8], ff[9], ff[10]);
		void* mem = (void*)((uint)ram - OFFSET_RAMROM); // R11
		void* rom = (void*)CHAR_PATTERN; // R12
//		void* fdiv = (void*)__aeabi_uidiv; // R3 1.2.2b31
	 	void* fdiv = (void*)funcDivU; // FUNC_DIVU; // R13
//		//		return f(mem, n);
		_g.err = ERR_SEGMENTATION_FAULT;
		n = f(n, mem, rom, fdiv); // 123; // f(n, mem, rom, fdiv); // 1.2.2b31
		//n = (int)(void*)f >> 16;
		//n = *(int16*)f;
		//n = kamakura(n, 2, 4, 5); // 123; // f(n, mem, rom, fdiv); // 1.2.2b31
		_g.err = 0;
	} else {
		command_error(ERR_ILLEGAL_ARGUMENT);
	}
	return n;
}
