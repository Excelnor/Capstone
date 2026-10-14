// timer Library Implementation
//
// Jonathan Bukasa
//
//
// Basic timer utility methods
//
// NOTE: These functions assume a SYSCLK frequency of 64MHz
//
// Created Oct 2025
//
///////////////////////////////////////////////////////////////////////

#include "timer.h"
#include <stdio.h>
#include "stm32g0xx.h"

// turn on peripheral clock for the desired timer
void _TIMER_ClockOn (TIM_TypeDef * pTim)
{
  if (pTim == TIM14)
  {
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN_Msk;
    return;
  }
  if (pTim == TIM1)
  {
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN_Msk;
    return;
  }
  if (pTim == TIM2)
  {
    RCC->APBENR1 |= RCC_APBENR1_TIM2EN_Msk;
    return;
  }
  if (pTim == TIM3)
  {
    RCC->APBENR1 |= RCC_APBENR1_TIM3EN_Msk;
    return;
  }
  if (pTim == TIM16)
  {
    RCC->APBENR2 |= RCC_APBENR2_TIM16EN_Msk;
    return;
  }
  if (pTim == TIM17)
  {
    RCC->APBENR2 |= RCC_APBENR2_TIM17EN_Msk;
    return;
  }

  // other timers managed here...


  // break the debugger here - unsupported argument
  __BKPT(42);
}


void _TIMER_DelayStart (TIM_TypeDef * pTim, _TIMER_Delay_Unit unit, uint16_t count, _TIMER_Delay_Interrupt_Option iopt)
{
  // we need to decrease the count argument by 1 when loaded into the ARR register
  //  because the timer counts 0 and up to the end of the ARR value
  // ARR has special meaning when zero (timer frozen), so the minimum value for ARR is 1
  //  which means the shortest count possible is 2 counts when arr is 1
  // This means the smallest parameterized value for count would be 2
  if (count < 2)
   count = 2;


  // power up the peripheral clock for the timer
  _TIMER_ClockOn(pTim);


  // if a timer is already running, shut it down
  pTim->CR1 &= ~TIM_CR1_CEN_Msk; // flip the switch, turn off the timer
     
  // PSC is buffered, like it or not, so it does not take effect until the
  //  next update event (so we need to force one)
  // we don't care about this with continuous wave generation (first pulse is weird)
  //  but now we do, because this is potentially our only pulse.
  switch (unit)
  {
    case _TIMER_Delay_Unit_ms:
      pTim->PSC = 64000-1;     // 64_000_000/64_000 = 1kHz timer clock
      pTim->ARR = count - 1;   // ~1ms x ms interval
      break;
    case _TIMER_Delay_Unit_us:
      pTim->PSC = 64-1;        // 64_000_000 / 64 = 1MHz timer clock
      pTim->ARR = count - 1;   // units are us
      break;
  }
  
  if (iopt == _TIMER_Delay_Interrupt_On)
  {
      pTim->DIER &= ~TIM_DIER_UIE_Msk;
      pTim->DIER |= TIM_DIER_UIE_Msk;
      //NVIC_EnableIRQ(pTim);
  }
  else
      pTim->DIER &= ~TIM_DIER_UIE_Msk;


  // force an event, otherwise we will get one event really quickly with a prescale of 1
  pTim->EGR |= TIM_EGR_UG_Msk;


  // clear the flag that we will have just set by forcing the reload
  pTim->SR = ~TIM_SR_UIF_Msk;
 
  // flip the switch, turn on the timer  
  pTim->CR1 |= TIM_CR1_CEN_Msk;  
}


int _TIMER_DelayComplete (TIM_TypeDef * pTim)
{
  // check to see if the overflow flag is set
  if (pTim->SR & TIM_SR_UIF_Msk)
  {
    // yes, so clear it (another cycle has already started)
    pTim->SR = ~TIM_SR_UIF_Msk;


    // return 'true' this cycle is complete
    return 1;
  }


  // return 'false' this cycle is not complete
  return 0;
}

