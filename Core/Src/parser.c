/*
File name: parser.c
Author: Dan Gabbay
Date: 05 Oct 2024
*/

/*********** Include files ***************/
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "proc_shell_comm.h"


#define TAB       0x09

unsigned char ConvertStringToIndex (unsigned char *userCmd,const unsigned char **commadModeFunctions,unsigned char tabSize)
{
    unsigned char i=0;    
    while ((i < tabSize) && (strcmp((char *) userCmd,(char *) *(commadModeFunctions+i))!=0)) i++;
    return (i);
}



void commandLnTokens (commandTokens *commandTokensPtr, unsigned char *commandLnStr, unsigned char rx_buff_size)
{
    unsigned char i;
    unsigned char j;
    commandTokensPtr->commStatus = 1;

    i=0;
    j=0;
    while ((i<rx_buff_size) && (*(commandLnStr+i))&&(j<MAX_NUM_OF_COMMAND_TOKENS))
        {
        while ((*(commandLnStr+i))&&(i<rx_buff_size)&&((*(commandLnStr+i)==' ')||(*(commandLnStr+i)==TAB))) i++; // Skip spaces and TABs but don�t go
                                                                                                                 // beyond buffer size or NULL character
        if ((*(commandLnStr+i)==0)||(i>=rx_buff_size)) break;
           
        if (*(commandLnStr+i)=='\"')
            {
            i++;
            commandTokensPtr->commandTok [j] = commandLnStr+i-1;
            j++;
            while ((*(commandLnStr+i))&&(i<rx_buff_size)&&(*(commandLnStr+i)!='\"')) i++;
            if (*(commandLnStr+i)!='\"')
                {
                commandTokensPtr->commStatus = 0;
                break;
                }
            else
                {
                i++;
                if (*(commandLnStr+i))
                    {
                    if (*(commandLnStr+i)!= ' ') commandTokensPtr->commStatus = 0;
                    *(commandLnStr+i)= 0;// End of the string
                    i++;
                    }
                }
            }
        else
            {
            commandTokensPtr->commandTok [j] = commandLnStr+i;
            j++;
            while ((*(commandLnStr+i))&&(i<rx_buff_size)&&(*(commandLnStr+i)!=' ')&&(*(commandLnStr+i)!=TAB)) i++; // End of token

            if (*(commandLnStr+i))
                {
                *(commandLnStr+i)= 0;// End of the string
                i++;
                }
            }
        }

     commandTokensPtr->numOfTokens = j;
     while ((*(commandLnStr+i))&&(i<rx_buff_size)&&((*(commandLnStr+i)==' ')||(*(commandLnStr+i)==TAB))) i++; // Skip spaces and TABs

        if ((j==MAX_NUM_OF_COMMAND_TOKENS)&&(*(commandLnStr+i)) != 0) // Too many tokens in command line...
            {
            commandTokensPtr->commStatus = 0;
            }
}
