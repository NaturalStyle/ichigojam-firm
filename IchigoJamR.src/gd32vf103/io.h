/*INLINE*/ void IJB_pwm(int port, int plen, int len);
INLINE void pwm_off(int port);
void io_init();
INLINE void io_set(int n);
/*INLINE*/ int analog_get(int ch);
/*INLINE*/ int io_get(void);
INLINE void IJB_led(int st);
/*INLINE*/ int IJB_in();
INLINE void IJB_clo();
INLINE int IJB_ana(int n);
/*INLINE*/ int IJB_btn(int n);
/*INLINE*/ void IJB_out(int port, int st);
/*INLINE*/ int IJB_i2c(uint8 writemode, uint16* param); // 0:success 1:io error 2:parm error
void IJB_dac(int port, int val);

int i2c(uint8_t wbf[], size_t nwb, uint8_t rbf[], size_t nrb);
int io_ana(int n);
int io_in(int n);
void io_out(int n);
void io_pwm(int n, uint32_t d);
void io_dac_off(int port);

#include "i2c.h"

int i2c0_init() { // 1.4以下のI2C共有時用
	return 0;
}


// PWM
// PWM 2,100  100kHz PWM  OUT2,3,4
// PWM 2,100,2000 で周期設定、2-4はまとめて設定、5は独立
// 	PWM n で、OUT出力からは除外される（設定してないと出力されない）
//  OUT n,0 でPWMは停止する
/*INLINE*/ void IJB_pwm(int port, int plen, int len) {
	if (!len) {
		len = 2000; // 20msec default
	}
	if (len < 0) { // len マイナスで prescaleを1に (未対応)
	}
    if (len < plen) {   // パルス幅が周期を越えたらPWMをやめる
        plen = 0;
    }
	io_pwmt(len);
	io_pwm(port, plen);
}
INLINE void pwm_off(int port) {
	io_pwm(port, 0);
}
// I/O

// port
int portout = 0;
int portpullup = 0;

/*
IN
	1 PIO0_10
	2 PIO0_11
	3 PIO0_5 (OD)
	4 PIO0_6
	5 PIO1_0 OUT1と共有
	6 PIO1_1 OUT2と共有
	7 PIO1_2 OUT3と共有
	8 PIO1_3 OUT4と共有
	OUT
	1 PIO1_0 IN5と共有
	2 PIO1_1 IN6と共有
	3 PIO1_2 IN7と共有
	4 PIO1_3 IN8と共有
	5 PIO1_9
	6 PIO1_8
	*/ 
