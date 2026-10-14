// Clocks Library Implementation (Template Version 1.0)
//
// Jonathan Bukasa
//
//
// Library for Clocks
//
// < 10.2.2025 >
//
///////////////////////////////////////////////////////////////////////

#include "clocks.h"
#include <stdio.h>
#include "stm32g0xx.h"
#include "gpio.h"

// *required* system/user includes go here	(?)

// local (service) declarations go here		(?)

// implementations from the header go here	(?)

void _CLOCK_PLL_TO_64 (void)
{
 // Step 1: FLASH
 // PWR->CR1 :: set VOS to 01 (voltage scaling Range 1)
 // can't be cleared in place, so use a variable to do this change
 uint32_t cur = PWR->CR1; // can't write 00 or 11, so direct register mod not possible?  
 // set bit appropriately here
 cur &= ~(PWR_CR1_VOS_Msk);
 cur |= (1U << PWR_CR1_VOS_Pos);   // VOS = 01
 PWR->CR1 = cur;
  
 // wait for VOSF in PWR->SR2 to go clear (4.4.6)
 while (PWR->SR2 & PWR_SR2_VOSF)
 {;;;}

 // 3.3.4 Flash requirements for increasing frequency
 // at 64 MHz we need two wait states
 // 3.7.1
 // *also*   
 // turn on instruction cache and CPU prefetch
 // once again, don't try this directly in the register

 // set bits appropriately here
 // latency to 2 wait states
 // leave debugger access on
 // cpu instruction cache enable
 // cpu prefetch enable
 // wait for what we wrote to take effect
 // looking for flash latency bits to show what we wrote to them

 cur = FLASH->ACR;
 cur &= ~(FLASH_ACR_LATENCY_Msk);   // clear latency
 cur |= (2U << FLASH_ACR_LATENCY_Pos); // LATENCY = 2 WS
 cur |= FLASH_ACR_ICEN_Msk;             // instruction cache
 cur |= FLASH_ACR_PRFTEN_Msk;           // prefetch enable
 cur |= FLASH_ACR_DBG_SWEN_Msk;         // debugger allowed
 FLASH->ACR = cur;  

 while ((FLASH->ACR & FLASH_ACR_LATENCY) != 0b010)
 {;;;}

 // STEP 2: HS116
 // clear to go fast... need to set PLL and use PLLR as clock source
 // turn on the HSI16 clock, need it as source of PLL
 // this should be on and ready out of reset (do it, in case this isn't out of reset)

 // turn on HSI in RCC->CR
 RCC->CR |= RCC_CR_HSION_Msk;
  
 // wait for HSI to show on and ready (HSIRDY)
 while (!(RCC->CR & RCC_CR_HSIRDY_Msk))
 {;;;}

 // now, bring SYSCLK via PLLRCLK clock up to 64MHz
 // 5.2.4
 /*
   The PLL configuration (selection of the input clock and multiplication factor) must be done
   before enabling the PLL. Once the PLL is enabled, these parameters cannot be changed.
   To modify the PLL configuration, proceed as follows:
   1. Disable the PLL by setting PLLON to 0 in Clock control register (RCC_CR).
   2. Wait until PLLRDY is cleared. The PLL is now fully stopped.
   3. Change the desired parameter.
   4. Enable the PLL again by setting PLLON to 1.
   5. Enable the desired PLL outputs by configuring PLLPEN, PLLQEN, and PLLREN in PLL
   configuration register (RCC_PLLCFGR).
 */

 // turn off PLL (should be off already) with RCC->CR
 RCC->CR &= ~RCC_CR_PLLON_Msk;
  
 // wait for PLL to fully stop (PLLRDY in RCC->CR)
 while (RCC->CR & RCC_CR_PLLRDY_Msk)
 {;;;}
  
 // set PLL clock parameters in RCC->PLLCFGR
 // write the the entire register at once, setting  
 // PLLR    = / 2
 // PLLREN  = 0
 // PLLQ    = / 4
 // PLLQEN  = 0
 // PLLP    = /2
 // PLLPEN  = 0
 // PLLN    = * 8
 // PLLM    = / 1
 // PLLSRC  = HSI16
      
 //RCC->PLLCFGR = 0b00100110000000100000100000000010;      
 _CLOCK_PLL_Custom (_CLOCK_PLL_M_1, 8, _CLOCK_PLL_R_2);     // 64Mhz

 // enable PLL R (main PLL) output in RCC->PLLCFGR
 RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN_Msk;  
 // turn on PLL and wait for status to show PLL is locked
 // turn on PLL in RCC->CR
 RCC->CR |= RCC_CR_PLLON_Msk;
  
 // look for lock in RCC->CR with PLLRDY bit  
 while (!(RCC->CR & RCC_CR_PLLRDY_Msk))
 {;;;}

 // select PLLR as clock source with RCC->CFGR register (SW bits)
 RCC->CFGR &= ~(RCC_CFGR_SW_Msk);      // (SW / SWS)?
 RCC->CFGR |= RCC_CFGR_SW_1;       // PLL

 // note: this is fVCO = fPLLIN x (N/M) = 16MHz x (8/1) = 128MHz
 //       then fPPLR = fVCO / R = 128MHz / 2 = 64MHz
 // note: fVCO must fall between 96 and 344 MHz
 // note: PLL input frequency must be between 2.66 and 16 MHz after the /M stage
  
 // how could you generate SYSCLK rates of 20MHz, 32MHz, 40MHz, and 48MHz
 // 20: M=/1, N=10, R=/8   32: M=/1, N=8, R=/4   40: M=/1, N=10, R=/4   48: M=/1, N=12, R=/4   
}

