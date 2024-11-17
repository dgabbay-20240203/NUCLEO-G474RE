/*
File name: crc32.h
Last change: 11 Mar 2018
Author: Dan Gabbay
*/



#ifndef CRC32_H
#define CRC32_H
#define CRC32_SEED (0xffffffff)
unsigned long crc32(unsigned long crc, const unsigned char *buf, unsigned long size);
#endif
