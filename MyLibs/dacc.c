#include "dacc.h"
#include "GPIO.h"


//  clamp a value to 12-bit DAC range (0..4095)

static uint16_t _DAC_Clamp12Bit(uint16_t value)
{
    if (value > 4095)
    {
        value = 4095;
    }
    return value;
}


// Initialize DAC Channel 1 on PA4 using software trigger

void _DAC_Init_CH1(void)
{
    // Configure PA4 as analog for DAC1_OUT1
    _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_Analog, _GPIO_Clock_On);

    // Enable DAC peripheral clock
    RCC->APBENR1 |= RCC_APBENR1_DAC1EN_Msk;

    // Disable channel before configuring
    DAC1->CR &= ~DAC_CR_EN1_Msk;

    // Use software trigger
    DAC1->CR &= ~DAC_CR_TEN1_Msk;      // clear trigger enable first
    DAC1->CR &= ~DAC_CR_TSEL1_Msk;     // software trigger source = 000
    DAC1->CR |= DAC_CR_TEN1_Msk;       // enable trigger

    // Enable DAC channel 1
    DAC1->CR |= DAC_CR_EN1_Msk;

    // Default output = 0 V
    DAC1->DHR12R1 = 0;
    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1_Msk;
}

// Initialize DAC Channel 2 on PA5 using software trigger

void _DAC_Init_CH2(void)
{
    // Configure PA5 as analog for DAC1_OUT2
    _GPIO_SetPinMode(GPIOA, 5, _GPIO_PinMode_Analog, _GPIO_Clock_On);

    // Enable DAC peripheral clock
    RCC->APBENR1 |= RCC_APBENR1_DAC1EN_Msk;

    // Disable channel before configuring
    DAC1->CR &= ~DAC_CR_EN2_Msk;

    // Use software trigger
    DAC1->CR &= ~DAC_CR_TEN2_Msk;      // clear trigger enable first
    DAC1->CR &= ~DAC_CR_TSEL2_Msk;     // software trigger source = 000
    DAC1->CR |= DAC_CR_TEN2_Msk;       // enable trigger

    // Enable DAC channel 2
    DAC1->CR |= DAC_CR_EN2_Msk;

    // Default output = 0 V
    DAC1->DHR12R2 = 0;
    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG2_Msk;
}


// Initialize both DAC channels

void _DAC_Init_Both(void)
{
    _DAC_Init_CH1();
    _DAC_Init_CH2();
}


// Write a 12-bit value to DAC Channel 1 and software-trigger the update

void _DAC_Write_CH1(uint16_t value)
{
    value = _DAC_Clamp12Bit(value);

    DAC1->DHR12R1 = value;
    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1_Msk;
}

// Write a 12-bit value to DAC Channel 2 and software-trigger the update

void _DAC_Write_CH2(uint16_t value)
{
    value = _DAC_Clamp12Bit(value);

    DAC1->DHR12R2 = value;
    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG2_Msk;
}


// Generic channel write

void _DAC_Write(_DAC_Channel channel, uint16_t value)
{
    value = _DAC_Clamp12Bit(value);

    if (channel == _DAC_Channel_1)
    {
        DAC1->DHR12R1 = value;
        DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1_Msk;
    }
    else if (channel == _DAC_Channel_2)
    {
        DAC1->DHR12R2 = value;
        DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG2_Msk;
    }
}


// Write both channels

void _DAC_Write_Both(uint16_t ch1Value, uint16_t ch2Value)
{
    ch1Value = _DAC_Clamp12Bit(ch1Value);
    ch2Value = _DAC_Clamp12Bit(ch2Value);

    DAC1->DHR12R1 = ch1Value;
    DAC1->DHR12R2 = ch2Value;

    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1_Msk;
    DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG2_Msk;
}