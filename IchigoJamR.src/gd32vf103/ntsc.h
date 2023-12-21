//* 
//* created by yrm (c) 2016
//* 
// #include "nrf.h"
// #include "ichigonquest.h"

#ifdef VSYNC
#undef VSYNC
#endif

#ifdef LINES
#undef LINES
#endif

int g_nScreenW;
int g_nScreenH;
int g_nScreenWH;

#define MAX_SCREEN_W 32
#define MAX_SCREEN_H 24

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


#define NTSC_S  488
#define NTSC_M  (NTSC_L - NTSC_S + 80)
#define NTSC_L  6102
#define NTSC_D  (NTSC_S * 2 + 400)

//#define LINES (262)
#define LINES 261 // 1.5beta5
#define VSYNC (3)
#define BLANK (3+44)
#define INITADDL 1

static uint16_t g_nLine = 0;
static uint8_t  g_nShift = 0;

uint8_t g_ntsc_addl = INITADDL;
uint8_t g_ntsc_mode = 0x00;

uint8_t g_ntsc_on = 0;

__attribute__ ((aligned(4)))
static uint8_t g_aLine0[MAX_SCREEN_W+1] = {};
__attribute__ ((aligned(4)))
static uint8_t g_aLine1[MAX_SCREEN_W+1] = {};

static uint8_t* g_pLine = g_aLine0;



void ntsc_onoff(int n) {
    if (n) {
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0011);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1011);
        
        g_ntsc_mode = (n & 0b1) ? 0x00 : 0xff;

        //g_nShift = (n-1) >> 1;
        g_nShift = _g.screen_big;
        SCREEN_W = 32 >> _g.screen_big; // 1.2big
        SCREEN_H = 24 >> _g.screen_big;
        g_nScreenW = MAX_SCREEN_W >> g_nShift;
        g_nScreenH = MAX_SCREEN_H >> g_nShift;
        g_nScreenWH = g_nScreenW * g_nScreenH;

        uint32_t psc = CTL0_PSC(0b011 + g_nShift);    // dafault: CTL0_PSC(0b011)

        if((SPI_CTL0(SPI0) & SPI_CTL0_PSC) != psc){
            while(SPI_STAT(SPI0) & SPI_STAT_TRANS);
            SPI_CTL0(SPI0) &= ~SPI_CTL0_SPIEN;
            SPI_CTL0(SPI0) = (SPI_CTL0(SPI0) & ~SPI_CTL0_PSC) | psc;

            screen_clear();
        }

        SPI_CTL1(SPI0) |= SPI_CTL1_DMATEN;
        SPI_CTL0(SPI0) |= SPI_CTL0_SPIEN;

        TIMER_INTF(TIMER3) = 0;
        TIMER_CTL0(TIMER3) |= TIMER_CTL0_CEN;

        TIMER_CTL0(TIMER5) &= ~TIMER_CTL0_CEN; // for USB
    } else {
        TIMER_CTL0(TIMER3) &= ~TIMER_CTL0_CEN;

        SPI_CTL1(SPI0) &= ~SPI_CTL1_DMATEN;
        SPI_CTL0(SPI0) &= ~SPI_CTL0_SPIEN;

        GPIO_BC(GPIOB) |= (1 << 4);
        GPIO_BC(GPIOB) |= (1 << 5);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);

        TIMER_CTL0(TIMER5) |= TIMER_CTL0_CEN; // for USB
    }
}

