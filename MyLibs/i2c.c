// I2C library
//
// Basic I2C functions, intended to be used in device specific libraries
//
// Simon Walker
// Created September 2024, initial build
//
// Version 1.0 - Initial Build
// Version 1.1 - Fix for Read function - corrected order of Start/Autoend
// Version 1.2 - Added 'unstick' code in init to free the bus
// Version 1.3 - Added GPIO code for inits, added two clocking options
//
// RehabGrid integration changes (October 2026):
// - Keep the original public functions and PB67/PB89 pin choices.
// - Bound all transfer/scan waits; detect NACK and peripheral errors.
// - Wait for STOP and clear flags after hardware-ended transfers.
// - Preserve software-end write -> repeated-start read transactions.
// - Validate buffer arguments and handle errors consistently.
//
// Return values for StartWrite/StartRead:
//    0 success, -1 timeout/invalid argument, -2 NACK,
//   -3 bus/peripheral error.
//
// NOTE: I2C_POLL_LIMIT counts CPU polling iterations, NOT milliseconds.
// NOTE: This driver is synchronous and not reentrant; only one caller
//       may access I2C1 at a time.
////////////////////////////////////////////////////////////////////////

#include "i2c.h"
#include "gpio.h"
#include "stm32g0xx.h"
#include <stdint.h>
#include <stddef.h>

#define I2C_POLL_LIMIT  1000000UL

// Diagnostic: preserve the I2C status before error cleanup
volatile uint32_t debugI2CErrorISR = 0;

// Clear latched flags that would otherwise affect the next transaction.
static void I2C1_ClearFlags(void)
{
  I2C1->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF |
              I2C_ICR_BERRCF | I2C_ICR_ARLOCF |
              I2C_ICR_OVRCF;
}

// On failure, request STOP if needed and wait a bounded time for release.
// This is best-effort cleanup: if a device physically holds SCL/SDA low,
// re-initialization/bus recovery may still be required.
static void I2C1_Abort(void)
{
  if (I2C1->ISR & I2C_ISR_BUSY)
    I2C1->CR2 |= I2C_CR2_STOP;

  uint32_t timeout = I2C_POLL_LIMIT;
  while ((I2C1->ISR & I2C_ISR_BUSY) && timeout-- > 0U)
  {
  }

  I2C1_ClearFlags();
}

// Wait for a requested event, or return a negative error code.
// The STOPF check detects an unexpected early STOP (e.g., after a NACK).
static int I2C1_WaitFlag(uint32_t flag)
{
  uint32_t timeout = I2C_POLL_LIMIT;

  while (timeout-- > 0U)
  {
    uint32_t isr = I2C1->ISR;

    if (isr & I2C_ISR_NACKF)
    {
      I2C1_Abort();
      return -2;
    }

    if (isr & (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR))
{
    debugI2CErrorISR = isr;
    I2C1_Abort();
    return -3;
}
    if (isr & flag)
      return 0;

   if ((isr & I2C_ISR_STOPF) && flag != I2C_ISR_STOPF)
{
    debugI2CErrorISR = isr;
    I2C1_Abort();
    return -3;
}
  }

  I2C1_Abort();
  return -1;
}

// Wait for bus idle before starting a NEW transaction.
// Do not call this between a software-end write and its repeated-start read.
static int I2C1_WaitIdle(void)
{
  uint32_t timeout = I2C_POLL_LIMIT;
  while (I2C1->ISR & I2C_ISR_BUSY)
  {
    if (timeout-- == 0U)
      return -1;
  }
  return 0;
}

