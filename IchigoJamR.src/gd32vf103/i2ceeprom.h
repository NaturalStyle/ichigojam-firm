#ifndef __I2C_EEPROM_H__
#define __I2C_EEPROM_H__


//uint8 cmdaddress[2]; // 1.3b2 削除 -> I2Cバッファ使用、でも容量20byte増 // +8byte

// writemode : I2C_WRITE, I2C_READ, ret: 1 ok, 0 err
/*INLINE*/ int accessEEPROM(int writemode, int address, uint8_t* data, int len) { // uint16 -> int 1.2b43
	int sla = 0b1010000;
//	if (address >= 64 * 1024)
	if (address >> 16) // 縮まる
		sla |= 0b100;
	uint8_t buf[1024 + 3];
	buf[0] = sla << 1;
	buf[1] = address >> 8;
	buf[2] = address;
	if (writemode) {
		if (len > 1024) {
			len = 1024; // limit 1024
		}
		for (int i = 0; i < len; i++) {
			buf[3 + i] = data[i];
		}
		return i2c(buf, 3 + len, NULL, 0);
	} else {
		if (!i2c(buf, 3, NULL, 0)) {
			return 0; // err
		}
		buf[0] |= 1;
		return i2c(buf, 1, data, len);
	}
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