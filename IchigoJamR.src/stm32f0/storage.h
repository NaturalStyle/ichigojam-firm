void flash_init() {
}
inline int IJB_file() {
	return _g.lastfile;
}
static inline int flash_write(uint8* flashaddr, uint8* data1k) { // flashaddr(0x08000000 + 1k * n) -> 0: ok, 1:erase_err, 2:write_err
	// unlock
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	
	int ok = 0;
	
	// erase
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = (uint32)flashaddr;
	FLASH->CR |= FLASH_CR_STRT;
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR &= ~FLASH_CR_PER;
	ok = FLASH->SR & FLASH_SR_EOP;
	FLASH->SR &= ~FLASH_SR_EOP;
	
	if (!ok) {
		return 1;
	}
	
	// write
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PG;
	uint16* dst = (uint16*)flashaddr; // need 16bit each
	uint16* src = (uint16*)data1k;
	for (int i = 0; i < 1024 / 2; i++) {
		dst[i] = src[i];
		while (FLASH->SR & FLASH_SR_BSY);
	}
	FLASH->CR &= ~FLASH_CR_PG;
	ok = FLASH->SR & FLASH_SR_EOP;
	FLASH->SR &= ~FLASH_SR_EOP;
	
	if (!ok) {
		return 2;
	}
	
	// lock
	FLASH->CR |= FLASH_CR_LOCK;
	
	return 0;
}

int IJB_save(int n, uint8* list, int size) {
	_g.lastfile = n;
	if (n < 0 && n > 3) {
		return 1;
	}
	uint8* buf = (uint8*)((0x08000000 + (24 * 1024)) - n * 1024);
	flash_write(buf, list);
	return 0;
}
int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (init)
		_g.lastfile = n;
	if (n < 0 && n > 3) {
		return -1;
	}
	uint8* buf = (uint8*)((0x08000000 + (24 * 1024)) - n * 1024);
	if (*(int16*)buf == -1) // 初期状態
		return -1;
	memcopy(list, buf, sizelimit);
	return sizelimit;
}
