// USART Library
//
// Functions to perform basic async subset of USART operations
//
// Jonathan Bukasa
// Created October 2025, initial build, ported from STM32L476 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include "usart.h"
#include <stdio.h>
#include "stm32g0xx.h"
#include "gpio.h"
#include "stdlib.h"     // string to integer
#include <math.h>

void _USART_ClockEnable (USART_TypeDef * pUSART)
{
    if (pUSART == USART1)
    {
        RCC->APBENR2 |= RCC_APBENR2_USART1EN_Msk;   // Enable USART1 clock
    }
    else if (pUSART == USART2)
    {
        RCC->APBENR1 |= RCC_APBENR1_USART2EN_Msk;   // Enable USART2 clock
    }
    else   // Invalid peripheral, do nothing
    {;;;}
}

// non-generic version of port init
// NOTE: assumes USART1 on PA9/PA10
void _USART_Init_USART1 (long bus_rate, long baud_rate)
{

}

// non-generic version of port init
// NOTE: assumes USART2 on PA2/PA3
void _USART_Init_USART2 (long bus_rate, long baud_rate)
{
  //  Ensure that the port clock is turned on for GPIOA
  _GPIO_ClockEnable(GPIOA);

  //Ensure that the peripheral clock is enabled for USART2 (RCC_APBENR1 (RM::5.4.15))
  _USART_ClockEnable(USART2);

  //Set pins PA2 and PA3 to alternate function 1 (DS::P40)
  _GPIO_SetPinAlternateFunction(GPIOA, 2, 1, _GPIO_Clock_On);
  _GPIO_SetPinAlternateFunction(GPIOA, 3, 1, _GPIO_Clock_On);

  //USART2->CR1 &= ~USART_CR1_UE_Msk;

  //Turn on the transmitter and receiver for the USART (USART_CR1 (RM::33.8.1))
  USART2->CR1 |= USART_CR1_TE_Msk;
  USART2->CR1 |= USART_CR1_RE_Msk;

  //Set the BAUD rate for the UART, using 10x fixed point arithmetic conversion with the provided bus rate and desired BAUD rate in the calculation. (USART_BRR (RM::33.8.5))
  USART2->BRR = ((((bus_rate * 10) / baud_rate) + 5) / 10);

  //Turn on the USART (USART_CR1 (RM::33.8.1)) (must be done after configuration) 
  USART2->CR1 |= USART_CR1_UE_Msk;
}

// generic blocking TxByte function
void _USART_TxByte (USART_TypeDef * pUSART, unsigned char data)
{
  //  Wait for the TXFNF flag of USART_ISR to set (USART_ISR (RM::33.8.9)). Note: the TXFNF register name is odd in the derivative file!
  while(!(pUSART->ISR & USART_ISR_TXE_TXFNF_Msk))
  {
    //Wait
  }

  //Write the data to USART_TDR (USART_TDR (RM::33.8.13))
  pUSART->TDR = data;
}

// generic non-blocking RxByte function
int _USART_RxByte (USART_TypeDef * pUSART, unsigned char * pData)
{
  //  If the RXFNE flag is set in USART_ISR, read USART_RDR and place the result at the target of the provided data pointer (pData). Return a value indicating that the data target is valid. Note: the RXFNE register name is odd in the derivative file!
  if (pUSART->ISR & USART_ISR_RXNE_RXFNE_Msk)
  {
    *pData = (unsigned char)pUSART->RDR;
    return 1;  // worked
  }
  //Otherwise, if the RXFNE flag is clear in USART_ISR, return a value indicating that the data target is not valid.
  return 0;   // there's nothing
}

// added prototypes / types for advanced UART library

// transmit the NUL terminated string one character at a time with _USART_TxByte
void _USART_TxString (USART_TypeDef * pUSART, char * pStr)
{
  //Iterate over non-NUL characters at pStr, using TxByte
  //When a NUL character is found (could be first) return
  while (*pStr)
  {
    _USART_TxByte(pUSART, *pStr);
    pStr++;
  }
}


