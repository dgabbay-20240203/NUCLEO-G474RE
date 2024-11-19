/*
File name: AT24C256B_i2c_eeprom.c
Author: Dan Gabbay
Date: 17 Nov 2024
*/
#include "stm32g4xx_hal.h"
#include "AT24C256B_i2c_eeprom.h"
#include "crc32.h"
#include "i2c1.h"


extern I2C_HandleTypeDef hi2c1;
struct sys_config sysConfig;

struct __attribute__((__packed__)) i2c_msgToTransmit {
    uint16_t mem_addr;
    struct sys_config sysConfig;
}buff;
const uint32_t header = 0xc5ac234b;



void save_sys_config(void)
{
	sysConfig.header = header;
	sysConfig.crc32 = crc32(CRC32_SEED, (const uint8_t *) &sysConfig, sizeof(sysConfig) - 4);

	buff.mem_addr = 0x0000; // Start address of EEPROM page 0
	buff.sysConfig = sysConfig;
	I2C1_tx_rx(0xa0, sizeof(buff), 1, (uint8_t *) &buff);
}

void restore_sys_config(void)
{
	buff.mem_addr = 0x0000; // Start address of EEPROM page 0, byte 0.
	HAL_I2C_Master_Transmit(&hi2c1, 0xa0, (uint8_t *)&buff, 2,200); // This is blocking, it takes around 300 microseconds to complete.
	I2C1_tx_rx(0xa1, sizeof(sysConfig), 2, (uint8_t *) &sysConfig); // This is non-blocking.
}
