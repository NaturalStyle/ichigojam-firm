#include "fake6502.h"

// test
/*
uint16 mos6502_pc;
uint8 sp, a, x, y, status;

void (*loopexternal)();

void step6502() {
  (*loopexternal)();
}

void hookexternal(void *funcptr) {
    loopexternal = funcptr;
}
*/

#define printf

//extern void step6502();
//extern uint16 pc;
//extern uint8 sp, a, x, y, status;

// zeropage #00-#ff
// sp 8bit, #0100-#01FF

uint8 mos6502_ram[0x200];
//uint8* mos6502_ram;

uint8 read6502(uint16 address) {
  /*
  if (address < 0x700) {
    return CHAR_PATTERN[address];
  }
  */
  if (address < 0x200) {
    return mos6502_ram[address];
  }
  if (address >= 0x700 && address - 0x700 < SIZE_RAM) {
    return ram[address - 0x700];
  }
  return 0;
}
void write6502(uint16 address, uint8 value) {
  /*
  if (address < 0x700) {
    return;
  }
  */
  if (address < 0x200) {
    mos6502_ram[address] = value;
  }
  if (address >= 0x700 && address < SIZE_RAM - 0x700) {
    ram[address - 0x700] = value;
  }
}

inline int IJB_usr(int ad, int n) {
  mos6502_pc = (uint16)ad;
  sp = 255;
  a = (uint8)n;

	int errflg = 0;
	int limit = 10000;
  printf("pc:%d sp:%d a:%d x:%d y:%d st:%d\n", mos6502_pc, sp, a, x, y, status);
	for (;;) {
    step6502();
    printf("pc:%d sp:%d a:%d x:%d y:%d st:%d\n", mos6502_pc, sp, a, x, y, status);
		if (mos6502_pc == 0) {
      break;
    }
		if (!--limit) {
			errflg = 2;
			break;
		}
	}
	if (errflg) {
		command_error(ERR_SEGMENTATION_FAULT);
	}
	return a;
}
