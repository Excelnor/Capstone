// timer Library Implementation
//
// Simon Walker
//
// Basic timer utility methods
//
// NOTE: These functions assume a SYSCLK frequency of 64MHz
//
// Created Oct 2025
//
///////////////////////////////////////////////////////////////////////

#ifndef _TIMER_Lib
#define _TIMER_Lib

#include "stm32g0xx.h"

typedef enum
{
  _TIMER_Delay_Unit_ms = 0, // timer delay function specifies milliseconds
  _TIMER_Delay_Unit_us = 1  // timer delay function specifies microseconds
} _TIMER_Delay_Unit;

typedef enum
{
  _TIMER_Delay_Interrupt_Off,
  _TIMER_Delay_Interrupt_On
} _TIMER_Delay_Interrupt_Option;

// prototypes go here
void _TIMER_ClockOn (TIM_TypeDef * pTim);
void _TIMER_DelayStart (TIM_TypeDef * pTim, _TIMER_Delay_Unit unit, uint16_t count, _TIMER_Delay_Interrupt_Option iopt);
int _TIMER_DelayComplete (TIM_TypeDef * pTim);

#endif