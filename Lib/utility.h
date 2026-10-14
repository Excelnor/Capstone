// Utility Library Header (Template Version 1.0)
//
// Library of all the code I think will be useful in the future
//
// Jonathan Bukasa
//
// Version 1.0
// Created January 14 2026, initial build, ported for stm32g031 board
// 
///////////////////////////////////////////////////////////////////////

#ifndef _utility_Lib
#define _utility_Lib

// *required* system includes go here
// *required* user includes go here

// type definitions go here

// prototypes go here
typedef enum { TRI_UP = 0, TRI_DOWN = 1 } TRI_DIR;

typedef struct
{
    int value;
    int min;
    int max;
    int step;
    TRI_DIR dir;
} TRI_Ramp;


void foo(int i);
void TogglePC6(int itr);
void GenerateDelays(float freq, float duty, int * high, int * low);
void _Delay_ms(int ms);
void _Systick_Setup (int load, int val, int ctrl);
void _Format_Time(long seconds, char* output);
void _ms_To_Seconds(long ms, char* output);
void decToBinary(int n);
void To_Eng(int n);;
int charToInt(char i[]);
void _DAC_Setup(int value);

#endif