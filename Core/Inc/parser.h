/*
File name: parser.h
Author: Dan Gabbay
Date: 25 May 2019
Device P/N: DSPIC33EP128MC202 (Microchip Technology)
*/


#ifndef PARSER_H
#define PARSER_H
unsigned char ConvertStringToIndex (unsigned char *userCmd,
                                    const unsigned char **commadModeFunctions,unsigned char tabSize);

//void commandLnTokens (commandTokens *commandTokensPtr, unsigned char *commandLnStr);

#endif
