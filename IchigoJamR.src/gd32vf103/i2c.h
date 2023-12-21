//* 
//* created by yrm (c) 2016
//* 
#include "gd32vf103.h"

//#define MAX_TIMEOUT         0x004FFFFF // 長過ぎる
#define MAX_TIMEOUT         0x000DFFFF

#define I2C_IDLE              0
#define I2C_BUSY              1
#define I2C_AERR              2
#define I2C_SMBALT            3
#define I2C_SMBTO             4
#define I2C_OUERR             5
#define I2C_LOSTARB           6
#define I2C_BERR              7
#define I2C_PECERR            8
#define I2C_TIME_OUT          9
#define I2C_OK                10

#define I2C_SPEED           400000  // 400kHz
#define SYS_CK_APB1         48000000
#define I2C_FREQ            (SYS_CK_APB1 / 1000000)

static volatile uint32_t state = I2C_IDLE;
static volatile uint32_t timeout = 0;

static volatile uint32_t iRead = 0;
static volatile uint32_t iWrite = 0;

volatile uint8_t* g_pI2CWriteBuffer = NULL;
volatile uint8_t* g_pI2CReadBuffer = NULL;
volatile uint32_t g_nI2CWriteLen = 0;
volatile uint32_t g_nI2CReadLen = 0;



void I2C0_EV_IRQHandler(void)
{
    uint32_t stat0 = I2C_STAT0(I2C0);
    if( stat0 & I2C_STAT0_SBSEND ){
        /* send slave address */
		I2C_DATA(I2C0) = g_pI2CWriteBuffer[iWrite++];

    }else
    if( stat0 & I2C_STAT0_ADDSEND ){
		uint16_t n = g_nI2CReadLen - iRead;
        if( (n == 1) || (n == 2) ){
            /* clear the ACKEN before the ADDSEND is cleared */
			I2C_CTL0(I2C0) &= ~I2C_CTL0_ACKEN;
		}

        /*clear ADDSEND bit */
		volatile uint32_t temp;
        temp = I2C_STAT0(I2C0);
        temp = I2C_STAT1(I2C0);
		(void)temp;

    }else
    if( (stat0 & I2C_STAT0_TBE) && (I2C_CTL1(I2C0) & I2C_CTL1_BUFIE) ){
        if( iWrite < g_nI2CWriteLen ){
            /* the master sends a data byte */
            I2C_DATA(I2C0) = g_pI2CWriteBuffer[iWrite++];
        }else{
            uint32_t tout = 1000;
			/* wait until the last byte is sent */
            while( --tout && !( I2C_STAT0(I2C0) & I2C_STAT0_BTC ) );

            /* the master sends a stop condition to I2C bus */
            I2C_CTL0(I2C0) |= I2C_CTL0_STOP;

			state = tout? I2C_OK: I2C_TIME_OUT;

            /* disable the I2C0 interrupt */
			I2C_CTL1(I2C0) &= ~I2C_CTL1_ERRIE;
		 	I2C_CTL1(I2C0) &= ~I2C_CTL1_EVIE;
			I2C_CTL1(I2C0) &= ~I2C_CTL1_BUFIE;
        }

    }else
    if( (stat0 & I2C_STAT0_RBNE) && (I2C_CTL1(I2C0) & I2C_CTL1_BUFIE) ){
        uint32_t tout = 1000;
        if( iRead < g_nI2CReadLen ){
			uint16_t n = g_nI2CReadLen - iRead;
			if( n == 3 ){
                /* wait until the second last data byte is received into the shift register */
                while( --tout && !( I2C_STAT0(I2C0) & I2C_STAT0_BTC ) );

                /* send a NACK for the last data byte */
                I2C_CTL0(I2C0) &= ~I2C_CTL0_ACKEN;

                /* read a data byte from I2C_DATA*/
                if(tout){
                    g_pI2CReadBuffer[iRead++] = I2C_DATA(I2C0);
                }else{
                    iRead += 1;
                }
            }else
			if( n == 2 ){
                /* wait until the second last data byte is received into the shift register */
                while( --tout && !( I2C_STAT0(I2C0) & I2C_STAT0_BTC ) );

                if(tout) {
                    /* read a data byte from I2C_DATA*/
                    g_pI2CReadBuffer[iRead++] = I2C_DATA(I2C0);
                    /* read a data byte from I2C_DATA*/
                    g_pI2CReadBuffer[iRead++] = I2C_DATA(I2C0);
                }else{
                    iRead += 2;
                }
            }else{
                /* read a data byte from I2C_DATA*/
                g_pI2CReadBuffer[iRead++] = I2C_DATA(I2C0);
            }
        }

        if( iRead == g_nI2CReadLen ){
            /* the master sends a stop condition to I2C bus */
            I2C_CTL0(I2C0) |= I2C_CTL0_STOP;

			state = tout? I2C_OK: I2C_TIME_OUT;

            /* disable the I2C0 interrupt */
            I2C_CTL1(I2C0) &= ~I2C_CTL1_ERRIE;
            I2C_CTL1(I2C0) &= ~I2C_CTL1_EVIE;
            I2C_CTL1(I2C0) &= ~I2C_CTL1_BUFIE;
        }
    }
}