// use escape sequences to place the cursor at the specified position
// this is the \x1B[Y;XH form with formatted argument replacement (sprintf)
void _USART_GotoXY (USART_TypeDef * pUSART, int iCol, int iRow)
{
  //You may do limited error checking on iCol/iRow
  if (iRow > 24 || iCol > 80 || iCol < 1|| iRow < 1)
    return;
  //Format an escape sequence based on \x1b[R;CH using sprintf
  char buffer[16];
  sprintf(buffer, "\x1b[%d;%dH", iRow, iCol);
  //Send the formatted string to the USART with Tx_String
  _USART_TxString(pUSART, buffer);
}


// use _USART_GotoXY and _USART_TxString to place the string
void _USART_TxStringXY (USART_TypeDef * pUSART, int iCol, int iRow, char * pStr)
{
  // Use the GotoXY and TxString functions to build this function.
  _USART_GotoXY(pUSART, iCol, iRow);  // moves cursor to desired position
  _USART_TxString(pUSART, pStr);    // prints text there
}


// use an escape sequence to clear the terminal
void _USART_ClearScreen (USART_TypeDef * pUSART)
{
  //Use the \x1b[2J and \x1b[H escape sequences to clear the screen and go home.
  char clear[] = "\x1b[2J";
  char home[] = "\x1b[H";

  // clear sequence
  for (int i=0;i<sizeof(clear); i++)
  {
    _USART_TxByte(pUSART, clear[i]);
  }

  // home sequence
  for (int i=0;i<sizeof(clear)-1; i++)
  {
    _USART_TxByte(pUSART, home[i]);
  }
}

void _USART_ClearLine(USART_TypeDef * pUSART, int col, int row)
{
  char clear[] = "\x1b[2K";

  _USART_GotoXY(pUSART, col, row);

    // clear sequence
  for (int i=0;i<sizeof(clear); i++)
  {
    _USART_TxByte(pUSART, clear[i]);
  }

}

void _USART_TableMaker(int n)
{
    char buffer[8];  // enough to hold "0000 " + null
    for (int c = 1; c <= n; c++)
    {
        // Format with 3 leading zeros and a space after
        sprintf(buffer, "%03d ", c);

        // Print it to the first row (1), multiplying the c variable shifts headers by 4 and adding to it (3  + c) provides a start position
        _USART_TxStringXY(USART2, 3 + c * 4, 1, buffer);
    }

    for (int r = 1; r <= n; r++)
    {
        // Format with 3 leading zeros and a space after
        sprintf(buffer, "%03d ", r);

        // Print it to the first column
        _USART_TxStringXY(USART2, 1, 3 + r, buffer);

      for (int c = 1; c <= n; c++)
      {
          int product = r*c;  // product of multiplication

          sprintf(buffer, "%3d ", product); // Format with 3 leading spaces and a space after

          // Combine columns and rows 
          _USART_TxStringXY(USART2, 3 + c * 4, 3 + r, buffer);
      }
    }
}

double _USART_StringToDouble(char num[])
{
  char *invalid;
  return strtod(num, &invalid); 
}

// generic version of RxByte (blocking)
unsigned char _USART_RxByteB (USART_TypeDef * pUSART);


// advanced:
// input up to (iBufferLength - 1) characters from the terminal
// supports backspace editing
// remote echo is used for accepted characters except enter
// if the user hits 'enter' the function will return (possibly with fewer characters)
// entry is NUL terminated at the end-of-input position
// buffer must be declared in calling scope, and must allocate at least iBufferLength bytes
// enforce type will limit character entry to specified values
int _USART_RxString (USART_TypeDef * pUSART, unsigned char * pTargetBuffer, unsigned short iBufferLength, _USART_RX_ENFORCE EnforceType);
