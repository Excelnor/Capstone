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
#include "i2c.h"
#include "VL53L1X_api.h"
#include "vl53l1_platform.h"


//#define PartA
//#define PartB
//#define PartC
//#define PartD
#define PartE



/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/

#ifdef PartA
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
#endif

#ifdef PartB

// Variables for SEGGER Watch
volatile uint16_t sensorId = 0;
volatile int sensorIdStatus = -1;
volatile int sensorInitStatus = -1;
volatile int testComplete = 0;

int main(void)
{
    _CLOCK_PLL_To_64();

    // Initialize I2C1: PB8 = SCL, PB9 = SDA
    _I2C1_Init_PB89(_I2C_SpeedMode_100);

    // Verify the VL53L1X model ID
    sensorIdStatus = VL53L1X_GetSensorId(
        0x52,
        (uint16_t *)&sensorId
    );

    // Initialize sensor only if identification succeeds
    if (sensorIdStatus == 0 && sensorId == 0xEACC)
    {
        sensorInitStatus = VL53L1X_SensorInit(0x52);
    }

    testComplete = 1;

    while (1)
    {
        __NOP();
    }
}

#endif

#ifdef PartC
// Variables for SEGGER Watch
volatile uint16_t sensorId = 0;
volatile uint16_t distance_mm = 0;
volatile uint8_t dataReady = 0;

volatile int sensorIdStatus = -1;
volatile int sensorInitStatus = -1;
volatile int startStatus = -1;
volatile int readyStatus = -1;
volatile int distanceStatus = -1;
volatile int clearStatus = -1;

volatile uint32_t measurementCount = 0;

int main(void)
{
    _CLOCK_PLL_To_64();

    // Initialize I2C1: PB8 = SCL, PB9 = SDA
    _I2C1_Init_PB89(_I2C_SpeedMode_100);

    // Verify sensor identification
    sensorIdStatus = VL53L1X_GetSensorId(
        0x52, (uint16_t *)&sensorId
    );

    if (sensorIdStatus == 0 && sensorId == 0xEACC)
    {
        // Initialize the VL53L1X
        sensorInitStatus = VL53L1X_SensorInit(0x52);

        if (sensorInitStatus == 0)
        {
            // Begin continuous distance measurements
            startStatus = VL53L1X_StartRanging(0x52);

            if (startStatus == 0)
            {
                while (1)
                {
                    dataReady = 0;

                    // Check whether a measurement is available
                    readyStatus = VL53L1X_CheckForDataReady(
                        0x52, (uint8_t *)&dataReady
                    );

                    if (readyStatus == 0 && dataReady)
                    {
                        // Read distance in millimeters
                        distanceStatus = VL53L1X_GetDistance(
                            0x52, (uint16_t *)&distance_mm
                        );

                        // Clear interrupt for next measurement
                        clearStatus = VL53L1X_ClearInterrupt(0x52);

                        if (distanceStatus == 0 &&
                            clearStatus == 0)
                        {
                            measurementCount++;
                        }
                    }
                }
            }
        }
    }

    // Remain here if initialization or ranging fails
    while (1)
    {
        __NOP();
    }
}
#endif

#ifdef PartD
// Variables for SEGGER Watch
volatile unsigned int touchState = 0;
volatile unsigned int previousState = 0;
volatile unsigned int touchCount = 0;

int main(void)
{
    _CLOCK_PLL_To_64();

    // Configure PA6 as input for TTP223 OUT
    _GPIO_SetPinMode(GPIOA, 6,
                    _GPIO_PinMode_Input,
                    _GPIO_Clock_On);

    // Disable internal pull-up and pull-down
    _GPIO_Set_PullType(GPIOA, 6, _GPIO_PullType_None);

    while (1)
    {
        // Read PA6 
        touchState = (_GPIO_GetPinIState(GPIOA, 6) != 0);

        // Count each new touch
        if (touchState == 1 && previousState == 0)
        {
            touchCount++;
        }

        // Save state for next iteration
        previousState = touchState;
    }
}

#endif

#ifdef PartB

#include "gpio.h"
#include "CLOCK.h"

volatile unsigned int ledTest = 0;

int main(void)
{
    _CLOCK_PLL_To_64();

    // Configure LED pins as outputs
    _GPIO_SetPinMode(GPIOA, 7, _GPIO_PinMode_Output, _GPIO_Clock_On); // Red
    _GPIO_SetPinMode(GPIOB, 0, _GPIO_PinMode_Output, _GPIO_Clock_On); // Green
    _GPIO_SetPinMode(GPIOB, 1, _GPIO_PinMode_Output, _GPIO_Clock_On); // Yellow

    // Initially turn OFF all LEDs
    _GPIO_PinClear(GPIOA, 7);
    _GPIO_PinClear(GPIOB, 0);
    _GPIO_PinClear(GPIOB, 1);

    while (1)
    {
        // LED control using SEGGER Watch
        if (ledTest == 1)
        {
            _GPIO_PinSet(GPIOA, 7);   // Red ON
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinClear(GPIOB, 1);
        }
        else if (ledTest == 2)
        {
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinSet(GPIOB, 0);   // Green ON
            _GPIO_PinClear(GPIOB, 1);
        }
        else if (ledTest == 3)
        {
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinSet(GPIOB, 1);   // Yellow ON
        }
        else
        {
            // All LEDs OFF
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinClear(GPIOB, 1);
        }
    }
}

#endif


#ifdef PartE


volatile unsigned int ledTest = 0;

int main(void)
{
    _CLOCK_PLL_To_64();

    // Configure LED pins as outputs
    _GPIO_SetPinMode(GPIOA, 7, _GPIO_PinMode_Output, _GPIO_Clock_On); // Red
    _GPIO_SetPinMode(GPIOB, 0, _GPIO_PinMode_Output, _GPIO_Clock_On); // Green
    _GPIO_SetPinMode(GPIOB, 1, _GPIO_PinMode_Output, _GPIO_Clock_On); // Yellow

    // Initially turn OFF all LEDs
    _GPIO_PinClear(GPIOA, 7);
    _GPIO_PinClear(GPIOB, 0);
    _GPIO_PinClear(GPIOB, 1);

    while (1)
    {
        // LED control using SEGGER Watch
        if (ledTest == 1)
        {
            _GPIO_PinSet(GPIOA, 7);   // Red ON
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinClear(GPIOB, 1);
        }
        else if (ledTest == 2)
        {
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinSet(GPIOB, 0);   // Green ON
            _GPIO_PinClear(GPIOB, 1);
        }
        else if (ledTest == 3)
        {
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinSet(GPIOB, 1);   // Yellow ON
        }
        else
        {
            // All LEDs OFF
            _GPIO_PinClear(GPIOA, 7);
            _GPIO_PinClear(GPIOB, 0);
            _GPIO_PinClear(GPIOB, 1);
        }
    }
}

#endif

/*************************** End of file ****************************/
