//* 
//* created by yrm (c) 2016
//* 
// #include "nrf.h"
// #include "ichigonquest.h"
#include <stdint.h>
#include "gd32vf103.h"
//#include "ichigolatte.h"

#define SCREEN_W 32
#define SCREEN_H 24
#define SCREEN_WH (SCREEN_W * SCREEN_H)
#define CHAR_H 8
extern uint8_t g_aFont[256][8];
extern uint8_t g_aScreen[SCREEN_WH];
extern uint8_t g_nCursorX;
extern uint8_t g_nCursorY;

// extern uint8_t* g_pScreen;
// uint8_t* g_pScreen = g_aScreen;
#define g_pScreen g_aScreen

#define FONT_NULL 0x20
#define FONT_FILL 0x8f

const uint8_t* g_pCursorShape = g_aFont[FONT_FILL];


#define NTSC_S  488
#define NTSC_M  (NTSC_L - NTSC_S + 80)
#define NTSC_L  6102
#define NTSC_D  (NTSC_S * 2 + 200)

#define LINES (262)
#define VSYNC (3)
#define BLANK (3+44)

static uint16_t g_nLine = 0;

uint8_t g_ntsc_mode = 0x00;

uint8_t g_ntsc_on = 0;

__attribute__ ((aligned(4)))
static uint8_t g_aLine[SCREEN_W+8] = {};

void ntsc_init() {
    g_nLine = 0;

    rcu_periph_clock_enable(RCU_TIMER1);
    
    TIMER_PSC(TIMER1) = 0; // prescaler
    TIMER_CAR(TIMER1) = NTSC_L - 1;
    TIMER_CH0CV(TIMER1) = NTSC_M - 1;
    TIMER_CH1CV(TIMER1) = NTSC_D - 1;
    TIMER_CH2CV(TIMER1) = NTSC_L - 1;
    TIMER_DMAINTEN(TIMER1) = TIMER_DMAINTEN_CH0IE | TIMER_DMAINTEN_CH2IE;
    TIMER_INTF(TIMER1) = 0;
    TIMER_CTL0(TIMER1) |= TIMER_CTL0_CEN;

    eclic_irq_enable(TIMER1_IRQn, 3, 3);
}



