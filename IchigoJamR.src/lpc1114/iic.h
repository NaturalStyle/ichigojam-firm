#ifndef I2C_DEFINED
#define I2C_DEFINED

//#define I2C_DEBUG

#if defined(I2C_DEBUG) && defined(DEBUG_PRINT)
#define I2C_Log(...)	xprintf(__VA_ARGS__);
#else
#define I2C_Log(...)
#endif

#include "LPC11xx.h"
#include <stdint.h>

/* I2C*/
#define	I2C0CONSET		(*(volatile uint32_t*)0x40000000)
#define	I2C0STAT		(*(volatile uint32_t*)0x40000004)
#define	I2C0DAT			(*(volatile uint32_t*)0x40000008)
#define	I2C0ADR0		(*(volatile uint32_t*)0x4000000C)
#define	I2C0SCLH		(*(volatile uint32_t*)0x40000010)
#define	I2C0SCLL		(*(volatile uint32_t*)0x40000014)
#define	I2C0CONCLR		(*(volatile uint32_t*)0x40000018)
#define	I2C0MMCTRL		(*(volatile uint32_t*)0x4000001C)
#define	I2C0ADR			( (volatile uint32_t*)0x4000001C)
#define	I2C0ADR1		(*(volatile uint32_t*)0x40000020)
#define	I2C0ADR2		(*(volatile uint32_t*)0x40000024)
#define	I2C0ADR3		(*(volatile uint32_t*)0x40000028)
#define	I2C0DATA_BUFFER	(*(volatile uint32_t*)0x4000002C)
#define	I2C0MASK		( (volatile uint32_t*)0x40000030)
#define	I2C0MASK0		(*(volatile uint32_t*)0x40000030)
#define	I2C0MASK1		(*(volatile uint32_t*)0x40000034)
#define	I2C0MASK2		(*(volatile uint32_t*)0x40000038)
#define	I2C0MASK3		(*(volatile uint32_t*)0x4000003C)
#define IOCON_PIO0_4	(*(volatile uint32_t*)0x40044030)
#define IOCON_PIO0_5	(*(volatile uint32_t*)0x40044034)
#define GPIO0DATA		(*(volatile uint32_t*)0x50003FFC)
#define GPIO0DIR		(*(volatile uint32_t*)0x50008000)
#define	PCI2C			5
#define PRESETCTRL		(*(volatile uint32_t*)0x40048004)
#define ISER			(*(volatile uint32_t*)0xE000E100)
#define	I2C_IRQn		15
//#define SYSAHBCLKCTRL	(*(volatile uint32_t*)0x40048080)

//#define	__set_SYSAHBCLKCTRL(p,v)	SYSAHBCLKCTRL = (SYSAHBCLKCTRL & ~(1 << (p))) | ((v) << (p))
#define __enable_irqn(n) ISER = 1 << (n)

// from LPC1100.h

// Data directiron flag (I2CCTRL.dir)
#define I2C_WRITE		0
#define I2C_READ		1
#define I2C_READ_ONLY	2

// I2C Control Set Register
#define I2CONSET_I2EN		0x00000040
#define I2CONSET_AA			0x00000004
#define I2CONSET_SI			0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

// I2C Control clear Register
#define I2CONCLR_AAC		0x00000004
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040


/* I2C transaction work area */
typedef struct { // 20byte -> 16byte (-4byte)
	//uint8_t enable; // そもそも使ってない
	//uint8_t stat;			/* I2C transaction status */ // _g.i2c_stat に移した 1.4b10
	uint8_t dir;			/* Read or write for secondary buffer I2C_WRITE/I2C_READ/I2C_READ_ONLY */
	uint8_t sla;			/* Slave address (0..127) */
	uint8_t ncmd;			/* Number of bytes of cmd */ // [] (1..4)
	uint8_t icmd;			/* Index of command byte */
//	uint8_t retry;			/* Number of retries of SLA+W (0..) when not ACKed */
	uint16_t retry;			/* Number of retries of SLA+W (0..) when not ACKed */
	uint16_t ndata;			/* Number of bytes of secondary data (0:no secondary data transfer) */
	uint8_t* cmd;			/* Command bytes follows SLA+W */
	uint8_t* data;			/* Pointer to secondary data buffer for read/write */
//	void (*eotfunc)(int);	/* Call-back function to notify end of transaction (0:not used) */
} I2CCTRL;


