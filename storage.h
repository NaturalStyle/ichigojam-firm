#include "hardware/flash.h"

#define FLASH_BLOCK_OFFSET 0x1F0000//フラッシュメモリの最後のブロック(31番目)の先頭アドレス

INLINE int IJB_file() {
    return _g.lastfile;
}

//TODO セーブ、ロードをEEPROMでも使えるように拡張する
// err:1 no_err:0
static int IJB_save(int n, uint8* list, int size) {
    int res;
    if (0 <= n && n < N_FLASH_STORAGE) {
        int offset = FLASH_BLOCK_OFFSET + n * FLASH_SECTOR_SIZE;
        //フラッシュメモリに書き込む時は排他制御する
        dvi_stop(&dvi0);
        int save = save_and_disable_interrupts();
        flash_range_erase(offset, FLASH_SECTOR_SIZE);
        flash_range_program(offset, list, FLASH_SECTOR_SIZE);
        dvi_start(&dvi0);
        restore_interrupts(save);
        res = 0;
    } else {
        res = 1;
    }
    return res;
}

// ret:size if:-1 err
static int IJB_load(int n, uint8* list, int sizelimit, int init) {
    if (0 <= n && n <= N_FLASH_STORAGE) {
        int offset = FLASH_BLOCK_OFFSET + n * FLASH_SECTOR_SIZE;
        const uint8_t* flash = (const uint8_t*)(XIP_BASE + offset);
        memcpy(list, flash, sizelimit);
        return sizelimit;
    } else {
        return -1;
    }
}