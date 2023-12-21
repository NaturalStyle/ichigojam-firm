/*
[終わったやつ]
* video	timer1
* keyboard	timer6
* in
* out
* btn
* pwm		timer2,4
* sound(pwm)	timer0
* led
* ana
* uart
* i2c
[まだ]
—
* spi
* dac
* deep sleep
*/

#define FLASH_BLOCKS (N_FLASH_STORAGE + 1) // == 16 64ROMだと8にするかも
#define FILE_SIZE (1 * 1024)
#define SYSTEM_PAGESIZE 1024
#define FILE_ADDR (FLASH_BASE + 0x20000 - FILE_SIZE * FLASH_BLOCKS)

uint8 g_nCursorX = 0;
uint8 g_nCursorY = 0;

// keyboard.c
void keyboard_scan(void);


// system.h
void system_init();
void enterDeepSleep(int waitsec);
inline void deepPowerDown();
inline void IJB_sleep();
inline static void IJB_reset();

int getSleepFlag() {
	return 0;
}
void system_init() {
}
inline void deepPowerDown() {
}
inline void IJB_sleep() {
}
inline static void IJB_reset() {
    eclic_system_reset();
}
void ntsc_line_tick() {
	_g.linecnt++;
}
void ntsc_frame_tick() {
	_g.vflag = 1;
    frames++;
    psg_tick();
}
int IJB_wait(int n, int active) { // if stop ret 1
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
        enterDeepSleep(n * 16); // msec, deep
	}
	return 0;
}

void ichigojam_main(void);

/*
int main() {
	ichigojam_main();
	return 0;
}
void _start() {
	main();
}
*/
//

#include "gd32vf103.h"
// #include "gd32vf103_timer.h"

#include "systick.h"

#include "drv_usb_hw.h"
#include "drv_usb_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"
//#include "ichigolatte.h"

#define SCREEN_WH (SCREEN_W * SCREEN_H)

// usbh_hid_keybd.c
extern uint8_t  g_usbkey;
extern uint16_t g_usbkey_delay;


#include "ntsc.h"

// extern char _eram;
// extern char _etext;
// extern char _data;
// extern char _edata;
// extern char _bss;
// extern char _ebss;
extern char _edbc;

//const char NULL_STR[] = "";
//const char E_FILE_ERROR[]   = "file error.";


volatile uint64_t g_msec = 0;

volatile int32_t g_env = 0;

#include "deepsleep.h"

//volatile uint8_t g_flash_files = 4;


/*
static void* mem_top = &_edbc;

size_t mem_check() {
    register void* sp asm("sp");
    return (size_t)sp - (size_t)mem_top;
}

void* mem_alloc(size_t size) {
    register void* pr = mem_top;
    
    mem_top += size;
    
    return pr;
}

void mem_free(void* mem){
    mem_top = mem;
}
*/

usb_core_driver usb_hid_core;

// void dummy(){}

void dbg_dev_init                    (void){}
void dbg_dev_deinit                  (void){}
void dbg_dev_attach                  (void){}
void dbg_dev_reset                   (void){}
void dbg_dev_detach                  (void){}
void dbg_dev_over_currented          (void){}
void dbg_dev_speed_detected          (uint32_t dev_speed){}
void dbg_dev_devdesc_assigned        (void *dev_desc){}
void dbg_dev_address_set             (void){}

void dbg_dev_cfgdesc_assigned        (usb_desc_config *cfg_desc,
                                        usb_desc_itf *itf_desc,
                                        usb_desc_ep *ep_desc){}

void dbg_dev_mfc_str                 (void *mfc_str){}
void dbg_dev_prod_str                (void *prod_str){}
void dbg_dev_seral_str               (void *serial_str){}
void dbg_dev_enumerated              (void){}
usbh_user_status dbg_dev_user_input  (void){return USBH_USER_RESP_OK;}
void dbg_dev_user_app                (void *user_app){}
void dbg_dev_not_supported           (void){}
void dbg_dev_error                   (void){}

usbh_user_cb user_callback_funs =
{
    dbg_dev_init,
    dbg_dev_deinit,
    dbg_dev_attach,
    dbg_dev_reset,
    dbg_dev_detach,
    dbg_dev_over_currented,
    dbg_dev_speed_detected,
    dbg_dev_devdesc_assigned,
    dbg_dev_address_set,
    dbg_dev_cfgdesc_assigned,
    dbg_dev_mfc_str,
    dbg_dev_prod_str,
    dbg_dev_seral_str,
    dbg_dev_enumerated,
    dbg_dev_user_input,
    dbg_dev_user_app,
    dbg_dev_not_supported,
    dbg_dev_error
};