void ntsc_init() {
    g_nLine = 0;
    {
        rcu_periph_clock_enable(RCU_SPI0);      // ntsc
        rcu_periph_clock_enable(RCU_DMA0);      // ntsc
        rcu_periph_clock_enable(RCU_TIMER3);    // ntsc
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0011);
        AFIO_PCF0 = (AFIO_PCF0 & ~AFIO_PCF0_SPI0_REMAP) | (0b1 << 0);
        GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1011);
    }
    {
        /* configure peripheral base address */
        DMA_CHPADDR(DMA0, DMA_CH2) = (uint32_t)&SPI_DATA(SPI0);

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


    
    eclic_irq_enable(TIMER3_IRQn, 15, 0);

    TIMER_PSC(TIMER3) = 0; // prescaler
    TIMER_CAR(TIMER3) = NTSC_L - 1;
    TIMER_CH0CV(TIMER3) = NTSC_M - 1;
    TIMER_CH1CV(TIMER3) = NTSC_D - 1;
    // TIMER_CH2CV(TIMER3) = NTSC_L - 1;
    // TIMER_DMAINTEN(TIMER3) = TIMER_DMAINTEN_CH0IE | TIMER_DMAINTEN_CH2IE;
    TIMER_DMAINTEN(TIMER3) = TIMER_DMAINTEN_UPIE | TIMER_DMAINTEN_CH0IE;
    TIMER_INTF(TIMER3) = 0;
    
    TIMER_CTL0(TIMER3) |= TIMER_CTL0_CEN;

    // for USB
    TIMER_CH2CV(TIMER3) = (NTSC_L-NTSC_S)/2 + NTSC_S - 1;
    TIMER_DMAINTEN(TIMER3) = TIMER_DMAINTEN_UPIE | TIMER_DMAINTEN_CH0IE | TIMER_DMAINTEN_CH2IE;

    ntsc_onoff(1);
}



