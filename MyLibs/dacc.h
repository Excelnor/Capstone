#ifndef DAC_H
#define DAC_H

#include <stdint.h>
#include "stm32g0xx.h"

// DAC channel selection
typedef enum
{
    _DAC_Channel_1 = 1,
    _DAC_Channel_2 = 2
} _DAC_Channel;

// Basic initialization
void _DAC_Init_CH1(void);
void _DAC_Init_CH2(void);
void _DAC_Init_Both(void);

// Write single-channel outputs (12-bit right aligned)
void _DAC_Write_CH1(uint16_t value);
void _DAC_Write_CH2(uint16_t value);

// Generic write by channel
void _DAC_Write(_DAC_Channel channel, uint16_t value);

// Optional helper: write both channels at once
void _DAC_Write_Both(uint16_t ch1Value, uint16_t ch2Value);

#endif