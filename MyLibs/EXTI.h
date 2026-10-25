// EXTI Library
//
// Functions perform basic EXTI configuration
//
// Simon Walker
// Created April 02, 2026 - Initial Build
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

// EXTI interrupts are selected by pin number, not port, so
// no two same port pin numbers may generate separate EXTI interrupts
// EXTI interrupts are consolidated on the B1 device as 0-1, 2-3, 4-15

#include "stm32g0xx.h"

#ifndef _EXTI_Header_Include
#define _EXTI_Header_Include

typedef enum
{
  _EXTI_PORT_A,       // 0x00 - GPIOA
  _EXTI_PORT_B,
  _EXTI_PORT_C,
  _EXTI_PORT_D,
  _EXTI_PORT_E,
  _EXTI_PORT_F        // 0x05 - GPIOF
} _EXTI_PORT;

typedef enum
{
  _EXTI_TRIG_None,    // no edge trigger (odd choice)
  _EXTI_TRIG_Rising,  // rising edge
  _EXTI_TRIG_Falling, // falling edge
  _EXTI_TRIG_Both     // both rising and falling edges
} _EXTI_TRIG;

typedef enum
{
  _EXTI_INT_Off,      // no interrupt from this line
  _EXTI_INT_On        // generate interrupts based on edge settings
} _EXTI_INT;

// configure a single EXTI line for port, trigger type, and interrupt support
void _EXTI_Set (int EXTILine, _EXTI_PORT port, _EXTI_TRIG trig, _EXTI_INT intoption);

// function to clear a pending falling edge flag
void _EXTI_ClearFEdge (int EXTILine);

// function to clear a pending rising edge flag
void _EXTI_ClearREdge (int EXTILine);

#endif