void io_init() {
    //rcu_periph_clock_enable(RCU_GPIOA);
    //rcu_periph_clock_enable(RCU_GPIOB);

	// led
	GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(14-8)) | GPIO_MODE_SET(14-8, 0b0011); // led(out7) PA14
	GPIO_BOP(GPIOA) = GPIO_BOP_CR14;

	// btn
	// 00:analog 01:floating input 10:input with pullup/down( OCTL 0:pull-down 1:pull-up) 11:reserved
	// 00:push pull, 01:open drain, 10:analog push-pull, 11:analog open drain
	GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b1000); // PA0
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP0; // pull-up

    // in
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b1000);          // in1 PA1
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP1; // pull-up
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);          // in2 PA4
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP4; // pull-up
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);          // in3 PA5
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP5; // pull-up
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(6)) | GPIO_MODE_SET(6, 0b1000);          // in4 PA6
	GPIO_BOP(GPIOA) = GPIO_BOP_BOP6; // pull-up

    // out
    GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(8-8)) | GPIO_MODE_SET(8-8, 0b0011);      // out1 PA8
    GPIO_BOP(GPIOA) = GPIO_BOP_CR8;
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(7)) | GPIO_MODE_SET(7, 0b0011);          // out2 PA7
    GPIO_BOP(GPIOA) = GPIO_BOP_CR7;
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b0011);          // out3 PB0
    GPIO_BOP(GPIOB) = GPIO_BOP_CR0;
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b0011);          // out4 PB1
    GPIO_BOP(GPIOB) = GPIO_BOP_CR1;
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(2)) | GPIO_MODE_SET(2, 0b0011);          // out5 PA2
    GPIO_BOP(GPIOA) = GPIO_BOP_CR2;
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(3)) | GPIO_MODE_SET(3, 0b0011);          // out6 PA3
    GPIO_BOP(GPIOA) = GPIO_BOP_CR3;
    portout =    0b00001111111; // OUT1-7, OUT8-11(in)
    portpullup = 0b11111111111; // pullup

	// i2c
	// i2c_init();

    // adc
    rcu_periph_clock_enable(RCU_ADC1);
    ADC_OVSCR(ADC1) &= ~ADC_OVSCR_DRES;
    ADC_OVSCR(ADC1) |= ADC_RESOLUTION_10B;
    ADC_CTL1(ADC1) |= ADC_CTL1_ADCON;

    // timer for pwm
    rcu_periph_clock_enable(RCU_TIMER0);    // pwm1
    rcu_periph_clock_enable(RCU_TIMER2);    // pwm2,3,4
    rcu_periph_clock_enable(RCU_TIMER4);    // pwm5,6

    // [timer0] CH0:pwm1
    TIMER_PSC(TIMER0) = 960 - 1;
    TIMER_CAR(TIMER0) = 2000 - 1;
    TIMER_CHCTL0(TIMER0) &= ~TIMER_CHCTL0_CH0COMCTL;
    TIMER_CHCTL0(TIMER0) |= (TIMER_OC_MODE_PWM0 << 0);
    TIMER_CHCTL2(TIMER0) |= TIMER_CHCTL2_CH0EN;
    TIMER_CTL0(TIMER0) |= TIMER_CTL0_ARSE;
    TIMER_CCHP(TIMER0) |= TIMER_CCHP_POEN;

    // [timer2] CH1:pwm2, CH2:pwm3, CH3:pwm4
    TIMER_PSC(TIMER2) = 960 - 1;
    TIMER_CAR(TIMER2) = 2000 - 1;
    TIMER_CHCTL0(TIMER2) &= ~TIMER_CHCTL0_CH1COMCTL;
    TIMER_CHCTL0(TIMER2) |= (TIMER_OC_MODE_PWM0 << 8);
    TIMER_CHCTL1(TIMER2) &= ~(TIMER_CHCTL1_CH2COMCTL | TIMER_CHCTL1_CH3COMCTL);
    TIMER_CHCTL1(TIMER2) |= ((TIMER_OC_MODE_PWM0 << 0) | (TIMER_OC_MODE_PWM0 << 8));
    TIMER_CHCTL2(TIMER2) |= (TIMER_CHCTL2_CH1EN | TIMER_CHCTL2_CH2EN | TIMER_CHCTL2_CH3EN);
    TIMER_CTL0(TIMER2) |= TIMER_CTL0_ARSE;

    // [timer4] CH2:pwm5, CH3:pwm6
    TIMER_PSC(TIMER4) = 960 - 1;
    TIMER_CAR(TIMER4) = 2000 - 1;
    TIMER_CHCTL1(TIMER4) &= ~(TIMER_CHCTL1_CH2COMCTL | TIMER_CHCTL1_CH3COMCTL);
    TIMER_CHCTL1(TIMER4) |= ((TIMER_OC_MODE_PWM0 << 0) | (TIMER_OC_MODE_PWM0 << 8));
    TIMER_CHCTL2(TIMER4) |= (TIMER_CHCTL2_CH2EN | TIMER_CHCTL2_CH3EN);
    TIMER_CTL0(TIMER4) |= TIMER_CTL0_ARSE;

    // PWM off
    for (int port = 1; port <= 6; port++) {
    	io_pwm(port, 0);
    }

    // DAC
    rcu_periph_clock_enable(RCU_DAC); // GPIOA_4, GPIOA_5
    io_dac_off(9);
    io_dac_off(10);
}
// ADC
/*INLINE*/ int analog_get(int ch) {
	return io_ana(ch);
}

#define ANA_THRESHOLD (1024 / 4)


INLINE void IJB_led(int st) {
	IJB_out(7, st != 0);
}
/*INLINE*/ int IJB_in() {
	return io_get();
}
// out buffer
int outbuffer = 0;
INLINE void io_set(int n) {
	outbuffer = n;
	io_out(n);
}