// typical init for Nucleo G031 board
// typical init for STMG041 proto board
// PB6 -> I2C1 SCL
// PB7 -> I2C1 SDA
int _I2C1_Init_PB67 (_I2C_SpeedMode smode)
{
  unsigned int const CLOCK_PIN_NUM = 6;
  unsigned int const DATA_PIN_NUM = 7;

  // run the clock for port B, or the pins won't work
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
  
  // dev
  RCC->APBENR1 |= RCC_APBENR1_I2C1EN; // turn on peripheral clock for I2C1 (5.4.15)
  
  // turn off, could be second run of init sequence
  I2C1->CR1 &= ~I2C_CR1_PE_Msk;

  // execute unstick clocking for devices that were interrupted while clocking out data
  // this should include clocking out enough pulses to transmit a byte with 
  // the data line idle? to form a nack condition so the device stops transmitting

  // TODO: if both lines are idle, this could probably be skipped...

  // During recovery, SCL must be open drain (never drive the bus high).
  _GPIO_Set_OutputType(GPIOB, CLOCK_PIN_NUM, _GPIO_OutputType_OpenDrain);
  _GPIO_PinSet(GPIOB, CLOCK_PIN_NUM);

  // GPIO up the pins to be non-alternate function (GPIO)
  // SCL should be output during bus recovery
  _GPIO_SetPinMode(GPIOB, CLOCK_PIN_NUM, _GPIO_PinMode_Output, _GPIO_Clock_On);
  // SDA should be input (should idle high)
  _GPIO_SetPinMode(GPIOB, DATA_PIN_NUM, _GPIO_PinMode_Input, _GPIO_Clock_On);

  // now clock out 12 slow pulses to end the clocking operation
  for (int i = 0; i < 12; ++i)
  {
    // go low
    _GPIO_PinClear(GPIOB, CLOCK_PIN_NUM);

    for (int k = 0; k < 1000; ++k) // want 10us pulses, measured @64MHz, this is ~ 280us
    {;;;}

    // go high
    _GPIO_PinSet(GPIOB, CLOCK_PIN_NUM);

    for (int k = 0; k < 1000; ++k)
    {;;;}
  }

  // now, back to our usual I2C init...
  _GPIO_SetPinMode(GPIOB, CLOCK_PIN_NUM, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_On);
  _GPIO_SetPinAlternateFunction(GPIOB, CLOCK_PIN_NUM, 6, _GPIO_Clock_On);
  _GPIO_SetPinMode(GPIOB, DATA_PIN_NUM, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_On);
  _GPIO_SetPinAlternateFunction(GPIOB, DATA_PIN_NUM, 6, _GPIO_Clock_On);

  // these pins must be configured in open drain mode for I2C operation
  _GPIO_Set_OutputType(GPIOB, CLOCK_PIN_NUM, _GPIO_OutputType_OpenDrain);
  _GPIO_Set_OutputType(GPIOB, DATA_PIN_NUM, _GPIO_OutputType_OpenDrain);

  // in case you forgot those resistors, but the internal pull-ups are waaaayyyy too weak
  _GPIO_Set_PullType(GPIOB, CLOCK_PIN_NUM, _GPIO_PullType_Up);
  _GPIO_Set_PullType(GPIOB, DATA_PIN_NUM, _GPIO_PullType_Up);

  // select HSI16 clock as the source for I2C1 (PCLK appears to be default)
  // this assumes that the HSU16 clock is turned on, but why would it not be?
  uint32_t ccipr = RCC->CCIPR;
  ccipr &= ~0b00000000000000000011000000000000;
  ccipr |=  0b00000000000000000010000000000000; // HSI16 as I2C1 source
  RCC->CCIPR = ccipr;

  // configure analog filter (or digital filter)
  I2C1->CR1 &= ~I2C_CR1_ANFOFF_Msk; // analog noise filter enabled

  // configure timing characteristics
  if (smode == _I2C_SpeedMode_100)
    I2C1->TIMINGR = 0x00503D58; // 100kHz from cube, tested on G031 Nucleo/G041 Proto (standard mode)
  else
  //if (smode == _I2C_SpeedMode_400)
    I2C1->TIMINGR = 0x00300617; // 400kHz from cube, tested on G031 Nucleo/G041 Proto (fast mode)
  
  // configure stretch settings
  I2C1->CR1 &= ~I2C_CR1_NOSTRETCH_Msk; // clock stretching enabled

  // turn on module
  I2C1->CR1 |= I2C_CR1_PE;

  return 0;
}



