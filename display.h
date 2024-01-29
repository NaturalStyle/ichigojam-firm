#define CHAR_MAX_COLS 32
#define CHAR_MAX_ROWS 24
static void video_on() {
    SCREEN_W = CHAR_MAX_COLS >> _g.screen_big;
    SCREEN_H = CHAR_MAX_ROWS >> _g.screen_big;
    dvi_start(&dvi0);
}

//TODO ckldivを使う必要があるか検討する
static void video_off(int clkdiv) {
    dvi_stop(&dvi0);
}

static void video_waitSync(uint num) {
    //何もしない
}