//__attribute__ ((section(".data")))
void TIMER3_IRQHandler() {
    if( (TIMER_INTF(TIMER3) & TIMER_INTF_CH0IF) && (TIMER_DMAINTEN(TIMER3) & TIMER_DMAINTEN_CH0IE) ){
        GPIO_BOP(GPIOB) = GPIO_BOP_BOP4;
        TIMER_INTF(TIMER3) &= ~TIMER_INTF_CH0IF;

    }else
    if( (TIMER_INTF(TIMER3) & TIMER_INTF_CH1IF) && (TIMER_DMAINTEN(TIMER3) & TIMER_DMAINTEN_CH1IE) ){
        if (g_ntsc_on) {
            /* disable DMA a channel */
            DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
            /* configure memory base address */
            DMA_CHMADDR(DMA0, DMA_CH2) = (uint32_t)g_pLine;
            /* configure the number of remaining data to be transferred */
            DMA_CHCNT(DMA0, DMA_CH2) = ((g_nScreenW + 1) & DMA_CHANNEL_CNT_MASK);
            /* DMA channel enable */
            DMA_CHCTL(DMA0, DMA_CH2) |= DMA_CHXCTL_CHEN;

            g_pLine = (g_pLine == g_aLine0)? g_aLine1 : g_aLine0;

            uint32_t y = ((g_nLine+1 - BLANK) >> g_nShift) / CHAR_H;
            uint32_t ya = ((g_nLine+1 - BLANK) >> g_nShift) % CHAR_H;
            if (g_nLine + 1 == BLANK+(CHAR_H*MAX_SCREEN_H)) {
                y = 0;
                ya = 0;
            }

            const uint8* crom1;
            const uint8* crom2;
            {
                //int chk = (8 << _g.screen_big) - 1;
                //uint8 offsety = ((lines - LINES_OFFSET) & chk) >> _g.screen_big;
                uint8 offsety = ya;
                crom1 = CHAR_PATTERN + offsety;
                crom2 = screen_pcg + offsety - ((0x100 - SIZE_PCG) * 8);
            }
            uint8_t invert = _g.screen_invert ? 0xff : 0; // invert or not

            uint8_t x = 0;
            while (x < g_nScreenW) {
                register uint32_t c = 0;
                if ((frames >> 4) & _g.cursorflg) {
                    if (x == _g.cursorx && y == _g.cursory) {
                        c = key_flg.insert ? 0xff : 0xf0;
                    }
                }
                
                register unsigned char* ps = g_pScreen + SCREEN_W * y + x;
                ps -= (ps < g_aScreen + SCREEN_WH) ? 0 : SCREEN_WH;
                
                //g_pLine[x] = (CHAR_PATTERN[(*ps << 3) + ya] ^ c) ^ g_ntsc_mode;
                uint8_t w = 0;
                if (*ps >= 0x100 - SIZE_PCG) {
                    w = crom2[*ps << 3];
                } else {
                    w = crom1[*ps << 3];
                }
                g_pLine[x] = w ^ c ^ g_ntsc_mode ^ invert;

                /* // latte
                register uint32_t c = (x==g_nCursorX && y==g_nCursorY) ? g_pCursorShape[ya] : 0x00;
                
                register unsigned char* ps = g_pScreen + g_nScreenW*y + x;
                ps -= (ps < g_aScreen+g_nScreenWH) ? 0 : g_nScreenWH;
                
                g_pLine[x] = (g_aFont[*ps][ya] ^ c) ^ g_ntsc_mode;
                */
                
                ++x;
            }
            g_pLine[x] = 0;     // Set the last bit to low
        }

        TIMER_INTF(TIMER3) &= ~TIMER_INTF_CH1IF;

    }else
    if( (TIMER_INTF(TIMER3) & TIMER_INTF_UPIF) && (TIMER_DMAINTEN(TIMER3) & TIMER_DMAINTEN_UPIE) ){
    // if( (TIMER_INTF(TIMER3) & TIMER_INTF_CH2IF) && (TIMER_DMAINTEN(TIMER3) & TIMER_DMAINTEN_CH2IE) ){
        GPIO_BOP(GPIOB) = GPIO_BOP_CR4;

        if(++g_nLine == LINES){
            g_nLine = 0;
            ntsc_frame_tick();
        }
        
        if(g_nLine == 0){
            TIMER_CH0CV(TIMER3) = NTSC_M - 1;                       // HSync -> VSync
        }else
        if(g_nLine == VSYNC){
            TIMER_CH0CV(TIMER3) = NTSC_S - 1;                       // VSync -> HSync
        }else
        if(g_nLine == BLANK){
            TIMER_INTF(TIMER3) &= ~TIMER_INTF_CH1IF;
            TIMER_DMAINTEN(TIMER3) |= TIMER_DMAINTEN_CH1IE;         // draw
        }else
        // if(g_nLine == BLANK+((CHAR_H<<0)*g_nScreenH)){
        if(g_nLine == BLANK+(CHAR_H*MAX_SCREEN_H)){
            TIMER_DMAINTEN(TIMER3) &= ~TIMER_DMAINTEN_CH1IE;        // no draw
        }

        // TIMER_INTF(TIMER3) &= ~TIMER_INTF_CH2IF;
        TIMER_INTF(TIMER3) &= ~TIMER_INTF_UPIF;
        ntsc_line_tick();
    } else if( (TIMER_INTF(TIMER3) & TIMER_INTF_CH2IF) && (TIMER_DMAINTEN(TIMER3) & TIMER_DMAINTEN_CH2IE) ){ // for USB
        /*
        // for Latte
        uint64_t t = get_timer_value() / (SystemCoreClock / 4 / 1000);
        if(g_msec != t){
            g_msec = t;
            //screen_blink();
        }
        */
        if (g_nLine % 16 == 0) {
            if (EXTI_INTEN & EXTI_INTEN_INTEN0) {
                EXTI_SWIEV |= EXTI_0;
            }

            keyboard_scan();

            /*
            // for Latte
            rand_shake(g_msec);

            // btn to stdin buf
            extern volatile uint8_t g_btn_last;
            if(g_btn_code && g_btn_last != io_btn()){
                if(g_btn_last){
                    // pushed
                    g_btn_last = 0;
                    
                    file_dread(FD_STDIN, (void*)&g_btn_code, 0);
                }else{
                    // unpushed
                    g_btn_last = 1;
                }
            }
            */
            if(g_usbkey){
                --g_usbkey_delay;

                if(g_usbkey_delay == 0){
                    keybrd_putsc_on(g_usbkey);
                    g_usbkey_delay = 50;
                }
            }
        }

        TIMER_INTF(TIMER3) &= ~TIMER_INTF_CH2IF;
    }
}

// void DMA0_Channel2_IRQHandler(){
//     if((DMA_INTF(DMA0) & DMA_FLAG_ADD(DMA_INT_FLAG_FTF, DMA_CH2)) && (DMA_CHCTL(DMA0, DMA_CH2) & DMA_CHXCTL_FTFIE)){
//         /* disable DMA a channel */
//         DMA_CHCTL(DMA0, DMA_CH2) &= ~DMA_CHXCTL_CHEN;

//         DMA_INTC(DMA0) |= DMA_FLAG_ADD(DMA_INT_FLAG_FTF, DMA_CH2);
//     }
// }


