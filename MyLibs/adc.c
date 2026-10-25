///////////////////////////////////////////////////////////////////////
//
// ADC Library
//
// Corrected for Part D external triggering by TIM3_TRGO.
//
// Key fix:
// In external-trigger mode, ADC must be armed with ADSTART
// after configuration so it waits for the timer trigger.
//
///////////////////////////////////////////////////////////////////////

#include "stm32g0xx.h"
#include "ADC.h"

static volatile uint16_t _ADC_LastResult = 0;
static volatile int      _ADC_ResultIsReady = 0;

static volatile uint32_t _ADC_AccumSum   = 0;
static volatile uint32_t _ADC_AccumCount = 0;

static volatile _ADC_ConversionInterruptOption _ADC_InterruptMode = _ADC_Interrupt_Disable;
static volatile _ADC_TriggerSelection          _ADC_TriggerMode   = _ADC_Trigger_Software;

static volatile int _ADC_CurrentChannel = 0;

////////////////////////////////////////////////////////////////////////
// Private helper: select one ADC channel
////////////////////////////////////////////////////////////////////////
static int _ADC_SelectChannel(int chan)
{
    if (chan < 0 || chan > 18)
        return 0;

    ADC1->CHSELR = (1UL << chan);
    _ADC_CurrentChannel = chan;
    return 1;
}

////////////////////////////////////////////////////////////////////////
// Public helper
////////////////////////////////////////////////////////////////////////
int _ADC_SetChannel(int chan)
{
    return _ADC_SelectChannel(chan);
}

