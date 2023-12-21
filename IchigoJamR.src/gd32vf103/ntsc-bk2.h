//* 
//* created by yrm (c) 2016
//* 
// #include "nrf.h"
// #include "ichigonquest.h"
#include <stdint.h>
#include "gd32vf103.h"
//#include "ichigolatte.h"

#define g_aScreen vram
// extern uint8_t* g_aScreen;
#define g_pScreen vram

//extern uint8_t g_nCursorX;
//extern uint8_t g_nCursorY;


#define NTSC_S  488
#define NTSC_M  (NTSC_L - NTSC_S + 80)
#define NTSC_L  6102
#define NTSC_D  (NTSC_S * 2 + 200)

#define NTSC_LINES (262)
#define NTSC_VSYNC (3)
#define NTSC_BLANK (3+44)
#define NTSC_INITADDL 1

static uint16_t g_nLine = 0;

uint8_t g_ntsc_addl = NTSC_INITADDL;
uint8_t g_ntsc_mode = 0x00;

uint8_t g_ntsc_on = 0;

__attribute__ ((aligned(4)))
#define MAX_SCREEN_W 32
//static uint8_t g_aLine[SCREEN_W+1] = {};
uint8_t g_aLine[MAX_SCREEN_W + 1] = {};



void ntsc_onoff(int n){
    if(n){
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0011);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1011);

        TIMER_INTF(TIMER1) = 0;
        TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;
        
        g_ntsc_mode = (n == 2) ? 0xff : 0x00;
    }else{
        TIMER_CTL0(TIMER1) &= ~TIMER_CTL0_CEN;

        GPIO_BC(GPIOB) |= (1 << 4);
        GPIO_BC(GPIOB) |= (1 << 5);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);
    }
}


void ntsc_init() {
    lchika(3);
    g_nLine = 0;

    {
        uint32_t reg = 0U;
        reg = SPI_CTL0(SPI0);
        reg &= ((uint32_t)0x00003040U);  /*!< SPI parameter initialization mask */

        /* select SPI as master or slave */
        reg |= SPI_MASTER;
        /* select SPI transfer mode */
        reg |= SPI_TRANSMODE_BDTRANSMIT;
        /* select SPI frame size */
        reg |= SPI_FRAMESIZE_8BIT;
        /* select SPI NSS use hardware or software */
        reg |= SPI_NSS_SOFT;
        /* select SPI LSB or MSB */
        reg |= SPI_ENDIAN_MSB;
        /* select SPI polarity and phase */
        reg |= SPI_CK_PL_LOW_PH_1EDGE;
        /* select SPI prescale to adjust transmit speed */
        reg |= SPI_PSC_16;

        /* write to SPI_CTL0 register */
        SPI_CTL0(SPI0) = (uint32_t)reg;
    }

    /* SPI DMA enable */
    SPI_CTL1(SPI0) |= SPI_CTL1_DMATEN;
    
    /* SPI enable */
    SPI_CTL0(SPI0) |= SPI_CTL0_SPIEN;



    {
        /* configure peripheral base address */
        DMA_CHPADDR(DMA0, DMA_CH2) = (uint32_t)&SPI_DATA(SPI0);

        /* configure memory base address */
        DMA_CHMADDR(DMA0, DMA_CH2) = (uint32_t)g_aLine;

        /* configure peripheral transfer width,memory transfer width and priority */
        uint32_t ctl;
        ctl = DMA_CHCTL(DMA0, DMA_CH2);
        ctl &= ~(DMA_CHXCTL_PWIDTH | DMA_CHXCTL_MWIDTH | DMA_CHXCTL_PRIO);
        ctl |= (DMA_PERIPHERAL_WIDTH_8BIT | DMA_MEMORY_WIDTH_8BIT | DMA_PRIORITY_ULTRA_HIGH);
        DMA_CHCTL(DMA0, DMA_CH2) = ctl;

        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_MNAGA;
        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_DIR;

        // DMA_CHCTL(DMA0, DMA_CH2) |= DMA_INT_FTF;
        // eclic_irq_enable(DMA0_Channel2_IRQn, 1, 3);
    }


    
    eclic_irq_enable(TIMER1_IRQn, 3, 3);

    TIMER_PSC(TIMER1) = 0; // prescaler
    TIMER_CAR(TIMER1) = NTSC_L - 1;
    TIMER_CH0CV(TIMER1) = NTSC_M - 1;
    TIMER_CH1CV(TIMER1) = NTSC_D - 1;
    // TIMER_CH2CV(TIMER1) = NTSC_L - 1;
    // TIMER_DMAINTEN(TIMER1) = TIMER_DMAINTEN_CH0IE | TIMER_DMAINTEN_CH2IE;
    TIMER_DMAINTEN(TIMER1) = TIMER_DMAINTEN_UPIE | TIMER_DMAINTEN_CH0IE;
    TIMER_INTF(TIMER1) = 0;
    
    TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;
}



