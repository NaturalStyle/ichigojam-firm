//#include <stddef.h>
//#include <stdint.h>
#include <sys/types.h>



#define LOGO "IchigoLatte %d.%d.%d%s by na-s.jp\n"
#define VER0 0
#define VER1 1
#define VER2 0
#define VER3 0
#define VERS ""

#define CHAR_W 8
#define CHAR_H 8
//#define SCREEN_W 32
//#define SCREEN_H 24
#define SCREEN_WH (SCREEN_W*SCREEN_H)

#define FLASH_FILES 4
#define FILE_SIZE 2048
#define FILE_ADDR (FLASH_BASE + 0x20000 - FILE_SIZE*FLASH_FILES)
#define SYSTEM_PAGESIZE 1024

extern const char NULL_STR[];
extern const char E_FILE_ERROR[];
extern volatile uint64_t g_msec;
extern volatile int32_t g_env;
extern volatile uint8_t g_flash_files;

size_t  mem_check();
void*   mem_alloc(size_t size);
void    mem_free(void* mem);

uint32_t rand_get();
void     rand_shake(uint16_t s);

int         flash_erase(uint8_t num);
int         flash_save(uint8_t num, const void* buf, size_t size);
const char* flash_load(uint8_t num);

void  _sleep(int msec, int deep);
char* _line(const char* last, size_t len);
void  _reset();



// // library - string.c
// void* memcpy(void* dst, const void* src, size_t n);
// void* memset(void* b, int c, size_t len);
// char* strncpy(char* dst, const char* src, size_t len);
// int strncmp(const char* s1, const char* s2, size_t n);

// library - stdio.c
int dprintf(int fd, const char *pFormat, ...);
signed int printf(const char *pFormat, ...);



// screen.c
extern unsigned char g_aScreen[];
extern unsigned char* g_pScreen;
extern uint8_t g_nCursorX;
extern uint8_t g_nCursorY;
extern uint8_t g_nCursorFont0;
extern uint8_t g_nCursorFont1;
extern const uint8_t* g_pCursorShape;
extern unsigned char* g_pPinned;

void screen_blink();
uint8_t screen_cursor(uint8_t nFont);
unsigned char* screen_buf(uint8_t x, uint8_t y);
void screen_regulize();
void screen_clear();
void screen_scroll_l();
void screen_scroll_r();
void screen_scroll_u();
void screen_scroll_d();
void screen_rscroll_l();
void screen_rscroll_r();
void screen_rscroll_u();
void screen_rscroll_d();
void screen_cursor_l();
void screen_cursor_r();
void screen_cursor_u();
void screen_cursor_d();
void screen_cursor_ls();
void screen_cursor_rs();
// void screen_putc(unsigned char c);
// void screen_puts(const char* c);

// font.c
#define FONT_NULL 0x20
#define FONT_FILL 0x8f

extern const uint8_t g_aFont[256-32][8];

// keyboard.c
extern volatile uint8_t g_jmp_code;
extern volatile uint8_t g_jmp_mute;
extern volatile uint8_t g_btn_code;

int keyboard_enqKeyBuf(uint8_t k);
uint8_t keyboard_deqKeyBuf();
void    keyboard_scan();

// usbh_hid_keybd.c
extern uint8_t  g_usbkey;
extern uint16_t g_usbkey_delay;

void keybrd_putsc_on(uint8_t sc);

// file.c
#define FD_STDIN  0
#define FD_STDOUT 1
// #define FD_STDERR 2  // for future
#define FD_USER   2
#define FD_USER2  3

#define O_RDONLY 0x00000000
#define O_WRONLY 0x00000001

extern const char* file_flash_buf;

int file_dopen(int fd, const char* name, size_t len, int flags);
int file_dclose(int fd);
int file_dread(int fd, void* buf, size_t count);
int file_dwrite(int fd, const void* buf, size_t count);
const void* file_ddirect(int fd);

// shell.c
void shell_run(int autorun);
