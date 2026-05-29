// picoに焼いたプログラムはフラッシュメモリの先頭に配置される
// プログラム領域の終わりは、build/IchigoJam_P.elf.map の flash_binary_end の下7桁を見ればわかるが、かなり余裕がある
// IchigoJam のプログラムはフラッシュメモリの後ろの方に保存する
// フラッシュメモリの容量 = 2M = 0x200000
// 1ブロックの容量 = 0x10000
// FLASH_SECTOR_SIZE = 0x1000 (単位はバイト)

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "all_includes.h"

/*
 * ===== MODIFIED =====
 * Internal Flash program storage
 *
 * SAVE0 / LOAD0 で操作するセクタは、一番後ろから2番目のセクタ。
 * 一番最後のセクタは KBD の設定値などに使う。
 *
 * 4KB版では 1スロット = 1セクタ = 4096バイト。
 */
#define FLASH_IJ_OFFSET (0x200000 - FLASH_SECTOR_SIZE * 2)

/*
 * External EEPROM file numbers
 *
 * Internal Flash : 0 .. 24
 * External I2C   : 100 ..
 */
#define EEPROM_OFFSET 100

/*
 * ===== MODIFIED =====
 * External EEPROM slot count
 *
 * Old version:
 *   128 slots (1024-byte program model)
 *
 * 4KB version:
 *   display / SAVE / LOAD range is fixed to 100 .. 131
 *   -> 32 slots
 *
 * Note:
 *   This is the accepted slot range in SAVE / LOAD.
 *   FILES0 display range in basic.h should match this.
 */
#define EEPROM_SIZE 32

/*
 * ===== MODIFIED =====
 * One program size
 *
 * IJB_SIZEOF_LIST includes management bytes,
 * so actual storable program size is:
 *   SIZELIMIT = IJB_SIZEOF_LIST - 2
 *
 * In the 4KB version, this becomes 4096 bytes.
 */
#define SIZELIMIT (IJB_SIZEOF_LIST - 2)

INLINE int IJB_file() {
    return _g.lastfile;
}

uint32_t calc_flash_offset(int n) {
    return FLASH_IJ_OFFSET - n * FLASH_SECTOR_SIZE;
}

uint32_t get_config_offset() {
    return calc_flash_offset(-1); // 一番最後のセクタ
}

uint8_t* get_flash(uint32_t offset) {
    return (uint8_t*)(XIP_BASE + offset);
}

// err:1 no_err:0
static int IJB_save(int n, uint8* list, int size) {
    _g.lastfile = n;
    int res;

    if (0 <= n && n < N_FLASH_STORAGE) {
        uint32_t offset = calc_flash_offset(n);

        // フラッシュメモリに書き込む時は排他制御する
        video_off(0);
        int save = save_and_disable_interrupts();
        flash_range_erase(offset, FLASH_SECTOR_SIZE);
        flash_range_program(offset, list, FLASH_SECTOR_SIZE);
        video_on();
        restore_interrupts(save);

        res = 0;

    } else if (EEPROM_OFFSET <= n && n < EEPROM_OFFSET + EEPROM_SIZE) {
        /*
         * ===== MODIFIED =====
         * External EEPROM slot mapping for 4KB version:
         *   SAVE100 -> address 0
         *   SAVE101 -> address 4096
         *   SAVE102 -> address 8192
         *   ...
         */
        res = write_eeprom((n - EEPROM_OFFSET) * SIZELIMIT, list, SIZELIMIT);

    } else {
        res = 1;
    }

    return res;
}

// ret:size if:-1 err
static int IJB_load(int n, uint8* list, int sizelimit, int init) {
    if (init) {
        _g.lastfile = n;
    }

    if (0 <= n && n < N_FLASH_STORAGE) {
        const uint8_t* flash = get_flash(calc_flash_offset(n));

        // TODO:
        // IchigoJam以外で使ったことのあるPicoでは正しく判定できないので、別の方法を考える
        if (*(int16*)flash == -1) { // 未使用領域の初期値
            *(uint16*)list = 0;
            return -1;
        }

        memcpy(list, flash, sizelimit);
        return sizelimit;

    } else if (EEPROM_OFFSET <= n && n < EEPROM_OFFSET + EEPROM_SIZE) {
        /*
         * ===== MODIFIED =====
         * External EEPROM slot mapping for 4KB version:
         *   LOAD100 -> address 0
         *   LOAD101 -> address 4096
         *   LOAD102 -> address 8192
         *   ...
         */
        return read_eeprom((n - EEPROM_OFFSET) * SIZELIMIT, list, sizelimit);

    } else {
        return -1;
    }
}

#endif