void _CLOCK_PLL_TO_32 (void)
{
 // Step 1: FLASH
 // PWR->CR1 :: set VOS to 01 (voltage scaling Range 1)
 // can't be cleared in place, so use a variable to do this change
 uint32_t cur = PWR->CR1; // can't write 00 or 11, so direct register mod not possible?  
 // set bit appropriately here
 cur &= ~(PWR_CR1_VOS_Msk);
 cur |= (1U << PWR_CR1_VOS_Pos);   // VOS = 01
 PWR->CR1 = cur;
  
 // wait for VOSF in PWR->SR2 to go clear (4.4.6)
 while (PWR->SR2 & PWR_SR2_VOSF)
 {;;;}

 // 3.3.4 Flash requirements for increasing frequency
 // at 64 MHz we need two wait states
 // 3.7.1
 // *also*   
 // turn on instruction cache and CPU prefetch
 // once again, don't try this directly in the register

 // set bits appropriately here
 // latency to 2 wait states
 // leave debugger access on
 // cpu instruction cache enable
 // cpu prefetch enable
 // wait for what we wrote to take effect
 // looking for flash latency bits to show what we wrote to them

 cur = FLASH->ACR;
 cur &= ~(FLASH_ACR_LATENCY_Msk);   // clear latency
 cur |= (2U << FLASH_ACR_LATENCY_Pos); // LATENCY = 2 WS
 cur |= FLASH_ACR_ICEN_Msk;             // instruction cache
 cur |= FLASH_ACR_PRFTEN_Msk;           // prefetch enable
 cur |= FLASH_ACR_DBG_SWEN_Msk;         // debugger allowed
 FLASH->ACR = cur;  

 while ((FLASH->ACR & FLASH_ACR_LATENCY) != 0b010)
 {;;;}

 // STEP 2: HS116
 // clear to go fast... need to set PLL and use PLLR as clock source
 // turn on the HSI16 clock, need it as source of PLL
 // this should be on and ready out of reset (do it, in case this isn't out of reset)

 // turn on HSI in RCC->CR
 RCC->CR |= RCC_CR_HSION_Msk;
  
 // wait for HSI to show on and ready (HSIRDY)
 while (!(RCC->CR & RCC_CR_HSIRDY_Msk))
 {;;;}

 // now, bring SYSCLK via PLLRCLK clock up to 64MHz
 // 5.2.4
 /*
   The PLL configuration (selection of the input clock and multiplication factor) must be done
   before enabling the PLL. Once the PLL is enabled, these parameters cannot be changed.
   To modify the PLL configuration, proceed as follows:
   1. Disable the PLL by setting PLLON to 0 in Clock control register (RCC_CR).
   2. Wait until PLLRDY is cleared. The PLL is now fully stopped.
   3. Change the desired parameter.
   4. Enable the PLL again by setting PLLON to 1.
   5. Enable the desired PLL outputs by configuring PLLPEN, PLLQEN, and PLLREN in PLL
   configuration register (RCC_PLLCFGR).
 */

 // turn off PLL (should be off already) with RCC->CR
 RCC->CR &= ~RCC_CR_PLLON_Msk;
  
 // wait for PLL to fully stop (PLLRDY in RCC->CR)
 while (RCC->CR & RCC_CR_PLLRDY_Msk)
 {;;;}
  
 // set PLL clock parameters in RCC->PLLCFGR
 // write the the entire register at once, setting  
 // PLLR    = / 2
 // PLLREN  = 0
 // PLLQ    = / 4
 // PLLQEN  = 0
 // PLLP    = /2
 // PLLPEN  = 0
 // PLLN    = * 8
 // PLLM    = / 1
 // PLLSRC  = HSI16
      
 //RCC->PLLCFGR = 0b00100110000000100000100000000010;      
 _CLOCK_PLL_Custom (_CLOCK_PLL_M_1, 8, _CLOCK_PLL_R_4);     

 // enable PLL R (main PLL) output in RCC->PLLCFGR
 RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN_Msk;  
 // turn on PLL and wait for status to show PLL is locked
 // turn on PLL in RCC->CR
 RCC->CR |= RCC_CR_PLLON_Msk;
  
 // look for lock in RCC->CR with PLLRDY bit  
 while (!(RCC->CR & RCC_CR_PLLRDY_Msk))
 {;;;}

 // select PLLR as clock source with RCC->CFGR register (SW bits)
 RCC->CFGR &= ~(RCC_CFGR_SW_Msk);      // (SW / SWS)?
 RCC->CFGR |= RCC_CFGR_SW_1;       // PLL

 // note: this is fVCO = fPLLIN x (N/M) = 16MHz x (8/1) = 128MHz
 //       then fPPLR = fVCO / R = 128MHz / 2 = 64MHz
 // note: fVCO must fall between 96 and 344 MHz
 // note: PLL input frequency must be between 2.66 and 16 MHz after the /M stage
  
 // how could you generate SYSCLK rates of 20MHz, 32MHz, 40MHz, and 48MHz
 // 20: M=/1, N=10, R=/8   32: M=/1, N=8, R=/4   40: M=/1, N=10, R=/4   48: M=/1, N=12, R=/4   
}