/* Status code (I2CCTRL.stat) */
enum {
	I2C_BUSY = 0,	/* An I2C transaction is in progress. Must not discard this structure */
	I2C_SUCCEEDED,	/* Transaction succeeded */
	I2C_TIMEOUT,	/* Failed due to slave not responded to addressing */
	I2C_ABORTED,	/* Failed due to slave not responded to sent data */
	I2C_ERROR		/* Failed due to bus error, arbitration failed or unknown error */
};


/* I2C control module API */
int i2c0_init(void);				/* Initialize I2C module  0:ok 1:err */
//int i2c0_start(volatile I2CCTRL*);	/* Start an I2C transaction */
int i2c0_start(void);	/* Start an I2C transaction */
void i2c0_abort(void);				/* Abort an I2C transaction in progress */

// from iic.c

/*------------------------------------------------------------------------/
/  LPC1100 I2C master control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

//#include "iic.h"

#define	SYSCLK		(12000000 * 4)	// 48MHz fixed
#define PCLK_I2C	SYSCLK			// PCLK freqency supplied to I2C module [Hz]

//Bus freqency [Hz]
//#define I2C0_RATE	400000 // 400kHz
//#define I2C0_RATE	100000 // 100kHz 1.2b39 for Wii Nunchuck, sakura.io LoRa , b56
//uint16_t i2cbps;// = 400; // default 400kHz 1.2b56 変更可能に
static inline void i2c0_bps(int bps) {
	/*
	if (!bps) {
		bps = 400;
	}
	*/
	_g.i2cbps = bps;
}

volatile I2CCTRL I2cCtrl;		/* Control structure for I2C transaction */ // stackに置いたらだめ？I2CBUFとか

/*static*/ // volatile I2CCTRL *Ctrl0;	/* Current I2C control structure */ // flgにすると軽くなる // 1.4b10
//uint8_t i2c_alive = 0; // flgにしたら容量増えてしまった

//#define I2C_RETRY_COM 100 // 1.2b39 by taisukef
//#define I2C_RETRY_COM 1200 // 1.2b41 400Hz -> 100Hz,  300 * 4
//#define I2C_RETRY_COM 300 // 1.2まで retryを16bit化必要 -> 1.2b45 -> 400kHzに戻した

// from 1.3b1
#define I2C_RETRY_COM 120000 // I2C_RETRY_COM / i2cbps
#define I2C_RETRY_COM_MAX 4800 // 適当
#define I2C_RETRY_INIT 300 // 1.2b42

/*
// test for 1.3b2
#define I2C_RETRY_COM 0
#define I2C_RETRY_COM_MAX 0
#define I2C_RETRY_INIT 0
*/

/*------------------------*/
/* Initialize I2C Module  */
/*------------------------*/