int getPortIN() {
    return ((portout & 0b11110000000) >> 7) | ((portout & 0b1111) << 4) | ((portout & 0b110000) << 5);
}
/*INLINE*/ int io_get(void) {
	int res = 0;
    int portin = getPortIN();
	for (int i = 1; i <= 11; i++) {
        if (!(portin & (1 << (i - 1)))) {
    		res |= io_in(i) << (i - 1);
        }
	}
	return res;
}

/*INLINE*/ void IJB_out(int port, int st) {
    // DAC off
    if (port == 9 || port == 10) {
        io_dac_off(port);
    }
    // PWM off
    if (port >= 1 && port <= 6) {
    	io_pwm(port, 0);
    }
    //
    if (st < 0) {
        if (st == -2) { // pullup IN
            int bport = 1 << (port - 1);
            if (port == 1) {
                GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(8-8)) | GPIO_MODE_SET(8-8, 0b1000);      // out1 PA8
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP8; // pull-up
            } else if (port == 2) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(7)) | GPIO_MODE_SET(7, 0b1000);          // out2 PA7
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP7; // pull-up
            } else if (port == 3) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b1000);          // out3 PB0
                GPIO_BOP(GPIOB) = GPIO_BOP_BOP0; // pull-up
            } else if (port == 4) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b1000);          // out4 PB1
                GPIO_BOP(GPIOB) = GPIO_BOP_BOP1; // pull-up
            } else if (port == 5) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(2)) | GPIO_MODE_SET(2, 0b1000);          // out5 PA2
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP2; // pull-up
            } else if (port == 6) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(3)) | GPIO_MODE_SET(3, 0b1000);          // out6 PA3
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP3; // pull-up
            } else if (port == 7) {
                // ignore LED pin setting
            } else if (port == 8) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b1000);          // in1 PA1 out8
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP1; // pull-up
            } else if (port == 9) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);          // in2 PA4 out9
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP4; // pull-up
            } else if (port == 10) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);          // in3 PA5 out10
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP5; // pull-up
            } else if (port == 11) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(6)) | GPIO_MODE_SET(6, 0b1000);          // in4 PA6 out11
                GPIO_BOP(GPIOA) = GPIO_BOP_BOP6; // pull-up
            }
            portout &= ~bport;
            portpullup |= bport;
        } else if (st == -1) {
            int bport = 1 << (port - 1);
            if (port == 1) {
                GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(8-8)) | GPIO_MODE_SET(8-8, 0b0100);      // out1 PA8
            } else if (port == 2) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(7)) | GPIO_MODE_SET(7, 0b0100);          // out2 PA7
            } else if (port == 3) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b0100);          // out3 PB0
            } else if (port == 4) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b0100);          // out4 PB1
            } else if (port == 5) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(2)) | GPIO_MODE_SET(2, 0b0100);          // out5 PA2
            } else if (port == 6) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(3)) | GPIO_MODE_SET(3, 0b0100);          // out6 PA3
            } else if (port == 7) {
                // ignore LED pin setting
            } else if (port == 8) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b0100);          // in1 PA1 out8
            } else if (port == 9) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0100);          // in2 PA4 out9
            } else if (port == 10) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b0100);          // in3 PA5 out10
            } else if (port == 11) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(6)) | GPIO_MODE_SET(6, 0b0100);          // in4 PA6 out11
            }
            portout &= ~bport;
            portpullup &= ~bport;
        }
        return;
    } else {
        int bport = 1 << (port - 1);
        if (!(portout & bport)) {
            if (port == 1) {
                GPIO_CTL1(GPIOA) = (GPIO_CTL1(GPIOA) & ~GPIO_MODE_MASK(8-8)) | GPIO_MODE_SET(8-8, 0b0011);      // out1 PA8
            } else if (port == 2) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(7)) | GPIO_MODE_SET(7, 0b0011);          // out2 PA7
            } else if (port == 3) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b0011);          // out3 PB0
            } else if (port == 4) {
                GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b0011);          // out4 PB1
            } else if (port == 5) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(2)) | GPIO_MODE_SET(2, 0b0011);          // out5 PA2
            } else if (port == 6) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(3)) | GPIO_MODE_SET(3, 0b0011);          // out6 PA3
            } else if (port == 7) {
                // ignore LED pin setting
            } else if (port == 8) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(1)) | GPIO_MODE_SET(1, 0b0011);          // in1 PA1 out8
            } else if (port == 9) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0011);          // in2 PA4 out9
            } else if (port == 10) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b0011);          // in3 PA5 out10
            } else if (port == 11) {
                GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(6)) | GPIO_MODE_SET(6, 0b0011);          // in4 PA6 out11
            }
            portout |= bport;
        }
    }
	if (port == 0) {
		outbuffer = st;
	} else {
		if (st) {
			outbuffer |= 1 << (port - 1);
		} else {
			outbuffer &= ~(1 << (port - 1));
		}
		io_pwm(port, 0); // todo 効率悪い、状態チェック
	}
	io_set(outbuffer);
}

