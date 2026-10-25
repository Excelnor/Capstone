/*

  LSM6DSOX  Testing

*/

#include <stdio.h>
#include <stdint.h>
#include "gpio.h"
#include "i2c.h"
#include "clocks.h"
#include "timer.h"

// LSM6DSOX registers
#define LSM6DSOX_ADDR_LOW   0x6A
#define LSM6DSOX_ADDR_HIGH  0x6B

#define WHO_AM_I             0x0F
#define CTRL1_XL             0x10
#define CTRL2_G              0x11
#define CTRL3_C              0x12

#define OUTX_L_G             0x22
#define OUTX_L_A             0x28

#define EXPECTED_WHO_AM_I    0x6C

static uint8_t imu_addr = LSM6DSOX_ADDR_LOW;


// perform one-time initializations before the main loop
void _OneTimeInits(void);
void LSM6DSOX_Test(void);

int main(void) {
  
  // kick it up to 64MHz
  _CLOCK_PLL_TO_64();

  // perform one-time startups for the program, but at full speed
  _OneTimeInits();  

  LSM6DSOX_Test();
}

void _OneTimeInits(void)
{
  // turn on GPIO clock for Port A - can't do anything without this
  _GPIO_ClockEnable(GPIOA);

  _I2C1_Init_PB67 (_I2C_SpeedMode_100);
  _TIMER_DelayStart(TIM14, _TIMER_Delay_Unit_ms, 100, _TIMER_Delay_Interrupt_Off);

}

// --------------------------------------------------
// Write one register
// --------------------------------------------------
static int IMU_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t data[2];

    data[0] = reg;
    data[1] = value;

    return _I2C1_StartWrite(
        imu_addr,
        2,
        data,
        _I2C_AutoEnd_Hardware
    );
}

// --------------------------------------------------
// Read one or more registers
// Uses a register-address write followed by a
// repeated-start read.
// --------------------------------------------------
static int IMU_ReadRegs(uint8_t reg, uint8_t *data,
                        uint8_t length)
{
    int result;

    // Send register address; keep bus active.
    result = _I2C1_StartWrite(
        imu_addr,
        1,
        &reg,
        _I2C_AutoEnd_Software
    );

    if (result != 0)
        return result;

    // Repeated-start read, then automatic STOP.
    return _I2C1_StartRead(
        imu_addr,
        length,
        data,
        _I2C_AutoEnd_Hardware
    );
}

// --------------------------------------------------
// Read and validate WHO_AM_I
// --------------------------------------------------
static int IMU_Check(uint8_t address)
{
    uint8_t id = 0;

    imu_addr = address;

    if (IMU_ReadRegs(WHO_AM_I, &id, 1) != 0)
        return 0;

    printf("I2C address: 0x%02X, WHO_AM_I: 0x%02X\r\n",
           imu_addr, id);

    return id == EXPECTED_WHO_AM_I;
}

// --------------------------------------------------
// Initialize accelerometer and gyroscope
// --------------------------------------------------
static int IMU_Init(void)
{
    uint8_t id = 0;

    if (!IMU_Check(LSM6DSOX_ADDR_LOW))
    {
        if (!IMU_Check(LSM6DSOX_ADDR_HIGH))
        {
            printf("ERROR: LSM6DSOX not detected!\r\n");
            return -1;
        }
    }

    if (IMU_ReadRegs(WHO_AM_I, &id, 1) != 0 ||
        id != EXPECTED_WHO_AM_I)
    {
        printf("ERROR: WHO_AM_I check failed!\r\n");
        return -1;
    }

    // Enable register auto-increment and block-data update.
    if (IMU_WriteReg(CTRL3_C, 0x44) != 0)
        return -1;

    // Accelerometer:
    // 104 Hz, +/- 2 g
    if (IMU_WriteReg(CTRL1_XL, 0x40) != 0)
        return -1;

    // Gyroscope:
    // 104 Hz, +/- 250 degrees/second
    if (IMU_WriteReg(CTRL2_G, 0x40) != 0)
        return -1;

    while(!_TIMER_DelayComplete(TIM14))
    {;;;}

    printf("LSM6DSOX initialized!\r\n");
    return 0;
}

// --------------------------------------------------
// Convert little-endian sensor bytes to signed int16
// --------------------------------------------------
static int16_t IMU_MakeInt16(uint8_t low, uint8_t high)
{
    return (int16_t)(((uint16_t)high << 8) | low);
}

// --------------------------------------------------
// Read accelerometer and gyroscope
// --------------------------------------------------
static int IMU_ReadRaw(int16_t accel[3], int16_t gyro[3])
{
    uint8_t data[12];

    // Gyro occupies 0x22-0x27.
    // Accelerometer occupies 0x28-0x2D.
    // Read both in one 12-byte burst.
    if (IMU_ReadRegs(OUTX_L_G, data, 12) != 0)
        return -1;

    gyro[0]  = IMU_MakeInt16(data[0],  data[1]);
    gyro[1]  = IMU_MakeInt16(data[2],  data[3]);
    gyro[2]  = IMU_MakeInt16(data[4],  data[5]);

    accel[0] = IMU_MakeInt16(data[6],  data[7]);
    accel[1] = IMU_MakeInt16(data[8],  data[9]);
    accel[2] = IMU_MakeInt16(data[10], data[11]);

    return 0;
}

// --------------------------------------------------
// Testing Function
// --------------------------------------------------
void LSM6DSOX_Test(void)
{
    int16_t accel[3];
    int16_t gyro[3];

    printf("\r\n--- LSM6DSOX TEST ---\r\n");

    if (IMU_Init() != 0)
    {
        printf("IMU initialization failed.\r\n");
        return;
    }

    while (1)
    {
        if (IMU_ReadRaw(accel, gyro) == 0)
        {
            printf(
                "ACC [raw]: X=%6d Y=%6d Z=%6d | "
                "GYRO [raw]: X=%6d Y=%6d Z=%6d\r\n",
                accel[0], accel[1], accel[2],
                gyro[0], gyro[1], gyro[2]
            );
        }
        else
        {
            printf("IMU read failed\r\n");
        }

        while(!_TIMER_DelayComplete(TIM14))
        {;;;}

    }
}


// --------------------------------------------------
// Converting raw sensor values to useful measurements
// --------------------------------------------------
void IMU_ConvertValues(int16_t accel[3], int16_t gyro[3])
{
  accel[0] *= 9.81/1000;;
  accel[1] *= 9.81/1000;;
  accel[2] *= 9.81/1000;;
  
  
}


/*************************** End of file ****************************/
