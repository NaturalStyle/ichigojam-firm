#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../stddef.h"
#include "config.h"
#include "../vars.h"
#include "display.h"
#include "io.h"

static inline void enterDeepSleep(int waitsec);
INLINE void deepPowerDown();
INLINE void IJB_sleep();
INLINE void IJB_reset();
static int IJB_wait(int n, int active); // if stop ret 1
INLINE void system_init();
INLINE int getSleepFlag();
// __disable_irq();
//	__enable_irq();
#define ichigojam_main Reset_Handler

INLINE int video_active();

// -- from system_LPC11xx.c

//#define __XTAL            (12000000UL)    /* Oscillator frequency             */
//#define __SYS_OSC_CLK     (    __XTAL)    /* Main oscillator frequency        */
#ifdef XTAL_358MHz
#define __IRC_OSC_CLK      (3579545UL)    /* x Internal RC oscillator frequency */
#else
#define __IRC_OSC_CLK     (12000000UL)    /* x Internal RC oscillator frequency */
#endif

uint32_t SystemCoreClock; // 4byte減る 1.2b44

void SystemCoreClockUpdate(void) { // Get Core Clock Frequency
	// __SYS_OSC_CLK == __IRC_OSC_CLK
#if SYSPLLCTRL_Val == 0x23
	SystemCoreClock = __IRC_OSC_CLK * 4 / LPC_SYSCON->SYSAHBCLKDIV; // 48MHz固定 8byte減る
#elif SYSPLLCTRL_Val == SYSPLLCTRL_Val_358MHz
	SystemCoreClock = __IRC_OSC_CLK * SYSPLLCTRL_VAL_RATIO / LPC_SYSCON->SYSAHBCLKDIV; // 50MHz固定 8byte減る
#else
	SystemCoreClock = (__IRC_OSC_CLK * ((LPC_SYSCON->SYSPLLCTRL & 0x01F) + 1)) / LPC_SYSCON->SYSAHBCLKDIV;
#endif

#ifdef USE_SYSTICK
	LPC_SYSCON->SYSTCKCAL = (SystemCoreClock / 100) - 1; // SysTick 使わない
#endif
}

#define WDMOD_WDEN		(1 << 0)
#define WDMOD_WDRESET	(1 << 1)
#define WDMOD_WDTOF		(1 << 2)
#define WDMOD_WDINT 	(1 << 3)

// Brown out Detect
#define BOD_DISABLE 0x00
#define BOD_SETTING BOD_DISABLE
#define BODCTRL (*(volatile uint32_t*)0x40048150)

INLINE void SystemInit(void) {
//	BODCTRL = BOD_SETTING; // brow out detect // default 0x00
	
	// auto clock ->
	int clksel = SYSPLLCLKSEL_Val;
//	clksel = 0; // test XTAL off
	//	if (clksel == 1) { // XTAL
#if SYSPLLCLKSEL_Val == 1
		if (LPC_SYSCON->SYSRSTSTAT & (1 << 2)) { // WDT bit によるリセット
			clksel = 0; // IRCにして再起動
		} else { // POR (もしくはそれ以外かもしれないが)
			LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 15); // WDT on
			int freqsel = 1; // 0:analog? 1:0.6MHz - 15:4.6Mhz
			int divsel = 64; // 2(fast) - 64(slow)
			LPC_SYSCON->WDTOSCCTRL = (divsel / 2 - 1) | (freqsel << 5); // 1, 64 = 9.3kHz
			LPC_SYSCON->PDRUNCFG &= ~(1 << 6); // WDT on
			
			LPC_SYSCON->WDTCLKSEL = 2; // WDT = watch dog oscillator
//			LPC_SYSCON->WDTCLKSEL = 0; // IRC
			LPC_SYSCON->WDTCLKUEN = 0;
			LPC_SYSCON->WDTCLKUEN = 1;
//			while (!(LPC_SYSCON->WDTCLKUEN & 1)); // Wait until updated
			
			LPC_SYSCON->WDTCLKDIV = 1;
//			LPC_WDT->WINDOW  // 早すぎるクリアでもリセットする機能
			
			// Setting watchdog work mode
			LPC_WDT->MOD |= WDMOD_WDEN | WDMOD_WDRESET; // enable、watch dog 割り込みが無いと自動リセット
			LPC_WDT->MOD &= ~WDMOD_WDTOF;	// Dropping watchdog timeout flag
//			LPC_WDT->WARNINT = 0x1ff; // 割り込み発生カウント 10bit max 3ff // 割り込みは使わない
//			NVIC_EnableIRQ(WDT_IRQn);
			
			LPC_WDT->TC = 0xff; // 最小値 0xff
			// feed
			LPC_WDT->FEED = 0xaa;
			LPC_WDT->FEED = 0x55;
		}