void I2C0_ER_IRQHandler(void)
{
    uint32_t stat0 = I2C_STAT0(I2C0);
    /* no acknowledge received */
	if( stat0 & I2C_STAT0_AERR ){
		state = I2C_AERR;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_AERR;
    }

    /* SMBus alert */
	if( stat0 & I2C_STAT0_SMBALT ){
		state = I2C_SMBALT;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_SMBALT;
    }

    /* bus timeout in SMBus mode */
	if( stat0 & I2C_STAT0_SMBTO ){
		state = I2C_SMBTO;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_SMBTO;
    }

    /* over-run or under-run when SCL stretch is disabled */
	if( stat0 & I2C_STAT0_OUERR ){
		state = I2C_OUERR;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_OUERR;
    }

    /* arbitration lost */
	if( stat0 & I2C_STAT0_LOSTARB ){
		state = I2C_LOSTARB;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_LOSTARB;
    }

    /* bus error */
	if( stat0 & I2C_STAT0_BERR ){
		state = I2C_BERR;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_BERR;
    }

    /* CRC value doesn't match */
	if( stat0 & I2C_STAT0_PECERR ){
		state = I2C_PECERR;
        I2C_STAT0(I2C0) &= ~I2C_STAT0_PECERR;
    }

    /* disable the error interrupt */
	I2C_CTL1(I2C0) &= ~I2C_CTL1_ERRIE;
	I2C_CTL1(I2C0) &= ~I2C_CTL1_EVIE;
	I2C_CTL1(I2C0) &= ~I2C_CTL1_BUFIE;

    i2c_init();
}

void i2c_init()
{
    rcu_periph_clock_enable(RCU_I2C0);
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(6)) | GPIO_MODE_SET(6, 0b1111);
    GPIO_CTL0(GPIOB) = (GPIO_CTL0(GPIOB) & ~GPIO_MODE_MASK(7)) | GPIO_MODE_SET(7, 0b1111);

     // reset I2C0
    I2C_CTL0(I2C0) |= I2C_CTL0_SRESET;
    _usleep(10);
    I2C_CTL0(I2C0) &= ~I2C_CTL0_SRESET;

    /* I2C clock configure */
    // i2c_clock_config(I2C0, 400000, I2C_DTCY_2);
	{
		I2C_CTL1(I2C0) &= ~I2C_CTL1_I2CCLK;
		I2C_CTL1(I2C0) |= I2C_FREQ;

		I2C_RT(I2C0) &= ~I2C_RT_RISETIME;
		I2C_RT(I2C0) |= (((I2C_FREQ * 300) / 1000) + 1);  // the maximum SCL rise time is 300ns in fast mode

		I2C_CKCFG(I2C0) &= ~I2C_CKCFG_CLKC;
		I2C_CKCFG(I2C0) |= SYS_CK_APB1 / (I2C_SPEED * 3);

		I2C_CKCFG(I2C0) |= I2C_CKCFG_FAST;
	}

    /* enable I2C0 */
    I2C_CTL0(I2C0) |= I2C_CTL0_I2CEN;

//    eclic_irq_enable(I2C0_EV_IRQn, 5, 0);
//    eclic_irq_enable(I2C0_ER_IRQn, 6, 0);
    eclic_irq_enable(I2C0_EV_IRQn, 1, 1);
    eclic_irq_enable(I2C0_ER_IRQn, 2, 1);
}



int i2c_proc() // 1: ok, 0: err
{
    iRead = 0;
    iWrite = 0;

    i2c_init();

    /* enable the I2C0 interrupt */
	I2C_CTL1(I2C0) |= I2C_CTL1_ERRIE;
	I2C_CTL1(I2C0) |= I2C_CTL1_EVIE;
	I2C_CTL1(I2C0) |= I2C_CTL1_BUFIE;

    /* enable acknowledge */
    I2C_CTL0(I2C0) |= I2C_CTL0_ACKEN;

    if(g_nI2CReadLen == 2){
        /* send ACK for the next byte */
		I2C_CTL0(I2C0) |= I2C_CTL0_POAP;
    }else{
        I2C_CTL0(I2C0) &= ~I2C_CTL0_POAP;
    }

	// while( I2C_STAT1(I2C0) & I2C_STAT1_I2CBSY ); 

    state = I2C_BUSY;
    timeout = 0;

    I2C_CTL0(I2C0) |= I2C_CTL0_START;

    while (state == I2C_BUSY) {
		if (timeout >= MAX_TIMEOUT) {
			state = I2C_TIME_OUT;
			break;
		}
		++timeout;
    }

    return state == I2C_OK;
}


