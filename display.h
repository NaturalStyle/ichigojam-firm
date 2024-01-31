#define CHAR_MAX_COLS 32
#define CHAR_MAX_ROWS 24

void video_on() {
    SCREEN_W = CHAR_MAX_COLS >> _g.screen_big;
    SCREEN_H = CHAR_MAX_ROWS >> _g.screen_big;
    dvi_start(&dvi0);
}

//TODO ckldivを使う必要があるか検討する
void video_off(int clkdiv) {
    dvi_stop(&dvi0);
}

inline int video_active() {
    return dvi0.started;
}

//TODO 何のためのコマンドか確認する
INLINE void IJB_lcd(uint mode) {
    //SWITCHを実行すると呼ばれる
    //何もしなくていい？
}

void video_waitSync(uint num) {
    //何もしない
}