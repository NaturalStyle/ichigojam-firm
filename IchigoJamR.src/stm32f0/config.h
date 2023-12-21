// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

#define VIDEO_LINES 262 // display.h LINES_FRAME

//#define PSG_TICK_FREQ 60
#define PSG_TICK_PER_SEC 60
#define PSG_TICK_FREQ (PSG_TICK_PER_SEC * VIDEO_LINES) // VIDEO_LINES -> LINESに統合を

#define PSG_TRUE_TONE // うまくいってない

#define IJB_TITLE "IchigoJam BASIC 1.2Sb" STRING2(IJB_BUILD) " jig.jp\n"