void _TIMER_PWM_Mode1_Setup(TIM_TypeDef * pTim, int Presc, int arr, int ccr1)
{
    _TIMER_ClockOn(pTim);
    pTim->PSC = Presc-1;                       // 64_000_000/64000 = 1kHz timer clock
    pTim->ARR = arr-1;                     // 1s interval : 1000 counts of 1ms
    pTim->CCR1 = ccr1;                       // channel 1 comparison value
    pTim->CCMR1 = 0b1101000;                // PWM mode 1, preload ON
    pTim->CCER |= TIM_CCER_CC1E_Msk;        // Enable Output. channel connected to the pin actual 
    pTim->EGR |= TIM_EGR_UG_Msk;            // reset is required for PWM
    pTim->SR = ~TIM_SR_UIF_Msk;             // clear the flag we just set
    pTim->CR1 |= TIM_CR1_CEN_Msk;           // flip the switch, turn on the timer
  
}

void _TIMER_PWM_Mode1_Setup_Channel(
    TIM_TypeDef *pTim,
    int channel,
    int Presc,
    int arr,
    int ccr)
{
    _TIMER_ClockOn(pTim);

    pTim->PSC = Presc - 1;
    pTim->ARR = arr - 1;

    if (channel == 1)
    {
        pTim->CCR1 = ccr;

        // CC1S = 00 → output
        // OC1M = 110 → PWM mode 1
        // OC1PE = 1 → preload
        pTim->CCMR1 &= ~(0xFF);
        pTim->CCMR1 |= 0b1101000;

        pTim->CCER |= TIM_CCER_CC1E_Msk;
    }
    else if (channel == 2)
    {
        pTim->CCR2 = ccr;

        // CH2 occupies bits 8-15 of CCMR1
        pTim->CCMR1 &= ~(0xFF << 8);
        pTim->CCMR1 |= (0b1101000 << 8);

        pTim->CCER |= TIM_CCER_CC2E_Msk;
    }
    else if (channel == 3)
    {
        pTim->CCR3 = ccr;

        // CH3 occupies bits 0-7 of CCMR2
        pTim->CCMR2 &= ~(0xFF);
        pTim->CCMR2 |= 0b1101000;

        pTim->CCER |= TIM_CCER_CC3E_Msk;
    }

    // Required for TIM1 output
    if (pTim == TIM1)
        pTim->BDTR |= TIM_BDTR_MOE;
}

void _TIMER_PWM_Start(TIM_TypeDef *pTim)
{
    pTim->EGR |= TIM_EGR_UG_Msk;
    pTim->SR = ~TIM_SR_UIF_Msk;
    pTim->CR1 |= TIM_CR1_CEN_Msk;
}

// Channel 1
void _TIMER_Input_Capture_Operation(TIM_TypeDef * pTim, int psc, int arr, _TIMER_Delay_Interrupt_Option iopt)
{
  _TIMER_ClockOn(pTim);
  pTim->PSC = psc-1;               // 64_000_000/PSC = Hz clock
  pTim->ARR = arr-1;               // ms interval : ARR counts of 1us
  pTim->CCMR1 = 0b00000001;       // input capture, no filter, no prescale
  pTim->CCER = TIM_CCER_CC1E_Msk; // input capture enabled to counter, rising-edge    pTim->CCER |= TIM_CCER_CC1P_Msk;       // falling-edge
  pTim->EGR |= TIM_EGR_UG_Msk;    // force reset to avoid a weird first event
  pTim->SR = ~TIM_SR_UIF_Msk;     // clear the flag we just set
  pTim->CR1 |= TIM_CR1_CEN_Msk;   // flip the switch, turn on the timer
  
    if (iopt == _TIMER_Delay_Interrupt_On)
  {
      pTim->DIER &= ~TIM_DIER_UIE_Msk;
      pTim->DIER |= TIM_DIER_UIE_Msk;
      //NVIC_EnableIRQ(pTim);
  }

  // Timer input capture interrupt
  //pTim->DIER &= ~TIM_DIER_CC1IE_Msk;
  //pTim->DIER |= TIM_DIER_CC1IE_Msk;
}

void _TIMER_Interrupt_Setup(IRQn_Type pTim)
{
 NVIC_EnableIRQ (pTim);  
  __enable_irq();
}


