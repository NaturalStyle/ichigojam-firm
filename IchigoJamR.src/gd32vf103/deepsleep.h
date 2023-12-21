#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__

inline void enterDeepSleep(int msec) {
    int deep = 1;
	if (deep) {
        /* enable PMU and BKPI clocks */
        rcu_periph_clock_enable(RCU_BKPI);
        rcu_periph_clock_enable(RCU_PMU);
        /* enable access to BKP domain */
        PMU_CTL |= PMU_CTL_BKPWEN;

        /* reset backup domain */
        RCU_BDCTL |= RCU_BDCTL_BKPRST;
        RCU_BDCTL &= ~RCU_BDCTL_BKPRST;

        /* enable IRC40K */
        RCU_RSTSCK |= RCU_RSTSCK_IRC40KEN;
        /* wait till IRC40K is ready */
        uint32_t tout = 0xFFFFF;   // #define OSC_STARTUP_TIMEOUT         ((uint32_t)0xFFFFFU)
        while( --tout && !( RCU_RSTSCK & RCU_RSTSCK_IRC40KSTB ) );
        
        /* select RCU_IRC40K as RTC clock source */
        RCU_BDCTL = (RCU_BDCTL & ~RCU_BDCTL_RTCSRC) | RCU_RTCSRC_IRC40K;

        /* enable RTC Clock */
        rcu_periph_clock_enable(RCU_RTC);

        /* wait for RTC registers synchronization */
        RTC_CTL &= ~RTC_CTL_RSYNF;
        while( !(RTC_CTL & RTC_CTL_RSYNF) );

        /* wait until last write operation on RTC registers has finished */
        while( !(RTC_CTL & RTC_CTL_LWOFF) );
        /* enter RTC configuration mode */
        RTC_CTL |= RTC_CTL_CMF;

        /* set RTC prescaler: set RTC period to 1ms */
        RTC_PSCL = 40 - 1;
        RTC_PSCH = 0;

        /* set RTC counter */
        RTC_CNTL = 0;
        RTC_CNTH = 0;

        /* set RTC alarm */
        RTC_ALRML = (msec - 1) & 0xffff;
        RTC_ALRMH = (msec - 1) >> 16;

        /* exit RTC configuration mode */
        RTC_CTL &= ~RTC_CTL_CMF;
        /* wait until last write operation on RTC registers has finished */
        while( !(RTC_CTL & RTC_CTL_LWOFF) );

        /* disable access to BKP domain */
        PMU_CTL &= ~PMU_CTL_BKPWEN;

        /* enable EXTI17(RTC Alarm) events */
        EXTI_EVEN |= EXTI_EVEN_EVEN17;
        /* enable rising edge trigger for wakeup */
        EXTI_RTEN |= EXTI_RTEN_RTEN17;



        pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFE_CMD);



        EXTI_EVEN &= ~EXTI_EVEN_EVEN17;
        EXTI_RTEN &= ~EXTI_RTEN_RTEN17;

        RCU_RSTSCK &= ~RCU_RSTSCK_IRC40KEN;

        rcu_periph_clock_disable(RCU_BKPI);
        rcu_periph_clock_disable(RCU_PMU);
        rcu_periph_clock_disable(RCU_RTC);

        /* enable HXTAL */
        RCU_CTL |= RCU_CTL_HXTALEN;
        /* wait until HXTAL is stable or the startup time is longer than HXTAL_STARTUP_TIMEOUT */
        tout = HXTAL_STARTUP_TIMEOUT;
        while( --tout && !( RCU_CTL & RCU_CTL_HXTALSTB ) );

        /* enable PLL */
        RCU_CTL |= RCU_CTL_PLLEN;
        /* wait until PLL is stable */
        while(! (RCU_CTL & RCU_CTL_PLLSTB) );

        /* select PLL as system clock */
        RCU_CFG0 = (RCU_CFG0 & ~RCU_CFG0_SCS) | RCU_CKSYSSRC_PLL;
        /* wait until PLL is selected as system clock */
        while( !(RCU_CFG0 & RCU_SCSS_PLL) );
	} else {
		register uint64_t t = g_msec + msec;
		while(g_msec < t);                      // 桁あふれも考えよ
	}
}



#endif	// __DEEPSLEEP_H__