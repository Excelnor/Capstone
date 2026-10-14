// Utility Library Implementation (Template Version 1.0)
//
// Jonathan Bukasa
//
//  Most random library in the market
//
// < 01.14.2026 >
//
///////////////////////////////////////////////////////////////////////

// always include your own header
#include "utility.h"
#include "gpio.h"
#include <stdio.h>
#include "stm32g0xx.h"
#include "stdlib.h"     // string to integer
#include <math.h>       // isalpha

// *required* system/user includes go here

// local (service) declarations go here

// implementations from the header go here 
void foo(int i)
{
  printf("The number is %d", i);
}

void TogglePC6(int itr)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN_Msk;    // Enable clock cycle Port A
    
    GPIOC->MODER &= ~GPIO_MODER_MODE6_Msk;  // Clear mode bits
    GPIOC->MODER |= GPIO_MODER_MODE6_0;    // Set to output (01)

    while(1)
    {
      GPIOC->ODR |= GPIO_ODR_OD6_Msk;

      for(int i=0; i<itr; ++i)
      {;;;}

      GPIOC->ODR &= ~GPIO_ODR_OD6_Msk;

      for(int i=0; i<itr; ++i)
      {;;;}
    }

}


const float delaySpeed = 0.8115f;
//const float delaySpeed = 4.3247f;
void GenerateDelays(float freq, float duty, int * high, int * low)
{
  float period = (1/freq) * 1e6;    // 1e6: converts to us
  float hightime = period * (duty/100.0f);    // T*0.25 for example
  float lowtime = period - hightime;

  //convert us to iterations
  *high = (int)hightime/delaySpeed;
  *low = (int)lowtime/delaySpeed;

  printf("High time: %f\n\r", hightime);
  printf("Low time: %f\n\r", lowtime);
}

// converts period (ms) into iterations
void _Delay_ms(int ms)
{
  int cycles = (int)((ms * 1000) / delaySpeed);
  for (int j=0; j<cycles;j++)
  {;;;}
}

void _Systick_Setup (int load, int val, int ctrl)
{
  SysTick->LOAD = load;
  SysTick->VAL = val;
  SysTick->CTRL = ctrl;
}


void _Format_Time(long seconds, char* output)
{
  int days = seconds/86400;
  int hours = (seconds/3600)%24;
  int minutes = (seconds/60)%60;
  int remainingSecs = seconds%60;

  sprintf(output, "%04d %02d:%02d:%02d", days, hours, minutes, remainingSecs);
}

void _Interrupt_Setup(IRQn_Type peripheral)
{
  NVIC_EnableIRQ(peripheral);
}

void _ms_To_Seconds(long ms, char* output)
{
  int sec = ms/1000;
  sprintf(output, "%02ds", sec);
}

int charToInt(char i[])
{
  int j = atoi(i);
  return j;
}

void To_Eng(int n)
{
  char buffer[40];

  int exp = log10(n);    // gets the exponent
  while ((exp%3)!=0)
  {
    exp-=1;
  }
  double mantissa = n/pow(10,exp);      // in the expression 2.456x10^6, 2.456 is the mantissa
  printf("%.2fx10^%d\n\r", mantissa, exp);
  sprintf(buffer, "%.2fx10^%d\n\r", mantissa, exp);     // formatting
}

//void decToBinary(int n) 
//{
//    // Array to store binary digits
//    int binaryNum[32]; // Assuming a 32-bit integer, adjust for larger types if needed

//    // Counter for binary array
//    int i = 0;
//    while (n > 0) {
//        // Store remainder (0 or 1) in the array
//        binaryNum[i] = n % 2;     //  1/0
//        n = n / 2;              // next value
//        i++;
//    }

//    //for (char ch = 'A'; ch <= 'Z'; ch++)
//    //  {
//    //    _USART_TxByte(USART2, ch);
//    //  }

//    char buffer[35];
//    int index = 0;
//    // Print the binary array in reverse order
//    for (int j = i - 1; j >= 0; j--) {
//        printf("%d", binaryNum[j]);
        
//        sprintf(&buffer[index++], "%d", binaryNum[j]);
//    }
//    buffer[index] = '\0';       // very important: announces the end of the buffer
//    _USART_TxString(USART2, buffer);
//}

void _DAC_Setup(int value)
{
     RCC->APBENR1 |= RCC_APBENR1_DAC1EN_Msk;                              // Enable the DAC clock
     DAC1->CR &= ~DAC_CR_EN1_Msk;                                          // Disable the DAC
     DAC1->CR &= ~DAC_CR_TEN1_Msk;                                         // Disable the DAC trigger
     DAC1->CR |= DAC_CR_TEN1_Msk;                                         // Enable triggering
     DAC1->CR &= ~DAC_CR_TSEL1_Msk;                                        // Select software as the trigger source
     DAC1->CR |= DAC_CR_EN1_Msk;                                           // Enable the DAC
     
     DAC1->DHR12R1 = value;                                                // update the DAC output on CH1 
     DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1_Msk;  
}

void TRI_Init(TRI_Ramp *r, int16_t min, int16_t max, int16_t step)
{
    r->min = min;
    r->max = max;
    r->step = step;
    r->value = min;
    r->dir = TRI_UP;
}


int16_t TRI_Update(TRI_Ramp *r)
{
    if (r->dir == TRI_UP)
    {
        r->value += r->step;

        if (r->value > r->max)
        {
            r->value = r->max - (r->value - r->max);  // reflect
            r->dir = TRI_DOWN;
        }
    }
    else
    {
        r->value -= r->step;

        if (r->value < r->min)
        {
            r->value = r->min + (r->min - r->value);  // reflect
            r->dir = TRI_UP;
        }
    }

    return r->value;
}


/* USE CASE EXAMPLE:
TRI_Ramp dacRamp;

int16_t DACV2 = 2.0f / 3.3f * 4095;
TRI_Init(&dacRamp, 0, DACV2, 64);

while (1)
{
  if (_TIMER_DelayComplete(TIM14))
  {
      int16_t val = TRI_Update(&dacRamp);

      DAC1->DHR12R1 = val;
      DAC1->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
  }

  // Change amplitude
  if (button_pressed)
  {
      int16_t DACV1 = 1.0f / 3.3f * 4095;
      int16_t DACV2 = 2.0f / 3.3f * 4095;

      dacRamp.max = (dacRamp.max == DACV2) ? DACV1 : DACV2;
  }
}


// To change frequency
step = (2 * (max - min) * f_desired) / update_rate; 

Example: 
f = 50 Hz
range = 0 -> 4095
update_rate = 10,000 Hz (Timer delay)
step = (2 * 4095 * 50) / 10000
     = 40.95 → 41
TRI_Init(&dacRamp, 0, 4095, 41);
*/