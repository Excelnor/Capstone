
#include "CLOCK.h"
#include <stdio.h>
#include "stm32g0xx.h" 


void _CLOCK_PLL_To_64 (void)
{
  // PWR->CR1 :: set VOS to 01 (voltage scaling Range 1)
  // can't be cleared in place, so use a variable to do this change
  uint32_t cur = PWR->CR1; // can't write 00 or 11, so direct register mod not possible?  
  // set bit appropriately here
  cur &= ~PWR_CR1_VOS;        // clear VOS[1:0]
  cur |=  PWR_CR1_VOS_0;      // set VOS=01 (Range 1)
  PWR->CR1 = cur;
  
  // wait for VOSF in PWR->SR2 to go clear (4.4.6)
  while ( PWR->SR2 & PWR_SR2_VOSF  )
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
  cur = FLASH->ACR;
  cur &= ~FLASH_ACR_LATENCY_Msk;       // clear latency bits
  cur |=  FLASH_ACR_LATENCY_1;   // 2 wait states @ 64 MHz
  cur |=  FLASH_ACR_DBG_SWEN_Msk; // leave debugger access on
  cur |=  FLASH_ACR_ICEN_Msk;          // instruction cache enable
  cur |=  FLASH_ACR_PRFTEN_Msk;        // prefetch enable
  FLASH->ACR = cur;

  // wait for what we wrote to take effect
  // looking for flash latency bits to show what we wrote to them
  while ( (FLASH->ACR & FLASH_ACR_LATENCY_Msk) != FLASH_ACR_LATENCY_1  )
  {;;;} 

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
while (RCC->CR & RCC_CR_PLLRDY_Msk) //wait untill PLLRDY is cleared
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
  RCC->PLLCFGR = 0b00100110000000100000100000000010;

  // enable PLL R (main PLL) output in RCC->PLLCFGR
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN_Msk;
  // turn on PLL and wait for status to show PLL is locked
  // turn on PLL in RCC->CR
  RCC->CR |= RCC_CR_PLLON_Msk;
  // look for lock in RCC->CR with PLLRDY bit  
  while ( !(RCC->CR & RCC_CR_PLLRDY_Msk)  )
  {;;;}

  // select PLLR as clock source with RCC->CFGR register (SW bits)
    RCC->CFGR &= ~(RCC_CFGR_SW_Msk);       // clear SW bits
    RCC->CFGR |= RCC_CFGR_SW_1;       // 

  // note: this is fVCO = fPLLIN x (N/M) = 16MHz x (8/1) = 128MHz
  //       then fPPLR = fVCO / R = 128MHz / 2 = 64MHz
  // note: fVCO must fall between 96 and 344 MHz
  // note: PLL input frequency must be between 2.66 and 16 MHz after the /M stage
  
  // how could you generate SYSCLK rates of 20MHz, 32MHz, 40MHz, and 48MHz  
} 





