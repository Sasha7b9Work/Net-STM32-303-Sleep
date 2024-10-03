// 2022/02/21 15:41:29 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Modules/BH1750/BH1750.h"
#include "Hardware/HAL/HAL.h"
#include <cstdlib>


namespace BH1750
{
    static const uint8 CMD_POWER_ON   = 0x01;
    static const uint8 CMD_RESET      = 0x03;
    static const uint8 CMD_MEASURE    = 0x10;

    static bool is_initialized = false;

    uint timeNext = 1;

    bool WriteAddrL(uint8);

    bool WriteAddrH(uint8);

    bool ReadAddrL(uint8 *);

    bool ReadAddrH(uint8 *);
}


bool BH1750::Init()
{
    is_initialized = WriteAddrL(CMD_POWER_ON) &&
            WriteAddrL(CMD_RESET) &&

            WriteAddrL(0x40) &&   // ���������������� 0.45
            WriteAddrL(0x7f) &&   // ���������������� 0.45
            WriteAddrL(0x10) &&   // ���������������� 0.45

      //    WriteAddrL(0x44);   // ��������������� 2
      //    WriteAddrL(0x6a);   // ��������������� 2
      //    WriteAddrL(0x10);   // ��������������� 2
      
            WriteAddrL(CMD_MEASURE);

    return is_initialized;
}


bool BH1750::IsInitialized()
{
    return is_initialized;
}


bool BH1750::GetMeasure(Measure *illumination)
{
    if (HAL_GetTick() < timeNext)
    {
        return false;
    }

    timeNext += TIME_MEASURE + ((uint)std::rand() % 100);

#ifdef IN_MODE_TEST

    static float value = 1.5f;

    value *= 1.98f;

    *illumination = value / 100.0f;

    if (value > 1e4f)
    {
        value = 1.3f;
    }

    return true;

#else

    BitSet32 result;

    if (ReadAddrL(&result.bytes[0]))
    {
        float value = (float)(result.bytes[1] | (result.bytes[0] << 8)) / 1.2f * (69.0f / 31.0f);

//        float value = (float)(result.byte[1] | (result.byte[0] << 8)) / 1.2f;

        illumination->Set(Measure::Illuminate, value);

        return true;
    }

    return false;

#endif
}


bool BH1750::WriteAddrL(uint8 byte)
{
    return HAL_I2C1::Write8(0x23, byte) == 0;
}


bool BH1750::WriteAddrH(uint8 byte)
{
    return HAL_I2C1::Write8(0x5c, byte) == 0;
}


bool BH1750::ReadAddrL(uint8 *buffer)
{
    return HAL_I2C1::Read16(0x23, buffer) == 0;
}


bool BH1750::ReadAddrH(uint8 *buffer)
{
    return HAL_I2C1::Read16(0x5c, buffer) == 0;
}
