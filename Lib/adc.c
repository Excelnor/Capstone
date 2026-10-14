// ADC Library Starter
//
// Functions perform very basic A/D reading on channel 0
//
// Jonathan Bukasa
// Created March 2026, initial build, demonstration version only
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include "stm32g0xx.h"
#include "adc.h"

// expect the following code to provoke an actual reading:
// show A/D value from enabled channel
//{
//  // start a conversion
//  ADC1->CR |= ADC_CR_ADSTART_Msk;
//  while (!(ADC1->ISR & ADC_ISR_EOC_Msk))
//  { ;;; }
//  ADC1->DR <<< contains sample
//}

// on the stm32g0b1, the following pins default to the corresponding AD channels:
// ADC_IN0  - PA0
// ADC_IN1  - PA1
// ADC_IN2  - PA2
// ADC_IN3  - PA3
// ADC_IN4  - PA4
// ADC_IN5  - PA5
// ADC_IN6  - PA6
// ADC_IN7  - PA7
// ADC_IN8  - PB0
// ADC_IN9  - PB1
// ADC_IN10 - PB2
// ADC_IN11 - PB10
// ADC_IN12 -> internal temperature sensor
// ADC_IN13 -> internal voltage reference
// ADC_IN14 -> VBAT monitoring
// ADC_IN15 - PB11
// ADC_IN16 - PB12
// ADC_IN17 - PC4
// ADC_IN18 - PC5

// pin configuration (input should be default)
// make PA0 an analog input
// _GPIO_SetPinMode (GPIOA, 0, _GPIO_PinMode_Analog, _GPIO_Clock_On);

// brings up channel 0, manual sampling, no interrupt
void _ADC_Init (void)
{
  //////////////////////////////////////////////////////////////////////////////////////////
  // 5.2.11
  //The ADC clock is derived from the system clock, or from the PLLPCLK output. It can reach
  //122 MHz and can be divided by the following prescalers values:
  //1,2,4,6,8,10,12,16,32,64,128 or 256 by configuring the ADC1_CCR register. It is
  //asynchronous to the AHB clock. Alternatively, the ADC clock can be derived from the AHB
  //clock of the ADC bus interface, divided by a programmable factor (1, 2 or 4). This
  //programmable factor is configured using the CKMODE bitfields in the ADC1_CCR.
  //If the programmed factor is 1, the AHB prescaler must be set to 1.

  // we can just use the system clock for now (assuming 64MHz in most cases)

  // turn on the A/D peripheral clock
  RCC->APBENR2 |= RCC_APBENR2_ADCEN_Msk;

  // turn off the A/D (if this is reentrant)
  ADC1->CR &= ~ADC_CR_ADEN_Msk;

  // auto off should be off by default, but do it anyway
  // auto off should not be on for calibration of the A to D
  ADC1->CFGR1 &= ~ADC_CFGR1_AUTOFF_Msk;

  // 15.3.2 -> ADC Voltage Regulator
  ADC1->CR |= ADC_CR_ADVREGEN_Msk; // requires setup delay, lol, 20uS
  
  { for (int i = 0; i < 100000; ) { i++; }} // delay, hopefully defeat opt

  // begin A/D calibration (15.3.3)
  ADC1->CR |= ADC_CR_ADCAL_Msk;
  
  // wait for calibration to complete (this sets CALFACT)
  while (ADC1->CR & ADC_CR_ADCAL_Msk)
  { ;;; }

  //The devices embed a voltage reference buffer which can be used as voltage reference for
  //ADC, DAC and also as voltage reference for external components through the VREF+ pin.
  // NOTE: When the VREF+ pin is double-bonded with VDDA pin in a package, the voltage reference
  //buffer is not available and must be kept disabled (refer to datasheet for packages pinout
  //description).
  // so... not available in the packages with reduced pins
  
  //VREFBUF->CSR = 0b011; // use internal reference at 2.048V
  //while (!(VREFBUF->CSR & VREFBUF_CSR_VRR_Msk))
  //{ ;;; } // wait for A/D reference buffer to be ready

  // the basic version will use the chip supply voltage as the reference (3.3V)
  // full input range of 0V to 3.3V provide full range response (0x000-0xFFF)

  // turn on A/D 15.3.4
  ADC1->ISR = ADC_ISR_ADRDY_Msk; // clear pending flag
  ADC1->CR |= ADC_CR_ADEN_Msk;
  while (!(ADC1->ISR & ADC_ISR_ADRDY_Msk))
  { ;;; }

  // 15.3.8
  // while the A/D can do channel scanning, we will opt for a single channel conversion (for now)

  // enable channel 0 - associated with PA0
  ADC1->CFGR1 = 0; // default, single conversion, right-aligned, 12-bit resolution
  // NOTE: could turn on auto-off here to save power

  // enable only channel 0 in the conversion sequence
  ADC1->CHSELR = 0x00000001;
}

// C H A N N E L S : 0 - 18
uint16_t _ADC_Sample (int chan)
{
  if (chan == 0)
  {
    // start a conversion
    ADC1->CR |= ADC_CR_ADSTART_Msk;
    while (!(ADC1->ISR & ADC_ISR_EOC_Msk))
    { ;;; }

    // read channel data (configured only for this channel, currently)
    return ADC1->DR;
  }

  // not a supported channel... yet...
  return 0;
}

uint16_t ADC_ReadChannel(int chan)
{
    // select channel
    ADC1->CHSELR = (1 << chan);

    // start conversion
    ADC1->CR |= ADC_CR_ADSTART_Msk;

    // wait for conversion to finish
    while (!(ADC1->ISR & ADC_ISR_EOC_Msk)) { ;;; }

    // return result
    return ADC1->DR;
}


void ADC_TIM_TRG_Config(TIM_TypeDef * pTim, ADC_TRG_Select trig_Select, IRQn_Type pTim_IRQ)
{
    TIM3->CR2 &= TIM_CR2_MMS_Msk;   
    TIM3->CR2 |= TIM_CR2_MMS_1;     // Master mode selection Update bit
    
    // Configure the A/D to be triggered by TIM3_TRG0
    ADC1->CFGR1 &= ~(ADC_CFGR1_EXTEN_Msk | ADC_CFGR1_EXTSEL_Msk);
    ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0;                             // Enable External trigger
    ADC1->CFGR1 |= trig_Select;       // External trigger selection: TRG3 

    ADC1->CR |= ADC_CR_ADSTART_Msk;       // Start Sampling once
    ADC1->IER |= ADC_IER_EOCIE_Msk;       // Enable interrupts on A/D

    //NVIC_EnableIRQ(pTim_IRQ);
    //NVIC_EnableIRQ(ADC1_COMP_IRQn);       // Enable A/D NVIC
}