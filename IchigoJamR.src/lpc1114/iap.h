#ifndef _IAP
#define _IAP

#include "LPC11xx.h"

// LPC1114 dip 28 
// Flash：32KB
// RAM：4KB
// num of sectors 8 (num0-7)
// sector size 4k

enum command_code {
	IAPCommand_Prepare_sector_for_write_operation = 50,
	IAPCommand_Copy_RAM_to_Flash,
	IAPCommand_Erase_sector,
	IAPCommand_Blank_check_sector,
	IAPCommand_Read_part_ID,
	IAPCommand_Read_Boot_Code_version,
	IAPCommand_Compare,
	IAPCommand_Reinvoke_ISP,
	IAPCommand_Read_UID,
	IAPCommand_Erase_page,
	IAPCommand_EEPROM_Write = 61,
	IAPCommand_EEPROM_Read,
};

#define FLASH_SECTOR_7 0x00007000
#define FLASH_SECTOR_SIZE_0_TO_15 (4 * 1024)

#define SAVED_FLASH ((char*)FLASH_SECTOR_7)

/* // #define 移行で、60byte減った！
int blankCheckFlash(int start, int end);
int eraseFlash(int start, int end);
int prepareFlash(int start, int end);
*/
int iapFlash(int cmd, int start, int end);
#define blankCheckFlash(start, end) iapFlash(IAPCommand_Blank_check_sector, start, end)
#define eraseFlash(start, end) iapFlash(IAPCommand_Erase_sector, start, end)
#define prepareFlash(start, end) iapFlash(IAPCommand_Prepare_sector_for_write_operation, start, end)

/* // #define 移行で、36byte削減
int writeFlash(uint8* source_addr, uint8* target_addr, int size);
int compareFlash(uint8* source_addr, uint8* target_addr, int size);
*/
int writeOrCompareFlash(int cmd, uint8* source_addr, uint8* target_addr, int size);
#define writeFlash(source_addr, target_addr, size) writeOrCompareFlash(IAPCommand_Copy_RAM_to_Flash, source_addr, target_addr, size)
#define compareFlash(source_addr, target_addr, size) writeOrCompareFlash(IAPCommand_Compare, source_addr, target_addr, size)

int readBootCodeVersion(void);
int readPartID(void);
int readUID(void);


// from iap.c

// iap
#define IAP_LOCATION 0x1fff1ff1
typedef void (*IAP)(unsigned int[], unsigned int[]);

//#define iap_entry ((IAP)IAP_LOCATION)
void iap_entry(unsigned int* u1, unsigned int* u2) { // define から関数 // 24316 -> 24308 8byte削減
	((IAP)IAP_LOCATION)(u1, u2);
}

//#define CCLK_KHZ (SystemCoreClock / 1000) // 厳密にはこれ
//#define CCLK_KHZ (SystemCoreClock >> 10) // /1024 ... ちょっといい加減にしてみた .. これだけで16byte減る
#define CCLK_KHZ (48000) // 固定にしてみた 8byte減るけど、、VIDEO0でクロック落としても大丈夫だった

//#define CCLK_KHZ ((int)SystemCoreClock / 1000) // 厳密にはこれ
//#define CCLK_KHZ (SystemCoreClock / 1024) // /1024 = >> 10 最適化される、ちょっといい加減にしてみた .. これだけで16byte減る

#define USER_FLASH_AREA_START FLASH_SECTOR_7
#define USER_FLASH_AREA_SIZE (FLASH_SECTOR_SIZE_0_TO_15 * 1)


enum error_code {
	CMD_SUCCESS,
	INVALID_COMMAND,
	SRC_ADDR_ERROR,
	DST_ADDR_ERROR,
	SRC_ADDR_NOT_MAPPED,
	DST_ADDR_NOT_MAPPED,
	COUNT_ERROR,
	INVALID_SECTOR,
	SECTOR_NOT_BLANK,
	SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
	COMPARE_ERROR,
	BUSY,
	PARAM_ERROR,
	ADDR_ERROR,
	ADDR_NOT_MAPPED,
	CMD_LOCKED,
	INVALID_CODE,
	INVALID_BAUD_RATE,
	INVALID_STOP_BIT,
	CODE_READ_PROTECTION_ENABLED
};


int readBootCodeVersion(void) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Read_Boot_Code_version;
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[1];
}

/** Read part identification number
 *
 *  @return    device ID
 *  @see       read_serial()
 */
int readPartID(void) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Read_part_ID;
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[1];
}


/** Read device serial number
 *
 *  @return    device serial number
 *  @see       read_ID()
 */
int readUID(void) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Read_UID;
	iap_entry(IAP_command, IAP_result);
	// result 0 - 3
	return (int)IAP_result[1];
}

int iapFlash(int cmd, int start, int end) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = cmd;
	IAP_command[1] = (unsigned int)start;  //  Start Sector Number
	IAP_command[2] = (unsigned int)end;    //  End Sector Number (should be greater than or equal to start sector number)
    IAP_command[3] = CCLK_KHZ;             //  CPU Clock Frequency (CCLK) in kHz
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[0];
}

