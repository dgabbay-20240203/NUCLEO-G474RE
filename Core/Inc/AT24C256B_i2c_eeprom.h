/*
File name: AT24C256B_i2c_eeprom.h
Author: Dan Gabbay
Date: 17 Nov 2024
*/

#ifndef AT24C256B_I2C_EEPROM_H
#define AT24C256B_I2C_EEPROM_H

#include "stm32g4xx_hal.h"

struct __attribute__((__packed__)) sys_config {
   uint32_t header;
   uint32_t unit_serial_num;
   uint8_t  secret_key[32];
   uint8_t  filler[20];
   uint32_t crc32;
};

void save_sys_config(void);
void restore_sys_config(void);

#endif