int i2c0_init(void) { // 0:ok 1:err
	I2C_Log("i2c0_init()\n")
	I2C_Log("IOCON_PIO0_5: %d\n", IOCON_PIO0_5)
	if (IOCON_PIO0_5 == 0) { // 接続チェックをスキップ
		I2C_Log("BUS RESET\n")
		IOCON_PIO0_4 = 0x00;	/* Detach I2C module from I/O pad */
		IOCON_PIO0_5 = 0x00;
	
		/* Bus Reset: Put I2C slaves to idle state */
		GPIO0DIR |= (1 << 4);
		GPIO0DIR &= ~(1 << 5);
		int err = 0;
		for (int n = 0; n < 10; n++) {
			GPIO0DATA |= (1 << 4);	/* SCL=H */
			for (int m = 0; m < 100; m++) {
				GPIO0DATA;
			}
			GPIO0DATA &= ~(1 << 4);	/* SCL=L*/
			for (int m = 0; m < 100; m++) {
				GPIO0DATA;
			}
			if (!(GPIO0DATA & (1 << 5))) {
				n = 0;	/* Check if SDA is high for 10 clock time */
				err++;
				if (err > I2C_RETRY_INIT) {
					I2C_Log("i2c0_init() error\n")
					return 1; // err
				}
			}
		}
		
//		__set_SYSAHBCLKCTRL(PCI2C, 1);	/* Enable I2C module */
		LPC_SYSCON->SYSAHBCLKCTRL |= 1 << PCI2C; // enable i2c module
	
		PRESETCTRL &= ~(1 << 1);	/* Reset I2C module */
		PRESETCTRL |=  (1 << 1);
	
		IOCON_PIO0_4 = 0x01; // I2C
		IOCON_PIO0_5 = 0x01; // I2C
	}
	I2C0CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;    // Deactivate I2C
	
	/* Set master bus speed */
	// 本当は動作周波数に合わせて変更する必要あり
	//	I2C0SCLH = I2C0SCLL = PCLK_I2C / 2 / I2C0_RATE;
	if (!_g.i2cbps)
		_g.i2cbps = 400;
//	I2C0SCLH = I2C0SCLL = (uint32_t)(PCLK_I2C / 1000 / 2) / _g.i2cbps; // 最小4 ... 最大6000kHz
	I2C0SCLH = I2C0SCLL = calcDivU(PCLK_I2C / 1000 / 2, _g.i2cbps); // 最小4 ... 最大6000kHz
	
	__enable_irqn(I2C_IRQn);
	
	I2C0CONSET = I2CONSET_I2EN;
	I2C_Log("i2c0_init() end\n")
	return 0;
}



/*--------------------------*/
/* Start an I2C Transaction */
/*--------------------------*/

//int i2c0_start(volatile I2CCTRL *ctrl) {	/* Pointer to the initialized I2C control structure */
int i2c0_start() {	/* Pointer to the initialized I2C control structure */
	//I2cCtrl.retry = (uint32_t)I2C_RETRY_COM / _g.i2cbps;
	I2cCtrl.retry = calcDivU(I2C_RETRY_COM, _g.i2cbps);
	if (I2cCtrl.retry > I2C_RETRY_COM_MAX) {
		I2cCtrl.retry = I2C_RETRY_COM_MAX;
	}
	
//	if (Ctrl0)
//		return 0;	/* Reject if an I2C transaction is in progress */

	I2C_Log("i2c0_start\n")
	//I2cCtrl.stat = I2C_BUSY;	/* An I2C transaction is in progress */
	_g.i2c_stat = I2C_BUSY;

	if (I2cCtrl.dir == I2C_READ && !I2cCtrl.ncmd) {
		I2cCtrl.dir = I2C_READ_ONLY;
	}

	//Ctrl0 = &I2cCtrl;			/* Register the I2C control strucrure as current transaction */
	//I2cCtrl.enable = 1;
//	i2c_alive = 1;

	// copy from init
	I2C0CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;    // Deactivate I2C
	I2C0CONSET = I2CONSET_I2EN;
	
	I2C0CONSET = I2CONSET_STA; // STA to generate start condition

	I2C_Log("i2c0_start end\n")
	return 1;
}



/*--------------------------*/
/* Abort I2C Transaction    */
/*--------------------------*/

void i2c0_abort(void) {
	I2C_Log("i2c0_abort\n")
	I2C0CONCLR = 0x6C;	/* Deactivate I2C */

	//Ctrl0 = 0;			/* Discard I2C control structure */
	//I2cCtrl.enable = 0;
	//i2c_alive = 0;
}


/*-------------------------*/
/* I2C0 Background Process */
/*-------------------------*/

#ifdef I2C_DEBUG
void xprintf(const char* fmt, ...);
#endif

