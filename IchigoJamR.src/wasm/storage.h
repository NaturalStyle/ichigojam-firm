#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "../stddef.h"
#include "../screen.h"

uint8 file[FILE_SIZE];

uint file_update_flg = 0;

static void flash_init() {
	/*
	for (int i = 0; i < 1024 * 4; i++) {
		file[i] = 0;
	}
	*/
}
static inline int IJB_file() {
	return _g.lastfile;
}
int storage_contains(int n) {
	return (n >= 0 && n < 4) || (n >= 100 && n <= 227);
}
static int IJB_save(int n, uint8* list, int size) {
	_g.lastfile = n;
	if (storage_contains(n)) {
		memcopy((void*)(file + 1024 * n), (const void*)list, 1024);
		file_update_flg = 1;
		return 0;
	}
	return 1;
}
static int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (init) {
		_g.lastfile = n;
	}
	if (storage_contains(n)) {
		memcopy((void*)list, (const void*)(file + 1024 * n), sizelimit);
		return sizelimit;
	}
	return -1;
}

// __STORAGE_H__
#endif