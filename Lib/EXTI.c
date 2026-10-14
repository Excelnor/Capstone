// GPIO Library Implementation (Template Version 1.0)
//
// Jonathan Bukasa
//
//
// Library for Exti Interrupts
//
// < 21.4.2026 >
//
///////////////////////////////////////////////////////////////////////

#include "EXTI.h"
#include <stdio.h>
#include "stm32g0xx.h"

// *required* system/user includes go here	(?)

// local (service) declarations go here		(?)

// implementations from the header go here	(?)

    ////// 15 14 13 12 :: EXTICR[3]
    ////// 11 10 09 08 :: EXTICR[2]
    ////// 07 06 05 04 :: EXTICR[1]
    ////// 03 02 01 00 :: EXTICR[0]
    // 0 x 00 00 00 00 :: 0x0->A,
                     //   0x1->B,
                    //    0x2->C, 
                     //   0x3->D, 
                     //   0x4->E, 
                     //   0x5->F 


// configure a single EXTI line for port, trigger type, and interrupt support
void _EXTI_Set (int EXTILine, _EXTI_PORT port, _EXTI_TRIG trig, _EXTI_INT intoption)
{
  // 1. Select EXTICR register (0–3) and position (4 bits per line)
    uint8_t index = EXTILine / 4;
    uint8_t shift = (EXTILine % 4) * 4;

    // 2. Clear then set port
    printf("%d\n\r", index);
    printf("0x%x\n\r", shift);

    EXTI->EXTICR[index] &= ~(0xFF << shift);
    EXTI->EXTICR[index] |=  (port << shift);

    // 3. Configure trigger
    if (trig & _EXTI_TRIG_Rising)
      EXTI->RTSR1 |= (1 << EXTILine);

    if (trig & _EXTI_TRIG_Falling)
      EXTI->FTSR1 |= (1 << EXTILine);

    // 4. Interrupt Config
    if (intoption == _EXTI_INT_On)
    {
        EXTI->IMR1 |= (1 << EXTILine);   // enable interrupt
    }
    else
    {
        EXTI->IMR1 &= ~(1 << EXTILine);  // disable interrupt
    }

    if (intoption == _EXTI_INT_On)
    {
        if (EXTILine <= 1)
            NVIC_EnableIRQ(EXTI0_1_IRQn);
        else if (EXTILine <= 3)
            NVIC_EnableIRQ(EXTI2_3_IRQn);
        else
            NVIC_EnableIRQ(EXTI4_15_IRQn);
    }
}

// function to clear a pending falling edge flag
void _EXTI_ClearFEdge(int EXTILine)
{
    EXTI->FPR1 = (1 << EXTILine);
}

// function to clear a pending rising edge flag
void _EXTI_ClearREdge(int EXTILine)
{
    EXTI->RPR1 = (1 << EXTILine);
}