void _CLOCK_PLL_Custom (_CLOCK_PLL_M M, unsigned char N, _CLOCK_PLL_R R)
{
    uint32_t cur;

   //range as in the question

    // N must be 8 to 86 
    if (N < 8 || N > 86)
        return;

    // M divider is (enum + 1) => M_1=0 -> /1, M_8=7 -> /8 
    uint32_t M_div = (uint32_t)M + 1U;

    // R divider is (enum + 1) => R_2=1 -> /2, R_8=7 -> /8 
    uint32_t R_div = (uint32_t)R + 1U;

    // PLL input frequency must be 2.66MHz to 16MHz after /M 
    const uint32_t HSI16 = 16000000UL;
    uint32_t f_pllin = HSI16 / M_div;
    if (f_pllin < 2660000UL || f_pllin > 16000000UL)
        return;

    // VCO must be 96MHz to 344MHz 
    uint32_t f_vco = f_pllin * (uint32_t)N;
    if (f_vco < 96000000UL || f_vco > 344000000UL)
        return;

    // PLLR must be <= 64MHz 
    uint32_t f_pllr = f_vco / R_div;
    if (f_pllr > 64000000UL)
        return;

   

    // Voltage scaling Range 1 (VOS=01) 
    cur = PWR->CR1;
    cur &= ~PWR_CR1_VOS;       // clear VOS[1:0]
    cur |=  PWR_CR1_VOS_0;     // set VOS=01 (Range 1)
    PWR->CR1 = cur;

    while (PWR->SR2 & PWR_SR2_VOSF)
    { ; }

    // FLASH wait states + cache/prefetch 
    cur = FLASH->ACR;
    cur &= ~FLASH_ACR_LATENCY_Msk;
    cur |=  FLASH_ACR_LATENCY_1;     // 2 wait states (safe for <=64 MHz)
    cur |=  FLASH_ACR_DBG_SWEN_Msk;
    cur |=  FLASH_ACR_ICEN_Msk;
    cur |=  FLASH_ACR_PRFTEN_Msk;
    FLASH->ACR = cur;

    while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != FLASH_ACR_LATENCY_1)
    { ; }

    // Ensure HSI16 ON and ready (PLL source) 
    RCC->CR |= RCC_CR_HSION_Msk;
    while (!(RCC->CR & RCC_CR_HSIRDY_Msk))
    { ; }

    // Disable PLL before changing parameters 
    RCC->CR &= ~RCC_CR_PLLON_Msk;
    while (RCC->CR & RCC_CR_PLLRDY_Msk)
    { ; }

    // Program PLLCFGR using ONLY M, N, R 
    uint32_t pllcfgr = 0;

    // PLLSRC = HSI16 
    pllcfgr |= RCC_PLLCFGR_PLLSRC_HSI;

    // PLLM field 
    pllcfgr &= ~RCC_PLLCFGR_PLLM_Msk;
    pllcfgr |= ((uint32_t)M << RCC_PLLCFGR_PLLM_Pos);

    // PLLN field 
    pllcfgr &= ~RCC_PLLCFGR_PLLN_Msk;
    pllcfgr |= ((uint32_t)N << RCC_PLLCFGR_PLLN_Pos);

    // PLLR field 
    pllcfgr &= ~RCC_PLLCFGR_PLLR_Msk;
    pllcfgr |= ((uint32_t)R << RCC_PLLCFGR_PLLR_Pos);

    // Disable P and Q outputs, enable R output 
    pllcfgr &= ~(RCC_PLLCFGR_PLLPEN_Msk | RCC_PLLCFGR_PLLQEN_Msk);
    pllcfgr |= RCC_PLLCFGR_PLLREN_Msk;

    RCC->PLLCFGR = pllcfgr;

    // Enable PLL and wait for lock 
    RCC->CR |= RCC_CR_PLLON_Msk;
    while (!(RCC->CR & RCC_CR_PLLRDY_Msk))
    { ; }

    // Switch SYSCLK to PLLR 
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_1;

    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_1)
    { ; }
}



#define LSE_STARTUP_TIMEOUT  500000U
int _CLOCK_LSE_ON(void)
{
    uint32_t timeout = 0;

    /* 1) Enable PWR peripheral clock (G0 uses APBENR1) */
    RCC->APBENR1 |= RCC_APBENR1_PWREN;

    /* 2) Enable backup domain write access */
    PWR->CR1 |= PWR_CR1_DBP;
    while ((PWR->CR1 & PWR_CR1_DBP) == 0U)
    {
        /* wait until backup domain is writable */
    }

    /* 3) (Optional but recommended) Set LSE drive strength */
    RCC->BDCR &= ~RCC_BDCR_LSEDRV;
    RCC->BDCR |=  RCC_BDCR_LSEDRV_1;   // medium-high drive

    /* 4) Enable LSE oscillator */
    RCC->BDCR |= RCC_BDCR_LSEON;

    /* 5) Wait for LSE ready with timeout */
    while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0U)
    {
        if (++timeout > LSE_STARTUP_TIMEOUT)
        {
            /* LSE failed to start */
            RCC->BDCR &= ~RCC_BDCR_LSEON;  // optional cleanup
            return -1;
        }
    }

     RCC->BDCR &= ~RCC_BDCR_RTCSEL_Msk;
    RCC->BDCR |=  RCC_BDCR_RTCSEL_0;   // RTC clock = LSE

    RCC->BDCR |= RCC_BDCR_RTCEN;

    return 0; // success
}