///////////////////////////////////////////////////////////////////////
//
// Lab Exam #02 - Solution
//
// PART A - A/D min avg max output to USART @ 500ms intervals
//        - User button resets stats
// PART B - DAC triangle output waveform, transition on user
//        - button toggle between 1Vpp and 2Vpp (could be
//        - centre aligned, or 0-1V, 0-2V)
//
// Author - Simon Walker
//
// Created Apr 09 2026
//
// Revision history managed in GIT
//
///////////////////////////////////////////////////////////////////////
//
// Configuration Part A
//
// GPIO   - PA5, digital output, User LED
// GPIO   - PC13, digital input, GPIO/EXTI interrupt (user button)
// PA0    - ADC Channel 0
// TIMER  - TIM14, configured for 500ms intervals, interrupt enabled
// USART2 - Terminal output, main program UI
// CLOCK  - 64MHz, using PLL from HSI16
//
///////////////////////////////////////////////////////////////////////
//
// Configuration Part B
//
// GPIO   - PA5, digital output, User LED
// GPIO   - PC13, digital input, GPIO/EXTI interrupt (user button)
// PA4    - DAC Channel 1 output
// TIMER  - TIM14, configured for DAC output cadence
// CLOCK  - 64MHz, using PLL from HSI16
//
///////////////////////////////////////////////////////////////////////

//#include "stm32g0xx.h"
#include "gpio.h"
#include "clocks.h"
#include "timer.h"
#include "usart.h"
#include "adc.h"
#include <stdio.h>

#define PART_A
//#define PART_B

#ifdef PART_A

volatile uint64_t G_ADCSUM = 0;
volatile uint32_t G_ADCCount = 0;
volatile uint16_t G_ADCMin = 0;
volatile uint16_t G_ADCMax = 0;

_GPIO_PinDefinition G_UserLED = { GPIOA, 5 };
_GPIO_PinDefinition G_UserButton = { GPIOC, 13 };

// local helper
float ADCtoVolts (uint16_t adc_raw)
{
  return (adc_raw / 4095.0f) * 3.3f;
}


int main(void)
{
  // kick SYSCLK up to 64MHz, going forward, always assumed to be used
  _CLOCK_PLL_To_64(_CLOCK_PLL_CLOCK_SOURCE_INT);

  // bring up USART2
  _USART_Init_USART2(64E6, 38400);

  // user LED setup
  _GPIO_SetPinMode(G_UserLED.pPort, G_UserLED.PinNumber, _GPIO_PinMode_Output, _GPIO_Clock_On);

  // user button setup
  _GPIO_SetPinMode(G_UserButton.pPort, G_UserButton.PinNumber, _GPIO_PinMode_Input, _GPIO_Clock_On);

  // interrupt on conversion complete, but use software triggering
  _ADC_Init(_ADC_ConversionInterruptOptionOn, _ADC_TriggerSelection_NONE);

  // start a timer on 500ms intervals to do updates to terminal
  _TIMER_DelayStart(TIM14, _TIMER_Delay_Unit_ms, 500, _TIMER_Delay_Interrupt_Off);

  // begin main program loop
  do
  {
    // a user button clears the stats
    if (!_GPIO_GetPinIState(G_UserButton.pPort, G_UserButton.PinNumber))
    {
      G_ADCCount = 0; // this will clear out min/max on next sample
      G_ADCSUM = 0;
    }

    // if timer is done, show results on terminal
    if (_TIMER_DelayComplete(TIM14))
    {
      // snapshot values
      uint64_t sum = G_ADCSUM;
      uint32_t count = G_ADCCount;
      uint16_t min = G_ADCMin;
      uint16_t max = G_ADCMax;

      float avg = 0;
      if (count)
        avg = (float)sum / count;

      // update USART
      {
        char buff[100];
        (void)sprintf (buff, "Min: %3.3X (%6.2fV) ", min, ADCtoVolts(min));
        _USART_TxStringXY(USART2, 10, 5, buff);
        (void)sprintf (buff, "Avg: %3.3X (%6.2fV) ", (int)avg, ADCtoVolts((int)avg));
        _USART_TxStringXY(USART2, 10, 6, buff);
        (void)sprintf (buff, "Max: %3.3X (%6.2fV) ", max, ADCtoVolts(max));
        _USART_TxStringXY(USART2, 10, 7, buff);
        (void)sprintf (buff, "Samples: %10.0d", count);
        _USART_TxStringXY(USART2, 10, 9, buff);
      }  
      
      // POL
      _GPIO_PinDToggle(G_UserLED);
    }

    // go do another conversion (pretty sure the previous sample will be done by now)
    // using timer triggering would certainly make this more deterministic
    _ADC_StartConversion();
  }
  while (1);
}

