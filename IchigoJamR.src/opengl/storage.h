#include <stdio.h>

void flash_init() {
}
inline int IJB_file() {
	return _g.lastfile;
}
int IJB_save(int n, uint8* list, int size) { // err:1 no_err:0
	if (!((n >= 0 && n < 4) || (n >= 100 && n < 100 + 128))) {
		return 1;
	}
	_g.lastfile = n;
	char fn[128];
	sprintf(fn, "ichigojam-file-%04d.bin", n);
	FILE* fp = fopen(fn, "wb");
	size = 1024;
	fwrite(list, 1, size, fp);
	if (fclose(fp) == EOF)
		return 1;
	return 0;
}
int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (!((n >= 0 && n < 4) || (n >= 100 && n < 100 + 128))) {
		return -1;
	}
	_g.lastfile = n;
	char fn[128];
	sprintf(fn, "ichigojam-file-%04d.bin", n);
	FILE* fp = fopen(fn, "rb");
	if (fp == NULL)
		return -1;
	int size = fread(list, 1, sizelimit, fp);
	if (size == 0) {
		size = -1;
	}
	fclose(fp);
	return size;
}
