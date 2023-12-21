#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__

// 0_8 wakeup に使用 (VIDEO1)
inline void enterDeepSleep(int waitsec);

#define WDMOD_WDEN		(1 << 0)
#define WDMOD_WDRESET	(1 << 1)
#define WDMOD_WDTOF		(1 << 2)
#define WDMOD_WDINT 	(1 << 3)

#define BF_SCR_SLEEPDEEP (1<<2)

#define BF_PDCFG_IRC          (1<<0 | 1<<1)
#define BF_PDCFG_FLASH        (1<<2)
#define BF_PDCFG_WDTOSC       (1<<6)
#define BF_PDCFG_BOD          (1<<3)
#define BF_PDCFG_RESERVEDMSK  (0xEDFF)

#define BF_STARTLOGIC_P0_8    (1<<8)

// configuration for Deep Sleep
#define BF_PDSLEEPCFG_WDT		0x18BF // watchdog oscillator
#define BF_PDSLEEPCFG_WDTBOR	0x18B7 // watchdog oscillator and brown-out reset

#define BF_SYSAHBCLKCTRL_SYSCLK   (1<<0)
#define BF_SYSAHBCLKCTRL_ROM      (1<<1)
#define BF_SYSAHBCLKCTRL_RAM      (1<<2)
#define BF_SYSAHBCLKCTRL_FLASHREG (1<<3)
#define BF_SYSAHBCLKCTRL_FLASH    (1<<4)
#define BF_SYSAHBCLKCTRL_GPIO     (1<<6)
#define BF_SYSAHBCLKCTRL_CT16B0   (1<<7)
#define BF_SYSAHBCLKCTRL_WDT      (1<<15)
#define BF_SYSAHBCLKCTRL_IOCON    (1<<16)

#define WDTCLKSEL_SEL_MAINCLK   1
#define WDTCLKSEL_SEL_WDOSC     2

#define MAINCLKSEL_SEL_IRCOSC   0
#define MAINCLKSEL_SEL_WDOSC    2

#define BF_TIMER_TCR_RESET      (1<<1)
#define BF_TIMER_TCR_RUN        (1<<0)

#define BF_TIMER_MCR_MATCHINTR0  (1<<0)
#define BF_TIMER_MCR_MATCHRESET0 (1<<1)
#define BF_TIMER_MCR_MATCHSTOP0  (1<<2)

#define TIMER_EMR_SETOUT        2
#define BF_TIMER_EMR_SETOUT0    (TIMER_EMR_SETOUT<<4)

#define BF_SYSTICK_COUNTFLAG    (1<<16)

#define BF_SYSAHBCLKCTRL_SLEEP     (BF_SYSAHBCLKCTRL_WDT | BF_SYSAHBCLKCTRL_FLASH  | BF_SYSAHBCLKCTRL_RAM  | BF_SYSAHBCLKCTRL_CT16B0)

#define DEEPSLEEP_TIME  (9300/60)

