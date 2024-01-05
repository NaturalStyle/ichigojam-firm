#include "IchigoJam.selected/stddef.h"
#include "IchigoJam.selected/screen.h"

static void video_on() {
    // pico sdk / picodvi api を叩いて作っていく
}

static void video_off(int clkdiv) {
    // pico sdk / picodvi api を叩いて作っていく
}

static void video_waitSync(uint num) {
    // pico sdk / picodvi api を叩いて作っていく
}

static inline void uart_putc(char c) {
    // pico sdk / picodvi api を叩いて作っていく
    putchar(c);
}

static void sound_switch(int on) {
    // pico sdk / picodvi api を叩いて作っていく
}

STATIC void put_chr(char c) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int stopExecute() {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

static int IJB_save(int n, uint8* list, int size) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
// 0:ok
static int IJB_load(int n, uint8* list, int sizelimit, int init) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
// ret:size if:-1 _g.err
static int i2c0_init() {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
/* Initialize I2C module  0:ok 1:_g.err */
INLINE int IJB_file() {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

static int IJB_wait(int n, int active) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_led(int on) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_random_seed(int n) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_ana(int n) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_clo() {
    // pico sdk / picodvi api を叩いて作っていく
}

static void IJB_sleep() {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_reset() {
    // pico sdk / picodvi api を叩いて作っていく
}

static void IJB_input(char** line) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE void IJB_bps(int uart, int i2c) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_i2c(uint8 writemode, uint16* param) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}
// -1:ad error 1:ok 0:i2c error
INLINE int IJB_peek(int ad) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_poke(int ad, int n) {
    // pico sdk / picodvi api を叩いて作っていく
}

INLINE int IJB_usr(int ad, int n) {
    // pico sdk / picodvi api を叩いて作っていく
    return 0;//###
}

INLINE void IJB_lcd(uint mode) {
    // pico sdk / picodvi api を叩いて作っていく
}

static inline void ws_out(int port, int nled, int reapeat) {
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_in() {
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_random(int n) {
    // pico sdk / picodvi api を叩いて作っていく
}

int IJB_btn(int n) {
    // pico sdk / picodvi api を叩いて作っていく
}

void IJB_out(int port, int st) {
    // pico sdk / picodvi api を叩いて作っていく
}

void IJB_pwm(int port, int plen, int len) {
    // pico sdk / picodvi api を叩いて作っていく
}