// ISR for conversion complete
void ADC1_COMP_IRQHandler (void)
{
  // read the sample (this clears the EOC flag)
  uint16_t sample = ADC1->DR;
    
  // starting point values
  if (!G_ADCCount)
    G_ADCMax = G_ADCMin = sample;
  else
  {
    // min/max updates
    if (sample > G_ADCMax)
      G_ADCMax = sample;
    if (sample < G_ADCMin)
      G_ADCMin = sample;
  }

  // do variables update
  G_ADCSUM += sample;
  ++G_ADCCount;
}

#endif

#ifdef PART_B

_GPIO_PinDefinition G_UserLED = { GPIOA, 5 };
_GPIO_PinDefinition G_UserButton = { GPIOC, 13 };

// keep track of ramp for DAC
struct SRampVals
{
  int16_t value;
  enum eDir { Up, Down } dir;
} SRampVals = {Up, 0};

// local helper
int GetButtonState (void)
{
  return _GPIO_GetPinIState(G_UserButton.pPort, G_UserButton.PinNumber);
}

int main(void)
{  
  int16_t DACV2 = 2.0f / 3.3f * 4095; // 2V value
  int16_t DACV1 = 1.0f / 3.3f * 4095; // 1V value
  int16_t DACMAX = DACV2;             // current DAC max, starts as 2V

  // kick SYSCLK up to 64MHz, going forward, always assumed to be used
  _CLOCK_PLL_To_64(_CLOCK_PLL_CLOCK_SOURCE_INT);

    // user LED setup
  _GPIO_SetPinMode(G_UserLED.pPort, G_UserLED.PinNumber, _GPIO_PinMode_Output, _GPIO_Clock_On);

  // user button setup
  _GPIO_SetPinMode(G_UserButton.pPort, G_UserButton.PinNumber, _GPIO_PinMode_Input, _GPIO_Clock_On);
  int PreviousState = GetButtonState();

  // bring up DAC, channel 1, software trigger only
  RCC->APBENR1 |= RCC_APBENR1_DAC1EN;   // Enable DAC clock
  
  DAC1->CR &= ~DAC_CR_EN1;          // Disable DAC before config
  DAC1->CR &= ~DAC_CR_TEN1;         // Disable trigger first
  DAC1->CR |= DAC_CR_TEN1;          // Enable trigger
  DAC1->CR &= ~DAC_CR_TSEL1;        // Select trigger source (software)
  DAC1->CR |= DAC_CR_EN1;           // Enable DAC

  // a timer is not required, but makes the output waveform more stable
  // the delay added with the timer means that the step size on the DAC needs to be larger than 1
  _TIMER_DelayStart(TIM14, _TIMER_Delay_Unit_us, 100, _TIMER_Delay_Interrupt_Off);

  // begin main program loop
  do
  {
    if (_TIMER_DelayComplete(TIM14))
    {
      // set current ramp value
      DAC1->DHR12R1 = SRampVals.value;   
      DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;  // Trigger output

      // update DAC value based on direction
      if (SRampVals.dir == Up)
      {
        SRampVals.value = SRampVals.value + 64;
        if (SRampVals.value > DACMAX)
        {
          SRampVals.value = DACMAX - (SRampVals.value - DACMAX);
          SRampVals.dir = Down;
        }
      }
      else
      {
        SRampVals.value = SRampVals.value - 64;
        if (SRampVals.value < 0)
        {
          SRampVals.value = -SRampVals.value;
          SRampVals.dir = Up;
        }
      }
    }

    // check for reset of values
    int CurState = GetButtonState();
    if ((PreviousState != CurState) && (!CurState))
      DACMAX = DACMAX == DACV2 ? DACV1 : DACV2;
    PreviousState = CurState;
  }
  while (1);
}

#endif