////////////////////////////////////////////////////////////////////////
// ADC initialization
////////////////////////////////////////////////////////////////////////
void _ADC_Init(_ADC_ConversionInterruptOption interruptOption,
               _ADC_TriggerSelection trigsel)
{
    _ADC_InterruptMode = interruptOption;
    _ADC_TriggerMode   = trigsel;

    _ADC_LastResult    = 0;
    _ADC_ResultIsReady = 0;
    _ADC_AccumSum      = 0;
    _ADC_AccumCount    = 0;
    _ADC_CurrentChannel = 0;

    // Turn on ADC clock
    RCC->APBENR2 |= RCC_APBENR2_ADCEN_Msk;

    // Disable ADC before configuration if already enabled
    if (ADC1->CR & ADC_CR_ADEN_Msk)
    {
        ADC1->CR |= ADC_CR_ADDIS_Msk;
        while (ADC1->CR & ADC_CR_ADEN_Msk)
        { ;;; }
    }

    // Make sure ADC not busy
    while (ADC1->CR & ADC_CR_ADSTART_Msk)
    { ;;; }

    // Disable AUTOFF
    ADC1->CFGR1 &= ~ADC_CFGR1_AUTOFF_Msk;

    // Enable ADC regulator
    ADC1->CR |= ADC_CR_ADVREGEN_Msk;
    for (volatile int i = 0; i < 100000; i++)
    { ;;; }

    // Calibrate ADC
    ADC1->CR |= ADC_CR_ADCAL_Msk;
    while (ADC1->CR & ADC_CR_ADCAL_Msk)
    { ;;; }

    // Reset configuration
    ADC1->CFGR1 = 0;

    ////////////////////////////////////////////////////////////////////
    // Trigger configuration
    ////////////////////////////////////////////////////////////////////
    switch (trigsel)
    {
        case _ADC_Trigger_TIM3_TRGO:
            // EXTEN = 01 : trigger on rising edge
            ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0;

            // EXTSEL bits for TIM3_TRGO
            // This matches the setting your original code intended.
            ADC1->CFGR1 |= ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0;
            break;

        case _ADC_Trigger_Software:
        default:
            break;
    }

    ////////////////////////////////////////////////////////////////////
    // Interrupt configuration
    ////////////////////////////////////////////////////////////////////
    ADC1->IER = 0;

    if (interruptOption == _ADC_Interrupt_Enable)
    {
        ADC1->IER |= ADC_IER_EOCIE_Msk;
        NVIC_ClearPendingIRQ(ADC1_COMP_IRQn);
        NVIC_EnableIRQ(ADC1_COMP_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(ADC1_COMP_IRQn);
        NVIC_ClearPendingIRQ(ADC1_COMP_IRQn);
    }

    // Default channel 0
    _ADC_SelectChannel(0);

    // Clear flags
    ADC1->ISR = ADC_ISR_ADRDY_Msk |
                ADC_ISR_EOC_Msk   |
                ADC_ISR_EOSMP_Msk |
                ADC_ISR_OVR_Msk;

    // Enable ADC
    ADC1->CR |= ADC_CR_ADEN_Msk;
    while (!(ADC1->ISR & ADC_ISR_ADRDY_Msk))
    { ;;; }

    ////////////////////////////////////////////////////////////////////
    // IMPORTANT FIX:
    // In external-trigger mode, arm the ADC so it waits for TIM3_TRGO.
    ////////////////////////////////////////////////////////////////////
    if (trigsel == _ADC_Trigger_TIM3_TRGO)
    {
        ADC1->ISR = ADC_ISR_EOC_Msk | ADC_ISR_OVR_Msk;
        ADC1->CR |= ADC_CR_ADSTART_Msk;
    }
}

////////////////////////////////////////////////////////////////////////
// Start one software-triggered conversion
////////////////////////////////////////////////////////////////////////
void _ADC_Start(int chan)
{
    if (!_ADC_SelectChannel(chan))
        return;

    if (_ADC_TriggerMode != _ADC_Trigger_Software)
        return;

    _ADC_ResultIsReady = 0;
    ADC1->ISR = ADC_ISR_EOC_Msk | ADC_ISR_OVR_Msk;
    ADC1->CR |= ADC_CR_ADSTART_Msk;
}

////////////////////////////////////////////////////////////////////////
// Blocking helper
////////////////////////////////////////////////////////////////////////
uint16_t _ADC_Sample(int chan)
{
    if (!_ADC_SelectChannel(chan))
        return 0;

    if (_ADC_TriggerMode != _ADC_Trigger_Software)
        return _ADC_LastResult;

    _ADC_ResultIsReady = 0;
    ADC1->ISR = ADC_ISR_EOC_Msk | ADC_ISR_OVR_Msk;
    ADC1->CR |= ADC_CR_ADSTART_Msk;

    if (_ADC_InterruptMode == _ADC_Interrupt_Disable)
    {
        while (!(ADC1->ISR & ADC_ISR_EOC_Msk))
        { ;;; }

        _ADC_LastResult = (uint16_t)ADC1->DR;
        _ADC_ResultIsReady = 1;
        return _ADC_LastResult;
    }

    while (!_ADC_ResultIsReady)
    { ;;; }

    return _ADC_LastResult;
}

uint16_t _ADC_GetLatestResult(void)
{
    return _ADC_LastResult;
}

int _ADC_ResultReady(void)
{
    return _ADC_ResultIsReady;
}

void _ADC_ClearResultReady(void)
{
    _ADC_ResultIsReady = 0;
}

////////////////////////////////////////////////////////////////////////
// Snapshot and clear accumulation
////////////////////////////////////////////////////////////////////////
void _ADC_GetAndClearAccumulation(uint32_t *pSum,
                                  uint32_t *pCount,
                                  uint16_t *pLastResult)
{
    __disable_irq();

    if (pSum)
        *pSum = _ADC_AccumSum;

    if (pCount)
        *pCount = _ADC_AccumCount;

    if (pLastResult)
        *pLastResult = _ADC_LastResult;

    _ADC_AccumSum   = 0;
    _ADC_AccumCount = 0;

    __enable_irq();
}

////////////////////////////////////////////////////////////////////////
// ADC ISR
////////////////////////////////////////////////////////////////////////
void ADC1_COMP_IRQHandler(void)
{
    if (ADC1->ISR & ADC_ISR_OVR_Msk)
    {
        ADC1->ISR = ADC_ISR_OVR_Msk;
    }

    if (ADC1->ISR & ADC_ISR_EOC_Msk)
    {
        uint16_t sample = (uint16_t)ADC1->DR;

        _ADC_LastResult = sample;
        _ADC_ResultIsReady = 1;

        _ADC_AccumSum   += sample;
        _ADC_AccumCount += 1;
    }
}