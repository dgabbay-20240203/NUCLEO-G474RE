/*
File name: i2c1.h
Author: Dan Gabbay
Creation date: 17 Nov 2024
 */

#ifndef I2C1_H
#define I2C1_H

void handleI2c1(void);
void I2C1_tx_rx(uint16_t I2C1_DevAddr, uint16_t I2C1_msgSize, uint8_t opCode, uint8_t* buf_ptr);
uint8_t I2C1_status(void);

#endif
