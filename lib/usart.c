// usart.c
//
// USART library implementation for STM32G0
// Matches usart.h (Simon Walker, CMPE1250)
//

#include "usart.h"
#include "gpio.h"
#include <stdio.h>

// ---------------------------------------------------------
// Helpers
// ---------------------------------------------------------

static uint32_t _USART_ComputeBRR(long bus_rate, long baud_rate)
{
    if (bus_rate <= 0 || baud_rate <= 0)
        return 1u;

    // 10x fixed-point style (rounded):
    // usartdiv10 = (bus * 10) / baud
    // BRR = (usartdiv10 + 5) / 10
    long usartdiv10 = (bus_rate * 10L) / baud_rate;
    long br = (usartdiv10 + 5L) / 10L;
    if (br <= 0)
        br = 1;
    return (uint32_t)br;
}

static int _USART_CharAllowed(unsigned char ch, _USART_RX_ENFORCE EnforceType)
{
    if (EnforceType == _USART_RX_ENFORCE_ANY)
    {
        // basic printable ASCII
        return (ch >= ' ' && ch <= '~');
    }
    else if (EnforceType == _USART_RX_ENFORCE_DIGIT)
    {
        return (ch >= '0' && ch <= '9');
    }
    else if (EnforceType == _USART_RX_ENFORCE_HEX)
    {
        if (ch >= '0' && ch <= '9') return 1;
        if (ch >= 'a' && ch <= 'f') return 1;
        if (ch >= 'A' && ch <= 'F') return 1;
        return 0;
    }

    return 0;
}

// ---------------------------------------------------------
// Init functions
// ---------------------------------------------------------

// USART1 on PA9 (TX), PA10 (RX), AF1
void _USART_Init_USART1 (long bus_rate, long baud_rate)
{
    // GPIOA clock
    _GPIO_ClockEnable(GPIOA);

    // USART1 clock on APB2
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;

    // PA9 = TX AF1, PA10 = RX AF1
    _GPIO_SetPinAlternateFunction(GPIOA, 9u, 1u, _GPIO_Clock_NoChange);
    _GPIO_SetPinAlternateFunction(GPIOA, 10u, 1u, _GPIO_Clock_NoChange);

    _GPIO_Set_PullType(GPIOA, 9, _GPIO_PullType_None);
    _GPIO_Set_PullType(GPIOA, 10, _GPIO_PullType_None);

    // Reset CR1, ensure UE=0
    USART1->CR1 = 0;

    // Set baud
    USART1->BRR = _USART_ComputeBRR(bus_rate, baud_rate);

    // Enable TX and RX
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enable USART
    USART1->CR1 |= USART_CR1_UE;
}

// USART2 on PA2 (TX), PA3 (RX), AF1
void _USART_Init_USART2 (long bus_rate, long baud_rate)
{
    // GPIOA clock
    _GPIO_ClockEnable(GPIOA);

    // USART2 clock on APB1
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    // PA2 = TX AF1, PA3 = RX AF1
    _GPIO_SetPinAlternateFunction(GPIOA, 2u, 1u, _GPIO_Clock_NoChange);
    _GPIO_SetPinAlternateFunction(GPIOA, 3u, 1u, _GPIO_Clock_NoChange);

    _GPIO_Set_PullType(GPIOA, 2, _GPIO_PullType_None);
    _GPIO_Set_PullType(GPIOA, 3, _GPIO_PullType_None);

    USART2->CR1 = 0;

    USART2->BRR = _USART_ComputeBRR(bus_rate, baud_rate);

    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;
}

// ---------------------------------------------------------
// Basic Tx / Rx
// ---------------------------------------------------------

// blocking TxByte
void _USART_TxByte (USART_TypeDef * pUSART, unsigned char data)
{
    if (pUSART == NULL)
        return;

    // wait for TX buffer empty
    while ((pUSART->ISR & USART_ISR_TXE_TXFNF) == 0u)
    {
        // busy wait
    }

    pUSART->TDR = (uint32_t)data;
}