#if 0
/** Blank check sector(s)
 *  
 *  @param    start    a Start Sector Number
 *  @param    end      an End Sector Number (should be greater than or equal to start sector number).
 *  @return error code: CMD_SUCCESS | BUSY | SECTOR_NOT_BLANK | INVALID_SECTOR
 */

// 使っている SECTOR_NOT_BLANK、空いている CMD_SUCCESS
int blankCheckFlash(int start, int end) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Blank_check_sector;
	IAP_command[1] = (unsigned int)start;  //  Start Sector Number
	IAP_command[2] = (unsigned int)end;    //  End Sector Number (should be greater than or equal to start sector number)
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[0];
}

/** Erase Sector(s)
 *  
 *  @param    start    a Start Sector Number
 *  @param    end      an End Sector Number (should be greater than or equal to start sector number).
 *  @return   error code: CMD_SUCCESS | BUSY | SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION | INVALID_SECTOR
 */

int eraseFlash(int start, int end) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
    IAP_command[0] = IAPCommand_Erase_sector;
    IAP_command[1] = (unsigned int)start;  //  Start Sector Number
    IAP_command[2] = (unsigned int)end;    //  End Sector Number (should be greater than or equal to start sector number)
    IAP_command[3] = CCLK_KHZ;             //  CPU Clock Frequency (CCLK) in kHz
    iap_entry(IAP_command, IAP_result);
    return (int)IAP_result[0];
}

/** Prepare sector(s) for write operation
 *  
 *  @param    start    a Start Sector Number
 *  @param    end      an End Sector Number (should be greater than or equal to start sector number).
 *  @return   error code: CMD_SUCCESS | BUSY | INVALID_SECTOR
 */

int prepareFlash(int start, int end) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Prepare_sector_for_write_operation;
	IAP_command[1] = (unsigned int)start;  //  Start Sector Number
	IAP_command[2] = (unsigned int)end;    //  End Sector Number (should be greater than or equal to start sector number).
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[0];
}
#endif

int writeOrCompareFlash(int cmd, uint8* source_addr, uint8* target_addr, int size) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
    IAP_command[0] = cmd;
    IAP_command[1] = (unsigned int)target_addr;    //  Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
    IAP_command[2] = (unsigned int)source_addr;    //  Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
    IAP_command[3] = size;                         //  Number of bytes to be compared; should be a multiple of 4.
	IAP_command[4] = CCLK_KHZ;                     //  CPU Clock Frequency (CCLK) in kHz.
    iap_entry(IAP_command, IAP_result);
    return (int)IAP_result[0];
}

#if 0
/** Copy RAM to Flash
 *  
 *  @param    source_addr    Source RAM address from which data bytes are to be read. This address should be a word boundary.
 *  @param    target_addr    Destination flash address where data bytes are to be written. This address should be a 256 byte boundary.
 *  @param    size           Number of bytes to be written. Should be 256 | 512 | 1024 | 4096.
 *  @return   error code: CMD_SUCCESS | SRC_ADDR_ERROR (Address not a word boundary) | DST_ADDR_ERROR (Address not on correct boundary) | SRC_ADDR_NOT_MAPPED | DST_ADDR_NOT_MAPPED | COUNT_ERROR (Byte count is not 256 | 512 | 1024 | 4096) | SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION | BUSY
 */

int writeFlash(uint8* source_addr, uint8* target_addr, int size) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
	IAP_command[0] = IAPCommand_Copy_RAM_to_Flash;
	IAP_command[1] = (unsigned int)target_addr;    //  Destination flash address where data bytes are to be written. This address should be a 256 byte boundary.
	IAP_command[2] = (unsigned int)source_addr;    //  Source RAM address from which data bytes are to be read. This address should be a word boundary.
	IAP_command[3] = size;                         //  Number of bytes to be written. Should be 256 | 512 | 1024 | 4096.
	IAP_command[4] = CCLK_KHZ;                     //  CPU Clock Frequency (CCLK) in kHz.
	iap_entry(IAP_command, IAP_result);
	return (int)IAP_result[0];
}


/** Compare <address1> <address2> <no of bytes>
 *  
 *  @param    source_addr Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
 *  @param    target_addr Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
 *  @param    size         Number of bytes to be compared; should be a multiple of 4.
 *  @return   error code: CMD_SUCCESS | COMPARE_ERROR | COUNT_ERROR (Byte count is not a multiple of 4) | ADDR_ERROR | ADDR_NOT_MAPPED     
 */

int compareFlash(uint8* source_addr, uint8* target_addr, int size) {
	unsigned int IAP_command[5];
	unsigned int IAP_result[5];
    IAP_command[0] = IAPCommand_Compare;
    IAP_command[1] = (unsigned int)target_addr;    //  Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
    IAP_command[2] = (unsigned int)source_addr;    //  Starting flash or RAM address of data bytes to be compared. This address should be a word boundary.
    IAP_command[3] = size;                         //  Number of bytes to be compared; should be a multiple of 4.
    iap_entry(IAP_command, IAP_result);
    return (int)IAP_result[0];
}
#endif

#endif