void _CLOCK_PLL_TO_12 (void)
{
 // Step 1: FLASH
 // PWR->CR1 :: set VOS to 01 (voltage scaling Range 1)
 // can't be cleared in place, so use a variable to do this change
 uint32_t cur = PWR->CR1; // can't write 00 or 11, so direct register mod not possible?  
 // set bit appropriately here
 cur &= ~(PWR_CR1_VOS_Msk);
 cur |= (1U << PWR_CR1_VOS_Pos);   // VOS = 01
 PWR->CR1 = cur;
  
 // wait for VOSF in PWR->SR2 to go clear (4.4.6)
 while (PWR->SR2 & PWR_SR2_VOSF)
 {;;;}

 // 3.3.4 Flash requirements for increasing frequency
 // at 64 MHz we need two wait states
 // 3.7.1
 // *also*   
 // turn on instruction cache and CPU prefetch
 // once again, don't try this directly in the register

 // set bits appropriately here
 // latency to 2 wait states
 // leave debugger access on
 // cpu instruction cache enable
 // cpu prefetch enable
 // wait for what we wrote to take effect
 // looking for flash latency bits to show what we wrote to them

 cur = FLASH->ACR;
 cur &= ~(FLASH_ACR_LATENCY_Msk);   // clear latency
 cur |= (2U << FLASH_ACR_LATENCY_Pos); // LATENCY = 2 WS
 cur |= FLASH_ACR_ICEN_Msk;             // instruction cache
 cur |= FLASH_ACR_PRFTEN_Msk;           // prefetch enable
 cur |= FLASH_ACR_DBG_SWEN_Msk;         // debugger allowed
 FLASH->ACR = cur;  

 while ((FLASH->ACR & FLASH_ACR_LATENCY) != 0b010)
 {;;;}

 // STEP 2: HS116
 // clear to go fast... need to set PLL and use PLLR as clock source
 // turn on the HSI16 clock, need it as source of PLL
 // this should be on and ready out of reset (do it, in case this isn't out of reset)

 // turn on HSI in RCC->CR
 RCC->CR |= RCC_CR_HSION_Msk;
  
 // wait for HSI to show on and ready (HSIRDY)
 while (!(RCC->CR & RCC_CR_HSIRDY_Msk))
 {;;;}

 // now, bring SYSCLK via PLLRCLK clock up to 64MHz
 // 5.2.4
 /*
   The PLL configuration (selection of the input clock and multiplication factor) must be done
   before enabling the PLL. Once the PLL is enabled, these parameters cannot be changed.
   To modify the PLL configuration, proceed as follows:
   1. Disable the PLL by setting PLLON to 0 in Clock control register (RCC_CR).
   2. Wait until PLLRDY is cleared. The PLL is now fully stopped.
   3. Change the desired parameter.
   4. Enable the PLL again by setting PLLON to 1.
   5. Enable the desired PLL outputs by configuring PLLPEN, PLLQEN, and PLLREN in PLL
   configuration register (RCC_PLLCFGR).
 */

 // turn off PLL (should be off already) with RCC->CR
 RCC->CR &= ~RCC_CR_PLLON_Msk;
  
 // wait for PLL to fully stop (PLLRDY in RCC->CR)
 while (RCC->CR & RCC_CR_PLLRDY_Msk)
 {;;;}
  
 // set PLL clock parameters in RCC->PLLCFGR
 // write the the entire register at once, setting  
 // PLLR    = / 2
 // PLLREN  = 0
 // PLLQ    = / 4
 // PLLQEN  = 0
 // PLLP    = /2
 // PLLPEN  = 0
 // PLLN    = * 8
 // PLLM    = / 1
 // PLLSRC  = HSI16
      
 //RCC->PLLCFGR = 0b00100110000000100000100000000010;      
 _CLOCK_PLL_Custom (_CLOCK_PLL_M_2, 12, _CLOCK_PLL_R_8);     

 // enable PLL R (main PLL) output in RCC->PLLCFGR
 RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN_Msk;  
 // turn on PLL and wait for status to show PLL is locked
 // turn on PLL in RCC->CR
 RCC->CR |= RCC_CR_PLLON_Msk;
  
 // look for lock in RCC->CR with PLLRDY bit  
 while (!(RCC->CR & RCC_CR_PLLRDY_Msk))
 {;;;}

 // select PLLR as clock source with RCC->CFGR register (SW bits)
 RCC->CFGR &= ~(RCC_CFGR_SW_Msk);      // (SW / SWS)?
 RCC->CFGR |= RCC_CFGR_SW_1;       // PLL

 // note: this is fVCO = fPLLIN x (N/M) = 16MHz x (8/1) = 128MHz
 //       then fPPLR = fVCO / R = 128MHz / 2 = 64MHz
 // note: fVCO must fall between 96 and 344 MHz
 // note: PLL input frequency must be between 2.66 and 16 MHz after the /M stage
  
 // how could you generate SYSCLK rates of 20MHz, 32MHz, 40MHz, and 48MHz
 // 20: M=/1, N=10, R=/8   32: M=/1, N=8, R=/4   40: M=/1, N=10, R=/4   48: M=/1, N=12, R=/4   
}