// __attribute__ ((section(".data")))
void TIMER1_IRQHandler() {
// GPIO_OCTL(GPIOA) ^= GPIO_PIN_2;

    if ((TIMER_INTF(TIMER1) & TIMER_INTF_CH0IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH0IE)) {
        GPIO_OCTL(GPIOC) |= GPIO_PIN_13;
        TIMER_INTF(TIMER1) = ~TIMER_INTF_CH0IF;

    } else if ((TIMER_INTF(TIMER1) & TIMER_INTF_CH1IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH1IE)) {
        uint32_t y = (g_nLine - BLANK) / CHAR_H;
        uint32_t ya = (g_nLine - BLANK) % CHAR_H;
        uint8_t x = 0;
        while (x < 2) {
            register uint32_t c = (x == g_nCursorX && y == g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            
            register unsigned char* ps = g_pScreen + SCREEN_W * y + x;
            ps -= (ps < g_aScreen + SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (g_aFont[*ps][ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }

        {
            /* configure peripheral base address */
            DMA_CHPADDR(DMA0, DMA_CH4) = (uint32_t)&SPI_DATA(SPI1);

            /* configure memory base address */
            DMA_CHMADDR(DMA0, DMA_CH4) = (uint32_t)g_aLine;

            /* configure the number of remaining data to be transferred */
            DMA_CHCNT(DMA0, DMA_CH4) = (sizeof(g_aLine) & DMA_CHANNEL_CNT_MASK);

            /* configure peripheral transfer width,memory transfer width and priority */
            uint32_t ctl;
            ctl = DMA_CHCTL(DMA0, DMA_CH4);
            ctl &= ~(DMA_CHXCTL_PWIDTH | DMA_CHXCTL_MWIDTH | DMA_CHXCTL_PRIO);
            ctl |= (DMA_PERIPHERAL_WIDTH_8BIT | DMA_MEMORY_WIDTH_8BIT | DMA_PRIORITY_LOW);
            DMA_CHCTL(DMA0, DMA_CH4) = ctl;

            DMA_CHCTL(DMA0, DMA_CH4) &= ~DMA_CHXCTL_PNAGA;
            DMA_CHCTL(DMA0, DMA_CH4) |= DMA_CHXCTL_MNAGA;
            DMA_CHCTL(DMA0, DMA_CH4) |= DMA_CHXCTL_DIR;
        }

         /* configure DMA mode */
        dma_circulation_disable(DMA0, DMA_CH4);
        dma_memory_to_memory_disable(DMA0, DMA_CH4);

        {
            uint32_t reg = 0U;
            reg = SPI_CTL0(SPI1);
            reg &= ((uint32_t)0x00003040U);  /*!< SPI parameter initialization mask */

            /* select SPI as master or slave */
            reg |= SPI_MASTER;
            /* select SPI transfer mode */
            reg |= SPI_FRAMESIZE_8BIT;
            /* select SPI frame size */
            reg |= SPI_FRAMESIZE_8BIT;
            /* select SPI NSS use hardware or software */
            reg |= SPI_NSS_HARD;
            /* select SPI LSB or MSB */
            reg |= SPI_ENDIAN_MSB;
            /* select SPI polarity and phase */
            reg |= SPI_CK_PL_LOW_PH_1EDGE;
            /* select SPI prescale to adjust transmit speed */
            reg |= SPI_PSC_8;

            /* write to SPI_CTL0 register */
            SPI_CTL0(SPI1) = (uint32_t)reg;

            SPI_I2SCTL(SPI1) &= (uint32_t)(~SPI_I2SCTL_I2SSEL);
        }

        /* SPI DMA enable */
        spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
        /* DMA channel enable */
        dma_channel_enable(DMA0, DMA_CH4);



        spi_enable(SPI1);
        // gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
        GPIO_CTL1(GPIOB) = (GPIO_CTL1(GPIOB) & 0b00001111111111111111111111111111) | (0b1011 << 28);



        while (x < SCREEN_W) {
            register uint32_t c = (x == g_nCursorX && y == g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            
            register unsigned char* ps = g_pScreen + SCREEN_W*y + x;
            ps -= (ps < g_aScreen+SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (g_aFont[*ps][ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }



        TIMER_INTF(TIMER1) = ~TIMER_INTF_CH1IF;
    } else if ((TIMER_INTF(TIMER1) & TIMER_INTF_CH2IF) && (TIMER_DMAINTEN(TIMER1) & TIMER_DMAINTEN_CH2IE)) {
        GPIO_OCTL(GPIOC) &= ~GPIO_PIN_13;

        if (++g_nLine == LINES){
            g_nLine = 0;
        }
        
        if(BLANK < g_nLine && g_nLine <= BLANK+(CHAR_H*SCREEN_H)){
            dma_deinit(DMA0, DMA_CH4);
            spi_i2s_deinit(SPI1);

            gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
        }

        if(g_nLine == 0){
            TIMER_CH0CV(TIMER1) = NTSC_M - 1;                       // HSync -> VSync
        }else
        if(g_nLine == VSYNC){
            TIMER_CH0CV(TIMER1) = NTSC_S - 1;                       // VSync -> HSync
        }else
        if(g_nLine == BLANK){
            TIMER_INTF(TIMER1) = ~TIMER_INTF_CH1IF;
            TIMER_DMAINTEN(TIMER1) |= TIMER_DMAINTEN_CH1IE;         // draw
        }else
        if(g_nLine == BLANK+(CHAR_H*SCREEN_H)){
            TIMER_DMAINTEN(TIMER1) &= ~TIMER_DMAINTEN_CH1IE;        // no draw
        }

        TIMER_INTF(TIMER1) = ~TIMER_INTF_CH2IF;
    }
}



