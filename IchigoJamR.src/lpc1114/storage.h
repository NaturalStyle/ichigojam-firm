#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "../stddef.h"
#include "../vars.h"
#include "iic.h"

#include "iap.h"

#define EEPROM_SAVE 1

#if EEPROM_SAVE == 1

#define boolean char
#include "i2ceeprom.h"

#define EEPROM_PSIZE 32
#define EEPROM_WSIZE 1024
#define EEPROM_OFFSET 100
#define EEPROM_SIZE 128

#endif

inline int IJB_file() {
	return _g.lastfile;
}

#include "iap.h"

#ifdef PROTECT_LAST_FLASH_FILE
#define LEN_FLASH_WRITE (LEN_FLASH - 1)
#endif

#ifndef LEN_FLASH_WRITE
#define LEN_FLASH_WRITE LEN_FLASH
#endif

#ifdef FLASH_ONLY_ONE

//#define _g.fileslot 7
#define FILESLOT 7

inline void flash_init() {
}

int flash_save(int n, uint8* src) {
	int sto = FILESLOT; // _g.fileslot;
	uint8* idst = (uint8*)(0x1000 * sto);
	if (prepareFlash(sto, sto))
		return 1; // err
	if (eraseFlash(sto, sto))
		return 1; // err
	if (prepareFlash(sto, sto))
		return 1; // err
	int rw = writeFlash(src, idst, 0x400);
	if (rw)
		return 1;
	if (compareFlash(src, idst, 0x400))
		return 1;
	return 0;
}

int IJB_save(int n, uint8* list, int size) { // err:1 no_err:0
	_g.lastfile = n;
#if EEPROM_SAVE == 1
	// eeprom 32KB
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
		if (i2c0_init())
			return -1;
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		for (int i = 0; i < EEPROM_WSIZE; i += EEPROM_PSIZE) {
			if (!accessEEPROM(I2C_WRITE,ad + i, (uint8_t*)(list + i), EEPROM_PSIZE)) {
				return 1;
			}
		}
		return 0;
	}
#endif
	if (n != 0)
		return 1;
	
	__disable_irq();
	int res = flash_save(n, list);
	__enable_irq();
	
	return res;
}

// 1block = 4KB
int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (init)
		_g.lastfile = n;
#if EEPROM_SAVE == 1
	// eeprom
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
		if (init) {
			if (i2c0_init())
				return -1;
		}
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		for (int i = 0; i < sizelimit; i += EEPROM_PSIZE) {
			if (!accessEEPROM(I2C_READ, ad + i, (uint8_t*)(list + i), EEPROM_PSIZE)) {
				return -1;
			}
		}
		if (*(int16*)list == -1) { // 初期状態
			*(uint16*)list = 0;
			return -1;
		}
		return sizelimit;
	}
#endif
	if (n != 0)
		return -1;
	
//	uint8* buf = (char*)(0x1000 * _g.fileslot + 0x400 * n); // 1.0.0 ver
	uint8* buf = (uint8*)(0x1000 * FILESLOT + 0x400 * n);
	if (*(int16*)buf == -1) // 初期状態
		return -1;
	memcopy(list, buf, sizelimit);
	return sizelimit;
}

#else // FLASH_ONLY_ONE // 2 slots version

/*
blank6 blank7 -> 6
blank6 use7 -> 7
use6 blank7 -> 6
use6 use7 -> 6

if (blank6 && !blank7) {
	select 7
}

if (!blank to) {
	blank to
	erase to
}
erase from
*/

void flash_init() {
	__disable_irq();
	_g.fileslot = 6;
	// 空いている時 0
	if ((!blankCheckFlash(6, 6)) && blankCheckFlash(7, 7)) {
		_g.fileslot = 7;
	}
	__enable_irq();
}

int flash_save(int n, uint8* src) {
	int sto = _g.fileslot == 7 ? 6 : 7;
	int sfrom = _g.fileslot;
	
	if (blankCheckFlash(sto, sto)) {
		if (prepareFlash(sto, sto))
			return 1; // err
		if (eraseFlash(sto, sto))
			return 1; // err
	}
	//	xprintf("save0 %d\n", sto);
	//	char* target = (char*)(5 * 0x1000 + 0x1000 * n);
	uint8 temp[0x100];
	for (int i = 0; i < 4; i++) {
		uint8* isrc = (uint8*)(0x1000 * sfrom + i * 0x400);
		if (i == n)
			isrc = src;
		uint8* idst = (uint8*)(0x1000 * sto + i * 0x400);
//	xprintf("save1 %x -> %x\n", isrc, idst);
//		if (writeFlash(isrc, idst, 0x400))
		//			return 1;
		for (int j = 0; j < 4; j++) {
			if (prepareFlash(sto, sto))
				return 1; // err
			memcopy(temp, isrc + 0x100 * j, 0x100);
			int rw = writeFlash(temp, idst + 0x100 * j, 0x100);
//			xprintf("save2 %d %d %d\n", i, j, rw);
			if (rw)
				return 1;
			if (compareFlash(temp, idst + 0x100 * j, 0x100))
				return 1;
		}
	}
	if (prepareFlash(sfrom, sfrom))
		return 1; // err
	if (eraseFlash(sfrom, sfrom))
		return 1; // err
	_g.fileslot = sto;
	return 0;
}

