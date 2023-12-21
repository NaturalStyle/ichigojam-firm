#include "z80.c"

static Z80Context context;

static byte context_mem_read_callback(int param, ushort address) {
  if (address >= 0x700 && address - 0x700 < SIZE_RAM) {
    return ram[address - 0x700];
  }
  return 0;
}

static void context_mem_write_callback(int param, ushort address, byte data) {
  if (address >= 0x700 && address - 0x700 < SIZE_RAM) {
    ram[address - 0x700] = data;
  }
}

inline int IJB_usr(int ad, int n) {
  context.memRead = context_mem_read_callback;
  context.memWrite = context_mem_write_callback;
  Z80RESET(&context);
  unsigned end_tstates;
  context.R1.wr.AF = (uint8)n;
  context.R1.wr.SP = 0x800;
  context.PC = ad;
	int errflg = 0;
	int limit = 10000;
  // printf("pc:%4x af:%d\n", context.PC, context.R1.wr.AF);
  for (;;) { // while (context.tstates < end_tstates) {
    Z80Execute(&context);
    // printf("pc:%4x af:%d\n", context.PC, context.R1.wr.AF);
		if (!--limit) {
			errflg = 2;
			break;
		}
    if (context.PC == 0) {
      break;
    }
  }
	if (errflg) {
		command_error(ERR_SEGMENTATION_FAULT);
	}
  return context.R1.wr.AF >> 8;
}
