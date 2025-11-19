// USART Library
//
// Functions to perform basic async subset of USART operations
//
// Simon Walker
// Created September 2024, initial build, ported from STM32L476 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#ifndef _USART_Lib
#define _USART_Lib

#include "stm32g0xx.h"

// non-generic version of port init
// NOTE: assumes USART1 on PA9/PA10
void _USART_Init_USART1 (long bus_rate, long baud_rate);

// non-generic version of port init
// NOTE: assumes USART2 on PA2/PA3
void _USART_Init_USART2 (long bus_rate, long baud_rate);

// generic blocking TxByte function
void _USART_TxByte (USART_TypeDef * pUSART, unsigned char data);

// generic non-blocking RxByte function
int _USART_RxByte (USART_TypeDef * pUSART, unsigned char * pData);

// added prototypes / types for advanced UART library

// transmit the NUL terminated string one character at a time with _USART_TxByte
void _USART_TxString (USART_TypeDef * pUSART, char * pStr);

// use escape sequences to place the cursor at the specified position
// this is the \x1B[Y;XH form with formatted argument replacement (sprintf)
void _USART_GotoXY (USART_TypeDef * pUSART, int iCol, int iRow);

// use _USART_GotoXY and _USART_TxString to place the string
void _USART_TxStringXY (USART_TypeDef * pUSART, int iCol, int iRow, char * pStr);

// use an escape sequence to clear the terminal
void _USART_ClearScreen (USART_TypeDef * pUSART);

// generic version of RxByte (blocking)
unsigned char _USART_RxByteB (USART_TypeDef * pUSART);

typedef enum
{
  _USART_RX_ENFORCE_ANY,    // permit any printable character
  _USART_RX_ENFORCE_DIGIT,  // permit only 0-9
  _USART_RX_ENFORCE_HEX     // permit only 0-9,a-f,A-F
} _USART_RX_ENFORCE;

// advanced:
// input up to (iBufferLength - 1) characters from the terminal
// supports backspace editing
// remote echo is used for accepted characters except enter
// if the user hits 'enter' the function will return (possibly with fewer characters)
// entry is NUL terminated at the end-of-input position
// buffer must be declared in calling scope, and must allocate at least iBufferLength bytes
// enforce type will limit character entry to specified values
int _USART_RxString (USART_TypeDef * pUSART, unsigned char * pTargetBuffer, unsigned short iBufferLength, _USART_RX_ENFORCE EnforceType);

#endif