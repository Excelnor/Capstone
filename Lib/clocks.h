// Clock library
//
// Functions to control the clocking options on the device
//
// Jonathan Bukasa
// Created Jan. 16 2025, initial build
//
// Version 1.1
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

typedef enum
{
  _CLOCK_PLL_CLOCK_SOURCE_INT = 0,
  _CLOCK_PLL_CLOCK_SOURCE_EXT = 1
} _CLOCK_PLL_CLOCK_SOURCE;


// Clock sources for MCO (bits 27:24)
typedef enum {
    MCO_NoClock  = 0,
    MCO_SYSCLK   = 1,
    MCO_HSI48    = 2, // STM32G0B1 only
    MCO_HSI16    = 3,
    MCO_HSE      = 4,
    MCO_PLLRCLK  = 5,
    MCO_LSI      = 6,
    MCO_LSE      = 7,
    MCO_PLLPCLK  = 8,
    MCO_PLLQCLK  = 9,
    MCO_RTCCLK   = 10,
    MCO_RTCWKUP  = 11
} MCO_Source;

// Prescaler values for MCO (bits 31:28)
typedef enum {
    MCO_DIV_1   = 0,
    MCO_DIV_2   = 1,
    MCO_DIV_4   = 2,
    MCO_DIV_8   = 3,
    MCO_DIV_16  = 4,
    MCO_DIV_32  = 5,
    MCO_DIV_64  = 6,
    MCO_DIV_128 = 7,
    MCO_DIV_256 = 8, // STM32G0B1 only
    MCO_DIV_512 = 9, // STM32G0B1 only
    MCO_DIV_1024= 10 // STM32G0B1 only
} MCO_Prescaler;

// bring up LSE if present
//int _CLOCK_LSE_ON (void);
int _LSE_ENABLE(void);

// standard, high perf mode @ 64MHz
//void _CLOCK_PLL_To_64 (_CLOCK_PLL_CLOCK_SOURCE clock_source);
void _CLOCK_PLL_TO_64 (void);
void _CLOCK_PLL_TO_32 (void);
void _CLOCK_PLL_TO_12 (void);

// custom, provide PLL values, will do nothing if out-of-range values provided
// M must leave PLL input 2.66MHz to 16MHz
// N must be 8 to 86, and must leave VCO 96MHz to 344MHz
// R must leave PLLR <= 64MHz
void _CLOCK_PLL_Custom (_CLOCK_PLL_M M, unsigned char N, _CLOCK_PLL_R R);
void _Clock_MCO_Setup(MCO_Source source, MCO_Prescaler prescaler);

#endif