// non-blocking RxByte
// returns 1 if char read, 0 if no data
int _USART_RxByte (USART_TypeDef * pUSART, unsigned char * pData)
{
    if (pUSART == NULL || pData == NULL)
        return 0;

    // check if RX data available
    if ((pUSART->ISR & USART_ISR_RXNE_RXFNE) != 0u)
    {
        uint32_t d = pUSART->RDR;
        *pData = (unsigned char)(d & 0xFFu);
        return 1;
    }

    return 0;
}

// ---------------------------------------------------------
// String / terminal helpers
// ---------------------------------------------------------

// transmit NUL-terminated string
void _USART_TxString (USART_TypeDef * pUSART, char * pStr)
{
    if (pUSART == NULL || pStr == NULL)
        return;

    while (*pStr != '\0')
    {
        _USART_TxByte(pUSART, (unsigned char)*pStr);
        pStr++;
    }
}

// position cursor using escape sequence ESC[row;colH]
void _USART_GotoXY (USART_TypeDef * pUSART, int iCol, int iRow)
{
    if (pUSART == NULL)
        return;

    char buf[24];

    if (iCol < 1) iCol = 1;
    if (iRow < 1) iRow = 1;

    int n = snprintf(buf, sizeof(buf), "\x1B[%d;%dH", iRow, iCol);

    for (int i = 0; i < n && buf[i] != '\0'; i++)
    {
        _USART_TxByte(pUSART, (unsigned char)buf[i]);
    }
}

// goto XY and transmit string
void _USART_TxStringXY (USART_TypeDef * pUSART, int iCol, int iRow, char * pStr)
{
    if (pUSART == NULL || pStr == NULL)
        return;

    _USART_GotoXY(pUSART, iCol, iRow);
    _USART_TxString(pUSART, pStr);
}

// clear screen and home cursor
void _USART_ClearScreen (USART_TypeDef * pUSART)
{
    if (pUSART == NULL)
        return;

    // ESC[2J = clear, ESC[H = home
    _USART_TxString(pUSART, "\x1B[2J");
    _USART_TxString(pUSART, "\x1B[H");
}

// ---------------------------------------------------------
// Blocking RxByte
// ---------------------------------------------------------

unsigned char _USART_RxByteB (USART_TypeDef * pUSART)
{
    if (pUSART == NULL)
        return 0;

    // wait for RX data available
    while ((pUSART->ISR & USART_ISR_RXNE_RXFNE) == 0u)
    {
        // busy wait
    }

    return (unsigned char)(pUSART->RDR & 0xFFu);
}

// ---------------------------------------------------------
// RxString with editing + enforcement
// ---------------------------------------------------------

int _USART_RxString (USART_TypeDef * pUSART,
                     unsigned char * pTargetBuffer,
                     unsigned short iBufferLength,
                     _USART_RX_ENFORCE EnforceType)
{
    if (pUSART == NULL || pTargetBuffer == NULL || iBufferLength < 1)
        return 0;

    unsigned short idx = 0;

    while (1)
    {
        unsigned char ch = _USART_RxByteB(pUSART);

        // ENTER (CR or LF) ends input
        if (ch == '\r' || ch == '\n')
        {
            // echo CRLF to move to next line
            _USART_TxByte(pUSART, '\r');
            _USART_TxByte(pUSART, '\n');

            pTargetBuffer[idx] = '\0';
            return (int)idx;
        }

        // backspace / delete
        if (ch == 0x08 || ch == 0x7F)
        {
            if (idx > 0)
            {
                idx--;
                pTargetBuffer[idx] = '\0';

                // erase last char on terminal: BS, space, BS
                _USART_TxByte(pUSART, '\b');
                _USART_TxByte(pUSART, ' ');
                _USART_TxByte(pUSART, '\b');
            }
            continue;
        }

        // enforce valid chars
        if (!_USART_CharAllowed(ch, EnforceType))
        {
            // ignore invalid char
            continue;
        }

        // store only if space left (reserve one for NUL)
        if (idx < (unsigned short)(iBufferLength - 1u))
        {
            pTargetBuffer[idx++] = ch;
            _USART_TxByte(pUSART, ch);  // echo accepted char
        }
        else
        {
            // buffer full – ignore extra chars, keep waiting for ENTER
        }
    }
}