int io_btn() {
    uint32_t org = GPIO_CTL0(GPIOA);
    GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(0)) | GPIO_MODE_SET(0, 0b1000);
    int r = (GPIO_ISTAT(GPIOA) >> 0) & 0b1;
    GPIO_CTL0(GPIOA) = org;
    return !r;
}

/*inline*/ int key_btn(int n);

/*INLINE*/ int IJB_btn(int n) {
	if (n == 0) {
		//return !((GPIO_ISTAT(GPIOA) >> 0) & 1);
		return io_btn();
	} else if (n < 0) {
		return keykbhit;
	} else {
		return key_btn(n);
	}
	return 0;
}
INLINE int IJB_ana(int n) {
	return analog_get(n);
}
INLINE void IJB_clo() {
	io_init();
}

// writemode: I2C_READ, I2C_WRITE
//#define I2C_READ 0 //??
//#define I2C_WRITE 1 //??
/*INLINE*/ int IJB_i2c(uint8 writemode, uint16* param) { // 0:success 1:io error 2:parm error
	// i2cad, int adcmd, int lencmd, int addata, int lendata

    uint8_t buf[1024];
    buf[0] = param[0] << 1;
    //buf[1] = 0;
    //buf[2] = 0;
    // buf[0] = param[0]<<1;
	int lencmd = param[2]; // 1-4byte それ以上はNG! -> 緩和
    uint8_t* src = (uint8_t*)(param[1] + (uint)ram - OFFSET_RAMROM);
    for (int i = 0; i < lencmd; i++) {
        buf[i + 1] = src[i];
        //put_num(src[i]);
    }
    lencmd++;

    uint8_t* addata = (uint8_t*)(param[3] + (uint)ram - OFFSET_RAMROM);
    int lendata = param[4];

    if (!writemode) {
        for (int i = 0; i < lendata; i++) {
            buf[lencmd + i] = addata[i];
            //put_num(src[i]);
        }
        lencmd += lendata;
        return !i2c(buf, lencmd, NULL, 0);
    } else {
        // int adcmd = param[1];
        //int lencmd = param[2] + 1; // 1-4byte それ以上はNG! -> 緩和
        // adcmd -= OFFSET_RAMROM;
        // if (lencmd > 0 && (adcmd < 0 || adcmd + lencmd > SIZE_RAM)) { // 長さ0でアドレスチェックしない 1.2b42
        // 	return 2; // address error
        // }
        // if (lendata > 0 && (addata < 0 || addata + lendata > SIZE_RAM)) { // 長さ0でアドレスチェックしない 1.2b42
        // 	return 2; // address error
        // }
        // if (writemode == I2C_READ) {
        i2c(buf, lencmd, NULL, 0);

        // uint8_t rbuf[16] = {};
        buf[0] |= 0b1;
        // i2c(buf, 1, rbuf, 16);

        //put_num(lendata);
        // printf("%s", rbuf);
        return !i2c(buf, 1, addata, lendata);
        // } else {
        // 	// todo
        // }
    }
}

//

