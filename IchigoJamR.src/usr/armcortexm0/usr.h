inline int IJB_usr(int ad, int n);

#define uint8_t uint8
#define uint16_t uint16
#define uint32_t uint32
#define uint64_t uint64
#define int8_t int8
#define int16_t int16
#define int32_t int32
#define int64_t int64

#include "../armcortexm0/armcortexm0.h"

__attribute__ ((aligned(2))) static const char ASM_DIV[] = {
	0x10, 0xB5, 0x02, 0x46, 0x00, 0x20, 0x01, 0x23, 0x1B, 0x04, 0x0C, 0x46, 0x5C, 0x43,
	0xA2, 0x42, 0x01, 0xDB, 0x12, 0x1B, 0x18, 0x44, 0x5B, 0x08, 0xF7, 0xD1, 0x11, 0x46, 0x10, 0xBD,
};
/*
	PUSH {LR,R4}
	R2=R0
	R0=0
	R3=1
	R3=R3<<16
@LOOP
	R4=R1
	R4*=R3
	R2-R4
	IF LT GOTO @SKIP
	R2=R2-R4
	R0+=R3
@SKIP
	R3=R3>>1
	IF !0 GOTO @LOOP
	R1=R2
	POP {PC,R4}
*/

uint32_t flg = 0;
uint8_t ROM[8 * 256 + sizeof(ASM_DIV)];

inline int IJB_usr(int ad, int n) {
	if (!flg) {
		memcpy(ROM, CHAR_PATTERN, 8 * 256);
		memcpy(ROM + 8 * 256, (uint8_t*)ASM_DIV, sizeof(ASM_DIV));
		flg = 1;
	}

	struct CPU cpu;
	uint8 stack[STACK_SIZE];
	cpu_init(&cpu, ROM, ram, stack);
	cpu.reg[13] = STACK_START + STACK_SIZE;
	cpu.reg[14] = 0xFF000000; // lr
	cpu.reg[15] = RAM_START + ad - 0x700;
//printf("%d\n", ad);
//printf("%x %x\n", ram[0], ram[1]);
	
	cpu.reg[0] = n;
	cpu.reg[1] = RAM_START - 0x700;
	cpu.reg[2] = ROM_START; // CHARACTER_ROM
	cpu.reg[3] = ROM_START + 8 * 256; // udiv

	//cpu.ignorejump = 1;
	/*
	for (int i = 0; i < sizeof(ASM_DIV); i += 2) {
		//cpu.reg[15] = i;
		if (cpu_execute(&cpu))
			return 1;
	}
	*/

	int errflg = 0;
	int limit = 10000;
	for (;;) {
		if (cpu_execute(&cpu)) {
			errflg = 1;
			break;
		}
		//printf("reg15: %x\n", cpu.reg[15]);
		//printf("reg15: %x, reg2:%d\n", cpu.reg[15], cpu.reg[2]);
		if (cpu.reg[15] == 0xFF000000)
			break;
		if (!--limit) {
			errflg = 2;
			break;
		}
	}
	//printf("errflg: %d\n", errflg);
	if (errflg) {
		command_error(ERR_SEGMENTATION_FAULT);
	}
	/*
	for (int i = 0; i < 16; i++) {
		printf("R[%d]: %d\n", i, cpu.reg[i]);
	}
	printf("%d / %d = %d ... %d\n", n, m, n / m, n % m);
	*/

	return cpu.reg[0];
}
