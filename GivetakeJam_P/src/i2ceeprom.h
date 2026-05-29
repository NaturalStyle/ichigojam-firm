#ifndef __I2CEEPROM_H__
#define __I2CEEPROM_H__

#include "all_includes.h"


//https://www.mouser.jp/datasheet/2/268/24AA1025_24LC1025_24FC1025_1024_Kbit_I2C_Serial_EE-2853738.pdf 参照
/*
client address は最下位ビットを 0 ビット目とすると、
0ビット目  : read/write 指定 (read=1, write=0) -> pico-sdk 側で指定
1〜2ビット目: 他EEPROM識別用 (ここでは 00 固定)
3ビット目  : EEPROM の上位アドレス指定に使用
4〜7ビット目: 1010 固定

24FC1025-I/P は 1Mbit = 128KB = 2^17 bytes なので、
A16 ビットを client address 側へ出す必要がある。
24LC64 / 24LC256 では A16 は不要だが、低アドレス領域では
0b1010000 のままで問題なくアクセスできる。

このため、client address は毎回 memory_address から再計算する。
*/

/*
 * ===== MODIFIED =====
 * Use the minimum safe page size so one binary can support:
 *   - 24LC64   :  32 bytes/page
 *   - 24LC256  :  64 bytes/page
 *   - 24FC1025 : 128 bytes/page
 *
 * By always writing in 32-byte chunks and never crossing a page boundary,
 * page wrap corruption can be avoided on all supported devices.
 */
#define EEPROM_SAFE_PAGE_SIZE 32

static uint8_t calc_client_address(int memory_address) {
    uint8_t res = 0b1010000;

    /*
     * ===== MODIFIED =====
     * For 24FC1025, A16 is mapped into client address bit2.
     * For smaller EEPROMs this bit is effectively unused, but low-address
     * access still works because res remains 0b1010000.
     */
    if ((memory_address >> 16) & 1) {
        res |= 0b100;
    }

    return res;
}

static void make_write_src(int memory_address, uint8_t* list, int len, uint8_t* src) {
    src[0] = (uint8_t)(memory_address >> 8);
    src[1] = (uint8_t)memory_address;

    for (int i = 0; i < len; i++) {
        src[2 + i] = list[i];
    }
}

/*
// ret:size if:-1 err
int read_eeprom(int memory_address, uint8_t* list, int len) {
    uint8_t client_address = calc_client_address(memory_address);
    uint8_t src[2];
    src[0] = (uint8_t)(memory_address >> 8);
    src[1] = (uint8_t)memory_address;
    int res = i2c_write_timeout_us(i2c_default, client_address, src, 2, true, TIMEOUT_US);
    if (res < 0) {
        return -1;
    }
    // res = i2c_read_timeout_us(i2c_default, client_address, list, len, false, TIMEOUT_US);
    res = i2c_read_timeout_us(i2c_default, client_address, list, 1, false, TIMEOUT_US);
    list++;
    res += i2c_read_timeout_us(i2c_default, client_address, list, len - 1, false, TIMEOUT_US);
    return res == len ? res : -1;
}
*/

/*
 * ===== MODIFIED =====
 * Read EEPROM in small chunks and recalculate client_address every time.
 *
 * Reasons:
 * 1. Safer for 24FC1025 when crossing 64KB boundary.
 * 2. Matches the write-side logic more closely.
 * 3. Easier to debug when an EEPROM device or bus is unstable.
 *
 * Return value:
 *   >=0 : bytes read
 *   -1  : error
 */
