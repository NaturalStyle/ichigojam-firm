//* 
//* created by yrm (c) 2016
//* 
// #include "nrf.h"
// #include "ichigonquest.h"
#include <stdint.h>
#include "gd32vf103.h"
//#include "ichigolatte.h"

//#define SCREEN_W 32
//#define SCREEN_H 24
//#define SCREEN_WH (SCREEN_W * SCREEN_H)
#define CHAR_H 8
// extern uint8_t g_aFont[256][8];
#define CHAR_PATTERN CHAR_PATTERN_JP
extern uint8_t CHAR_PATTERN[];

#define g_aScreen vram
extern uint8_t* g_aScreen;

extern uint8_t g_nCursorX;
extern uint8_t g_nCursorY;

// extern uint8_t* g_pScreen;
// uint8_t* g_pScreen = g_aScreen;
#define g_pScreen g_aScreen

#define FONT_NULL 0x20
#define FONT_FILL 0x8f

// const uint8_t* g_pCursorShape = g_aFont[FONT_FILL];

// config video1
//#define VIDEO1_PC13
//#define VIDEO1_PA13
#define VIDEO1_PB4
//#define VIDEO1_PB8

// config video2
//#define VIDEO2_PB15
#define VIDEO2_PB5

// config timer
//#define VIDEO_TIMER1
//#define VIDEO_TIMER2
#define VIDEO_TIMER3


#define NTSC_S  488
#define NTSC_M  (NTSC_L - NTSC_S + 80)
#define NTSC_L  6102
#define NTSC_D  (NTSC_S * 2 + 200)

#define NTSC_LINES (262)
#define NTSC_VSYNC (3)
#define NTSC_BLANK (3 + 44)

static uint16_t g_nLine = 0;

uint8_t g_ntsc_mode = 0x00;

uint8_t g_ntsc_on = 0;

__attribute__ ((aligned(4)))
//static uint8_t g_aLine[SCREEN_W + 8];
#define SCREEN_W_MAX 32
static uint8_t g_aLine[SCREEN_W_MAX + 8];

#define AFIO_PCF0_SWJ_CFG_OFFSET 24 // from AFIO_PCF0_SWJ_CFG

#if defined(VIDEO1_PC13)
#define VIDEO_CLK_PORT GPIOC
#define VIDEO_CLK_PIN GPIO_PIN_13
#define VIDEO_CLK_MODE GPIO_MODE_OUT_PP
#elif defined(VIDEO1_PA13)
#define VIDEO_CLK_PORT GPIOA
#define VIDEO_CLK_PIN GPIO_PIN_13
#define VIDEO_CLK_MODE GPIO_MODE_OUT_PP
#elif defined(VIDEO1_PB4)
#define VIDEO_CLK_PORT GPIOB
#define VIDEO_CLK_PIN GPIO_PIN_4
#define VIDEO_CLK_MODE GPIO_MODE_OUT_PP
#elif defined(VIDEO1_PB8)
#define VIDEO_CLK_PORT GPIOB
#define VIDEO_CLK_PIN GPIO_PIN_8
#define VIDEO_CLK_MODE GPIO_MODE_OUT_PP
#else
error!
#endif

#if defined(VIDEO2_PB15)
#define VIEDO_SPI_EN RCU_SPI1
#define VIDEO_SPI SPI1
#define VIDEO_SPI_PIN GPIO_PIN_15
#define VIDEO_DMA DMA0
#define VIDEO_DMA_CH DMA_CH4
#define VIDEO_DMA_EN RCU_DMA0
#elif defined(VIDEO2_PB5)
#define VIDEO_SPI_EN RCU_SPI0
#define VIDEO_SPI SPI0
#define VIDEO_SPI_PIN GPIO_PIN_5
#define VIDEO_DMA_EN RCU_DMA0
#define VIDEO_DMA DMA0
#define VIDEO_DMA_CH DMA_CH2
#else
error!
#endif

