// Clock library
//
// Functions to control the clocking options on the device
//
// Hamid Jalil
// Created September 2024, initial build
//
// Version 1.0
//
//////////////////////////////////////////////////////////

#ifndef _CLOCK_Lib
#define _CLOCK_Lib

#include "inttypes.h"

typedef enum
{
  _CLOCK_PLL_R_2 = 1,
  _CLOCK_PLL_R_3 = 2,
  _CLOCK_PLL_R_4 = 3,
  _CLOCK_PLL_R_5 = 4,
  _CLOCK_PLL_R_6 = 5,
  _CLOCK_PLL_R_7 = 6,
  _CLOCK_PLL_R_8 = 7
} _CLOCK_PLL_R;

typedef enum
{
  _CLOCK_PLL_M_1 = 0,
  _CLOCK_PLL_M_2 = 1,
  _CLOCK_PLL_M_3 = 2,
  _CLOCK_PLL_M_4 = 3,
  _CLOCK_PLL_M_5 = 4,
  _CLOCK_PLL_M_6 = 5,
  _CLOCK_PLL_M_7 = 6,
  _CLOCK_PLL_M_8 = 7
} _CLOCK_PLL_M;

void _CLOCK_PLL_To_64 (void);

int _CLOCK_LSE_ON (void);

// custom, provide PLL values, will do nothing if out-of-range values provided
// M must leave PLL input 2.66MHz to 16MHz
// N must be 8 to 86, and must leave VCO 96MHz to 344MHz
// R must leave PLLR <= 64MHz
void _CLOCK_PLL_Custom (_CLOCK_PLL_M M, unsigned char N, _CLOCK_PLL_R R);

#endif