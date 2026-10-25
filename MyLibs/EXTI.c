// EXTI Library
//
// Functions perform basic EXTI configuration
//
// Simon Walker
// Created April 02, 2026 - Initial Build
//
// Version 1.0
//
///////////////////////////////////////////////////////////////////////

#include "EXTI.h"

// configure a single EXTI line for port, trigger type, and interrupt support
void _EXTI_Set(int EXTILine, _EXTI_PORT port, _EXTI_TRIG trig, _EXTI_INT intoption)
{
    // ------------------------------------------------------------
    // basic range check
    // valid EXTI lines on this device are 0..15
    // ------------------------------------------------------------
    if (EXTILine < 0 || EXTILine > 15)
    {
        return;
    }

    // ------------------------------------------------------------
    // enable SYSCFG clock
    // needed to access EXTICR port-routing registers
    // ------------------------------------------------------------
    RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN_Msk;

    // ------------------------------------------------------------
    // route selected GPIO port to selected EXTI line
    //
    // EXTICR layout:
    //   EXTICR[0] -> EXTI 0..3
    //   EXTICR[1] -> EXTI 4..7
    //   EXTICR[2] -> EXTI 8..11
    //   EXTICR[3] -> EXTI 12..15
    //
    // each EXTI line uses 8 bits in STM32G0 EXTICR:
    // bits [7:0], [15:8], [23:16], [31:24]
    // ------------------------------------------------------------
    {
        int regindex = EXTILine / 4;          // which EXTICR register
        int shift    = (EXTILine % 4) * 8;    // which byte inside that register

        EXTI->EXTICR[regindex] &= ~(0xFFu << shift);
        EXTI->EXTICR[regindex] |=  ((uint32_t)port << shift);
    }

    // ------------------------------------------------------------
    // clear existing trigger settings first
    // then apply requested trigger mode
    // ------------------------------------------------------------
    EXTI->RTSR1 &= ~(1u << EXTILine);
    EXTI->FTSR1 &= ~(1u << EXTILine);

    switch (trig)
    {
        case _EXTI_TRIG_Rising:
            EXTI->RTSR1 |= (1u << EXTILine);
            break;

        case _EXTI_TRIG_Falling:
            EXTI->FTSR1 |= (1u << EXTILine);
            break;

        case _EXTI_TRIG_Both:
            EXTI->RTSR1 |= (1u << EXTILine);
            EXTI->FTSR1 |= (1u << EXTILine);
            break;

        case _EXTI_TRIG_None:
        default:
            // no trigger bits set
            break;
    }

    // ------------------------------------------------------------
    // interrupt enable / disable
    // IMR1 controls whether the EXTI line generates an interrupt
    // ------------------------------------------------------------
    if (intoption == _EXTI_INT_On)
    {
        EXTI->IMR1 |= (1u << EXTILine);

        // --------------------------------------------------------
        // enable correct NVIC IRQ group
        //
        // G0B1 grouping:
        //   EXTI0_1_IRQn   for lines 0..1
        //   EXTI2_3_IRQn   for lines 2..3
        //   EXTI4_15_IRQn  for lines 4..15
        // --------------------------------------------------------
        if (EXTILine <= 1)
        {
            NVIC_EnableIRQ(EXTI0_1_IRQn);
        }
        else if (EXTILine <= 3)
        {
            NVIC_EnableIRQ(EXTI2_3_IRQn);
        }
        else
        {
            NVIC_EnableIRQ(EXTI4_15_IRQn);
        }
    }
    else
    {
        EXTI->IMR1 &= ~(1u << EXTILine);

        // note:
        // we do not disable NVIC here, because another EXTI line in
        // the same IRQ group may still be using that interrupt.
    }

    // ------------------------------------------------------------
    // clear any stale pending flags so an old edge does not
    // immediately trigger after configuration
    // ------------------------------------------------------------
    EXTI->RPR1 = (1u << EXTILine);   // clear pending rising flag
    EXTI->FPR1 = (1u << EXTILine);   // clear pending falling flag
}


// function to clear a pending falling edge flag
void _EXTI_ClearFEdge(int EXTILine)
{
    if (EXTILine < 0 || EXTILine > 15)
    {
        return;
    }

    EXTI->FPR1 = (1u << EXTILine);
}


// function to clear a pending rising edge flag
void _EXTI_ClearREdge(int EXTILine)
{
    if (EXTILine < 0 || EXTILine > 15)
    {
        return;
    }

    EXTI->RPR1 = (1u << EXTILine);
}