#if defined(VIDEO_TIMER1)
#define VIDEO_TIMER_EN RCU_TIMER1
#define VIDEO_TIMER TIMER1
#define VIDEO_TIMER_IRQ TIMER1_IRQn
#define VIDEO_TIMER_IRQHandler TIMER1_IRQHandler
#elif defined(VIDEO_TIMER2)
#define VIDEO_TIMER_EN RCU_TIMER2
#define VIDEO_TIMER TIMER2
#define VIDEO_TIMER_IRQ TIMER2_IRQn
#define VIDEO_TIMER_IRQHandler TIMER2_IRQHandler
#elif defined(VIDEO_TIMER3)
#define VIDEO_TIMER_EN RCU_TIMER3
#define VIDEO_TIMER TIMER3
#define VIDEO_TIMER_IRQ TIMER3_IRQn
#define VIDEO_TIMER_IRQHandler TIMER3_IRQHandler
#else
error
#endif

void ntsc_init() {
    g_nLine = 0;

    rcu_periph_clock_enable(VIDEO_TIMER_EN);
    rcu_periph_clock_enable(VIDEO_SPI_EN);
    rcu_periph_clock_enable(VIDEO_DMA_EN);
    
    TIMER_PSC(VIDEO_TIMER) = 0; // prescaler
    TIMER_CAR(VIDEO_TIMER) = NTSC_L - 1;
    TIMER_CH0CV(VIDEO_TIMER) = NTSC_M - 1;
    TIMER_CH1CV(VIDEO_TIMER) = NTSC_D - 1;
    TIMER_CH2CV(VIDEO_TIMER) = NTSC_L - 1;
    TIMER_DMAINTEN(VIDEO_TIMER) = TIMER_DMAINTEN_CH0IE | TIMER_DMAINTEN_CH2IE;
    TIMER_INTF(VIDEO_TIMER) = 0;
    TIMER_CTL0(VIDEO_TIMER) |= TIMER_CTL0_CEN;

    eclic_irq_enable(VIDEO_TIMER_IRQ, 3, 3);

    // ntsc
    #if defined(VIEDO1_PA13)
    // gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP, ENABLE); // 効かない
    AFIO_PCF0 = (AFIO_PCF0 & 0xF8FFFFFF) | 0x04000000; // これが必要 https://blog.csdn.net/zoomdy/article/details/101386700
    #elif defined(VIDEO1_PB4)
    AFIO_PCF0 = (AFIO_PCF0 & ~AFIO_PCF0_SWJ_CFG) | (0b001 << AFIO_PCF0_SWJ_CFG_OFFSET); // 000:JTAG_DP reset state, 001:JTAG_DP but without NJTRST, 010:JTAG-DP Disabled
    #endif
    gpio_init(VIDEO_CLK_PORT, VIDEO_CLK_MODE, GPIO_OSPEED_50MHZ, VIDEO_CLK_PIN);
    
    #if defined(VIDEO2_PB5)
    // AFIO_PCF0 &= ~AFIO_PCF0_SPI2_REMAP; // for PIN5 default設定でok
    #endif
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, VIDEO_SPI_PIN);

    //
    /*
        GPIO_BC(GPIOB) |= (1 << 4);
        GPIO_BC(GPIOB) |= (1 << 5);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);
    */
}

void ntsc_line_tick();
void ntsc_frame_tick();

