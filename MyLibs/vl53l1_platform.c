
#include "vl53l1_platform.h"
#include "i2c.h"
#include "TIMER.h"

#include <stdint.h>
#include <stddef.h>

#define VL53L1_MAX_TRANSFER  32U

// Temporary diagnostics for SEGGER Watch
volatile int debugWriteResult = 99;
volatile int debugReadResult  = 99;

// ST API uses an 8-bit address (0x52).
// Our I2C library expects a 7-bit address (0x29).
static uint8_t SensorAddress(uint16_t dev)
{
    return (uint8_t)(dev >> 1);
}

// Write sensor register address followed by data.
int8_t VL53L1_WriteMulti(uint16_t dev, uint16_t index,
                         uint8_t *pdata, uint32_t count)
{
    if ((pdata == NULL && count != 0U) ||
        count > VL53L1_MAX_TRANSFER)
        return -1;

    uint8_t buffer[VL53L1_MAX_TRANSFER + 2U];

    // VL53L1X register addresses are 16-bit, MSB first.
    buffer[0] = (uint8_t)(index >> 8);
    buffer[1] = (uint8_t)(index & 0xFF);

    for (uint32_t i = 0; i < count; i++)
        buffer[i + 2U] = pdata[i];

    int result = _I2C1_StartWrite(
        SensorAddress(dev),
        (unsigned char)(count + 2U),
        buffer,
        _I2C_AutoEnd_Hardware
    );

    return (result == 0) ? 0 : -1;
}

// Write register address, then perform repeated-start read.
int8_t VL53L1_ReadMulti(uint16_t dev, uint16_t index,
                        uint8_t *pdata, uint32_t count)
{
    if (pdata == NULL || count == 0U ||
        count > VL53L1_MAX_TRANSFER)
        return -1;

    uint8_t reg[2];

    reg[0] = (uint8_t)(index >> 8);
    reg[1] = (uint8_t)(index & 0xFF);

    // Software end: do not generate STOP.
    int result = _I2C1_StartWrite(
        SensorAddress(dev),
        2,
        reg,
        _I2C_AutoEnd_Software
    );

    debugWriteResult = result;

    if (result != 0)
        return -1;

    // Repeated START, then receive data and STOP.
    result = _I2C1_StartRead(
        SensorAddress(dev),
        (unsigned char)count,
        pdata,
        _I2C_AutoEnd_Hardware
    );

    debugReadResult = result;

    return (result == 0) ? 0 : -1;
}

int8_t VL53L1_WrByte(uint16_t dev, uint16_t index,
                     uint8_t data)
{
    return VL53L1_WriteMulti(dev, index, &data, 1);
}

int8_t VL53L1_WrWord(uint16_t dev, uint16_t index,
                     uint16_t data)
{
    uint8_t bytes[2] = {
        (uint8_t)(data >> 8),
        (uint8_t)data
    };

    return VL53L1_WriteMulti(dev, index, bytes, 2);
}

int8_t VL53L1_WrDWord(uint16_t dev, uint16_t index,
                      uint32_t data)
{
    uint8_t bytes[4] = {
        (uint8_t)(data >> 24),
        (uint8_t)(data >> 16),
        (uint8_t)(data >> 8),
        (uint8_t)data
    };

    return VL53L1_WriteMulti(dev, index, bytes, 4);
}

int8_t VL53L1_RdByte(uint16_t dev, uint16_t index,
                     uint8_t *data)
{
    return VL53L1_ReadMulti(dev, index, data, 1);
}

int8_t VL53L1_RdWord(uint16_t dev, uint16_t index,
                     uint16_t *data)
{
    if (data == NULL)
        return -1;

    uint8_t bytes[2];

    int8_t status = VL53L1_ReadMulti(
        dev, index, bytes, 2
    );

    if (status == 0)
    {
        *data = ((uint16_t)bytes[0] << 8) |
                 bytes[1];
    }

    return status;
}

int8_t VL53L1_RdDWord(uint16_t dev, uint16_t index,
                      uint32_t *data)
{
    if (data == NULL)
        return -1;

    uint8_t bytes[4];

    int8_t status = VL53L1_ReadMulti(
        dev, index, bytes, 4
    );

    if (status == 0)
    {
        *data = ((uint32_t)bytes[0] << 24) |
                ((uint32_t)bytes[1] << 16) |
                ((uint32_t)bytes[2] << 8) |
                 bytes[3];
    }

    return status;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
    (void)dev;

    if (wait_ms < 0)
        return -1;

    while (wait_ms > 0)
    {
        uint16_t chunk = (wait_ms > 1000)
                         ? 1000
                         : (uint16_t)wait_ms;

        _TIMER_DelayStart(
            TIM3,
            _TIMER_Delay_Unit_ms,
            chunk,
            _TIMER_Delay_Interrupt_Off
        );

        while (!_TIMER_DelayComplete(TIM3))
        {
        }

        wait_ms -= chunk;
    }

    return 0;
}
