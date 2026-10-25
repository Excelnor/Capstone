#include "TIMER.h"
#include <stdint.h>

volatile uint32_t testCNT = 0;
volatile uint32_t testCR1 = 0;
volatile uint32_t testSR = 0;

void _TIMER_ClockOn(TIM_TypeDef *pTim)
{
    if (pTim == TIM1)
        RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
    else if (pTim == TIM3)
        RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
    else if (pTim == TIM14)
        RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    else if (pTim == TIM16)
        RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
    else if (pTim == TIM17)
        RCC->APBENR2 |= RCC_APBENR2_TIM17EN;
}

void _TIMER_DelayStart(TIM_TypeDef *pTim,_TIMER_Delay_Unit unit,uint16_t count, _TIMER_Delay_Interrupt_Option iopt)
{
    _TIMER_ClockOn(pTim);

    // stop timer while configuring
    pTim->CR1 &= ~TIM_CR1_CEN;

    // clean state
    pTim->DIER &= ~TIM_DIER_UIE;
    pTim->CNT = 0;

    // set PSC/ARR for 64 MHz timer clock
    if (unit == _TIMER_Delay_Unit_us)
    {
        pTim->PSC = 63;         // 1 kHz tick  => 1 tick = 1 us
        pTim->ARR = (count > 0) ? (count - 1) : 0;
    }
    else
    {
        pTim->PSC = 63999;      // 1 kHz tick  => 1 tick = 1 ms
        pTim->ARR = (count > 0) ? (count - 1) : 0;
    }

    // latch PSC/ARR
    pTim->EGR |= TIM_EGR_UG;

    //  UG can set UIF -> clear UIF AFTER UG
    pTim->SR &= ~TIM_SR_UIF;

    // optional interrupt enable
    if (iopt == _TIMER_Delay_Interrupt_On)
        pTim->DIER |= TIM_DIER_UIE;

    // start timer
    pTim->CR1 |= TIM_CR1_CEN;
}

int _TIMER_DelayComplete(TIM_TypeDef *pTim)
{
    testCNT = pTim->CNT;
    testCR1 = pTim->CR1;
    testSR  = pTim->SR;

    if (pTim->SR & TIM_SR_UIF)
    {
        pTim->SR &= ~TIM_SR_UIF;
        pTim->CR1 &= ~TIM_CR1_CEN;
        return 1;
    }
    return 0;
}
