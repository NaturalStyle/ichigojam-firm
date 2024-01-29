extern hid_keyboard_report_t now_key_report;
extern uint8_t const keycode2ascii[128][4];
extern struct dvi_inst dvi0;

static inline void uart_putc(char c) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("uart_putc");
    putchar(c);
}

static void sound_switch(int on) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("sound_switch");
}

STATIC void put_chr(char c) {
    if (_g.uartmode_txd > 0) { // 1.0.2b12 uartを先に
        uart_putc(c);
    }
    screen_putc(c);
}

INLINE int stopExecute() {
    // pico sdk / picodvi api を叩いて作っていく
    // printf("stopexecute");
    return _g.key_flg_esc;
}

/* Initialize I2C module  0:ok 1:_g.err */
static int i2c0_init() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("i2c0_init");
    return 0;//###
}

static int IJB_wait(int n, int active) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_wait");
    uint64 before = time_us_64();
    while (time_us_64() - before < n * 16000) {
        if (stopExecute())
            return 1;
    }
    // if (active) {
    //     if (n < 0) { // ver 1.2b10
    //         while (time_us_64() - before < n * 16000) {
    //             if (stopExecute())
    //                 return 1;
    //         }
    //     } else {
    //         for (int i = 0; i < n; i++) {
    //             if (stopExecute())
    //                 return 1;
    //             video_waitSync(1);
    //         }
    //     }
    //     return 0;
    // } else if (n > 0) {
    //     // enterDeepSleep(n * 16); // msec, deep
    // }
    return 0;
}

INLINE void IJB_led(int on) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_led");
}

INLINE void IJB_random_seed(int n) {
    random_seed(n);
    printf("ijb_random_seed");
}

int IJB_random(int n) {
    uint r = rnd();
    if (n <= 0) {
        return 0;
    }
    return (r >> 1) % n; // マイナスがでてしまう対処、udivが使われる div?? こっちの方が小さい
    printf("ijb_random");
}

INLINE int IJB_ana(int n) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_ana");
    return 0;//###
}

INLINE void IJB_clo() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_clo");
}

static void IJB_sleep() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_sleep");
}

INLINE void IJB_reset() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_reset");
}

static void IJB_input(char** line) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_input");
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_uart");
}

INLINE void IJB_bps(int uart, int i2c) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_bps");
}

INLINE int IJB_i2c(uint8 writemode, uint16* param) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_i2c");
    return 0;//###
}
// -1:ad error 1:ok 0:i2c error
INLINE int IJB_peek(int ad) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_peek");
    return 0;//###
}

INLINE void IJB_poke(int ad, int n) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_poke");
}

INLINE int IJB_usr(int ad, int n) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_usr");
    return 0;//###
}

INLINE void IJB_lcd(uint mode) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_lcd");
}

static inline void ws_out(int port, int nled, int reapeat) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ws_out");
}

void IJB_pwm(int port, int plen, int len) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_pwm");
}