// __attribute__ ((section(".data")))
void VIDEO_TIMER_IRQHandler() {
// GPIO_OCTL(GPIOA) ^= GPIO_PIN_2;

    if ((TIMER_INTF(VIDEO_TIMER) & TIMER_INTF_CH0IF) && (TIMER_DMAINTEN(VIDEO_TIMER) & TIMER_DMAINTEN_CH0IE)) {
        GPIO_OCTL(VIDEO_CLK_PORT) |= VIDEO_CLK_PIN;
        TIMER_INTF(VIDEO_TIMER) = ~TIMER_INTF_CH0IF;
    } else if ((TIMER_INTF(VIDEO_TIMER) & TIMER_INTF_CH1IF) && (TIMER_DMAINTEN(VIDEO_TIMER) & TIMER_DMAINTEN_CH1IE)) {
        uint32_t y = (g_nLine - NTSC_BLANK) / CHAR_H;
        uint32_t ya = (g_nLine - NTSC_BLANK) % CHAR_H;
        uint8_t x = 0;
        while (x < 2) {
            // register uint32_t c = (x == g_nCursorX && y == g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            register uint32_t c = 0;
    		if ((frames >> 4) & _g.cursorflg) {
                if (x == _g.cursorx && y == _g.cursory) {
                    c = key_flg.insert ? 0xff : 0xf0;
                } else {
                    __asm volatile("nop"); // カーソル無い時の速度調整
                }
            } else {
                __asm volatile("nop"); // カーソル無い時の速度調整
                __asm volatile("nop");
                __asm volatile("nop");
                __asm volatile("nop");
                __asm volatile("nop");
                __asm volatile("nop");
             }
            
            register unsigned char* ps = g_pScreen + SCREEN_W * y + x;
            ps -= (ps < g_aScreen + SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (CHAR_PATTERN[(*ps << 3) + ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }

        {
            /* configure peripheral base address */
            DMA_CHPADDR(VIDEO_DMA, VIDEO_DMA_CH) = (uint32_t)&SPI_DATA(VIDEO_SPI);

            /* configure memory base address */
            DMA_CHMADDR(VIDEO_DMA, VIDEO_DMA_CH) = (uint32_t)g_aLine;

            /* configure the number of remaining data to be transferred */
            DMA_CHCNT(VIDEO_DMA, VIDEO_DMA_CH) = (sizeof(g_aLine) & DMA_CHANNEL_CNT_MASK);

            /* configure peripheral transfer width,memory transfer width and priority */
            uint32_t ctl;
            ctl = DMA_CHCTL(VIDEO_DMA, VIDEO_DMA_CH);
            ctl &= ~(DMA_CHXCTL_PWIDTH | DMA_CHXCTL_MWIDTH | DMA_CHXCTL_PRIO);
            ctl |= (DMA_PERIPHERAL_WIDTH_8BIT | DMA_MEMORY_WIDTH_8BIT | DMA_PRIORITY_LOW);
            DMA_CHCTL(VIDEO_DMA, VIDEO_DMA_CH) = ctl;

            DMA_CHCTL(VIDEO_DMA, VIDEO_DMA_CH) &= ~DMA_CHXCTL_PNAGA;
            DMA_CHCTL(VIDEO_DMA, VIDEO_DMA_CH) |= DMA_CHXCTL_MNAGA;
            DMA_CHCTL(VIDEO_DMA, VIDEO_DMA_CH) |= DMA_CHXCTL_DIR;
        }

         /* configure DMA mode */
        dma_circulation_disable(VIDEO_DMA, VIDEO_DMA_CH);
        dma_memory_to_memory_disable(VIDEO_DMA, VIDEO_DMA_CH);

        {
            uint32_t reg = 0U;
            reg = SPI_CTL0(VIDEO_SPI);
            reg &= ((uint32_t)0x00003040U);  /*!< SPI parameter initialization mask */

            /* select SPI as master or slave */
            reg |= SPI_MASTER;
            /* select SPI transfer mode */
            reg |= SPI_TRANSMODE_BDTRANSMIT;
            /* select SPI frame size */
            reg |= SPI_FRAMESIZE_8BIT;
            /* select SPI NSS use hardware or software */
            // reg |= SPI_NSS_HARD;
            reg |= SPI_NSS_SOFT;
            /* select SPI LSB or MSB */
            reg |= SPI_ENDIAN_MSB;
            /* select SPI polarity and phase */
            reg |= SPI_CK_PL_LOW_PH_1EDGE;
            /* select SPI prescale to adjust transmit speed */
            //reg |= SPI_PSC_8;
            reg |= SPI_PSC_16;

            /* write to SPI_CTL0 register */
            SPI_CTL0(VIDEO_SPI) = (uint32_t)reg;

            SPI_I2SCTL(VIDEO_SPI) &= (uint32_t)(~SPI_I2SCTL_I2SSEL);
        }

        /* SPI DMA enable */
        spi_dma_enable(VIDEO_SPI, SPI_DMA_TRANSMIT);
        /* DMA channel enable */
        dma_channel_enable(VIDEO_DMA, VIDEO_DMA_CH);

        spi_enable(VIDEO_SPI);

        #if defined(VIDEO2_PB15)
        // gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15); // ng?
        GPIO_CTL1(GPIOB) = (GPIO_CTL1(GPIOB) & 0b00001111111111111111111111111111) | (0b1011 << ((15 - 8) * 4)); // << 28
        #elif defined(VIDEO2_PB5)
        // for PIN_5
        //gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, VIDEO_SPI_PIN); // ng?
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) &   0b11111111000011111111111111111111) | (0b1011 << (5 * 4)); // << 20
        #else
        error
        #endif

        while (x < SCREEN_W) {
            //register uint32_t c = (x==g_nCursorX && y==g_nCursorY) ? g_pCursorShape[ya] : 0x00;
            register uint32_t c = 0;
    		if ((frames >> 4) & _g.cursorflg) {
                if (x == _g.cursorx && y == _g.cursory) {
                    c = key_flg.insert ? 0xff : 0xf0;
                }
            }
            
            register unsigned char* ps = g_pScreen + SCREEN_W * y + x;
            ps -= (ps < g_aScreen + SCREEN_WH) ? 0 : SCREEN_WH;
            
            g_aLine[x] = (CHAR_PATTERN[(*ps << 3) + ya] ^ c) ^ g_ntsc_mode;
            ++x;           
        }

        TIMER_INTF(VIDEO_TIMER) = ~TIMER_INTF_CH1IF;
    } else if ((TIMER_INTF(VIDEO_TIMER) & TIMER_INTF_CH2IF) && (TIMER_DMAINTEN(VIDEO_TIMER) & TIMER_DMAINTEN_CH2IE) ){
        GPIO_OCTL(VIDEO_CLK_PORT) &= ~VIDEO_CLK_PIN;

        if (++g_nLine == NTSC_LINES) {
            g_nLine = 0;
            ntsc_frame_tick();
        }
        
        if (NTSC_BLANK < g_nLine && g_nLine <= NTSC_BLANK + (CHAR_H * SCREEN_H)) {
            dma_deinit(VIDEO_DMA, VIDEO_DMA_CH);
            spi_i2s_deinit(VIDEO_SPI);

            gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, VIDEO_SPI_PIN);
        }

        if (g_nLine == 0) {
            TIMER_CH0CV(VIDEO_TIMER) = NTSC_M - 1;                       // HSync -> NTSC_VSYNC
        } else if (g_nLine == NTSC_VSYNC) {
            TIMER_CH0CV(VIDEO_TIMER) = NTSC_S - 1;                       // NTSC_VSYNC -> HSync
        } else if (g_nLine == NTSC_BLANK) {
            TIMER_INTF(VIDEO_TIMER) = ~TIMER_INTF_CH1IF;
            TIMER_DMAINTEN(VIDEO_TIMER) |= TIMER_DMAINTEN_CH1IE;         // draw
        } else if (g_nLine == NTSC_BLANK + (CHAR_H * SCREEN_H)) {
            TIMER_DMAINTEN(VIDEO_TIMER) &= ~TIMER_DMAINTEN_CH1IE;        // no draw
        }

        TIMER_INTF(VIDEO_TIMER) = ~TIMER_INTF_CH2IF;
    }
    ntsc_line_tick();
}