int _LSE_ENABLE(void)
{
  unsigned long reg = RCC->BDCR;
  //printf("LSEON bit: %lu\n", (reg >> 0) & 0x1);    // bit 0 (reg >> n -> n: bit number)
  //printf("LSERDY bit: %lu\n", (reg >> 1) & 0x1);    // bit 1 (reg >> n -> n: bit number)

  RCC-> APBENR1 |= RCC_APBENR1_PWREN_Msk;            // Enable Power Interface Clock 
  RCC-> APBENR1 |= RCC_APBENR1_RTCAPBEN_Msk;         // Enable RTC APB Clock
  PWR->CR1 |= PWR_CR1_DBP_Msk;                       // Disable RTC domain write protection | Enable write access
  
  //reg = RCC->BDCR;
  //for (int i = 31; i >= 0; i--) 
  //{
  //  printf("%lu", (reg >> i) & 1);
  //}
  //printf("\n");

  RCC->BDCR |= RCC_BDCR_LSEON_Msk;                   // Enable LSEON bit

  reg = RCC->BDCR;
  //printf("LSEON bit: %lu\n", (reg >> 0) & 0x1);    // bit 0 (reg >> n -> n: bit number)
  //printf("LSERDY bit: %lu\n", (reg >> 1) & 0x1);

  long timeout = 0;
  
  // wait for LSERDY bit 
  while (!(RCC->BDCR & RCC_BDCR_LSERDY_Msk))
  {
    if (timeout++ > 5E9)         // around a minute if my math is correct
    {
      //RCC->BDCR &= ~(RCC_BDCR_LSEON_Msk);
      //PWR->CR1 |= (PWR_CR1_DBP_Msk);
      return 0;      // Fail
    }
  }

  reg = RCC->BDCR;
  //printf("LSERDY bit: %lu\n", (reg >> 1) & 0x1);

  // Optional LSE as clock source
  //RCC->BDCR &= ~RCC_BDCR_RTCSEL_Msk;    // Clear current RTC source
  //RCC->BDCR |= RCC_BDCR_BDRST_Msk;     // domain reset
  //RCC->BDCR &= ~RCC_BDCR_BDRST_Msk;
  //RCC->BDCR |= RCC_BDCR_RTCSEL_0;       // Select LSE       

  // select the LSE as the RTC clock source
  if (!(RCC->BDCR & RCC_BDCR_RTCSEL_Msk))
  {
    // no clock selected, proceed
    RCC->BDCR |= RCC_BDCR_RTCSEL_0;
    RCC->BDCR |= RCC_BDCR_RTCEN_Msk;
  } 

  //Enable the RTC domain write protection | disable write access
  PWR->CR1 &= ~PWR_CR1_DBP_Msk;
  return 1;      // success     
}

