static void video_on() {
    dvi_start(&dvi0);
}

//TODO ckldivを使う必要があるか検討する
static void video_off(int clkdiv) {
    dvi_stop(&dvi0);
}

static void video_waitSync(uint num) {
    //何もしない
}