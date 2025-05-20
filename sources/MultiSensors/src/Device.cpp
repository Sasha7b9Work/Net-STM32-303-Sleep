// 2022/04/27 11:48:13 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Device.h"
#include "Modules/BME280/BME280.h"
#include "Modules/BH1750/BH1750.h"
#include "Hardware/Timer.h"
#include "Hardware/InterCom.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL.h"


namespace Device
{
    static void ProcessMeasure(const Measure &, uint time);
}


void Device::Init()
{
    HAL::Init();

    EnergySwitch::Init();

    BME280::Init();                    // ѕытаемс€ инициализировать датчик давлени€

    BH1750::Init();

    BME280::IsInitialized();
    BH1750::IsInitialized();

    HAL_USART1::SetModeHC12();

    InterCom::SetDirection((Direction::E)(Direction::HC12));
}


void Device::Update()
{
    Measure temp;
    Measure pressure;
    Measure humidity;
    Measure dew_point;

    uint time = TIME_MS;

    if (BME280::GetMeasures(&temp, &pressure, &humidity, &dew_point))
    {
        ProcessMeasure(temp, time);
        ProcessMeasure(pressure, time);
        ProcessMeasure(humidity, time);
        ProcessMeasure(dew_point, time);
    }

    HAL_ADC::GetVoltage();

    EnergySwitch::Update();

    HAL_USART1::Update();
}


void Device::ProcessMeasure(const Measure &measure, uint time)
{
    if (measure.correct)
    {
        InterCom::Send(measure, time);
    }
}