//	}
#else
	/*
			LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 15); // WDT on
			int freqsel = 1; // 0:analog? 1:0.6MHz - 15:4.6Mhz
			int divsel = 64; // 2(fast) - 64(slow)
			LPC_SYSCON->WDTOSCCTRL = (divsel / 2 - 1) | (freqsel << 5); // 1, 64 = 9.3kHz
			LPC_SYSCON->PDRUNCFG &= ~(1 << 6); // WDT on
			
			LPC_SYSCON->WDTCLKSEL = 2; // WDT = watch dog oscillator
//			LPC_SYSCON->WDTCLKSEL = 0; // IRC
			LPC_SYSCON->WDTCLKUEN = 0;
			LPC_SYSCON->WDTCLKUEN = 1;
//			while (!(LPC_SYSCON->WDTCLKUEN & 1)); // Wait until updated
			
			LPC_SYSCON->WDTCLKDIV = 1;
//			LPC_WDT->WINDOW  // 早すぎるクリアでもリセットする機能
			
			// Setting watchdog work mode
			LPC_WDT->MOD |= WDMOD_WDEN | WDMOD_WDRESET; // enable、watch dog 割り込みが無いと自動リセット
			LPC_WDT->MOD &= ~WDMOD_WDTOF;	// Dropping watchdog timeout flag
//			LPC_WDT->WARNINT = 0x1ff; // 割り込み発生カウント 10bit max 3ff // 割り込みは使わない
//			NVIC_EnableIRQ(WDT_IRQn);
			
			LPC_WDT->TC = 0xff; // 最小値 0xff
			// feed
			LPC_WDT->FEED = 0xaa;
			LPC_WDT->FEED = 0x55;
	*/
#endif	
	// <- auto clock end
	
	/* // 何も変更しなくてok
	{
		void* volatile v = (void*)&g_pfnVectors[0];
		// If vector table symbol is not at vector table address (0)
		if (v != 0) {
			// Then assume need to map vectors to RAM
			LPC_SYSCON->SYSMEMREMAP = ((LPC_SYSCON->SYSMEMREMAP) & ~3) | 1;
		}// else
		//	LPC_SYSCON->SYSMEMREMAP |= 3; // Map to flash
	}
	*/
	
#if (CLOCK_SETUP)                                 /* Clock Setup              */
#if (SYSCLK_SETUP)                                /* System Clock Setup       */
#if (SYSOSC_SETUP)                                /* System Oscillator Setup  */
  LPC_SYSCON->PDRUNCFG     &= ~(1 << 5);          /* Power-up System Osc      */
  LPC_SYSCON->SYSOSCCTRL    = SYSOSCCTRL_Val;
	for (uint32_t i = 0; i < 200; i++) {
		__NOP();
	}
	LPC_SYSCON->SYSPLLCLKSEL  = clksel; // SYSPLLCLKSEL_Val;   /* Select PLL Input         */
  LPC_SYSCON->SYSPLLCLKUEN  = 0x01;               /* Update Clock Source      */
  LPC_SYSCON->SYSPLLCLKUEN  = 0x00;               /* Toggle Update Register   */
  LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
  while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));     /* Wait Until Updated       */
#if (SYSPLL_SETUP)                                /* System PLL Setup         */
  LPC_SYSCON->SYSPLLCTRL    = SYSPLLCTRL_Val;
  LPC_SYSCON->PDRUNCFG     &= ~(1 << 7);          /* Power-up SYSPLL          */
  while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));	      /* Wait Until PLL Locked    */
#endif
#endif
#if (WDTOSC_SETUP)                                /* Watchdog Oscillator Setup*/
  LPC_SYSCON->WDTOSCCTRL    = WDTOSCCTRL_Val;
  LPC_SYSCON->PDRUNCFG     &= ~(1 << 6);          /* Power-up WDT Clock       */
