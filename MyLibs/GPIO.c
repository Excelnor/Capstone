 #include "GPIO.h"

///////////////////////////////////////////////////////////////////////
// GPIO Library Implementation
// Hamid Jalil Njoya
// created on Jn 17 2026
// Updated GPIO
///////////////////////////////////////////////////////////////////////

// turn on port clock for desired port
void _GPIO_ClockEnable(GPIO_TypeDef *pPort)
{
  if (pPort == GPIOA)
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

  if (pPort == GPIOB)
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

  if (pPort == GPIOC)
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

   if (pPort == GPIOD)
    RCC->IOPENR |= RCC_IOPENR_GPIODEN;
}

// inline pin-definition versions 
void _GPIO_PinDToggle(_GPIO_PinDefinition PinDef)
{
  _GPIO_PinToggle(PinDef.pPort, PinDef.PinNumber);
}

void _GPIO_PinDSet(_GPIO_PinDefinition PinDef)
{
  _GPIO_PinSet(PinDef.pPort, PinDef.PinNumber);
}

void _GPIO_PinDClear(_GPIO_PinDefinition PinDef)
{
  _GPIO_PinClear(PinDef.pPort, PinDef.PinNumber);
}

// basic output manipulation
void _GPIO_PinToggle(GPIO_TypeDef *pPort, int PinNumber)
{
  pPort->ODR ^= (1u << PinNumber);
}

void _GPIO_PinSet(GPIO_TypeDef *pPort, int PinNumber)
{
  pPort->BSRR = (1u << PinNumber);
}

void _GPIO_PinClear(GPIO_TypeDef *pPort, int PinNumber)
{
  pPort->BSRR = (1u << (PinNumber + 16));
}

// set pin mode, with option to enable clock
void _GPIO_SetPinMode(GPIO_TypeDef *pPort, int PinNumber, _GPIO_PinMode mode, _GPIO_ClockOption clkop)
{
  if (clkop == _GPIO_Clock_On)
    _GPIO_ClockEnable(pPort);

  // clear the 2 mode bits
  pPort->MODER &= ~(0x3u << (2 * PinNumber));
  // set new mode
  pPort->MODER |= ((uint32_t)mode << (2 * PinNumber));
}

// read input state (IDR)
int _GPIO_GetPinIState(GPIO_TypeDef *pPort, int PinNumber)
{
  return (pPort->IDR & (1u << PinNumber));
}

// read output buffered state (ODR)
int _GPIO_GetPinOState(GPIO_TypeDef *pPort, int PinNumber)
{
  return (pPort->ODR & (1u << PinNumber));
}

// set output type: push-pull / open-drain
int _GPIO_Set_OutputType(GPIO_TypeDef *pPort, int PinNumber, _GPIO_OutputType otype)
{
  if (PinNumber < 0 || PinNumber > 15)
    return -1;

  if (otype == _GPIO_OutputType_OpenDrain)
    pPort->OTYPER |= (1u << PinNumber);
  else
    pPort->OTYPER &= ~(1u << PinNumber);

  return 0;
}

// set pull type: none / pull-up / pull-down
int _GPIO_Set_PullType(GPIO_TypeDef *pPort, int PinNumber, _GPIO_PullType ptype)
{
  if (PinNumber < 0 || PinNumber > 15)
    return -1;

  // clear 2 pull bits
  pPort->PUPDR &= ~(0x3u << (2 * PinNumber));
  // set new pull type
  pPort->PUPDR |= ((uint32_t)ptype << (2 * PinNumber));

  return 0;
}

// set alternate function for a pin (and optionally enable clock)
// NOTE:  should also set pin mode to AF using _GPIO_SetPinMode(...,_GPIO_PinMode_AlternateFunction,...)
int _GPIO_SetPinAlternateFunction(GPIO_TypeDef *pPort,unsigned int PinNumber,unsigned int FunctionNumber, _GPIO_ClockOption clkop)
{
  if (PinNumber > 15)
    return -1;

  if (FunctionNumber > 15)
    return -1;

  if (clkop == _GPIO_Clock_On)
    _GPIO_ClockEnable(pPort);

  if (PinNumber <= 7)
  {
    unsigned int shift = 4u * PinNumber;

    pPort->AFR[0] &= ~(0xFu << shift);
    pPort->AFR[0] |= ((uint32_t)FunctionNumber << shift);
  }
  else
  {
    unsigned int shift = 4u * (PinNumber - 8u);

    pPort->AFR[1] &= ~(0xFu << shift);
    pPort->AFR[1] |= ((uint32_t)FunctionNumber << shift);
  }

  return 0;
}
