// keyboard.c
int keyboard_enqKeyBuf(uint8_t k);
uint8_t keyboard_deqKeyBuf(void);
void keyboard_scan(void);
void keyboard_setKeymap(int mode);
int keyboard_getKeymap(void);

//
void keyboard_init() { // from key_init
    // try for ps2
    {
        GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(12-8)) | GPIO_MODE_SET(12-8, 0b1000);      // kbd1
        GPIO_BOP(GPIOA) |= GPIO_PIN_12;
        GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b1000);      // kbd2
        GPIO_BOP(GPIOA) |= GPIO_PIN_11;

        _msleep(1);
				
        if ((GPIO_ISTAT(GPIOA) & GPIO_PIN_12) && (GPIO_ISTAT(GPIOA) & GPIO_PIN_11)) {

            // enable ps2 int
            EXTI_INTEN |= EXTI_INTEN_INTEN12;
            EXTI_FTEN |= EXTI_FTEN_FTEN12;
            eclic_irq_enable(EXTI10_15_IRQn, 8, 0);

            _msleep(1300);                          // waiting for waking up kbd
            if (ps2_send(0xff)) {                     // 0xff is Reset
                _msleep(800);                       // 'v' is comming
                while (keyboard_deqKeyBuf());	    // clear buffer
            } else {
                // disable ps2 int
                EXTI_INTEN &= ~EXTI_INTEN_INTEN12;
                eclic_irq_disable(EXTI10_15_IRQn);

                GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(12-8)) | GPIO_MODE_SET(12-8, 0b0100);      // kbd1
                GPIO_BOP(GPIOA) &= ~GPIO_PIN_12;
                GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b0100);      // kbd2
                GPIO_BOP(GPIOA) &= ~GPIO_PIN_11;
            }
        } else {
            GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(12-8)) | GPIO_MODE_SET(12-8, 0b0100);      // kbd1
            GPIO_BOP(GPIOA) &= ~GPIO_PIN_12;
            GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(11-8)) | GPIO_MODE_SET(11-8, 0b0100);      // kbd2
            GPIO_BOP(GPIOA) &= ~GPIO_PIN_11;
        }
    }
    if (!ps2_sent()) {
		//if (1) {
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
			//TIMER_CAR(TIMER5) = 96000 - 1;
			TIMER_DMAINTEN(TIMER5) = TIMER_DMAINTEN_UPIE;
			TIMER_INTF(TIMER5) = 0;
			// TIMER_CTL0(TIMER5) |= TIMER_CTL0_CEN;
			eclic_irq_enable(TIMER5_IRQn, 0, 0);

		// software interrupt
			EXTI_INTEN |= EXTI_INTEN_INTEN0;
			eclic_irq_enable(EXTI0_IRQn, 0, 0);
		}
	}


void EXTI0_IRQHandler() {
    if (EXTI_PD & EXTI_0) {
        usbh_core_task(&usb_hid_core, &usb_host);
        EXTI_PD |= EXTI_0;
    }
}


void keybrd_putsc_on(uint8_t uid);

