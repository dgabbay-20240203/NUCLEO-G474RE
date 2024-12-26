/*
File name: secured_message.h
Author: Dan Gabbay
Creation date: 26 Dec 2024
*/


#ifndef SECURED_MESSAGE_H
#define SECURED_MESSAGE_H
#include "stm32g4xx_hal.h"

struct __attribute__((__packed__)) secured_message // We keep the message size constant 100 bytes.
{
  uint32_t seed;                // Monotonic 32-BIT seed (32-BIT UNIX timestamp is preferred if available).
  uint8_t payload[64];          // It is up to the user to define the payload.
  uint8_t sha256_signature[32]; // The SHA256 signature is also dependent on a 256-BIT secreted
                                // key that is not part of the message.
};

#endif

