#include <stdio.h>
#include <sys/stat.h>

// for mkdir
#ifdef WIN32
#include <direct.h>
//#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

void flash_init() {
	_g.lastfile = 0;
}
inline int IJB_file() {
	return _g.lastfile;
}
#define MAX_DATAPATH_LEN 1024
#ifdef WIN32
wchar_t datapath[MAX_DATAPATH_LEN];
void storage_setDirectory(wchar_t* path) {
#else
char datapath[MAX_DATAPATH_LEN];
void storage_setDirectory(char* path) {
#endif
	if (!path || wcslen(path) > MAX_DATAPATH_LEN - 1) {
		return;
	}
	wcscpy(datapath, path);
	for (int i = wcslen(datapath) - 1; i >= 0; i--) {
		if (datapath[i] == '/' || datapath[i] == '\\') {
			datapath[i + 1] = 0;
			break;
		}
	}
}
int ishostnameinclude = 0;
void storage_setHostNameInclude(int b) {
	ishostnameinclude = b;
}
int ishostnamehexenc = 0;
void storage_setHostNameHexEncoding(int b) {
	ishostnamehexenc = b;
}
#ifdef WIN32
void storage_mkdir(wchar_t* fn) {
	if (_wmkdir(fn) == -1) {
//		return 1;
	}
}
#else
void storage_mkdir(char* fn) {
	if (!mkdir(fn, 0777)) {
//		return 1;
	}
}
#endif

#ifdef WIN32
int storage_getFilePath(wchar_t* fn, int n, wchar_t* ext) {
#else
int storage_getFilePath(char* fn, int n, char* ext) {
#endif
	if (wcslen(datapath) > 256)
		return 0;
	_snwprintf(fn, 256, WCS("%sfiles"), datapath);
	storage_mkdir(fn);
	if (ishostnameinclude) {
#ifdef WIN32
		#ifdef WIN_64BIT
		wchar_t hostname[128];
		DWORD hostnamelen = 128;
		GetComputerName((char*)hostname, &hostnamelen); // ?
		#else
		wchar_t hostname[128];
		DWORD hostnamelen = 128;
		GetComputerName(hostname, &hostnamelen);
		#endif
#else
		char hostname[128];
		gethostname(hostname, sizeof(hostname));
#endif
		if (ishostnamehexenc) {
			#ifdef WIN32
			wchar_t enc[128];
			#else
			char enc[128];
			#endif
			int len = wcslen(hostname);
			if (len > 64) {
				len = 64;
			}
			const char* hex = "0123456789ABCDEF";
			for (int i = 0; i < len; i++) {
				char c = hostname[i];
				enc[i * 2] = hex[(c >> 4) & 0xf];
				enc[i * 2 + 1] = hex[c & 0xf];
			}
			enc[len * 2] = 0;
			_snwprintf(fn, 150, WCS("%sfiles/%s"), datapath, enc);
			storage_mkdir(fn);
			_snwprintf(fn, 150, WCS("%sfiles/%s/%d.%s"), datapath, enc, n, ext);
		} else {
			_snwprintf(fn, 150, WCS("%sfiles/%s"), datapath, hostname);
			storage_mkdir(fn);
			_snwprintf(fn, 150, WCS("%sfiles/%s/%d.%s"), datapath, hostname, n, ext);
		}
	} else {
		//	sprintf(fn, "%sichigojam-files/ichigojam-file-%04d.bin", datapath, n);
		_snwprintf(fn, 150, WCS("%sfiles/%d.%s"), datapath, n, ext);
	}
	return 1;
}

int16 storage_list_getNumber(uint8* list, uint16 index) {
	return *(int16*)(list + index);
}
uint8 storage_list_getLength(uint8* list, uint16 index) {
	return *(uint8*)(list + index + 2);
}
int IJB_save(int n, uint8* list, int size) { // err:1 no_err:0
	if (!((n >= 0 && n < 4) || (n >= 100 && n < 100 + 128))) {
		return 1;
	}
	_g.lastfile = n;
	#ifdef WIN32
	wchar_t fn[512];
	#else
	char fn[512];
	#endif
	
	// bin
	if (!storage_getFilePath(fn, n, WCS("bin")))
		return -1;
	FILE* fp = _wfopen(fn, WCS("wb"));
	if (fp == NULL) {
		return 1;
	}
	size = 1024;
	if (fwrite(list, 1, size, fp) < size) {
		fclose(fp);
		return 1;
	}
	if (fclose(fp) == EOF) {
		return 1;
	}
	
	// txt
	if (!storage_getFilePath(fn, n, WCS("txt")))
		return -1;
	fp = _wfopen(fn, WCS("wb"));
	if (fp == NULL) {
		return 1;
	}
	int index = 0;
	#ifdef WIN32
	wchar_t buf[1025];
	#else
	char buf[1025];
	#endif
	for (;;) {
		int num = storage_list_getNumber(list, index);
		if (num == 0) {
			break;
		}
//		printf("line %d len %d\n", num, *(char*)(list + index + 2));
		#ifdef WIN32
		wchar_t linebuf[1024];
		mbstowcs(linebuf, list + index + 3, 1024);
		_snwprintf(buf, 1024, L"%d %s\r\n", num, linebuf);
		#else
		_snwprintf(buf, 1024, WCS("%d %s\r\n"), num, list + index + 3);
		#endif
		int size = wcslen(buf);
		if (fwrite(buf, 1, size, fp) < size) {
			fclose(fp);
			return 1;
		}
		index += storage_list_getLength(list, index) + 4;
	}
	if (fclose(fp) == EOF) {
		return 1;
	}
	
	return 0;
}
int storage_load_txt(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	// txt
	#ifdef WIN32
	wchar_t fn[512];
	#else
	char fn[512];
	#endif
	if (!storage_getFilePath(fn, n, WCS("txt")))
		return -1;
	FILE* fp = _wfopen(fn, WCS("rb"));
	if (fp == NULL) {
		return -1;
	}
	#define LOAD_BUF_LEN 2048
	char buf[LOAD_BUF_LEN];
	int size = fread(buf, 1, LOAD_BUF_LEN, fp);
	if (size == 0) {
		size = -1;
	}
	uint8* limit = list + sizelimit;
	int index = 0;
	int lastline = 0;
	for (;;) {
		while ((buf[index] == '\r' || buf[index] == '\n') && index < size) {
			index++;
		}
		if (index == size) {
			break;
		}
		
		int line = atoi(buf + index);
		if (line <= lastline || line >= 32767) {
			break;
		}
		lastline = line;
		while (buf[index - 1] != ' ' && index < size) {
			index++;
		}
		if (index == size) {
			break;
		}
		int s1 = index;
		while (buf[index] != '\r' && buf[index] != '\n' && index < size) {
			index++;
		}
		if (index == size) {
			break;
		}
		int s2 = index;
		
		int len = s2 - s1;
		//		printf("line %d len %d\n", line, len);
		int maxlen = N_LINEBUF - (getBeam(line) + 1) - (len & 1);
		if (len > maxlen) {
			len = maxlen;
		}
		for (int i = 0; i < len; i++) {
			*(list + 3 + i) = *(buf + s1 + i);
		}
		*(list + 3 + len) = '\0';
		if ((len & 1) == 1) {
			len++;
			*(list + 3 + len) = '\0';
		}
		*(uint16*)list = line;
		*(uint8*)(list + 2) = len;
		list += 4 + len;
		if (index == size || list >= limit) {
			break;
		}
	}
	if (list < limit) {
		*(uint16*)list = 0;
	}
	fclose(fp);
	return index;
}

int IJB_load(int n, uint8* list, int sizelimit, int init) { // ret:size if:-1 err
	if (!((n >= 0 && n < 4) || (n >= 100 && n < 100 + 128))) {
		return -1;
	}
	if (init) {
		_g.lastfile = n;
	}
	#ifdef WIN32
	wchar_t fn[512];
	#else
	char fn[512];
	#endif
	if (!storage_getFilePath(fn, n, WCS("bin")))
		return -1;
	FILE* fp = _wfopen(fn, WCS("rb"));
	if (fp == NULL) {
		return storage_load_txt(n, list, sizelimit, init);
	}
	int size = fread(list, 1, sizelimit, fp);
	if (size == 0) {
		size = -1;
	}
	fclose(fp);
	return size;
}
