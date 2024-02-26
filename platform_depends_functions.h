extern struct dvi_inst dvi0;

/* Initialize I2C module  0:ok 1:_g.err */
static int i2c0_init() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("i2c0_init");
    return 0;//###
}

static void IJB_sleep() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_sleep");
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_uart");
}

INLINE int IJB_i2c(uint8 writemode, uint16* param) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_i2c");
    return 0;//###
}
// -1:ad error 1:ok 0:i2c error

INLINE int IJB_usr(int ad, int n) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_usr");
    return 0;//###
}

static inline void ws_out(int port, int nled, int reapeat) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ws_out");
}

void IJB_dac(int port, int val) {

}
