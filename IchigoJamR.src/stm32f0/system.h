/*
stm32版 todo

RESETでフリーズ問題
I/Oを調整
PWM設定
書き込み機、RESETを一瞬作動させたい、ソフトでできる？

*/

#include "stm32f0.h"
#include "vectors.h"
#include "core_cm0.h"

INLINE void system_init();
void enterDeepSleep(int waitsec);
//inline void deepPowerDown();
inline void IJB_sleep();
inline static void IJB_reset();

int getSleepFlag() {
	// ボタンの入力も加味する
	return (PWR->CSR & PWR_CSR_WUF) || IJB_btn(0);
}
//void enterDeepSleep(int waitsec) {
//}
inline void deepPowerDown() {
}
void blink(int);
inline void IJB_sleep() {
//	blink(2);
	
	__disable_irq();
	// Standby & WKUP1(PA0)
	// PA0 = WKUP1、自動的にプルダウンになる
	PWR->CSR |= PWR_CSR_EWUP1; // wakeup後も有効なレジスタ
	
	PWR->CR &= ~PWR_CR_LPDS; // not Stop mode
	PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF | PWR_CR_PDDS; // WUF after 2cycle
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__DSB(); // Flushes pipeline before sleep
	__WFI(); // WKUP1で復帰しない場合は、APB1ENRをチェック
	
	for (;;); // 念のため
}

inline static void IJB_reset() {
	NVIC_SystemReset(); // 効かない!?
}
int IJB_wait(int n, int active) { // if stop ret 1
	active = 1;
	if (active) {
		if (n < 0) { // ver 1.2b10
			linecnt = 0;
			n = -n;
			while (linecnt < n) {
				if (stopExecute())
				return 1;
			}
		} else {
			for (int i = 0; i < n; i++) {
				if (stopExecute())
					return 1;
				video_waitSync();
			}
		}
		return 0;
	} else if (n > 0) {
	}
	return 0; // キー押されたら 1 とかにする？？
}

#define ichigojam_main Reset_Handler

void changeClockTo48MHz() { // auto
	// 48MHz = external 12MHz x 4
	FLASH->ACR |= 0b10001; // 1<<4:enable prefetch, 1 wait state
	
	RCC->CR |= 1 << 16; // HSEON
	int timeout = 10000;
	while (!(RCC->CR & (1 << 17))) { // wait HSERDY
		timeout--;
		if (!timeout)
			break;
	}
	if (!timeout) { // IRC
		RCC->CFGR = (RCC->CFGR & ~(0b1111111 << 15)) | ((1 << 15) | ((6 - 2) << 18)); // PLL=HSI/PREDIV, PLLMUL6
	} else {
		RCC->CFGR = (RCC->CFGR & ~(0b1111111 << 15)) | ((2 << 15) | ((4 - 2) << 18)); // PLL=HSE/PREDIV, PLLMUL4
	}
	RCC->CR |= 1 << 24; // PLLON
	while (RCC->CR & (1 << 25));
	RCC->CFGR |= 2; // SW = PLL
	while ((RCC->CFGR & 12) != 8);
}


extern unsigned int __data_section_table;

static inline void c_init() {
	// clear RAM
	memclear((uint8*)SRAM_LOC, SRAM_LEN);
	// copy init value to RAM
	uint* ad = &__data_section_table;
	memcopy((uint8*)SRAM_LOC, (uint8*)ad[0], ad[1]);
}

//
INLINE void system_init() {
	changeClockTo48MHz();
	c_init();
	
	// power
	RCC->AHBENR |= AHBENR_IOPA | AHBENR_IOPB | AHBENR_DMA;
	RCC->APB2ENR |= APB2ENR_SYSCFGCOMP | APB2ENR_TIM1 | APB2ENR_SPI1 | APB2ENR_USART1 | APB2ENR_TIM17;
	RCC->APB1ENR |= APB1ENR_TIM3 | APB1ENR_PWR;
	
	// フラッシュ書き込みから起動には必要（割り込みを有効にするため）  // リリース時には不要
	SYSCFG->CFGR1 &= ~0b11; // MEM_MODEをFlashにする
}