int IJB_save(int n, uint8* list, int size) {
	_g.lastfile = n;
#if EEPROM_SAVE == 1
	// eeprom 32KB
//	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE - 1)) { // 100-226 までの127コ保存可能、先頭1kは予約
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
//		if (LPC_GPIO0->MASKED_ACCESS[(1 << 4)] == 0) {
//			return -1;
//		}
		//		__disable_irq();
//		screen_off();
		if (i2c0_init())
			return -1;
		
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		uint8_t chk[EEPROM_PSIZE];
		for (int i = 0; i < EEPROM_WSIZE; i += EEPROM_PSIZE) {
			if (!accessEEPROM(I2C_WRITE, ad + i, (uint8*)(list + i), EEPROM_PSIZE)) {
				return 1;
			}
			// add verify +48byte
			if (!accessEEPROM(I2C_READ, ad + i, chk, EEPROM_PSIZE)) {
				return 1;
			}
			for (int j = 0; j < EEPROM_PSIZE; j++) {
				if (*(uint8_t*)(list + i + j) != chk[j])
					return 1;
			}
		}
	/*
	IOCON_PIO0_5 = 0x0;
	GPIO0DIR &= ~(1<<5);
	*/
//		screen_on();
//		__enable_irq();
		return 0;
	}
#endif
	/*
	if (n < 0 || n >= 3 * 4)
		return -1;
	char* target = (char*)(5 * 0x1000 + 0x400 * n);
	*/
	#ifdef ROM_MODE
	if (n < 0 || n >= _g.lastfile - 7) // must be true to compile the flash_save
		return 1;
	#else
	if (n < 0 || n >= LEN_FLASH_WRITE)
		return 1;
	#endif
	
	__disable_irq();
	int res = flash_save(n, list);
	__enable_irq();
	
	return res;
}

// 1block = 4KB
int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (init)
		_g.lastfile = n;
#if EEPROM_SAVE == 1
	// eeprom
//	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE - 1)) { // 100-226 までの127コ保存可能、先頭1kは予約
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
//		if (LPC_GPIO0->MASKED_ACCESS[(1 << 4)] == 0) {
//			return -1;
//		}
		//		__disable_irq();
		if (init) {
			/*
			IOCON_PIO0_4 = 0x0;
			LPC_GPIO0->DIR &= ~(1 << 4);
			if (LPC_GPIO0->MASKED_ACCESS[(1 << 4)] == 0) { // この判定では効かない 0_5の状態が影響する
				return -1;
			}
			*/
			
			if (i2c0_init())
				return -1;
		}
		/*
		n -= EEPROM_OFFSET;
		struct EEPROMHeader head;
		if (!accessEEPROM(I2C_READ, 0, (uint8_t*)&head, sizeof(struct EEPROMHeader)))
			return -1;
		if (head.id != EEPROM_ID)
			return -1;
		if (n >= head.sizekb)
			return -1;
		*/
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		for (int i = 0; i < sizelimit; i += EEPROM_PSIZE) {
			if (!accessEEPROM(I2C_READ, ad + i, (uint8_t*)(list + i), EEPROM_PSIZE)) {
//				*(uint16*)list = 0;
				return -1;
			}
		}
	/*
	IOCON_PIO0_5 = 0x0;
	GPIO0DIR &= ~(1<<5);
	*/
		/*
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		short len = sizelimit;
//		accessEEPROM(I2C_READ, ad, (char*)&len, 2);
		short l = len;
		for (int i = 0; i < len; i += EEPROM_PSIZE) {
			if (l > EEPROM_PSIZE)
				l = EEPROM_PSIZE;
			if (!accessEEPROM(I2C_READ, ad + i, (uint8_t*)(list + i), l)) {
				return -1;
			}
			l -= EEPROM_PSIZE;
			if (l <= 0)
				break;
		}
		*/
//		__enable_irq();
		if (*(int16*)list == -1) { // 初期状態
			*(uint16*)list = 0;
			return -1;
		}
//		xprintf("size %d\n", sizelimit);
//		*(uint16*)list = 0;
		return sizelimit;
	}
#endif
	
	/*
	if (n < 0 || n >= 3 * 4)
		return -1;
	char* buf = (char*)(5 * 0x1000 + 0x400 * n);
	*/
	if (n < 0 || n >= LEN_FLASH)
		return -1;
	
	uint8* buf = (uint8*)(0x1000 * _g.fileslot + 0x400 * n); // 1.0.0 ver
//	xprintf("load %d %x\n", _g.fileslot, buf);
	if (*(int16*)buf == -1) // 初期状態
		return -1;
//	for (int i = 0; i < sizelimit; i++)
	//		list[i] = buf[i];
	memcopy(list, buf, sizelimit);
	return sizelimit;
}

#endif // FLASH_ONLY_ONE

// __STORAGE_H__
#endif