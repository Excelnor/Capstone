/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include "GPIO.h"
#include "TIMER.h"
#include "CLOCK.h"

/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/


// Onboard user LED: PA5
_GPIO_PinDefinition UserLED = {GPIOA, 5};

int main(void)
{
    // Configure system clock to 64 MHz
    _CLOCK_PLL_To_64();

    // Configure PA5 as an output and enable GPIOA clock
    _GPIO_SetPinMode(UserLED.pPort,
                     UserLED.PinNumber,
                     _GPIO_PinMode_Output,
                     _GPIO_Clock_On);

    // Start with LED OFF
    _GPIO_PinDClear(UserLED);

    while (1)
    {
        // Turn LED ON
        _GPIO_PinDSet(UserLED);

        // Wait 500 milliseconds using TIM3
        _TIMER_DelayStart(TIM3,
                          _TIMER_Delay_Unit_ms,
                          500,
                          _TIMER_Delay_Interrupt_Off);

        while (!_TIMER_DelayComplete(TIM3))
        {
        }

        // Turn LED OFF
        _GPIO_PinDClear(UserLED);

        // Wait another 500 milliseconds
        _TIMER_DelayStart(TIM3,
                          _TIMER_Delay_Unit_ms,
                          500,
                          _TIMER_Delay_Interrupt_Off);

        while (!_TIMER_DelayComplete(TIM3))
        {
        }
    }
}


/*************************** End of file ****************************/