usbh_host usb_host = {
    .class_cb = &usbh_hid_cb,
    .usr_cb   = &user_callback_funs
};



void _sleep(int msec, int deep){
	if(deep){
// ###
	}else{
		register uint64_t t = g_msec + msec;
		while(g_msec < t);
	}
}

void ledon();
void lchika(int n);
void led_red();

int main(void) {
    SystemInit();

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);

    // NTSC
    rcu_periph_clock_enable(RCU_SPI0);      // ntsc
    rcu_periph_clock_enable(RCU_DMA0);      // ntsc
    rcu_periph_clock_enable(RCU_TIMER1);    // ntsc
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0011);
    AFIO_PCF0 = (AFIO_PCF0 & ~AFIO_PCF0_SPI0_REMAP) | (0b1 << 0);
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1011);


    eclic_global_interrupt_enable();
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL2_PRIO2);

    // JTAG disable
    AFIO_PCF0 = (AFIO_PCF0 & ~AFIO_PCF0_SWJ_CFG) | (0b100 << 24);

    /*
    // test for Wie Lite
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);  //user LED
    GPIO_OCTL(GPIOA) |= GPIO_PIN_8;
    for (;;);
    */

    eclic_global_interrupt_enable();

    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL2_PRIO2);

    // USB keyboard
    rcu_periph_clock_enable(RCU_USBFS);     // kbd
    rcu_periph_clock_enable(RCU_TIMER6);    // kbd
    usb_rcu_config();
    // usb_timer_init();
    /* configure GPIO pin used for switching VBUS power and charge pump I/O */
    // usb_vbus_config();
    usbh_init(&usb_hid_core, USB_CORE_ENUM_FS, &usb_host);
    /* enable interrupts */
    usb_intr_config();

    // 1msec timer
    rcu_periph_clock_enable(RCU_TIMER5);
    TIMER_PSC(TIMER5) = 1;
    TIMER_CAR(TIMER5) = 48000 - 1;
    TIMER_DMAINTEN(TIMER5) = TIMER_DMAINTEN_UPIE;
    TIMER_INTF(TIMER5) = 0;
    TIMER_CTL0(TIMER5) |= TIMER_CTL0_CEN;
    eclic_irq_enable(TIMER5_IRQn, 0, 0);

    // printf(LOGO, VER0, VER1, VER2, VERS);
    ichigojam_main();
    return 0;
}

void keybrd_putsc_on();

void TIMER5_IRQHandler() { // 1msec timer for USB
    if ((TIMER_INTF(TIMER5) & TIMER_INTF_UPIF) && (TIMER_DMAINTEN(TIMER5) & TIMER_DMAINTEN_UPIE)) {
        ++g_msec;

        if (g_usbkey) {
            --g_usbkey_delay;
            if (g_usbkey_delay == 0) {
                keybrd_putsc_on(g_usbkey);
                g_usbkey_delay = 50;
            }
        }

        usbh_core_task(&usb_hid_core, &usb_host);
        keyboard_scan();
        // screen_blink(); // for cursor
        // rand_shake(g_msec);

        TIMER_INTF(TIMER5) = ~TIMER_INTF_UPIF;
    }
}


// for debug
#define LCHIKA_PIN GPIO_PIN_14
void ledon() {
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCHIKA_PIN);
    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(14 - 8)) | GPIO_MODE_SET(14 - 8, 0b0011);
    GPIO_OCTL(GPIOA) |= LCHIKA_PIN;
}
void lchika(int n) {
    //#define LCHIKA_PIN GPIO_PIN_1
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LCHIKA_PIN);
    while (n--) {
        GPIO_OCTL(GPIOA) |= LCHIKA_PIN;
        delay_1ms(100);
        GPIO_OCTL(GPIOA) &= ~LCHIKA_PIN;
        delay_1ms(100);
    }
    //delay_1ms(100);
}

void led_red() {
    GPIO_OCTL(GPIOA) ^= GPIO_PIN_1;
}

void led_yellow() {
    GPIO_OCTL(GPIOA) ^= GPIO_PIN_2;
}