#endif
  LPC_SYSCON->MAINCLKSEL    = MAINCLKSEL_Val;     /* Select PLL Clock Output  */
  LPC_SYSCON->MAINCLKUEN    = 0x01;               /* Update MCLK Clock Source */
  LPC_SYSCON->MAINCLKUEN    = 0x00;               /* Toggle Update Register   */
  LPC_SYSCON->MAINCLKUEN    = 0x01;
  while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       /* Wait Until Updated       */
#endif

#if SYSAHBCLKDIV_Val != 1
  LPC_SYSCON->SYSAHBCLKDIV  = SYSAHBCLKDIV_Val;
#endif
  LPC_SYSCON->SYSAHBCLKCTRL = AHBCLKCTRL_Val;
  //LPC_SYSCON->SSP0CLKDIV    = SSP0CLKDIV_Val; // 1.4.1 removed -> displayで設定してる
  //LPC_SYSCON->UARTCLKDIV    = UARTCLKDIV_Val; // 1.4.1 removed -> keybord_ps2で設定してる
  #if SSP1CLKDIV_Val != 0 // 8byte増
	  LPC_SYSCON->SSP1CLKDIV    = SSP1CLKDIV_Val;
  #endif
#endif

	SystemCoreClockUpdate();
	
	// wdt off
//	LPC_WDT->FEED = 0xaa;
//	LPC_WDT->FEED = 0x55;
	LPC_SYSCON->PDRUNCFG |= 1 << 6; // WDT off
	LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 15); // WDT off
}


//--

// dataセクション使わないようにすると不要になる
//extern unsigned int __data_section_table;
/*
extern unsigned int __data_section_table_end;
//extern unsigned int __bss_section_table;
extern unsigned int __bss_section_table_end;
*/

INLINE void system_init() {
	memclear((uint8*)0x10000000, 0x1000); // 4k RAM clear
	
	/*
	uint* ad = &__data_section_table; // set const
	memcopy((uint8*)0x10000000, (uint8*)ad[0], ad[1]);
	//memcopy((uint8*)(0x10000000 + SIZE_RAM), (uint8*)ad[0], ad[1]); // RAMを先頭に持ってくる場合
	*/

	SystemInit(); // 統合すればinline化できる
}

INLINE int getSleepFlag() {
	
	//		LPC_IOCON->PIO0_2 = 0b11010000; // 0xd0 pullup
	//	LPC_IOCON->PIO0_2 = 0b11001000; // 0xc8 pulldown
	//	LPC_GPIO0->DIR &= ~(1 << 2); // 最初は入力だから消してもいい
	//	int sleepflg = (LPC_PMU->GPREG3 >> 11) & 1; //
	//	int sleepflg = ((LPC_PMU->GPREG3 >> 11) & 1) || LPC_GPIO0->MASKED_ACCESS[1 << 2]; // SNDピンがHIGHでもsleep復帰
	//		int sleepflg2 = !LPC_GPIO0->MASKED_ACCESS[1 << 2]; // SNDピンがGNDでもsleep復帰 ver 1.2 // 高速復帰モード？
	//	int sleepflg = LPC_GPIO0->MASKED_ACCESS[1 << 2]; // SNDピンがHIGHでもsleep復帰
	//	int sleepflg = !LPC_GPIO0->MASKED_ACCESS[1 << 2]; // SNDピンがGNDでもsleep復帰
	//		sleepflg2 = 0; // 1.2b28 SOUNDプルダウン無効化
	
	// ver 1.2.3 wait 追加
	// 1.2.3 読み込みは IchigoJam U で電源が安定する 150nsec後 = 7200命令分、150/(1/48000000*1000*1000)
	// 1<<13 = 8192 ( >7200) .. 1loop 5tick (include nop)  1 << 10
	/*
	for (int i = 1 << 10; i >= 0; i--) {
		__NOP();
	}
	*/
	
	//		int sleepflg = sleepflg2 || ((LPC_PMU->GPREG3 >> 11) & 1) || IJB_btn(0);
//	int sleepflg = ((LPC_PMU->GPREG3 >> 11) & 1) || IJB_btn(0);
//	int sleepflg = ((LPC_PMU->GPREG3 >> 11) & 1) || IJB_ana(0) == 0; // 1.2b43
	int sleepflg = ((LPC_PMU->GPREG3 >> 11) & 1) || IJB_ana(0) < 4; // 1.2.3 .. 4未満なら自動起動に変更
	//int sleepflg = ((LPC_PMU->GPREG3 >> 11) & 1) || IJB_ana(0) < 6; // 1.2.3 .. 4未満なら自動起動に変更 // 1.4test 10ならok 電源並列させるとだめなバグ対策？
	LPC_PMU->GPREG3 &= ~(1 << 11);
	return sleepflg;
}