inline void enterDeepSleep(int time) { // tick 1/60
	uint bksysctrl = LPC_SYSCON->SYSAHBCLKCTRL;
	LPC_SYSCON->SYSAHBCLKCTRL |= BF_SYSAHBCLKCTRL_WDT;  // Turn on clock to WDT register block
	
	LPC_WDT->TC = -1; // 最小値 0xff、最大にしておきリセットを防ぐ
	LPC_SYSCON->WDTCLKDIV = 255; // 最大にしてリセットを防ぐ
	LPC_WDT->FEED = 0xaa;
	LPC_WDT->FEED = 0x55;

	LPC_SYSCON->PDRUNCFG &= ~(1 << 6); // WDT on
    LPC_SYSCON->PDAWAKECFG = LPC_SYSCON->PDRUNCFG;      // Configure PDAWAKECFG to restore PDRUNCFG on wake up
	LPC_SYSCON->PDSLEEPCFG = BF_PDSLEEPCFG_WDT; // Configure deep sleep with WDT oscillator

	// Configure Watchdog Oscillator
	int freqsel = 1; // 0:analog? 1:0.6MHz - 15:4.6Mhz
	int divsel = 64; // 2(fast) - 64(slow)
	LPC_SYSCON->WDTOSCCTRL = (divsel / 2 - 1) | (freqsel << 5); // 1, 64 = 9.3kHz (48MHz時、12MHzなら1/4)
	LPC_WDT->MOD &= ~WDMOD_WDRESET;	// Dropping watchdog timeout flag、自動リセットはoff
//	LPC_WDT->MOD &= ~(WDMOD_WDTOF | WDMOD_WDRESET);	// Dropping watchdog timeout flag、自動リセットはoff
	LPC_WDT->MOD |= WDMOD_WDEN; // enable、watch dog

    LPC_TMR16B0->TCR = BF_TIMER_TCR_RESET; // reset timer

    LPC_TMR16B0->PR = DEEPSLEEP_TIME;
	LPC_TMR16B0->MR0 = time;
    LPC_TMR16B0->MCR = BF_TIMER_MCR_MATCHSTOP0 | BF_TIMER_MCR_MATCHRESET0;
//	LPC_GPIO0->DIR &= ~(1 <<8); // 0_8 IN?
	LPC_GPIO0->DIR |= (1 << 8); // 0_8 OUT? ... どちらでもいける
    LPC_IOCON->PIO0_8 = 0xc2; // Set IOCON register on P0.8 to match function 0xd2でいい？？
	LPC_GPIO0->DATA &= ~(1 << 8); // これが必要だった！
	
    /* Configure Wakeup I/O */
    /* Specify the start logic to allow the chip to be waken up using PIO0_8 */
    LPC_SYSCON->STARTAPRP0          |=  BF_STARTLOGIC_P0_8; // Rising edge
    LPC_SYSCON->STARTRSRP0CLR       =   BF_STARTLOGIC_P0_8; // Clear pending bit
    LPC_SYSCON->STARTERP0           |=  BF_STARTLOGIC_P0_8; // Enable Start Logic
    NVIC_EnableIRQ(WAKEUP8_IRQn); // -> WAKEUP_IRQHandler
	
	LPC_TMR16B0->EMR = BF_TIMER_EMR_SETOUT0; // set timer to drive P0_8 high at match

	/* // 電力に意味はなかった、OUTをINに変えると省電力
	// PLLをIRCに切り替えておく
	LPC_SYSCON->SYSPLLCLKSEL  = 0; // PLL = IRC
	LPC_SYSCON->SYSPLLCLKUEN  = 1; // Update Clock Source
	LPC_SYSCON->SYSPLLCLKUEN  = 0; // oggle Update Register
	LPC_SYSCON->SYSPLLCLKUEN  = 1;
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 1)); // Wait Until Updated
	*/
	
	// Shut down clocks to almost everything
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_SLEEP;

    /*SCB->*/SCR |= BF_SCR_SLEEPDEEP; // Set SLEEPDEEP bit so MCU will enter DeepSleep mode on __WFI();
	
	// Switch main clock to low-speed WDO
	uint bkclksel = LPC_SYSCON->MAINCLKSEL;
    LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_SEL_WDOSC;
    LPC_SYSCON->MAINCLKUEN = 0;
    LPC_SYSCON->MAINCLKUEN = 1; // toggle to enable
    LPC_SYSCON->MAINCLKUEN = 0;
	
    // Preload clock selection for quick switch back to IRC on wakeup -> WAKEUP0_8_IRQHandler
	LPC_SYSCON->MAINCLKSEL = bkclksel; // MAINCLKSEL_SEL_IRCOSC;

    LPC_TMR16B0->TCR = BF_TIMER_TCR_RUN; // start sleep timer
	
	// ignore keyboard (from keyboard_ps2.h)
	#define PS2_WAKEUP_IRQn WAKEUP7_IRQn // 1.2b52
	NVIC_DisableIRQ(PS2_WAKEUP_IRQn); // 1.2b52
	
	__WFI();                            // Enter deep sleep mode (sleep mode in DEBUG)
	LPC_SYSCON->SYSAHBCLKCTRL = bksysctrl;	// Restore clocks to chip modules
	
	NVIC_EnableIRQ(PS2_WAKEUP_IRQn); // 1.2b52
	
	LPC_SYSCON->MAINCLKUEN = 1;         // Restore main clock to IRC 12 MHz
	LPC_TMR16B0->EMR = 0;					// Clear match bit on timer
	/*SCB->*/SCR &= ~BF_SCR_SLEEPDEEP; // Clear SLEEPDEEP bit so MCU will enter Sleep mode on __WFI();
	NVIC_DisableIRQ(WAKEUP8_IRQn); // -> WAKEUP_IRQHandler
	
}

void WAKEUP0_8_IRQHandler(void) {
	LPC_SYSCON->STARTRSRP0CLR = BF_STARTLOGIC_P0_8; // Clear pending bit on start logic // これはないとだめ
    LPC_SYSCON->STARTERP0 &= ~BF_STARTLOGIC_P0_8; // Disaable Start Logic
}

#endif	// __DEEPSLEEP_H__