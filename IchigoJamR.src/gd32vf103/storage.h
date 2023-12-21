// SYSTEM_PAGESIZE 1024
// ../src//gd32vf103/system.h:#define FILE_ADDR (FLASH_BASE + 0x20000 - FILE_SIZE * FLASH_FILES)
//  アドレス最後から 設定1KB, file0, file1, file2 ..

#define EEPROM_SAVE 1

void flash_init() {
}

const char* flash_getAddress(uint8_t num) { // 0:setting, 1- files
    //return num < FLASH_FILES ? (void*)(FILE_ADDR + FILE_SIZE * num) : NULL; // 前から
    return num < FLASH_BLOCKS ? (void*)(FILE_ADDR + FILE_SIZE * (FLASH_BLOCKS - 1) - FILE_SIZE * num) : NULL; // 後ろから
}
int flash_erase(uint8_t num) {
    // register int r = 0;

    const char* adr = flash_getAddress(num);

    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END);
    fmc_flag_clear(FMC_FLAG_WPERR);
    fmc_flag_clear(FMC_FLAG_PGERR);

    /* erase the flash pages */
    uint32_t erase_counter;
    for (erase_counter = 0; erase_counter < FILE_SIZE / SYSTEM_PAGESIZE; erase_counter++) {
        fmc_page_erase((uint32_t)adr + (SYSTEM_PAGESIZE * erase_counter));
        fmc_flag_clear(FMC_FLAG_END);
        fmc_flag_clear(FMC_FLAG_WPERR);
        fmc_flag_clear(FMC_FLAG_PGERR);
    }

    /* lock the main FMC after the erase operation */
    fmc_lock();

    // /* check flash whether has been erased */
    // uint32_t* ptrd = (uint32_t*)adr;
    // uint32_t i;
    // for(i = 0; i < (FILE_SIZE>>2); i++){
    //     if(0xFFFFFFFF != (*ptrd)){
    //         r = -1;
    //         break;
    //     }else{
    //         ptrd++;
    //     }
    // }
    
    return 0; // r;
}
void flash_write(int num, uint8* list, int size) {
	flash_erase(num);
	const char* adr = flash_getAddress(num);
	fmc_unlock();	/* unlock the flash program/erase controller */

	/* program flash */
	uint32_t w = 0;
	while (w < size) {
		fmc_word_program((uint32_t)adr + w, *((uint32_t*)(list + w)));
		w += 4;
		fmc_flag_clear(FMC_FLAG_END);
		fmc_flag_clear(FMC_FLAG_WPERR);
		fmc_flag_clear(FMC_FLAG_PGERR);
	}
	/* lock the main FMC after the program operation */
	fmc_lock();
}

inline int IJB_file() {
	return _g.lastfile;
}

#if EEPROM_SAVE == 1
#include "i2ceeprom.h"

#define EEPROM_PSIZE 32
#define EEPROM_WSIZE 1024
#define EEPROM_OFFSET 100
#define EEPROM_SIZE 128

#endif

int IJB_save(int n, uint8* list, int size) {
	_g.lastfile = n;

#if EEPROM_SAVE == 1
	// eeprom 32KB
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
		//if (i2c0_init())
		//	return -1;
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		for (int i = 0; i < EEPROM_WSIZE; i += EEPROM_PSIZE) {
			if (!accessEEPROM(1, ad + i, (uint8_t*)(list + i), EEPROM_PSIZE)) {
				return 1; // err
			}
			video_waitSync(1);
		}
		return 0;
	}
#endif

	if (n < 0 || n >= N_FLASH_STORAGE) {
		return -1;
	}
    flash_write(n + 1, list, FILE_SIZE);
	return 0;
}
int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (init)
		_g.lastfile = n;
	
#if EEPROM_SAVE == 1
	// eeprom
	if (n >= EEPROM_OFFSET && n < (EEPROM_OFFSET + EEPROM_SIZE)) { // 100-227 までの128コ保存可能
        /*
		if (init) {
			if (i2c0_init())
				return -1;
		}
        */
		int ad = (n - EEPROM_OFFSET) * EEPROM_WSIZE;
		for (int i = 0; i < sizelimit; i += EEPROM_PSIZE) {
			if (!accessEEPROM(0, ad + i, (uint8_t*)(list + i), EEPROM_PSIZE)) {
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

	if (n < 0 || n >= N_FLASH_STORAGE) {
		return -1;
	}
	const char* adr = flash_getAddress(n + 1);
	if (*(int16*)adr == -1) // 初期状態?
		return -1;
	memcopy(list, adr, sizelimit);
	return sizelimit;
}
