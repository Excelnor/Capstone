#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

typedef enum
{
    _ADC_Interrupt_Disable = 0,
    _ADC_Interrupt_Enable
} _ADC_ConversionInterruptOption;

typedef enum
{
    _ADC_Trigger_Software = 0,
    _ADC_Trigger_TIM3_TRGO
} _ADC_TriggerSelection;

void _ADC_Init(_ADC_ConversionInterruptOption interruptOption,
               _ADC_TriggerSelection trigsel);

int _ADC_SetChannel(int chan);

uint16_t _ADC_Sample(int chan);
void _ADC_Start(int chan);

uint16_t _ADC_GetLatestResult(void);
int      _ADC_ResultReady(void);
void     _ADC_ClearResultReady(void);

void _ADC_GetAndClearAccumulation(uint32_t *pSum,
                                  uint32_t *pCount,
                                  uint16_t *pLastResult);

#endif