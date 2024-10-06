/*
File name: proc_shell_comm.h
Author: Dan Gabbay
Date: 25 May 2019
Device P/N: DSPIC33EP128MC202 (Microchip Technology)
*/


#ifndef PROC_SHELL_COMM_H
#define PROC_SHELL_COMM_H

#define NUM_OF_COM 3 // Must equal to the number of commands.
#define MAX_NUM_OF_COMMAND_TOKENS 17

#define CTRL_C    0x03
#define BACKSPACE 0x7f
#define TAB       0x09
#define LINE_FEED 0x0a
#define CR_ENTER  0x0d
#define CTRL_Z    0x1a
#define ESC       0x1b
#define BACKSLASH 0x5c

typedef struct {
unsigned char *commandTok [MAX_NUM_OF_COMMAND_TOKENS];
unsigned char commStatus;
unsigned char numOfTokens;

} commandTokens;

extern commandTokens comm_tokens;
#endif
