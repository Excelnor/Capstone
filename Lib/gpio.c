// GPIO Library Implementation (Template Version 1.0)
//
// Jonathan Bukasa
//
//
// Library for GPIO ports
//
// < 10.2.2025 >
//
///////////////////////////////////////////////////////////////////////

#include "gpio.h"
#include <stdio.h>
#include "stm32g0xx.h"

// *required* system/user includes go here	(?)

// local (service) declarations go here		(?)

// implementations from the header go here	(?)

void _GPIO_ClockEnable (GPIO_TypeDef * pPort)
{
  if (pPort == GPIOA) 
  {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN_Msk; 
  }
  else if (pPort == GPIOB)
  {
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN_Msk; 
  }
  else if (pPort == GPIOC)
  {
        RCC->IOPENR |= RCC_IOPENR_GPIOCEN_Msk; 
  }
  else if (pPort == GPIOD)
  {
        RCC->IOPENR |= RCC_IOPENR_GPIODEN_Msk; 
  }
  else if (pPort == GPIOF)
  {
        RCC->IOPENR |= RCC_IOPENR_GPIOFEN_Msk; 
  }
  else
  {;;;}

}


void inline _GPIO_PinToggle (GPIO_TypeDef * pPort, int PinNumber)
{
//  //Validate that the port pin is valid (0-15)
  if (PinNumber < 0 || PinNumber > 15) {return;}
//  //Use the ODR register of the provided port to toggle the pin state (XOR operation)
  pPort->ODR ^= (1U << PinNumber);      // 0000001 unsigned << 6 : 1000000 Sets bit high for that pin.
//  //Checking that the pin is configured as an output is NOT required
}

void _GPIO_PinDToggle (_GPIO_PinDefinition PinDef)
{
  //  //Validate that the port pin is valid (0-15)
  if (PinDef.PinNumber < 0 || PinDef.PinNumber > 15) {return;}
//  //Use the ODR register of the provided port to toggle the pin state (XOR operation)
  PinDef.pPort->ODR ^= (1U << PinDef.PinNumber);      // 0000001 unsigned << 6 : 1000000 Sets bit high for that pin.
//  //Checking that the pin is configured as an output is NOT required
}

void inline _GPIO_PinSet (GPIO_TypeDef * pPort, int PinNumber)
{
//  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {return;}
//  //Use the BSRR register of the provided port to set the pin
    pPort->BSRR = (1U << PinNumber);
//  //Checking that the pin is configured as an output is NOT required
}

void _GPIO_PinDSet (_GPIO_PinDefinition PinDef)
{
  //  //Validate that the port pin is valid (0-15)
    if (PinDef.PinNumber < 0 || PinDef.PinNumber > 15) {return;}
//  //Use the BSRR register of the provided port to set the pin
    PinDef.pPort->BSRR = (1U << PinDef.PinNumber);
//  //Checking that the pin is configured as an output is NOT required
}


void inline _GPIO_PinClear (GPIO_TypeDef * pPort, int PinNumber)
{
//  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {return;}
//  //Use the BSRR register of the provided port to clear the pin
    pPort->BSRR = ~(1U << PinNumber);   
//  //Checking that the pin is configured as an output is NOT required
}

void _GPIO_PinDClear (_GPIO_PinDefinition PinDef)
{
 //  //Validate that the port pin is valid (0-15)
    if (PinDef.PinNumber < 0 || PinDef.PinNumber > 15) {return;}
//  //Use the BSRR register of the provided port to clear the pin
    PinDef.pPort->BSRR = ~(1U << PinDef.PinNumber);   
//  //Checking that the pin is configured as an output is NOT required
 
}

void _GPIO_SetPinMode (GPIO_TypeDef * pPort, int PinNumber, _GPIO_PinMode mode, _GPIO_ClockOption clkop)
{
    // Turn on clock  (CLOCK NEEDS TO BE ENABLED FIRST BEFORE SETTING UP THE MODE)
    if (clkop == _GPIO_Clock_On)
    {
      _GPIO_ClockEnable(pPort);
    }

//  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {return;}
//  //Clear the appropriate MODER bits for the provided pin
    pPort->MODER &= ~(3U << (PinNumber*2));     // 3U: 0b11 unsigned. Since moder 2 bits per pin: pinNumber times 2 

//  //Write the provided GPIO_PinMode bits to the appropriate MODER bits for the provided pin
    pPort->MODER |= ((mode & 3U) << (PinNumber * 2));
}

int _GPIO_GetPinIState (GPIO_TypeDef * pPort, int PinNumber)
{
//  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {;;;}
//  //Use the IDR register of the provided port to fetch the pin state
//  //Return a true value if the pin is set, return a false value if not
    if (pPort->IDR & (1U << PinNumber)){return 1;}
    else {return 0;}
}

int _GPIO_GetPinOState (GPIO_TypeDef * pPort, int PinNumber)
{
//  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {;;;}
//  //Use the ODR register of the provided port to fetch the buffered output state
//  //Return a true value if the buffered output state is set, return a false value if not
    if (pPort->ODR & (1U << PinNumber)){return 1;}
    else {return 0;}
}


int _GPIO_SetPinAlternateFunction(GPIO_TypeDef * pPort, unsigned int PinNumber, unsigned int FunctionNumber, _GPIO_ClockOption clkop)
{
  //Validate that the port pin is valid (0-15)
    if (PinNumber < 0 || PinNumber > 15) {return -1;}
  //Validate that the function number is valid (0-7)
    if (FunctionNumber < 0 || FunctionNumber > 7) {return -2;}
  // Turn on clock  
  if (clkop == _GPIO_Clock_On)
    _GPIO_ClockEnable(pPort);

  // Pins 0-7
  if (PinNumber<8)
  {
    // AFRL: altenate function register AF0
    pPort->AFR[0] &= ~(0xFU << (PinNumber*4));            // clear previous bits
    pPort->AFR[0] |= (FunctionNumber << (PinNumber * 4));           // Sets mode
  }
  else
  {
    // AFRH: AF1
    unsigned int pin = PinNumber - 8;    // sets pin index to a range of 0-7 instead of 8-15 so shifts aren't wrong
    pPort->AFR[1] &= ~(0xFU << (pin*4)); 
    pPort->AFR[1] |= (FunctionNumber << (pin * 4));
  }

  _GPIO_SetPinMode(pPort, PinNumber, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_On);

  return 0;   // success

}