// Cortex-M0 SCB
#define	CPUID	(*(volatile uint32_t*)0xE000ED00)
#define	ICSR	(*(volatile uint32_t*)0xE000ED04)
#define	AIRCR	(*(volatile uint32_t*)0xE000ED0C)
#define	SCR		(*(volatile uint32_t*)0xE000ED10)
#define	CCR		(*(volatile uint32_t*)0xE000ED14)
#define	SHPR	( (volatile uint32_t*)0xE000ED14)
#define SCR_SLEEPDEEP 0b100
#define PCON_DPDEN 0b10

#include "deepsleep.h"
//#include "lightsleep.h"

INLINE void deepPowerDown() {
	// 保存可能データ GPREG0 - GPREG4
	LPC_PMU->PCON |= PCON_DPDEN;
	SCR |= SCR_SLEEPDEEP;
	LPC_SYSCON->PDRUNCFG |= 0b11; // IRCOUT_PD IRC_PD
	__WFI();
}

INLINE void IJB_sleep() {
	// 1.1 b16 押されている間、SLEEPしない
	int bkflg = 1;
	for (;;) {
		//		IJB_wait(6); // 1.1 b15 0.1秒待つ
		video_waitSync(6);
		/*
		for (int j = 0; j < 6; j++) {
			video_waitSync();
		}
		*/
		int flg = IJB_btn(0);
		if ((!flg) && (!bkflg)) {
			break;
		}
		bkflg = flg;
	}
	//	while (IJB_btn(0));  // 1.1b15
	//	random_save(); // 1.0.2 一旦 乱数セーブはやめる
	LPC_PMU->GPREG3 |= 1 << 11; // sleep flg ver 1.0.1
	
	deepPowerDown();
}
INLINE void IJB_reset() {
	NVIC_SystemReset();
}
STATIC int IJB_wait(int n, int active) { // if stop ret 1
	if (active) {
		if (n < 0) { // ver 1.2b10
			_g.linecnt = 0;
			n = -n;
			while (_g.linecnt < n) {
				if (stopExecute())
				return 1;
			}
		} else {
			for (int i = 0; i < n; i++) {
				if (stopExecute())
					return 1;
				video_waitSync(1);
			}
		}
		return 0;
	} else if (n > 0) {
		//	deepSleep(3 * 60);
		int clkdiv = LPC_SYSCON->SYSAHBCLKDIV;
		if (video_active()) { // LPC_TMR16B0->PWMC) { // when video on
			clkdiv = 0;
			video_off(0);
		}
		NVIC_DisableIRQ(TIMER_16_0_IRQn);
		enterDeepSleep(n);
		
		// クロックを戻す
		LPC_SYSCON->SYSPLLCLKSEL  = LPC_SYSCON->SYSPLLCLKSEL; // SYSPLLCLKSEL_Val;   /* Select PLL Input         */
		LPC_SYSCON->SYSPLLCLKUEN  = 1;               /* Update Clock Source      */
		LPC_SYSCON->SYSPLLCLKUEN  = 0;               /* Toggle Update Register   */
		LPC_SYSCON->SYSPLLCLKUEN  = 1;
		while (!(LPC_SYSCON->SYSPLLCLKUEN & 1));     /* Wait Until Updated       */
		
		if (!clkdiv) {
			video_on();
		} else {
			NVIC_EnableIRQ(TIMER_16_0_IRQn); // これだけじゃだめ
			
			// from video_on	// 必要だった 1.2b47
			LPC_TMR16B0->MCR = (0b000 << 0) | (0b001 << 3) | (0b010 << 6); // MR0 noop(MAT0), MR1 interrupt, MR2 reset
			LPC_TMR16B0->PR = 0;
//			LPC_TMR16B0->MR0 = FPORCH;
//			LPC_TMR16B0->MR1 = SIGNAL_OFFSET;
//			LPC_TMR16B0->MR2 = VSYNC;
			LPC_TMR16B0->TCR = 1;
			
			video_off(clkdiv);
		}
	}
	return 0; // キー押されたら 1 とかにする？？
}

#endif	// __SYSTEM_H__