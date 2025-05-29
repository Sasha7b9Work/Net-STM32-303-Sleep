// 2022/04/27 11:48:13 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Device.h"
#include "Modules/BME280/BME280.h"
#include "Modules/BH1750/BH1750.h"
#include "Hardware/Timer.h"
#include "Hardware/InterCom.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL.h"
#include "Modules/HC12/HC12.h"


namespace Device
{
    static bool bme280_ready = false;

    static void ProcessMeasure(const Measure &, uint time);
}


void Device::Init()
{
    HAL::Init();

    EnergySwitch::Init();

    Timer::Delay(25);

    BME280::Init();                    // ѕытаемс€ инициализировать датчик давлени€

    BH1750::Init();

    BME280::IsInitialized();
    BH1750::IsInitialized();
    
    InterCom::SetDirection((Direction::E)(Direction::HC12));

    HC12::TransmitString("Device enabled\r");
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
        bme280_ready = true;

        ProcessMeasure(temp, time);
        ProcessMeasure(pressure, time);
        ProcessMeasure(humidity, time);
        ProcessMeasure(dew_point, time);
    }

    if (bme280_ready)
    {
        HC12::TransmitString("Device asleep now\r");

        EnergySwitch::TurnOff();
    }
}


void Device::ProcessMeasure(const Measure &measure, uint time)
{
    if (measure.correct)
    {
        InterCom::Send(measure, time);
    }
}
