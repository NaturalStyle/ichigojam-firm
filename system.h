//TODO ちゃんと低電力化しているか確かめる
static inline void enterDeepSleep(int wait_us) {
    bool active = video_active();
    video_off(0);
    sleep_us(wait_us);
    if (active) {
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
        enterDeepSleep(n * 16666); // usec, deep
    }
    return 0;
}