// created by Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/
// license CC BY-NC-SA 4.0 http://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

inline void sound_init();
static inline void sound_tick();

uint64_t g_sndstop = 0;

inline void sound_init() {
    _g.psgratio = 1;

    rcu_periph_clock_enable(RCU_AF);
    AFIO_PCF0 = (AFIO_PCF0 & ~AFIO_PCF0_TIMER1_REMAP) | (0b01 << 8);    // timer1 remap
    rcu_periph_clock_enable(RCU_TIMER1);    // sound

    // sound
    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(15-8)) | GPIO_MODE_SET(15-8, 0b0011);
    GPIO_BOP(GPIOA) = GPIO_BOP_CR15;

    // timer for sound
    eclic_irq_enable(TIMER1_IRQn, 12, 0);
    TIMER_CHCTL0(TIMER1) &= ~TIMER_CHCTL0_CH0COMCTL;
    TIMER_CHCTL0(TIMER1) |= (TIMER_OC_MODE_PWM0 << 0);
    TIMER_CHCTL2(TIMER1) |= TIMER_CHCTL2_CH0EN;
    TIMER_DMAINTEN(TIMER1) = TIMER_DMAINTEN_UPIE;
    TIMER_CTL0(TIMER1) |= TIMER_CTL0_ARSE;

    /*
	// psg_beep(440 * 2, 300);
	uint32_t hz = 440 * 2;
	uint32_t msec = 100;
    if (hz && msec) {
        g_sndstop = get_timer_value() + (msec * (SystemCoreClock / 4 / 1000));

        GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(15-8)) | GPIO_MODE_SET(15-8, 0b1011);

        TIMER_PSC(TIMER1) = 96 - 1;
        TIMER_CAR(TIMER1) = (1000000 / (int32_t)hz) - 1;
        TIMER_CH0CV(TIMER1) = (1000000 / (int32_t)hz) / 2 - 1;
        TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;
    }
    */
}
static inline void sound_tick() {
}

static void sound_switch(int on) {
    if (!on) {
        sound_tone(0);
    }
}

static inline void sound_tone(int t) {
    if (t) {
        //put_num(t);
        //uint32_t hz = t;
        //uint32_t len = (1000000 / (int32_t)hz);
        uint32_t len = t;
        //g_sndstop = get_timer_value() + (msec * (SystemCoreClock / 4 / 1000));

        GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(15-8)) | GPIO_MODE_SET(15-8, 0b1011);

        TIMER_PSC(TIMER1) = (28 * 4) - 1; // 96 - 1; // TIMERx_PSC 16bit
        //TIMER_PSC(TIMER1) = (28 * 4) - 1; // 96 - 1; // TIMERx_PSC 16bit
        TIMER_CAR(TIMER1) = len - 1; // TIMERx_CAR 16bit
        TIMER_CH0CV(TIMER1) = (len >> 1) - 1; // TIMERx_CH0CV 16bit
        TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;
    } else {
        TIMER_CTL0(TIMER1) &= ~TIMER_CTL0_CEN;
        GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(15-8)) | GPIO_MODE_SET(15-8, 0b0011);
        GPIO_BOP(GPIOA) = GPIO_BOP_CR15;
    }
}

void TIMER1_IRQHandler() {   // sound
    if ((TIMER_INTF(TIMER1) & TIMER_INTF_UPIF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_UPIE)) {
        /*
        if (g_sndstop <= get_timer_value()) {
            TIMER_CTL0(TIMER1) &= ~TIMER_CTL0_CEN;
        }
        */

        TIMER_INTF(TIMER1) &= ~TIMER_DMAINTEN_UPIE;
    }
}
