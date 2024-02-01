extern struct dvi_inst dvi0;

static void sound_switch(int on) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("sound_switch");
}

/* Initialize I2C module  0:ok 1:_g.err */
static int i2c0_init() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("i2c0_init");
    return 0;//###
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

static inline void ws_out(int port, int nled, int reapeat) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ws_out");
}

void IJB_pwm(int port, int plen, int len) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_pwm");
}

void IJB_dac(int port, int val) {

}
