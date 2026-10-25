#include <stdio.h>
#include "stm32g0xx.h"
#include "GPIO.h"    
#include "USART.h"

// USART Library
//
// Functions to perform basic async subset of USART operations
//
// Hamid Jalil
// Created January 2026, initial build, ported from STM32G0B1RE version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////


// NOTE: assumes USART2 on PA2 (TX) / PA3 (RX)
void _USART_Init_USART2(long bus_rate, long baud_rate)
{
    _GPIO_ClockEnable(GPIOA);
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    // PA2 / PA3 -> Alternate Function
    _GPIO_SetPinMode(GPIOA, 2, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);
    _GPIO_SetPinMode(GPIOA, 3, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);

    // AF1 -> USART2
    _GPIO_SetPinAlternateFunction(GPIOA, 2, 1, _GPIO_Clock_NoChange);
    _GPIO_SetPinAlternateFunction(GPIOA, 3, 1, _GPIO_Clock_NoChange);

    // Enable receiver and transmitter
    USART2->CR1 |= USART_CR1_RE_Msk;
    USART2->CR1 |= USART_CR1_TE_Msk;

    // Baud rate calculation (same as your original)
    int baud = (((bus_rate * 10) / baud_rate) + 5) / 10;
    USART2->BRR = baud;

    // Enable USART
    USART2->CR1 |= USART_CR1_UE_Msk;
}


// NOTE: assumes USART1 on PA9 (TX) / PA10 (RX)
void _USART_Init_USART1(long bus_rate, long baud_rate)
{
    _GPIO_ClockEnable(GPIOA);
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;

    // PA9 / PA10 -> Alternate Function
    _GPIO_SetPinMode(GPIOA, 9,  _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);
    _GPIO_SetPinMode(GPIOA, 10, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);

    // AF1 -> USART1
    _GPIO_SetPinAlternateFunction(GPIOA, 9,  1, _GPIO_Clock_NoChange);
    _GPIO_SetPinAlternateFunction(GPIOA, 10, 1, _GPIO_Clock_NoChange);

    // Enable receiver and transmitter
    USART1->CR1 |= USART_CR1_RE_Msk;
    USART1->CR1 |= USART_CR1_TE_Msk;

    // Baud rate calculation (same as your original)
    int baud = (((bus_rate * 10) / baud_rate) + 5) / 10;
    USART1->BRR = baud;

    // Enable USART
    USART1->CR1 |= USART_CR1_UE_Msk;
}


//#ifdef __STM32G0B1_SUBFAMILY
// NOTE: STM32G0B1RE assumes USART6 on PA4 (TX) / PA5 (RX)
// IMPORTANT: PA4/PA5 use AF3 for USART6 on G0B1 
/*void _USART_Init_USART6(long bus_rate, long baud_rate)
{
    _GPIO_ClockEnable(GPIOA);
    RCC->APBENR2 |= RCC_APBENR2_USART6EN;

    // PA4 / PA5 -> Alternate Function
    _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);
    _GPIO_SetPinMode(GPIOA, 5, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_NoChange);

    // AF3 -> USART6
    _GPIO_SetPinAlternateFunction(GPIOA, 4, 3, _GPIO_Clock_NoChange);
    _GPIO_SetPinAlternateFunction(GPIOA, 5, 3, _GPIO_Clock_NoChange);

    // Enable receiver and transmitter
    USART6->CR1 |= USART_CR1_RE_Msk;
    USART6->CR1 |= USART_CR1_TE_Msk;

    // Baud rate calculation 
    int baud = (((bus_rate * 10) / baud_rate) + 5) / 10;
    USART6->BRR = baud;

    // Enable USART
    USART6->CR1 |= USART_CR1_UE_Msk;
}
#endif
*/

// generic blocking TxByte function
void _USART_TxByte(USART_TypeDef *pUSART, unsigned char data)
{    while (!(pUSART->ISR & USART_ISR_TXE_TXFNF_Msk))
    {
        ;;
    }
    pUSART->TDR = data;
}