// __attribute__ ((section(".data")))
void TIMER1_IRQHandler(){
    if( (TIMER_INTF(TIMER1) & TIMER_INTF_CH0IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH0IE) ){
        GPIO_OCTL(GPIOB) |= GPIO_PIN_4;
        TIMER_INTF(TIMER1) &= ~TIMER_INTF_CH0IF;

    }else
    if( (TIMER_INTF(TIMER1) & TIMER_INTF_CH1IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH1IE) ){
        uint32_t y = (g_nLine - NTSC_BLANK) / CHAR_H;
        uint32_t ya = (g_nLine - NTSC_BLANK) % CHAR_H;
        uint8_t x = 0;
        while (x < 2) {
            register uint32_t c = 0; //(x==g_nCursorX && y==g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            
            register unsigned char* ps = g_pScreen + SCREEN_W*y + x;
            ps -= (ps < g_aScreen+SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (g_aFont[*ps][ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }



        /* disable DMA a channel */
        DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
        /* configure the number of remaining data to be transferred */
        DMA_CHCNT(DMA0, DMA_CH2) = (sizeof(g_aLine) & DMA_CHANNEL_CNT_MASK);
        /* DMA channel enable */
        DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_CHEN;



        while (x < SCREEN_W) {
            register uint32_t c = 0; // (x==g_nCursorX && y==g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            
            register unsigned char* ps = g_pScreen + SCREEN_W*y + x;
            ps -= (ps < g_aScreen+SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (g_aFont[*ps][ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }



        TIMER_INTF(TIMER1) &= ~TIMER_INTF_CH1IF;
    }else
    if( (TIMER_INTF(TIMER1) & TIMER_INTF_UPIF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_UPIE) ){
    // if( (TIMER_INTF(TIMER1) & TIMER_INTF_CH2IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH2IE) ){
        GPIO_OCTL(GPIOB) &= ~GPIO_PIN_4;

        if(++g_nLine == NTSC_LINES){
            g_nLine = 0;
        }
        
        if(g_nLine == 0){
            TIMER_CH0CV(TIMER1) = NTSC_M - 1;                       // HSync -> VSync
        }else
        if(g_nLine == NTSC_VSYNC){
            TIMER_CH0CV(TIMER1) = NTSC_S - 1;                       // VSync -> HSync
        }else
        if(g_nLine == NTSC_BLANK){
            TIMER_INTF(TIMER1) &= ~TIMER_INTF_CH1IF;
            TIMER_DMAINTEN(TIMER1) |= TIMER_DMAINTEN_CH1IE;         // draw
        }else
        if(g_nLine == NTSC_BLANK+(CHAR_H*SCREEN_H)){
            TIMER_DMAINTEN(TIMER1) &= ~TIMER_DMAINTEN_CH1IE;        // no draw
        }

        // TIMER_INTF(TIMER1) &= ~TIMER_INTF_CH2IF;
        TIMER_INTF(TIMER1) &= ~TIMER_DMAINTEN_UPIE;
    }
}

// void DMA0_Channel2_IRQHandler(){
//     if((DMA_INTF(DMA0) & DMA_FLAG_ADD(DMA_INT_FLAG_FTF, DMA_CH2)) && (DMA_CHCTL(DMA0, DMA_CH2) & DMA_CHXCTL_FTFIE)){
//         /* disable DMA a channel */
//         DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;

//         DMA_INTC(DMA0) |= DMA_FLAG_ADD(DMA_INT_FLAG_FTF, DMA_CH2);
//     }
// }


