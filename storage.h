#define FLASH_BLOCK_OFFSET (0x200000 - FLASH_SECTOR_SIZE * 2)//SAVE0,LOAD0で読み出す部分の先頭(一番後ろのセクターはKBDの設定値などを保存するのに使う)

INLINE int IJB_file() {
    return _g.lastfile;
}

uint32_t calc_offset(int n) {
    return FLASH_BLOCK_OFFSET - n * FLASH_SECTOR_SIZE;
}

uint32_t get_config_offset() {
    return calc_offset(-1);
}

uint8_t* get_flash(uint32_t offset) {
    return (uint8_t*)(XIP_BASE + offset);
}

//TODO セーブ、ロードをEEPROMでも使えるように拡張する
// err:1 no_err:0
static int IJB_save(int n, uint8* list, int size) {
    int res;
    if (0 <= n && n < N_FLASH_STORAGE) {
        uint32_t offset = calc_offset(n);
        //フラッシュメモリに書き込む時は排他制御する
        video_off(0);
        int save = save_and_disable_interrupts();
        flash_range_erase(offset, FLASH_SECTOR_SIZE);
        flash_range_program(offset, list, FLASH_SECTOR_SIZE);
        video_on();
        restore_interrupts(save);
        res = 0;
    } else {
        res = 1;
    }
    return res;
}

// ret:size if:-1 err
static int IJB_load(int n, uint8* list, int sizelimit, int init) {
    if (0 <= n && n < N_FLASH_STORAGE) {
        uint32_t offset = calc_offset(n);
        const uint8_t* flash = get_flash(offset);
        memcpy(list, flash, sizelimit);
        return sizelimit;
    } else {
        return -1;
    }
}