int _I2C1_Init_PB89(_I2C_SpeedMode smode)
{
  unsigned int const CLOCK_PIN_NUM = 8;
  unsigned int const DATA_PIN_NUM = 9;

  // run the clock for port B, or the pins won't work
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
  
  // dev
  RCC->APBENR1 |= RCC_APBENR1_I2C1EN; // turn on peripheral clock for I2C1 (5.4.15)
  
  // turn off, could be second run of init sequence
  I2C1->CR1 &= ~I2C_CR1_PE_Msk;

  // execute unstick clocking for devices that were interrupted while clocking out data
  // this should include clocking out enough pulses to transmit a byte with 
  // the data line idle? to form a nack condition so the device stops transmitting

  // TODO: if both lines are idle, this could probably be skipped...

  // During recovery, SCL must be open drain (never drive the bus high).
  _GPIO_Set_OutputType(GPIOB, CLOCK_PIN_NUM, _GPIO_OutputType_OpenDrain);
  _GPIO_PinSet(GPIOB, CLOCK_PIN_NUM);

  // GPIO up the pins to be non-alternate function (GPIO)
  // SCL should be output during bus recovery
  _GPIO_SetPinMode(GPIOB, CLOCK_PIN_NUM, _GPIO_PinMode_Output, _GPIO_Clock_On);
  // SDA should be input (should idle high)
  _GPIO_SetPinMode(GPIOB, DATA_PIN_NUM, _GPIO_PinMode_Input, _GPIO_Clock_On);

  // now clock out 12 slow pulses to end the clocking operation
  for (int i = 0; i < 12; ++i)
  {
    // go low
    _GPIO_PinClear(GPIOB, CLOCK_PIN_NUM);

    for (int k = 0; k < 1000; ++k) // want 10us pulses, measured @64MHz, this is ~ 280us
    {;;;}

    // go high
    _GPIO_PinSet(GPIOB, CLOCK_PIN_NUM);

    for (int k = 0; k < 1000; ++k)
    {;;;}
  }

  // now, back to our usual I2C init...
  _GPIO_SetPinMode(GPIOB, CLOCK_PIN_NUM, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_On);
  _GPIO_SetPinAlternateFunction(GPIOB, CLOCK_PIN_NUM, 6, _GPIO_Clock_On);
  _GPIO_SetPinMode(GPIOB, DATA_PIN_NUM, _GPIO_PinMode_AlternateFunction, _GPIO_Clock_On);
  _GPIO_SetPinAlternateFunction(GPIOB, DATA_PIN_NUM, 6, _GPIO_Clock_On);

  // these pins must be configured in open drain mode for I2C operation
  _GPIO_Set_OutputType(GPIOB, CLOCK_PIN_NUM, _GPIO_OutputType_OpenDrain);
  _GPIO_Set_OutputType(GPIOB, DATA_PIN_NUM, _GPIO_OutputType_OpenDrain);

  // in case you forgot those resistors, but the internal pull-ups are waaaayyyy too weak
  _GPIO_Set_PullType(GPIOB, CLOCK_PIN_NUM, _GPIO_PullType_Up);
  _GPIO_Set_PullType(GPIOB, DATA_PIN_NUM, _GPIO_PullType_Up);

  // select HSI16 clock as the source for I2C1 (PCLK appears to be default)
  // this assumes that the HSU16 clock is turned on, but why would it not be?
  uint32_t ccipr = RCC->CCIPR;
  ccipr &= ~0b00000000000000000011000000000000;
  ccipr |=  0b00000000000000000010000000000000; // HSI16 as I2C1 source
  RCC->CCIPR = ccipr;

  // configure analog filter (or digital filter)
  I2C1->CR1 &= ~I2C_CR1_ANFOFF_Msk; // analog noise filter enabled

  // configure timing characteristics
  if (smode == _I2C_SpeedMode_100)
    I2C1->TIMINGR = 0x00503D58; // 100kHz from cube, tested on G031 Nucleo/G041 Proto (standard mode)
  else
  //if (smode == _I2C_SpeedMode_400)
    I2C1->TIMINGR = 0x00300617; // 400kHz from cube, tested on G031 Nucleo/G041 Proto (fast mode)
  
  // configure stretch settings
  I2C1->CR1 &= ~I2C_CR1_NOSTRETCH_Msk; // clock stretching enabled

  // turn on module
  I2C1->CR1 |= I2C_CR1_PE;

  return 0;
}

int _I2C1_IsBusy(void)
{
  return (I2C1->ISR & I2C_ISR_BUSY);
}

