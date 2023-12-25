#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/vreg.h"
#include "pico/sem.h"

#include "dvi.h"
#include "dvi_serialiser.h"
#include "common_dvi_pin_configs.h"
#include "sprite.h"

#include "bsp/board.h"
#include "tusb.h"

// for IchigoJam BASIC
#define IJB_BUILD 0
#define IJB_VER 255
#define NO_MEMCPY
#define uart_putc ichigojam_uart_putc
#include "config.h"
#include "IchigoJam.selected/stddef.h"
#include "IchigoJam.selected/lang.h"
#include "IchigoJam.selected/vars.h"
#include "IchigoJam.selected/ram.h"
#include "IchigoJam.selected/screen.h"
#include "IchigoJam.selected/error.h"
#include "ichigojam-jp.fnt.h"
#include "IchigoJam.selected/psg.h"
#include "IchigoJam.selected/basic.h"
//---

// TMDS bit clock 252 MHz
// DVDD 1.2V (1.1V seems ok too)
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define VREG_VSEL VREG_VOLTAGE_1_20
#define DVI_TIMING dvi_timing_640x480p_60hz

#define CHAR_ROWS 24
#define CHAR_COLS 32

#define FONT_SIZE 8

#define MARGIN_WIDTH (FRAME_WIDTH - CHAR_COLS * FONT_SIZE) / 2
#define MARGIN_HEIGHT (FRAME_HEIGHT - CHAR_ROWS * FONT_SIZE) / 2

#define LED_PIN 25

struct dvi_inst dvi0;
uint16_t framebuf[FRAME_WIDTH * FRAME_HEIGHT];
extern uint8* vram;
uint8_t char_code;
uint16_t now = 0;

void core1_main() {
	dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
	dvi_start(&dvi0);
	dvi_scanbuf_main_16bpp(&dvi0);
	__builtin_unreachable();
}

void core1_scanline_callback() {
	// Discard any scanline pointers passed back
	uint16_t* bufptr;
	while (queue_try_remove_u32(&dvi0.q_colour_free, &bufptr))
		;
	// // Note first two scanlines are pushed before DVI start
	static uint scanline = 2;
	bufptr = &framebuf[FRAME_WIDTH * scanline];
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	scanline = (scanline + 1) % FRAME_HEIGHT;
}

//指定したvramの位置の1文字をframebufに反映する
void vram_to_framebuf(int vram_x, int vram_y) {
	int c = vram[vram_y * CHAR_COLS + vram_x];
	for (int y = 0; y < FONT_SIZE; y++) {
		for (int x = 0; x < FONT_SIZE; x++) {
			int pixel = (CHAR_PATTERN[c * FONT_SIZE + y] & (0x80 >> x)) ? 0xffff : 0x0000;
			framebuf[(y + vram_y * FONT_SIZE + MARGIN_HEIGHT) * FRAME_WIDTH
				+ (x + vram_x * FONT_SIZE + MARGIN_WIDTH)] = pixel;
		}
	}
}

void vram_to_framebuf_all() {
	for (int y = 0;y < _g.screenh;y++) {
		for (int x = 0;x < _g.screenw;x++) {
			vram_to_framebuf(x, y);
		}
	}
}









STATIC void exec(char* s) {
	key_flg.bkinsert = key_flg.insert;
	key_flg.insert = 1;

	_g.screen_insertmode = 1;
	_g.key_flg_esc = 0;
	screen_showCursor(0);
	int res = basic_execute(s);
	//screen_showCursor(1); // 1.4b10
	if (_g.cursory == -1)
		_g.cursory = 0;
	if (res == BASIC_RESULT_EXECUTE && !noresmode) {// stop, exec, edit, err
		put_str("OK\n");
	}
	#ifndef IJB_USE_EXCEPTION // 例外使うときには使わないコード
//	if (res != 2) { // 1.2b36 追加、edit時(==2)以外に限定
	//if (res == BASIC_RESULT_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
	if (res == BASIC_RESULT_STOP_OR_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
		key_clearKey();
	}
	#endif

	key_flg.insert = key_flg.bkinsert;
}









