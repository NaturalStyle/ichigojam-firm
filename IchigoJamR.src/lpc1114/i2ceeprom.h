#ifndef __I2C_EEPROM_H__
#define __I2C_EEPROM_H__

#include "keyboard.h"
#include "iic.h"

// i2c eeprom

#if defined(I2C_DEBUG) && defined(DEBUG_PRINT)
void i2c_eot(int stat) { /* I2C transaction status */
	xprintf("i2c_eot %d\n", stat);
	if (stat == I2C_SUCCEEDED) {
		// I2cCtrl.ndata は使えない
		xprintf("read %x %x %x %x\n", I2cBuff[0], I2cBuff[1], I2cBuff[2], I2cBuff[3]);
	}
}
#endif

//uint8 cmdaddress[2]; // 1.3b2 削除 -> I2Cバッファ使用、でも容量20byte増 // +8byte

// writemode : I2C_WRITE, I2C_READ
/*INLINE*/ boolean accessEEPROM(int writemode, int address, uint8_t* data, int len) { // uint16 -> int 1.2b43
//	I2CCTRL I2cCtrl; // ここにおいても動かない alignの関係？？
	I2cCtrl.sla = 0b1010000;
//	if (address >= 64 * 1024)
	if (address >> 16) // 縮まる
		I2cCtrl.sla |= 0b100;
	I2cCtrl.ncmd = 2;
	uint8_t* cmdaddress = ram + OFFSET_RAM_I2CBUF; // 増える +8byte -ram4byte
//	uint8_t* cmdaddress = _g.cmdaddress; // 増える
	cmdaddress[0] = address >> 8;
	cmdaddress[1] = address;
	I2cCtrl.cmd = cmdaddress; // big endian
	I2cCtrl.dir = writemode;
	I2cCtrl.ndata = len;
	I2cCtrl.data = (uint8_t*)data;
//	I2cCtrl.eotfunc = 0; //i2c_eot;
//	xprintf("read start\n");
	//i2c0_start(&I2cCtrl);
	i2c0_start();
//	while (!I2cCtrl.stat) {
	while (!_g.i2c_stat) {
		if (stopExecute()) {// あると動かない？？
			i2c0_abort(); // 1.2b56
			return 0;
		}
	}
//	xprintf("read end %d\n", I2cCtrl.stat);
	//return I2cCtrl.stat == I2C_SUCCEEDED;
	return _g.i2c_stat == I2C_SUCCEEDED;
}

/*

// 一緒にすると縮む
INLINE boolean saveEEPROM(int address, uint8_t* data, int len) {
//	I2CCTRL I2cCtrl; // ここにおいても動かない alignの関係？？
	I2cCtrl.sla = 0b1010000;
	if (address >= 64 * 1024)
		I2cCtrl.sla |= 0b100;
	I2cCtrl.retry = I2C_RETRY;
	I2cCtrl.ncmd = 2;
	I2cCtrl.cmd = cmdad;
	cmdad[0] = (address >> 8) & 0xff;
	cmdad[1] = address & 0xff;
	I2cCtrl.dir = I2C_WRITE;
	I2cCtrl.ndata = len;
	I2cCtrl.data = (uint8_t*)data;
//	I2cCtrl.eotfunc = 0; //i2c_eot;
	i2c0_start(&I2cCtrl);
	while (!I2cCtrl.stat) {
//		if (stopExecute()) // あると動かない？？
//			return 0;
	}
	return I2cCtrl.stat == I2C_SUCCEEDED;
}
INLINE boolean loadEEPROM(int address, uint8_t* data, int len) {
//	I2CCTRL I2cCtrl; // ここにおいても動かない
	I2cCtrl.sla = 0b1010000; // 0x50
	if (address >= 64 * 1024)
		I2cCtrl.sla |= 0b100;
	I2cCtrl.retry = I2C_RETRY;
	I2cCtrl.ncmd = 2;
	I2cCtrl.cmd = cmdad;
	cmdad[0] = (address >> 8) & 0xff;
	cmdad[1] = address & 0xff;
	I2cCtrl.dir = I2C_READ;
	I2cCtrl.ndata = len; // 読み込むデータ量
	I2cCtrl.data = (uint8_t*)data;
//	I2cCtrl.eotfunc = 0; //i2c_eot;
	i2c0_start(&I2cCtrl);
	while (!I2cCtrl.stat) {
		if (stopExecute()) { // あると動かない？？
			return 0;
		}
	}
	return I2cCtrl.stat == I2C_SUCCEEDED;
}
*/

// __I2C_EEPROM_H__
#endif