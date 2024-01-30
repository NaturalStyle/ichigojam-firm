//TODO ちゃんと低電力化しているか確かめる
static inline void enterDeepSleep(int waitus) {
    bool is_video_on = dvi0.started;
    video_off(0);
    sleep_us(waitus);
    if (is_video_on) {
        video_on();
    }
}

static int IJB_wait(int n, int active) {
    if (active) {
        if (n < 0) { // ver 1.2b10
            _g.linecnt = 0;
            n = -n;
            while (_g.linecnt < n) {
                if (stopExecute())
                    return 1;
            }
        } else {
            uint64 start = time_us_64();
            while (time_us_64() - start < n * 16666) { //16666 ≒ 1000000 / 60
                if (stopExecute())
                    return 1;
            }
        }
        return 0;
    } else if (n > 0) {
        enterDeepSleep(n * 16666); // msec, deep
    }
    return 0;
}