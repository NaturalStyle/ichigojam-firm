//* 
//* created by yrm (c) 2016
//* 
#define ICHIGOJAM

#include "gd32vf103.h"
#ifdef ICHIGOJAM
#include "ps2.h"
#else
#include "ichigolatte.h"
#endif


kb_frame g_frames[32] = {};
kb_status g_rw = { 0,0,0 };



struct{
	uint8_t bit;
	uint8_t data;
	uint8_t ack;
}g_sending = { 11, 0, 1 };



int ps2_send(uint8_t d){
	g_sending.data = d;

    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(12-8)) | GPIO_MODE_SET(12-8, 0b0011);      // kbd1 -> out
	GPIO_BOP(GPIOA) = GPIO_BOP_CR12;

	_usleep(100);

    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b0011);      // kbd2 -> out
	GPIO_BOP(GPIOA) = GPIO_BOP_CR11;

    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(12-8)) | GPIO_MODE_SET(12-8, 0b1000);      // kbd1 -> in
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP12;

	g_sending.bit = 0;
	g_sending.ack = 1;

	uint32_t timeout = get_timer_value() + (200 * (SystemCoreClock / 4 / 1000));	// 200msec
	while(g_sending.bit < 11 && get_timer_value() < timeout);

	GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b1000);      // kbd2 -> in
	GPIO_BOP(GPIOA) |= GPIO_PIN_11;

	return !g_sending.ack;
}

int ps2_sent(){
	return !g_sending.ack;
}

// __attribute__ ((section(".data")))
void EXTI10_15_IRQHandler(){
    if (EXTI_PD & EXTI_12){
		register uint32_t s = GPIO_ISTAT(GPIOA);   // early fetch

		// for sending
		if(g_sending.bit < 11){
			if(g_sending.bit < 8){
				GPIO_BOP(GPIOA) = (g_sending.data&(1<<g_sending.bit)) ? GPIO_BOP_BOP11 : GPIO_BOP_CR11;
			}else
			if(g_sending.bit == 8){
				// parity
				uint8_t d = g_sending.data;
				d = d^(d>>4);
				d = d^(d>>2);
				d = d^(d>>1);
				GPIO_BOP(GPIOA) = (d&0b1) ? GPIO_BOP_CR11 : GPIO_BOP_BOP11;
			}else
			if(g_sending.bit == 9){
				GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b1000);      // kbd2 -> in
		        GPIO_BOP(GPIOA) |= GPIO_PIN_11;
			}else{	// last bit (10)
				g_sending.ack = ((GPIO_ISTAT(GPIOA) & GPIO_PIN_11) >> 11);
			}

			++g_sending.bit;
			goto end;
		}

		// for receiving
		if(g_rw.waiting_for_r) goto end;

		g_frames[g_rw.w].data |= ((s&GPIO_PIN_11)>>11) << g_frames[g_rw.w].bit;

		if(10 < ++g_frames[g_rw.w].bit){
			//C bits description
			//C [EP76543210S]    S:start P:parity(odd) E:end
			//C [1P........0]

			// parity
			register uint8_t o = (g_frames[g_rw.w].data>>1)&0xff;{
				o^=o>>4; o^=o>>2; o^=o>>1;
				o = ~o & 0b1;
			}

			// start end & parity
			if( (g_frames[g_rw.w].data & 0b11000000001) == (0b10000000000 | (o<<9)) ){
				RW_INC(g_rw.w);
				g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;
			}else{
				g_rw.waiting_for_r = WFR_ERR;
			}
		}

end:
        EXTI_PD |= EXTI_12;
    }
}


