
#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

WEAK void IntDefaultHandler(void);

//void Reset_Handler(void) ALIAS(IntDefaultHandler);
void Reset_Handler(void);
void NMI_Handler(void) ALIAS(IntDefaultHandler);
void HardFault_Handler(void);
void SVC_Handler(void) ALIAS(IntDefaultHandler);
void PendSV_Handler(void) ALIAS(IntDefaultHandler);
void SysTick_Handler(void) ALIAS(IntDefaultHandler);


void CAN_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP1_IRQHandler(void) ALIAS(IntDefaultHandler);
//void I2C_IRQHandler(void) ALIAS(IntDefaultHandler);
void I2C_IRQHandler(void);
//void TIMER16_0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER16_0_IRQHandler(void);
void TIMER16_1_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER32_0_IRQHandler(void) ALIAS(IntDefaultHandler);
void TIMER32_1_IRQHandler(void) ALIAS(IntDefaultHandler);
void SSP0_IRQHandler(void) ALIAS(IntDefaultHandler);
//void UART_IRQHandler(void) ALIAS(IntDefaultHandler);
void UART_IRQHandler(void);
void ADC_IRQHandler(void) ALIAS(IntDefaultHandler);
void WDT_IRQHandler(void) ALIAS(IntDefaultHandler);
void BOD_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT3_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT2_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT1_IRQHandler(void) ALIAS(IntDefaultHandler);
void PIOINT0_IRQHandler(void) ALIAS(IntDefaultHandler);
void WAKEUP_IRQHandler(void) ALIAS(IntDefaultHandler);
void WAKEUP0_7_IRQHandler(void);
//void WAKEUP0_8_IRQHandler(void) ALIAS(IntDefaultHandler);
void WAKEUP0_8_IRQHandler(void);

#define STACK_SIZE (1152) // 8byte align
uint8_t mstk[STACK_SIZE + 2688/*SIZE_RAM*/] __attribute__ ((aligned(8), section(".STACK")));
//#define INITIAL_MSP	&(mstk[STACK_SIZE])

extern void (*const __Vectors[])(void);

__attribute__ ((section(".isr_vector")))
void (*const __Vectors[])(void) = {
//    (void (*const)(void))__STACK_TOP,		    				// The initial stack pointer
    (void*)(mstk + STACK_SIZE),	 			// The initial stack pointer
	Reset_Handler,                          // The reset handler
    NMI_Handler,                            // The NMI handler
    HardFault_Handler,                      // The hard fault handler
    0,                      				// Reserved
    0,                      				// Reserved
    0,                      				// Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    SVC_Handler,                      	    // SVCall handler
    0,                      				// Reserved
    0,                                      // Reserved
    PendSV_Handler,                      	// The PendSV handler
    SysTick_Handler,                      	// The SysTick handler

    // Wakeup sources for the I/O pins:
    //   PIO0 (0:11)
    //   PIO1 (0)
    WAKEUP_IRQHandler,                      // PIO0_0  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_1  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_2  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_3  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_4  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_5  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_6  Wakeup
    WAKEUP0_7_IRQHandler,                   // PIO0_7  Wakeup
    WAKEUP0_8_IRQHandler,                   // PIO0_8  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_9  Wakeup
    WAKEUP_IRQHandler,                      // PIO0_10 Wakeup
    WAKEUP_IRQHandler,                      // PIO0_11 Wakeup
    WAKEUP_IRQHandler,                      // PIO1_0  Wakeup
    
    CAN_IRQHandler,							// C_CAN Interrupt
    SSP1_IRQHandler, 						// SPI/SSP1 Interrupt
    I2C_IRQHandler,                      	// I2C0
    TIMER16_0_IRQHandler,                   // CT16B0 (16-bit Timer 0)
    TIMER16_1_IRQHandler,                   // CT16B1 (16-bit Timer 1)
    TIMER32_0_IRQHandler,                   // CT32B0 (32-bit Timer 0)
    TIMER32_1_IRQHandler,                   // CT32B1 (32-bit Timer 1)
    SSP0_IRQHandler,                      	// SPI/SSP0 Interrupt
    UART_IRQHandler,                      	// UART0

    0, 				                     	// Reserved
    0,                      				// Reserved

    ADC_IRQHandler,                      	// ADC   (A/D Converter)
    WDT_IRQHandler,                      	// WDT   (Watchdog Timer)
    BOD_IRQHandler,                      	// BOD   (Brownout Detect)
    0,                      				// Reserved
    PIOINT3_IRQHandler,                     // PIO INT3
    PIOINT2_IRQHandler,                     // PIO INT2
    PIOINT1_IRQHandler,                     // PIO INT1
    PIOINT0_IRQHandler,                     // PIO INT0
};

//extern void IJB_pwm(int port, int plen, int len);

const uint16_t API[32] __attribute__ ((aligned(4), section(".api"))); // 消されてしまう

/*
const void* API[16] __attribute__ ((aligned(4), section(".api"))) = { // 消されてしまう
    IJB_pwm,
};
*/

__attribute__ ((aligned(0x100), section(".after_vectors"))) // APIが消えるからalignで確保
void IntDefaultHandler(void) {
	for (;;);
}

