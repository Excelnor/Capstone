// I2C library
//
// Basic I2C functions, intended to be used in device specific libraries
//
// Simon Walker
// Created September 2024, initial build
// 
// Version 1.0
//
////////////////////////////////////////////////////////////////////////

typedef enum
{
  _I2C_SpeedMode_100,
  _I2C_SpeedMode_400
} _I2C_SpeedMode;

typedef enum
{
  _I2C_AutoEnd_Software = 0,
  _I2C_AutoEnd_Hardware = 1
} _I2C_AutoEndOption;

typedef enum
{
  _I2C_BusState_Idle = 0,
  _I2C_BusState_Busy = 1
} _I2C_BusState;

// standard init for STM32G031 Nucleo board
int _I2C1_Init_PB67 (_I2C_SpeedMode smode);

// is the bus busy?
int _I2C1_IsBusy (void);

// start a write transaction
int _I2C1_StartWrite (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption);

// start a read transaction
int _I2C1_StartRead (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption);

// scan the bus, populating a table with discovered addresses
void _I2C1_BusScan (unsigned char * pAddressTable);