void TIMER5_IRQHandler() {   // on1msec when only VIDEO0
    if ((TIMER_INTF(TIMER5) & TIMER_INTF_UPIF) && (TIMER_DMAINTEN(TIMER5) & TIMER_DMAINTEN_UPIE)) {
        //g_msec = get_timer_value() / (SystemCoreClock / 4 / 1000);

        usbh_core_task(&usb_hid_core, &usb_host);
        keyboard_scan();

		/* // for Latte
        // screen_blink();
        //rand_shake(g_msec);

        // btn to stdin buf
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
		
        if (g_usbkey){
            --g_usbkey_delay;

            if(g_usbkey_delay == 0){
                keybrd_putsc_on(g_usbkey);
                g_usbkey_delay = 50;
            }
        }

        TIMER_INTF(TIMER5) &= ~TIMER_INTF_UPIF;
    }
}

/*
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
*/

//
const char* flash_getAddress(uint8_t num);
void flash_write(int num, uint8* list, int size);


static inline uint key_getKeyboardID() {
	return keyboard_getKeymap();
}
void key_send_reset() {
}

struct keyflg_def key_flg;

INLINE void IJB_kbd(uint mode) { // 0:US, 1:JP 1.5b1
	//int m = keyboard_getKeymap();
	//IJB_led(m);
	keyboard_setKeymap(mode);
	uint8 buf[4];
	buf[0] = mode;
	flash_write(0, buf, 4);
}
inline static void key_init() {
	keyboard_init();

	const char* settings = flash_getAddress(0);
	int mode = settings[0];
	keyboard_setKeymap(mode);
}

INLINE void key_enable(uint8 b) {
//	key_enable_flg = b;
}
//void uart_checker();


char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！

#define keykbhit keybuf[-1]

/*inline*/ int key_btn(int n) {
	//	uart_checker();
	n -= n == 88 ? 88 - 5 : 28; // X or not
	return (keykbhit & (1 << n)) != 0;
}
void key_setBtn(int n, int m) { // n:28-32,88
	n -= n == 88 ? 88 - 5 : 28; // X or not
	if (m) {
		keykbhit |= 1 << n;
	} else {
		keykbhit &= ~(1 << n);
	}
}

int keyboard_enqKeyBufWithoutKana(uint8_t k);

void key_pushc(int c) {
	keyboard_enqKeyBufWithoutKana(c);
}
int kanamode = 0;

int key_getKey(void) {
	int ch = keyboard_deqKeyBuf();
	if (!ch) {
		return -1;
	}
	// romaji
	/*
	if (ch == 0xf) {
		kanamode = !kanamode;
		key_kana_buf0 = 0;
	}
	if (kanamode) {
		ch = romajikana_input(ch);
		if (!ch)
			return -1;
	}
	*/
	return ch;


	/*
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	(*keybuf)--;
	for (uint8 i = 0; i < *keybuf; i++)
		keybuf[i + 1] = keybuf[i + 2];
	return res;
	*/
}
void keyboard_clearQueue(void);
void key_clearKey() {
	//	keykbhit = 0; // ver1.2b5 clear しないように変更
	//*keybuf = 0;
	keyboard_clearQueue();
}

#define DEFAULT_UARTMODE_TXD 2
#define DEFAULT_UARTMODE_RXD 1

void UARTInit(int bps) {
	rcu_periph_clock_enable(RCU_USART0);
	// uart
	GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(9 - 8)) | GPIO_MODE_SET(9 - 8, 0b1011); // txd out
	//GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(10 - 8)) | GPIO_MODE_SET(10 - 8, 0b0100); // rxd out
	GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(10 - 8)) | GPIO_MODE_SET(10 - 8, 0b1000); // rxd in
	GPIO_OCTL(GPIOA) |= GPIO_PIN_10; // pull-up // 1.5b7
	
    USART_CTL0(USART0) &= ~USART_CTL0_UEN;

    uint32_t udiv = (SystemCoreClock + bps / 2) / bps;
    uint16_t intdiv = udiv & 0xfff0;
    uint16_t fradiv = udiv & 0x000f;
    USART_BAUD(USART0) = ((USART_BAUD_FRADIV | USART_BAUD_INTDIV) & (intdiv | fradiv));

    USART_CTL0(USART0) |= USART_CTL0_TEN | USART_CTL0_REN;

    //eclic_irq_enable(USART0_IRQn, 1, 0); // 元
    eclic_irq_enable(USART0_IRQn, 4, 0); // IchigoLatte
    USART_CTL0(USART0) |= USART_CTL0_RBNEIE; // あると落ちる、ないと割り込みこない -> なおった
    USART_CTL0(USART0) |= USART_CTL0_UEN;

#if DEFAULT_UARTMODE_TXD != 0
	_g.uartmode_txd = DEFAULT_UARTMODE_TXD; // 最初のクリア信号を送らない
#endif
	_g.uartmode_rxd = DEFAULT_UARTMODE_RXD;
}

INLINE void uart_init(void) {
	UARTInit(115200);
}

void USART0_IRQHandler() {
	if ((USART_STAT(USART0) & USART_STAT_RBNE) && (USART_CTL0(USART0) & USART_CTL0_RBNEIE)) {
		//keyboard_enqKeyBuf(USART_DATA(USART0) & 0xff);
		int c = USART_DATA(USART0) & 0xff;
		if (_g.uartmode_rxd) {
			// キーフラグ シリアルからはトグルする // 1.2b47 シリアル無効時はkey状態もescも効かないようにする（いつからだ？）
			if (c >= 28 && c <= 32) {
				keykbhit ^= 1 << (c - 28);
			} else if (c == 27) {
	//			_g.key_flg_esc = 1;
	//			_g.key_flg_esc = _g.uartmode_rxd != 2; // 1.1b14
				_g.key_flg_esc = (_g.uartmode_rxd & 2) == 0; // 1.2b41
			}
			// ここまで

			if (c == '\r' && (_g.uartmode_rxd & 4)) { // 1.2b41 CR mode
				c = '\n';
			}
				/*
			if (firstuart) {
				if (_g.uartmode_rxd == 2) {
					key_pushc('\'');
				}
				firstuart = 0;
			}
				*/
//			key_pushc(c);
			key_pushc(c);
//			firstuart = c == '\n';
		}

	}
}

INLINE void IJB_uart(int16 txd, int16 rxd) {
	_g.uartmode_txd = txd;
	_g.uartmode_rxd = rxd;
}

void uart_putc(char c) { // 1.1b7 uart3でバッファモード, b12 バッファモードやめる？
	if (_g.uartmode_txd == 3) {
		// no buffering
		if (c == '\n') {
			uart_putc('\r');
		}
	}
	while (!(USART_STAT(USART0) & USART_STAT_TBE));
	USART_DATA(USART0) = c;
	while (!(USART_STAT(USART0) & USART_STAT_TC));
}

/*inline*/ void uart_bps(int n) { // 0:115200, -1:57600, -2:38400
	if (n == 0) {
		n = 115200;
	} else if (n == -1) {
		n = 57600;
	} else if (n == -2) {
		n = 38400;
	}
	UARTInit(n);
}

STATIC void put_chr(char c) {
	if (_g.uartmode_txd > 0) { // 1.0.2b12 uartを先に
		uart_putc(c);
	}
	screen_putc(c);
}

// basic interface
inline int stopExecute(void) {
	return _g.key_flg_esc;
}
void key_setESC(void) {
	_g.key_flg_esc = 1;
}

