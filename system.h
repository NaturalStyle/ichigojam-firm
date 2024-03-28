#ifndef __SYSTEM_H__
#define __SYSTEM_H__

static uint scb_orig, clock0_orig, clock1_orig;
static bool awake;

int getSleepFlag() {
    return IJB_btn(0);
}

void record_clocks() {
    scb_orig = scb_hw->scr;
    clock0_orig = clocks_hw->sleep_en0;
    clock1_orig = clocks_hw->sleep_en1;
}

void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig) {

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);
    set_uart_bps();

    return;
}

static void sleep_callback(void) {
    awake = true;
}

static void rtc_sleep(int sec) {
    sec++;//スリープする時間は (t_alarm と tの差) - 1秒、なので調整する
    int min = sec / 60;
    sec = sec % 60;

    datetime_t t = {
            .year = 2000,
            .month = 1,
            .day = 1,
            .dotw = 0,
            .hour = 0,
            .min = 0,
            .sec = 0
    };

    // Alarm 10 seconds later
    datetime_t t_alarm = {
            .year = 2000,
            .month = 1,
            .day = 1,
            .dotw = 0,
            .hour = 0,
            .min = min,
            .sec = sec
    };

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);

    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}

//TODO ちゃんと低電力化しているか確かめる
//RTCの仕様上、1秒刻みでしかディープスリープの秒数を指定できない
//TODO 端数はsleep_msで誤魔化す？
static inline void enterDeepSleep(int sec) {
    if (sec == 0) {
        return;
    }
    bool active = video_active();
    video_off(0);
    //pico-playground/sleep/hello_sleep/hello_sleep.c　10秒スリープするプログラムと書かれているが、実際は9秒
    //https://ghubcoder.github.io/posts/awaking-the-pico/ 参照
    record_clocks();
    sleep_run_from_xosc();
    awake = false;
    rtc_sleep(sec);
    while (!awake) {
        __wfi();//入れないとスリープから復帰できない時がある、謎
        // printf("Should be sleeping\n");
    }
    recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
    if (active) {
        sleep_ms(1);//入れないとおかしくなる時がある
        video_on();
    }
}

//TODO ボタンを一瞬だけ押すと普通に起動してしまうので、修正する(LRUN0が実行されるのが正しい)
static void IJB_sleep() {
    while (IJB_btn(0)) {
        //ボタンを押している間はスリープに入らない
    }
    video_off(0);
    //pico-playground/sleep/hello_dormant/hello_dormant.c 参照
    record_clocks();
    sleep_run_from_xosc();
    sleep_goto_dormant_until_pin(BTN, false, false);//ボタンを押すまでスリープし続ける
    recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
    IJB_reset();
}

INLINE void IJB_reset() {
    //https://forums.raspberrypi.com/viewtopic.php?t=308166 参照
    watchdog_reboot(0, SRAM_END, 0);
    for (;;) {
        __wfi();
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
        enterDeepSleep(n / 60);
    }
    return 0;
}

#endif