/*
	CT16B0 .. video sync
*/
#define SYSAHBCLKCTRL_ROM			(1 << 1)
#define SYSAHBCLKCTRL_RAM			(1 << 2)
#define SYSAHBCLKCTRL_FLASHREG		(1 << 3)
#define SYSAHBCLKCTRL_FLASHARRAY	(1 << 4) // 止めると止まる
#define SYSAHBCLKCTRL_I2C			(1 << 5)
#define SYSAHBCLKCTRL_GPIO			(1 << 6)
#define SYSAHBCLKCTRL_CT16B0		(1 << 7)
#define SYSAHBCLKCTRL_CT16B1		(1 << 8)
#define SYSAHBCLKCTRL_CT32B0		(1 << 9)
#define SYSAHBCLKCTRL_CT32B1		(1 << 10)
#define SYSAHBCLKCTRL_SSP0			(1 << 11)
#define SYSAHBCLKCTRL_UART			(1 << 12)
#define SYSAHBCLKCTRL_ADC			(1 << 13)
#define SYSAHBCLKCTRL_WDT			(1 << 15)
#define SYSAHBCLKCTRL_IOCON			(1 << 16)
#define SYSAHBCLKCTRL_CAN			(1 << 17)
#define SYSAHBCLKCTRL_SSP1			(1 << 18)

#define BF_STARTLOGIC_P0_8    (1<<8)
#define BF_TIMER_MCR_MATCHINTR0  (1<<0)
#define BF_TIMER_MCR_MATCHRESET0 (1<<1)
#define BF_TIMER_MCR_MATCHSTOP0  (1<<2)
#define TIMER_EMR_SETOUT        2
#define BF_TIMER_EMR_SETOUT0    (TIMER_EMR_SETOUT<<4)
#define BF_TIMER_TCR_RESET      (1<<1)
#define BF_TIMER_TCR_RUN        (1<<0)


inline void lightSleep(int sleepcnt) {
	uint bkclkdiv = LPC_SYSCON->SYSAHBCLKDIV;
	if (LPC_TMR16B0->PWMC)
		bkclkdiv = 0;
	video_off(250); // clkdiv 255  min:1 max:255
	
	LPC_PMU->PCON &= ~PCON_DPDEN;
	SCR &= ~SCR_SLEEPDEEP;
	uint bkpower = LPC_SYSCON->SYSAHBCLKCTRL;
//	LPC_SYSCON->SYSAHBCLKCTRL = SYSAHBCLKCTRL_IOCON | SYSAHBCLKCTRL_GPIO | SYSAHBCLKCTRL_RAM | SYSAHBCLKCTRL_ROM | SYSAHBCLKCTRL_CT16B0 | SYSAHBCLKCTRL_FLASHREG | SYSAHBCLKCTRL_FLASHARRAY;
	LPC_SYSCON->SYSAHBCLKCTRL = SYSAHBCLKCTRL_IOCON | SYSAHBCLKCTRL_GPIO | SYSAHBCLKCTRL_RAM | SYSAHBCLKCTRL_ROM | SYSAHBCLKCTRL_CT16B0 | SYSAHBCLKCTRL_FLASHARRAY;
	LPC_GPIO0->DIR |= 1 << 8;
	LPC_IOCON->PIO0_8 = 0xc2; // Set IOCON register on P0.8 to match function 0xd2でいい？？
	
    LPC_TMR16B0->TCR = BF_TIMER_TCR_RESET; // reset timer
	LPC_TMR16B0->EMR = BF_TIMER_EMR_SETOUT0; // set timer to drive P0_8 high at match
	LPC_TMR16B0->PWMC = 0; // PWMC off
    LPC_TMR16B0->MCR = BF_TIMER_MCR_MATCHSTOP0 | BF_TIMER_MCR_MATCHRESET0;
	LPC_TMR16B0->PR = 4800 * 10; // 4800*10 = 48000 < 0xffff
	LPC_TMR16B0->MR0 = sleepcnt * (1000 / 250); // 250=clkdiv
	LPC_SYSCON->STARTAPRP0          |=  BF_STARTLOGIC_P0_8; // Rising edge
    LPC_SYSCON->STARTRSRP0CLR       =   BF_STARTLOGIC_P0_8; // Clear pending bit
    LPC_SYSCON->STARTERP0           |=  BF_STARTLOGIC_P0_8; // Enable Start Logic
    NVIC_EnableIRQ(WAKEUP8_IRQn);
    LPC_TMR16B0->TCR = BF_TIMER_TCR_RUN; // start sleep timer
	
	__WFI();
	
    NVIC_DisableIRQ(WAKEUP8_IRQn);
	LPC_SYSCON->SYSAHBCLKCTRL = bkpower;
	
	/*
	if (bkclkdiv) {
		video_off(bkclkdiv);
	} else {
		video_on();
	}
	*/
	video_on();
	if (bkclkdiv) {
		video_off(bkclkdiv);
	}
}

void WAKEUP0_8_IRQHandler(void) {
	LPC_SYSCON->STARTRSRP0CLR = BF_STARTLOGIC_P0_8; // Clear pending bit on start logic
}