void _Clock_MCO_Setup(MCO_Source source, MCO_Prescaler prescaler)
{
    // Clear MCOPRE (31:28) and MCOSEL (27:24)
    RCC->CFGR &= ~((0xF << 28) | (0xF << 24));

    // Set prescaler and source
    RCC->CFGR |= ((prescaler & 0xF) << 28) | ((source & 0xF) << 24);
  
    _GPIO_SetPinAlternateFunction(GPIOA, 8, 0, _GPIO_Clock_On); 
}

// custom, provide PLL values, will do nothing if out-of-range values provided
// M must leave PLL input 2.66MHz to 16MHz
// N must be 8 to 86, and must leave VCO 96MHz to 344MHz
// R must leave PLLR <= 64MHz
void _CLOCK_PLL_Custom (_CLOCK_PLL_M M, unsigned char N, _CLOCK_PLL_R R)
{
  float fin = 16000000.0/(M+1);
  float fvco = fin*N;
  float fout = fvco/(R+1);

  if (fin < 2660000.0 || fin > 16000000.0)        // 2.66MHz-16MHz
    return;
  
  if (N > 86 || N < 8)          // 8-86
    return;
  
  if (fvco > 344000000.0 || fvco < 96000000.0)       // 96MHz-344MHz
    return;
  
  if (fout > 64000000.0)        // <= 64MHz
    return;


  unsigned int cfgrtemp = 0;

  cfgrtemp &= ~(RCC_PLLCFGR_PLLR_Msk);           // PLLR    = / 2
  cfgrtemp |= ((R) << (RCC_PLLCFGR_PLLR_Pos));
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLREN_Msk);         // PLLREN  = 0
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLQ_Msk);
  cfgrtemp |= (3 << (RCC_PLLCFGR_PLLQ_Pos));      // 3: 0b011 = /4
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLQEN_Msk);         // PLLQEN  = 0
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLP_Msk);
  cfgrtemp |= (1 << RCC_PLLCFGR_PLLP_Pos);        // 1: 0b00001 = /2
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLPEN_Msk);           // PLLPEN  = 0
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLN_Msk);
  cfgrtemp |= ((N) << RCC_PLLCFGR_PLLN_Pos);           // PLLN    = * 8
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLM_Msk);
  cfgrtemp |= ((M) << RCC_PLLCFGR_PLLM_Pos);           // PLLM    = / 1
  
  cfgrtemp &= ~(RCC_PLLCFGR_PLLSRC_Msk);
  cfgrtemp |= (2 << RCC_PLLCFGR_PLLSRC_Pos);      // 2: 0b10 = HSI16

  RCC->PLLCFGR = cfgrtemp;
}