//#define I2C_RETRY 100
#define I2C_RETRY 1
// #define I2C_RETRY 10
int i2c(uint8_t wbf[], size_t nwb, uint8_t rbf[], size_t nrb) {
    extern volatile uint8_t* g_pI2CWriteBuffer;
    extern volatile uint8_t* g_pI2CReadBuffer;
    extern volatile uint32_t g_nI2CWriteLen;
    extern volatile uint32_t g_nI2CReadLen;
    
    g_pI2CWriteBuffer = wbf;
    g_pI2CReadBuffer = rbf;
    g_nI2CWriteLen = nwb;
    g_nI2CReadLen = nrb;
    
    for (int i = 0; i < I2C_RETRY; i++) {
        if (i2c_proc()) {
            return 1;
        }
    }
    return 0;
}

int io_ana(int n) {
    if (n < 0 || n > 11 || n == 5) {
        return 0;
    }
    if (n == 0) {
        n = 9;
    }
    n--;
    int portin = getPortIN();
    if (portin & (1 << n)) {
        return 0; // OUT mode
    }
    static struct {
        volatile uint32_t* ctl;
        uint8_t            p;
    } const gpio[] = {
        { &GPIO_CTL0(GPIOA), 1 }, // in1
        { &GPIO_CTL0(GPIOA), 4 }, // in2
        { &GPIO_CTL0(GPIOA), 5 }, // in3
        { &GPIO_CTL0(GPIOA), 6 }, // in4
        { NULL, 3 },              // in5 (out1) disable
        { &GPIO_CTL0(GPIOA), 7 }, // in6 (out2)
        { &GPIO_CTL0(GPIOB), 8 }, // in7 (out3)
        { &GPIO_CTL0(GPIOB), 9 }, // in8 (out4)
        { &GPIO_CTL0(GPIOA), 0 }, // in9 == in0 (btn)
        { &GPIO_CTL0(GPIOA), 2 }, // in10 (out5)
        { &GPIO_CTL0(GPIOA), 3 }, // in11 (out6)
    };
    /*
		return (GPIO_ISTAT(GPIOA) >> 8) & 1;      // out1 PA8 // in5
    } else if (n == 6) {
		return (GPIO_ISTAT(GPIOA) >> 7) & 1;      // out2 PA7 // in6
    } else if (n == 7) {
		return (GPIO_ISTAT(GPIOB) >> 0) & 1;      // out3 PB0 // in7
    } else if (n == 8) {
		return (GPIO_ISTAT(GPIOB) >> 1) & 1;      // out4 PB1 // in8
	} else if (n == 9) {
		return (GPIO_ISTAT(GPIOA) >> 0) & 1; // btn
    } else if (n == 10) {
		return (GPIO_ISTAT(GPIOA) >> 2) & 1;      // out5 PA2 // in10
    */
 
#ifdef PKG_DJ
	if(g_dj) return dj_ana(n);
#endif

    uint32_t org = *gpio[n].ctl;
    int p2 = gpio[n].p;
    if (p2 >= 8) {
        p2 -= 8;
    }
    *gpio[n].ctl = (*gpio[n].ctl & ~GPIO_MODE_MASK(p2)) | GPIO_MODE_SET(p2, 0b0000);
    _msleep(1);     // 元がpullupの場合、ANA値が大きくなる。analogモードにしてから電位が落ち着くまで待つ。

    ADC_RSQ2(ADC1) = gpio[n].p;
    ADC_CTL1(ADC1) |= ADC_CTL1_ADCON;

    while(!(ADC_STAT(ADC1) & ADC_FLAG_EOC));
    ADC_STAT(ADC1) &= ~ADC_FLAG_EOC;

    int r = ADC_RDATA(ADC1) & 0xffff;

    *gpio[n].ctl = org;
    return r;
}