void I2C_IRQHandler(void) {
	volatile I2CCTRL* i2c = &I2cCtrl; // Ctrl0;
	//volatile I2CCTRL* i2c = Ctrl0;
	if (!i2c) {	/* Spurious Interrupt */
//	if (!i2c_alive) {
		I2C0CONCLR = 0x6C;	/* Disable I2C */
		return;
	}
	uint8_t res = 0;

	I2C_Log("stat:%2x d:%d io:%d cmd:%d", I2C0STAT & 0xff, I2C0DAT, i2c->dir, i2c->ncmd)
	// wii nunchuck
	// init write: 8 18 28 28 commoand 2byte, data 0byte
	// read : 8 40 50? command 0byte, data 6byte
	//    8 18 10 40 50 50 50 ?
	
	// sakura
	// write: 1, 3, 5, 5, 5, 5, 5, 5, 0 command 2byte + data 4byte
	// write: 1, 3, 5, 5, ..... 0, command 0byte + data 4 byte
	// -> 1, 3, 2, 8, 10, 10, 10, 11 
	// 2017.1.26 -  8 20 8 20 8 2 8 20 8 20 8 18 という番号
	
	// sakura IoT b: 8, 18, 28, 28, 28,10,8,40,50,50,50,50
	// read 8,18,28,28,...,28,10,8,40,50,50,50,58 - 最新
	// sakura new board: 8 20 8 20 8 20 ... 見つからない？
	
	// eeprom
	// load: 8 18 28 28 10 8 40 50 50 ... 58
	// save: 8 18 28 28 ... 8 20(retry) 30 ??
	
	// HT16K33
	// write 1byte: 8 20 8 20... 見つからない時の挙動
	
	// IS31FL3730
	// write: 8 18 8 18 10 38
	
	// Sense HAT #46 joystick
	// read: 8 18 28 10 40 58, 8がない
	
	// AM2320 #5C
	// write command: 8 20 8 18 28 28 28
	// read data: 8 40 50 50 50 50 50 50 50 58
	
	switch (I2C0STAT & 0xff) {
	case 0x08:	// START condition has been generated -> 0x18(write), 0x20(err), 0x40(read)
		if (i2c->dir != I2C_WRITE && !i2c->ncmd) { // wii nuncheck ok, sakura ok!
			I2C0DAT = (i2c->sla << 1) | 1;	// Transmit SLA+R -> 0x40
			I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI */ // b41
		} else {
			I2C0DAT = i2c->sla << 1;	// Transmit an SLA+W -> 0x18
			I2C0CONCLR = I2CONCLR_SIC | I2CONCLR_STAC;	/* Clear STA+SI */ // ok sakura, microdothat
//			I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI */ // NG
		}
		break;

	case 0x20: {/* SLA+W has been transmitted, ACK not received */
		uint16_t rct = i2c->retry;
		if (rct) {
			I2C0CONSET = I2CONSET_STO | I2CONSET_STA;	// Generate STOP + START condition // 1.2b41 for EEPROM
			i2c->retry = rct - 1;
		} else {
			I2C0CONSET = I2CONSET_STO;	/* Generate STOP condition */
			res = I2C_TIMEOUT;	/* Retry count over */
		}
		I2C0CONCLR = I2CONCLR_SIC;	// Clear SI
		break;
	}
	case 0x18:	// SLA+W has been transmitted, ACK received -> 0x28
		if (i2c->ncmd) {
			I2C0DAT = i2c->cmd[0];	/* Transmit 1st command byte */
			i2c->icmd = 1;
			i2c->ncmd--;
			I2C0CONCLR = I2CONCLR_SIC;	// STAもいる??
			break;
		}
		// else throw to the 0x028
	case 0x28: {	// DATA has been transmitted, ACK received -> 0x28, 0x10, 0x30(slave end), 0x10(to recv)
		uint16_t n = i2c->ncmd;
		if (n) {	/* There is any command byte to be transmitted */
			i2c->ncmd--;
			I2C0DAT = i2c->cmd[i2c->icmd++];	/* Set a command byte to be sent */
		} else {	/* All command byte have been transmitted */
			n = i2c->ndata;
			if (i2c->dir == I2C_WRITE) {
				if (n) {	/* There is any data to write */
					i2c->ndata--;
					I2C0DAT = *i2c->data++;	/* Set a data byte to be sent */
				} else {	/* No data to write */
					I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
					res = I2C_SUCCEEDED;	/* Succeeded */
				}
			} else { // I2C_READ or I2C_READ_ONLY
				if (n) {	/* There is any data to read */
					I2C0CONSET = I2CONSET_STA;		/* Generate REPEATED START condition */
				} else {	/* No data to receive */
					I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
					res = I2C_SUCCEEDED;	/* Succeeded */
				}
			}
		}
		I2C0CONCLR = I2CONCLR_SIC;	// clear SI
		break;
	}
	case 0x10:	// REPEATED START condition has been generated 0x40(read), 0x48, 0x38(err)
		I2C0DAT = (i2c->sla << 1) | 1;	/* Transmit SLA+R */
		if (i2c->dir != I2C_READ_ONLY) {
			I2C0CONSET = I2CONSET_STO;		// Generate STOP condition - さくらに必要、nunchuckに必要、sense hatはコマンド分離
		}
		//		I2C0CONCLR = I2CONCLR_SIC | I2CONCLR_STAC;	/* Clear STA+SI */
		I2C0CONCLR = I2CONCLR_SIC; /* Clear SI */ // b41 STOをつけないとさくらで使えない？
		break;

	case 0x21:	/* DATA has been transmitted, ACK not received */
		I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
		I2C0CONCLR = I2CONCLR_SIC;		/* Clear SI */
		res = I2C_ABORTED;	/* Aborted by slave */
		break;


	case 0x40:	// SLA+R has been transmitted, ACK received -> 0x50(remain data), 0x58(last data)
		if (i2c->ndata > 1) {	/* Two or more bytes left */
			I2C0CONSET = I2CONSET_AA;	/* Set AA */
		} else if (i2c->ndata == 1) {
			I2C0CONCLR = I2CONCLR_SIC;	// clear SI
		} else {
			I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
			I2C0CONCLR = I2CONCLR_SIC;		/* Clear SI */
			res = I2C_SUCCEEDED;	// Succeeded
		}
		if (i2c->dir == I2C_WRITE) {
			I2C0CONCLR = I2CONCLR_SIC;	// clear SI
		} else {
			I2C0CONCLR = I2CONCLR_SIC | I2CONCLR_STAC;	// clear SI+STA
		}
		break;

	case 0x48:	/* SLA+R has been transmitted, ACK not received */
		I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
		I2C0CONCLR = I2CONCLR_SIC;		/* Clear SI */
		res = I2C_ABORTED;	/* Aborted by slave */
		break;

	case 0x50:	// Data has been received, ACK sent
		*i2c->data++ = I2C0DAT;	/* Get received DATA */
		i2c->ndata--;
		if (i2c->ndata > 2) {	/* Two or more bytes left */
			I2C0CONSET = I2CONSET_AA;	/* Set AA */
			I2C0CONCLR = I2CONCLR_SIC;	// clear SI
		} else if (i2c->ndata == 2) { // two bytes left
			I2C0CONCLR = I2CONCLR_SIC;	// clear SI -> 0x58
		} else {
			I2C0CONCLR = I2CONCLR_SIC | I2CONCLR_AAC;	// clear SI+AA -> 0x58
		}
		break;

	case 0x58:	/* DATA has been received, ACK not sent */
		*i2c->data = I2C0DAT;
		I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
		I2C0CONCLR = I2CONCLR_SIC;		/* Clear SI */
		res = I2C_SUCCEEDED;	/* Succeeded */
		break;
	
	case 0x30: // DATA has been recieved, ACK not sent スレーブから終了ときた場合
		I2C0CONSET = I2CONSET_STO;		/* Generate STOP condition */
		I2C0CONCLR = I2CONCLR_SIC;		/* Clear SI */
		res = I2C_SUCCEEDED;	/* Succeeded */
		break;
	
	case 0x00:	/* Bus error */
	case 0x38:	/* Arbitration lost */
	default:	/* Unknown status */
		I2C0CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;    // Deactivate I2C
		res = I2C_ERROR;	/* Unknown error */
	}
	I2C_Log(" res:%d\n", res)
	if (res) {	/* End of I2C transaction? */
		//Ctrl0 = 0;			/* Release I2C control structure */
		//I2cCtrl.enable = 0;
		//i2c_alive = 0;
		//i2c->stat = res;	/* Set result */
		_g.i2c_stat = res;
		
//		if (i2c->eotfunc) i2c->eotfunc(res);	/* Notify EOT if call-back function is specified */
	}
}

#endif
