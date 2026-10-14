// ADC Library
//
// Functions perform very basic A/D reading (in development)
//
// Jonathan Bukasa
// Created March 2026, initial build, demonstration version only
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdio.h>
//#include "gpio.h"
//#include "timer.h"
//#include "clocks.h"
#include "usart.h"

typedef enum
{
    ADC_TRG_TIM1_TRGO2 = 0,                                                       // 000 (TRG0)
    ADC_TRG_TIM1_CC4   = ADC_CFGR1_EXTSEL_0,                                      // 001 (TRG1)
    ADC_TRG_TIM2_TRGO  = ADC_CFGR1_EXTSEL_1,                                      // 010 (TRG2)
    ADC_TRG_TIM3_TRGO  = ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1,                 // 011 (TRG3)
    ADC_TRG_TIM15_TRGO = ADC_CFGR1_EXTSEL_2,                                      // 100 (TRG4)
    ADC_TRG_TIM6_TRGO  = ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_2,                 // 101 (TRG5)
    ADC_TRG_TIM4_TRGO  = ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_2,                 // 110 (TRG6)
    ADC_TRG_EXTI11     = ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_2 // 111 (TRG7)

} ADC_TRG_Select;

void _ADC_Init (void);
uint16_t _ADC_Sample (int chan);
uint16_t ADC_ReadChannel(int chan);
void ADC_TIM_TRG_Config(TIM_TypeDef * pTim, ADC_TRG_Select trig_Select, IRQn_Type pTim_IRQ);