int io_in(int n) {
#ifdef PKG_DJ
	if(g_dj) return dj_in(n);
#endif
	if (n >= 1 && n <= 4) {
		static const uint8_t shift[] = { 1, 4, 5, 6 };
        uint8_t pin = shift[n - 1];
        //GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(pin)) | GPIO_MODE_SET(pin, 0b1000);
    	//GPIO_OCTL(GPIOA) |= GPIO_PIN_1; // pull-up
		return (GPIO_ISTAT(GPIOA) >> pin) & 0b1;
    } else if (n == 5) {
		return (GPIO_ISTAT(GPIOA) >> 8) & 1;      // out1 PA8 // in5
    } else if (n == 6) {
		return (GPIO_ISTAT(GPIOA) >> 7) & 1;      // out2 PA7 // in6
    } else if (n == 7) {
		return (GPIO_ISTAT(GPIOB) >> 0) & 1;      // out3 PB0 // in7
    } else if (n == 8) {
		return (GPIO_ISTAT(GPIOB) >> 1) & 1;      // out4 PB1 // in8
	} else if (n == 9) {
		return (GPIO_ISTAT(GPIOA) >> 0) & 1; // btn
    } else if (n == 10) {
		return (GPIO_ISTAT(GPIOA) >> 2) & 1;      // out5 PA2 // in10
    } else if (n == 11) {
		return (GPIO_ISTAT(GPIOA) >> 3) & 1;      // out6 PA3 // in11
	}
	return 0;
}

void io_out(int n) {
    uint32_t a = 0;
    uint32_t b = 0;
    for (int port = 1; port <= 11; port++) {
        int bport = (1 << (port - 1));
        if (portout & bport) {
            if (n & bport) {
                if (port == 1) {
                    a |= 1 << 8; // out1 PA8
                } else if (port == 2) {
                    a |= 1 << 7; // out2 PA7
                } else if (port == 3) {
                    b |= 1 << 0; // out3 PB0
                } else if (port == 4) {
                    b |= 1 << 1; // out4 PB1
                } else if (port == 5) {
                    a |= 1 << 2; // out5 PA2
                } else if (port == 6) {
                    a |= 1 << 3; // out6 PA3
                } else if (port == 7) {
                    a |= 1 << 14; // btn PA14
                } else if (port == 8) {
                    a |= 1 << 1; // in1 PA1
                } else if (port == 9) {
                    a |= 1 << 4; // in2 PA4
                } else if (port == 10) {
                    a |= 1 << 5; // in3 PA5
                } else if (port == 11) {
                    a |= 1 << 6; // in4 PA6;
                }
            } else {
                if (port == 1) {
                    a |= 1 << (8 + 16); // out1 PA8
                } else if (port == 2) {
                    a |= 1 << (7 + 16); // out2 PA7
                } else if (port == 3) {
                    b |= 1 << (0 + 16); // out3 PB0
                } else if (port == 4) {
                    b |= 1 << (1 + 16); // out4 PB1
                } else if (port == 5) {
                    a |= 1 << (2 + 16); // out5 PA2
                } else if (port == 6) {
                    a |= 1 << (3 + 16); // out6 PA3
                } else if (port == 7) {
                    a |= 1 << (14 + 16); // btn PA14
                } else if (port == 8) {
                    a |= 1 << (1 + 16); // in1 PA1
                } else if (port == 9) {
                    a |= 1 << (4 + 16); // in2 PA4
                } else if (port == 10) {
                    a |= 1 << (5 + 16); // in3 PA5
                } else if (port == 11) {
                    a |= 1 << (6 + 16); // in4 PA6;
                }
            }
        }
    }
    GPIO_BOP(GPIOA) = a;
    GPIO_BOP(GPIOB) = b;

    /*
    d = GPIO_OCTL(GPIOA) & ~(GPIO_PIN_8 | GPIO_PIN_7 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_14);
    {
        d |= ((n>>0)&0b1) << 8;
        d |= ((n>>1)&0b1) << 7;
        d |= ((n>>4)&0b1) << 2;
        d |= ((n>>5)&0b1) << 3;
        d |= ((n>>6)&0b1) << 14;
    }
    GPIO_OCTL(GPIOA) = d;

    d = GPIO_OCTL(GPIOB) & ~(GPIO_PIN_0 | GPIO_PIN_1);
    {
        d |= ((n>>2)&0b1) << 0;
        d |= ((n>>3)&0b1) << 1;
    }
    GPIO_OCTL(GPIOB) = d;
    */

#ifdef PKG_DJ
	if(g_dj) dj_out(n);
#endif
}