int main() {
	board_init();
	stdio_init_all();

	vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
#ifdef RUN_FROM_CRYSTAL
	set_sys_clock_khz(12000, true);
#else
	// Run system at TMDS bit clock
	set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
#endif

	setup_default_uart();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);

	printf("Configuring DVI\n");

	dvi0.timing = &DVI_TIMING;
	dvi0.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
	dvi0.scanline_callback = core1_scanline_callback;
	dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());

	// Once we've given core 1 the framebuffer, it will just keep on displaying
	// it without any intervention from core 0
	sprite_fill16(framebuf, 0x0000, FRAME_WIDTH * FRAME_HEIGHT);
	uint16_t* bufptr = framebuf;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);
	bufptr += FRAME_WIDTH;
	queue_add_blocking_u32(&dvi0.q_colour_valid, &bufptr);

	printf("Core 1 start\n");
	multicore_launch_core1(core1_main);

	// init host stack on configured roothub port
	tuh_init(BOARD_TUH_RHPORT);
	_g.screenw = CHAR_COLS;
	_g.screenh = CHAR_ROWS;
	screen_clear();

	for (int y = 0; y < CHAR_ROWS; ++y) {
		for (int x = 0; x < CHAR_COLS; ++x) {
			vram[y * CHAR_COLS + x] = (y * CHAR_COLS + x) % (256 - 32) + 32;
		}
	}
	for (int y = 0; y < CHAR_ROWS; ++y) {
		for (int x = 0; x < CHAR_COLS; ++x) {
			vram_to_framebuf(x, y);
		}
	}
	char_code = 0;
	while (1) {
		tuh_task();
		if (char_code) {
			screen_putc(char_code);
		}
		vram_to_framebuf_all();

		//暫定的にCでスクリーンクリアする
		if (char_code == 'c') {
			screen_clear();
		}
		char_code = 0;
		// __wfe();
	}









    // from IchigoJamR.src
	char* linebuf = (char*)ram + OFFSET_RAM_LINEBUF;
	if (*linebuf) {
//		put_str(ERR_MESSAGES[ERR_STACK_OVERFLOW - 1]);
//		put_chr('\n');
		if (_g.cursory == -1) { // 1.4.1 前にもってくる
			_g.cursory = 0;
		}
		if (!noresmode) {
			if (!_g.err) {
				_g.err = ERR_COMPLEX_EXPRESSION;
			}
			basic_printError();
//			put_str(ERR_MESSAGES[_g.err - 1]);
//			put_chr('\n');
		}
		// from exec
//		screen_showCursor(1);
	//	if (res != 2) { // 1.2b36 追加、edit時(==2)以外に限定
		//if (res == BASIC_RESULT_ERR) { // 1.3b4 エラー停止の時だけ、キークリア
		key_clearKey(); // 1.3b4 エラー停止の時だけ、キークリア

		key_flg.insert = key_flg.bkinsert;
	}
	for (;;) {
		tuh_task();     //MOTTEKITA



    	// put_num(hid_desc.header.bLength); // bCountryCode);
		//put_num(usb_host.dev_prop.dev_desc.idVendor);
		if (_g.sleepflg) {
			_g.sleepflg = 0;
			*linebuf = 1;

			#define BOOT_WAIT2 25	// 1.4b10 -> b11 ここに移動
			video_waitSync(BOOT_WAIT2);

			exec("LRUN");
		}
		screen_showCursor(1); // 1.4b10
		IJB_random(1);
		video_waitSync(1); // 消すとUART受信漏れ発生?
		
		int key = key_getKey();
		if (key < 0)
			continue;
		if (_g.uartmode_txd & 4) { // 1.2b62 UART echo back
			uart_putc(key); // 1.3b2
//			put_chr(key);
		}
		if (key == 27)
			continue;

		_g.screen_insertmode = key_flg.insert;

		screen_putc(key);

		if (key == '\n') {
			uint8* s = screen_gets();
	//		put_str(s);
			if (*s == '\'') { // 1.1b14
			} else if (*s != 0) {
				uint8 i;
				for (i = 0; i < N_LINEBUF; i++) {
					linebuf[i] = s[i];
					if (!s[i])
						break;
				}
//				_g.screen_insertmode = 1;
				if (s[i]) {
//					put_str("Too long line\n");
					put_str("Too long\n"); // 1.2b45
				} else {
					linebuf[i] = 0; // いっぱいまで入れるとバグっていた 1.2b32
					exec(linebuf);
				}
			}
		}
	}










	__builtin_unreachable();
}









static void video_on(){
    // pico sdk / picodvi api を叩いて作っていく
}

static void video_off(int clkdiv){
    // pico sdk / picodvi api を叩いて作っていく
}

static void video_waitSync(uint){
    // pico sdk / picodvi api を叩いて作っていく
}

static inline void ichigojam_uart_putc(char c){
    // pico sdk / picodvi api を叩いて作っていく
}

static void sound_switch(int on){
    // pico sdk / picodvi api を叩いて作っていく
}

STATIC void put_chr(char c){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int stopExecute(){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

static int IJB_save(int n, uint8* list, int size){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
 // 0:ok
static int IJB_load(int n, uint8* list, int sizelimit, int init){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
 // ret:size if:-1 _g.err
static int i2c0_init(){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
 /* Initialize I2C module  0:ok 1:_g.err */
INLINE int IJB_file(){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

static int IJB_wait(int n, int active){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_led(int on){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_random_seed(int n){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_ana(int n){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_clo(){
    // pico sdk / picodvi api を叩いて作っていく
}

static void IJB_sleep(){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_reset(){
    // pico sdk / picodvi api を叩いて作っていく
}

static void IJB_input(char** line){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_uart(int16 txd, int16 rxd){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_bps(int uart, int i2c){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_i2c(uint8 writemode, uint16* param){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
 // -1:ad error 1:ok 0:i2c error
INLINE int IJB_peek(int ad){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_poke(int ad, int n){
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_usr(int ad, int n){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_lcd(uint mode){
    // pico sdk / picodvi api を叩いて作っていく
}

static void key_clearKey(){
    // pico sdk / picodvi api を叩いて作っていく
}

static inline uint key_getKeyboardID(){
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

static inline void ws_out(int port, int nled, int reapeat){
    // pico sdk / picodvi api を叩いて作っていく
}



struct keyflg_def key_flg;

int key_getKey(void){
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_in(){
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_random(int n){
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_btn(int n){
    // pico sdk / picodvi api を叩いて作っていく
}

void IJB_out(int port, int st){
    // pico sdk / picodvi api を叩いて作っていく
}

void IJB_pwm(int port, int plen, int len){
    // pico sdk / picodvi api を叩いて作っていく
}