int read_eeprom(int memory_address, uint8_t* list, int len) {
    int total = 0;

    while (len > 0) {
        /*
         * ===== MODIFIED =====
         * Chunk size is limited to EEPROM_SAFE_PAGE_SIZE for simplicity.
         * Read operations themselves are not page-limited like writes,
         * but small chunks keep address handling safe and consistent.
         */
        int tlen = len > EEPROM_SAFE_PAGE_SIZE ? EEPROM_SAFE_PAGE_SIZE : len;

        uint8_t client_address = calc_client_address(memory_address);

        uint8_t src[2];
        src[0] = (uint8_t)(memory_address >> 8);
        src[1] = (uint8_t)memory_address;

        int res = i2c_write_timeout_us(i2c_default, client_address, src, 2, true, TIMEOUT_US);
        if (res < 0) {
            return -1;
        }

        res = i2c_read_timeout_us(i2c_default, client_address, list, tlen, false, TIMEOUT_US);
        if (res != tlen) {
            return -1;
        }

        memory_address += tlen;
        list += tlen;
        len -= tlen;
        total += tlen;
    }

    return total;
}

/*
// err:1 no_err:0
int write_eeprom(int memory_address, uint8_t* list, int len) {
    uint8_t client_address = calc_client_address(memory_address);
    uint8_t src[EEPROM_PAGE_SIZE + 2];
    while (len > 0) {
        int tlen = len > EEPROM_PAGE_SIZE ? EEPROM_PAGE_SIZE : len;
        len -= tlen;
        make_write_src(memory_address, list, tlen, src);
        memory_address += tlen;
        list += tlen;
        int res = i2c_write_timeout_us(i2c_default, client_address, src, tlen + 2, false, TIMEOUT_US);
        sleep_us(5000 * I2C_DEFAULT_BPS / (double)i2c_baudrate);
        if (res < 0) {
            return 1;
        }
    }
    return 0;
}
*/

/*
 * ===== MODIFIED =====
 * Write EEPROM safely with page-boundary protection.
 *
 * Important:
 * - Never cross an EEPROM page boundary in a single write command.
 * - Recalculate client_address on every chunk, needed for 24FC1025.
 * - Use 32-byte safe chunks for compatibility with all supported devices.
 *
 * Return value:
 *   0 : success
 *   1 : error
 */
int write_eeprom(int memory_address, uint8_t* list, int len) {
    uint8_t src[EEPROM_SAFE_PAGE_SIZE + 2];

    while (len > 0) {
        /*
         * ===== MODIFIED =====
         * Recalculate client address every chunk.
         * This is required when memory_address crosses a 64KB bank
         * on 24FC1025.
         */
        uint8_t client_address = calc_client_address(memory_address);

        /*
         * ===== MODIFIED =====
         * Do not cross page boundary.
         * This avoids page-wrap corruption on 24LC64 / 24LC256.
         */
        int page_remain = EEPROM_SAFE_PAGE_SIZE - (memory_address % EEPROM_SAFE_PAGE_SIZE);
        int tlen = len > page_remain ? page_remain : len;

        make_write_src(memory_address, list, tlen, src);

        int res = i2c_write_timeout_us(
            i2c_default,
            client_address,
            src,
            tlen + 2,
            false,
            TIMEOUT_US
        );

        /*
         * EEPROM internal write cycle wait.
         * A fixed wait is simple and stable.
         * 5ms is generally safe for these devices.
         */
        sleep_us(5000);

        if (res < 0) {
            return 1;
        }

        memory_address += tlen;
        list += tlen;
        len -= tlen;
    }

    return 0;
}

// ===== MODIFIED =====
// Check whether external I2C EEPROM exists at address 0x50.
//
// Returns:
//   1 = EEPROM exists
//   0 = not found
//
// Note:
//   24LC64 / 24LC256 / 24FC1025 all respond at 0x50.
//   For reliability, retry a few times because EEPROM may still be busy
//   just after a write cycle.
S_INLINE int i2c_eeprom_exists() {
    uint8_t dummy;
    uint8_t addr = 0x50;

    // I2C初期化
    i2c0_init();

    for (int i = 0; i < 3; i++) {
        int res = i2c_read_timeout_us(i2c_default, addr, &dummy, 1, false, TIMEOUT_US);
        if (res == 1) {
            return 1;
        }
        sleep_us(2000);
    }
    return 0;
}

#endif