// generic non-blocking RxByte function
int _USART_RxByte(USART_TypeDef *pUSART, unsigned char *pData)
{
    if (pUSART->ISR & USART_ISR_RXNE_RXFNE_Msk)
    {
        *pData = (unsigned char)(pUSART->RDR & 0xFFu);
        return 1;
    }
    return 0;
}

// transmit the NUL terminated string one character at a time with _USART_TxByte
void _USART_TxString(USART_TypeDef *pUSART, char *pStr)
{
    if (pStr == NULL)
        return;

    while (*pStr != '\0')
    {
        _USART_TxByte(pUSART, (unsigned char)(*pStr));
        pStr++;
    }
}

// use escape sequences to place the cursor at the specified position
void _USART_GotoXY(USART_TypeDef *pUSART, int iCol, int iRow)
{
    char escSeq[16];

    if (iCol <= 0 || iRow <= 0)
        return;

    // ESC[row;colH
    sprintf(escSeq, "\x1B[%d;%dH", iRow, iCol);
    _USART_TxString(pUSART, escSeq);
}

// use _USART_GotoXY and _USART_TxString to place the string
void _USART_TxStringXY(USART_TypeDef *pUSART, int iCol, int iRow, char *pStr)
{
    if (pStr == NULL || iCol <= 0 || iRow <= 0)
        return;

    _USART_GotoXY(pUSART, iCol, iRow);
    _USART_TxString(pUSART, pStr);
}

// use an escape sequence to clear the terminal
void _USART_ClearScreen(USART_TypeDef *pUSART)
{
    _USART_TxString(pUSART, "\x1B[2J"); // clear screen
    _USART_TxString(pUSART, "\x1B[H");  // cursor home
}

// generic version of RxByte (blocking)
unsigned char _USART_RxByteB(USART_TypeDef *pUSART)
{
    while (!(pUSART->ISR & USART_ISR_RXNE_RXFNE_Msk))
    {
        ;;
    }
    return (unsigned char)(pUSART->RDR & 0xFFu);
}


// advanced RxString with enforce types and backspace support
int _USART_RxString(USART_TypeDef *pUSART,unsigned char *pTargetBuffer,unsigned short iBufferLength,_USART_RX_ENFORCE EnforceType)
{
    if (pTargetBuffer == NULL || iBufferLength < 1)
        return 0;

    unsigned short index = 0;

    while (index < (iBufferLength - 1))
    {
        // wait for a byte
        while (!(pUSART->ISR & USART_ISR_RXNE_RXFNE_Msk))
            ;;

        unsigned char ch = (unsigned char)(pUSART->RDR & 0xFFu);

        // backspace (ASCII 8 or DEL 127)
        if (ch == 8 || ch == 127)
        {
            if (index > 0)
            {
                index--;
                _USART_TxByte(pUSART, '\b');
                _USART_TxByte(pUSART, ' ');
                _USART_TxByte(pUSART, '\b');
            }
            continue;
        }

        // enter (CR or LF)
        if (ch == '\r' || ch == '\n')
        {
            _USART_TxByte(pUSART, '\r');
            _USART_TxByte(pUSART, '\n');
            break;
        }

        // enforce type
        int isValid = 0;
        switch (EnforceType)
        {
        case _USART_RX_ENFORCE_ANY:
            if (ch >= 32 && ch <= 126)
                isValid = 1;
            break;

        case _USART_RX_ENFORCE_DIGIT:
            if (ch >= '0' && ch <= '9')
                isValid = 1;
            break;

        case _USART_RX_ENFORCE_HEX:
            if ((ch >= '0' && ch <= '9') ||
                (ch >= 'A' && ch <= 'F') ||
                (ch >= 'a' && ch <= 'f'))
                isValid = 1;
            break;
        }

        // store + echo
        if (isValid)
        {
            pTargetBuffer[index++] = ch;
            _USART_TxByte(pUSART, ch);
        }
    }

    pTargetBuffer[index] = '\0';
    return (int)index;
}