int _I2C1_StartRead(unsigned char address, unsigned char bytecount,
                    unsigned char *pData, _I2C_AutoEndOption endoption)
{
  // A read may follow a software-ended write: the bus is intentionally
  // busy and TC is set. Therefore, do NOT wait for BUSY to clear here.
  if (address > 0x7FU || bytecount == 0U || pData == NULL)
    return -1;

  // Configure the 7-bit address, number of bytes and read direction.
  // Clear AUTOEND for now. The instructor's original read sequence
  // requests START before enabling AUTOEND for this repeated-start read.
  // This ordering is under hardware test; do not treat it as verified yet.
  uint32_t cr2 = I2C1->CR2;
  cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_NBYTES_Msk |
           I2C_CR2_RD_WRN_Msk | I2C_CR2_AUTOEND_Msk |
           I2C_CR2_RELOAD_Msk);
  cr2 |= ((uint32_t)address << 1);
  cr2 |= ((uint32_t)bytecount << I2C_CR2_NBYTES_Pos);
  cr2 |= I2C_CR2_RD_WRN;
  I2C1->CR2 = cr2;

  // Request START (repeated START after the register-address write).
  I2C1->CR2 |= I2C_CR2_START;

  // For a hardware-ended read, enable STOP generation after START.
  // This follows the instructor's Version 1.1 ordering.
  if (endoption == _I2C_AutoEnd_Hardware)
    I2C1->CR2 |= I2C_CR2_AUTOEND;

  for (unsigned int i = 0; i < bytecount; ++i)
  {
    int status = I2C1_WaitFlag(I2C_ISR_RXNE);
    if (status != 0)
      return status;

    pData[i] = (unsigned char)I2C1->RXDR;
  }

  if (endoption == _I2C_AutoEnd_Software)
  {
    // TC indicates that bytes are complete; leave bus active.
    return I2C1_WaitFlag(I2C_ISR_TC);
  }

  // Hardware AUTOEND issues STOP after the last received byte.
  int status = I2C1_WaitFlag(I2C_ISR_STOPF);
  if (status != 0)
    return status;

  I2C1_ClearFlags();
  return 0;
}

int _I2C1_StartWrite(unsigned char address, unsigned char bytecount,
                     unsigned char *pData, _I2C_AutoEndOption endoption)
{
  // A write starts a NEW transaction. A software-ended write is followed
  // by StartRead(), not by another StartWrite() in this API.
  if (address > 0x7FU || bytecount == 0U || pData == NULL)
    return -1;

  int status = I2C1_WaitIdle();
  if (status != 0)
    return status;

  I2C1_ClearFlags();

  // Configure 7-bit target address, NBYTES, write direction, AUTOEND.
  uint32_t cr2 = I2C1->CR2;
  cr2 &= ~(I2C_CR2_SADD_Msk | I2C_CR2_NBYTES_Msk |
           I2C_CR2_RD_WRN_Msk | I2C_CR2_AUTOEND_Msk |
           I2C_CR2_RELOAD_Msk);
  cr2 |= ((uint32_t)address << 1);
  cr2 |= ((uint32_t)bytecount << I2C_CR2_NBYTES_Pos);
  if (endoption == _I2C_AutoEnd_Hardware)
    cr2 |= I2C_CR2_AUTOEND;
  I2C1->CR2 = cr2;

  I2C1->CR2 |= I2C_CR2_START;

  // TXIS signals that TXDR can accept the next byte.
  // Check NACK, bus errors and timeout while waiting for each byte.
  for (unsigned int i = 0; i < bytecount; ++i)
  {
    status = I2C1_WaitFlag(I2C_ISR_TXIS);
    if (status != 0)
      return status;

    I2C1->TXDR = pData[i];
  }

  if (endoption == _I2C_AutoEnd_Software)
  {
    // No STOP: wait for TC so StartRead() can issue repeated START.
    return I2C1_WaitFlag(I2C_ISR_TC);
  }

  // Hardware AUTOEND: STOP is generated after the final byte.
  status = I2C1_WaitFlag(I2C_ISR_STOPF);
  if (status != 0)
    return status;

  I2C1_ClearFlags();
  return 0;
}

// Scan addresses 0x08..0x7D. Requires a 128-byte output array.
// Each address probe uses NBYTES=0, software end; ACK -> TC, NACK ->
// NACKF/STOPF. Always release bus before probing the next address.
void _I2C1_BusScan(unsigned char *pAddressTable)
{
  if (pAddressTable == NULL)
    return;

  for (unsigned int i = 0; i < 128U; ++i)
    pAddressTable[i] = 0;

  for (unsigned int addr = 0x08U; addr < 0x7EU; ++addr)
  {
    if (I2C1_WaitIdle() != 0)
    {
      I2C1_Abort();
      break; // Bus stuck: don't continue scanning.
    }

    I2C1_ClearFlags();

    // A zero-byte write transmits only the address.
    I2C1->CR2 = ((uint32_t)addr << 1);
    I2C1->CR2 |= I2C_CR2_START;

    uint32_t timeout = I2C_POLL_LIMIT;
    int acknowledged = 0;

    while (timeout-- > 0U)
    {
      uint32_t isr = I2C1->ISR;
      if (isr & (I2C_ISR_NACKF | I2C_ISR_STOPF |
                 I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR))
        break;
      if (isr & I2C_ISR_TC)
      {
        acknowledged = 1;
        break;
      }
    }

    if (acknowledged)
      pAddressTable[addr] = (unsigned char)addr;

    // Stop and clear flags whether the address ACKed or NACKed.
    I2C1_Abort();
  }
}