void io_pwm(int n, uint32_t d) {
	if (n < 1 || n > 6) {
		return;
	}
    portout |= 1 << (n - 1);

    static struct {
        volatile uint32_t* ctl;
        uint8_t            p;
    } const gpio[] = {
        { &GPIO_CTL1(GPIOA), 8-8 },
        { &GPIO_CTL0(GPIOA), 7 },
        { &GPIO_CTL0(GPIOB), 0 },
        { &GPIO_CTL0(GPIOB), 1 },
        { &GPIO_CTL0(GPIOA), 2 },
        { &GPIO_CTL0(GPIOA), 3 }
    };
    
    static volatile uint32_t* const cv[] = {
        &TIMER_CH0CV(TIMER0),
        &TIMER_CH1CV(TIMER2),
        &TIMER_CH2CV(TIMER2),
        &TIMER_CH3CV(TIMER2),
        &TIMER_CH2CV(TIMER4),
        &TIMER_CH3CV(TIMER4)
    };

    int m = n - 1;

    *gpio[m].ctl = (*gpio[m].ctl & ~GPIO_MODE_MASK(gpio[m].p)) | GPIO_MODE_SET(gpio[m].p, d? 0b1011 : 0b0011);
    *cv[m] = d;

    if (m == 0) {
        if (d) {
            TIMER_CTL0(TIMER0) |= TIMER_CTL0_CEN;
        } else {
            TIMER_CTL0(TIMER0) &= ~TIMER_CTL0_CEN;
        }
    } else if (m == 1 || m == 2 || m == 3) {
        if (*cv[1] || *cv[2] || *cv[3]) {
            TIMER_CTL0(TIMER2) |= TIMER_CTL0_CEN;
        } else {
            TIMER_CTL0(TIMER2) &= ~TIMER_CTL0_CEN;
        }
    } else {
        if (*cv[4] || *cv[5]) {
            TIMER_CTL0(TIMER4) |= TIMER_CTL0_CEN;
        } else {
            TIMER_CTL0(TIMER4) &= ~TIMER_CTL0_CEN;
        }
    }

#ifdef PKG_DJ
	if(g_dj) dj_pwm(n, d);
#endif
}

void io_pwmt(uint32_t t) {
    TIMER_CAR(TIMER0) = t - 1;
    TIMER_CAR(TIMER2) = t - 1;
    TIMER_CAR(TIMER4) = t - 1;

#ifdef PKG_DJ
	if(g_dj) dj_pwmt(t);
#endif
}

void io_dac(int port, int a) { // port only IN2(OUT9), IN3(OUT10), a 0-0xfff(4095) -> 0-1023
    if (a < 0) {
        a = 0;
    } else if (a >= 1023) {
        a = 1023;
    }
    a <<= 2; // *4
    if (port == 9) {
        GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0000);
        DAC_CTL |= DAC_CTL_DEN0; // IN2/OUT9
        DAC0_R12DH = a & 0xfff;
        portout |= 1 << (9 - 1);
    } else if (port == 10) {
        GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b0000);
        DAC_CTL |= DAC_CTL_DEN1; // IN3/OUT10
        DAC1_R12DH = a & 0xfff;
        portout |= 1 << (10 - 1);
    }
}
void io_dac_off(int port) {
    if (port == 9) {
        DAC_CTL &= ~DAC_CTL_DEN0; // IN2/OUT9
        int bport = 1 << (9 - 1);
        portout &= ~bport;
        if (portpullup & bport) {
            GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b1000);          // in2 PA4
            GPIO_BOP(GPIOA) = GPIO_BOP_BOP4; // pull-up
        } else {
            GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(4)) | GPIO_MODE_SET(4, 0b0100);          // in2 PA4
        }
    } else if (port == 10) {
        DAC_CTL &= ~DAC_CTL_DEN1; // IN3/OUT10
        int bport = 1 << (10 - 1);
        portout &= ~bport;
        if (portpullup & bport) {
            GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b1000);          // in3 PA5
            GPIO_BOP(GPIOA) = GPIO_BOP_BOP5; // pull-up
        } else {
            GPIO_CTL0(GPIOA) = (GPIO_CTL0(GPIOA) & ~GPIO_MODE_MASK(5)) | GPIO_MODE_SET(5, 0b0100);          // in3 PA5
        }
    }
}
void IJB_dac(int port, int val) {
    io_dac(port, val);
}
