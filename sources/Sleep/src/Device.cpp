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
    static int bme280_ready = 0;
    static int bh1750_ready = 0;

    static bool ProcessMeasure(const Measure &, uint time);
}


void Device::Init()
{
    HAL::Init();

    Timer::Delay(3000);
    
    EnergySwitch::Init();

    BME280::Init();

    BH1750::Init();

    BME280::IsInitialized();
    BH1750::IsInitialized();
    
    InterCom::SetDirection((Direction::E)(Direction::HC12));
}


void Device::Update()
{
    static int counter = 0;

    Measure temp;
    Measure pressure;
    Measure humidity;
    Measure dew_point;
    Measure illuminace;

    uint time = TIME_MS;

    if (BME280::GetMeasures(&temp, &pressure, &humidity, &dew_point))
    {
        if (ProcessMeasure(temp, time) &&
            ProcessMeasure(pressure, time) &&
            ProcessMeasure(humidity, time) &&
            ProcessMeasure(dew_point, time))
        {
            bme280_ready++;
        }
    }

    if (BH1750::GetMeasure(&illuminace))
    {
        if (ProcessMeasure(illuminace, time))
        {
            bh1750_ready++;
        }
    }

    if (counter++ > 1000 || (bme280_ready > 3 && bh1750_ready > 3))
    {
        EnergySwitch::TurnOff();
    }
}


bool Device::ProcessMeasure(const Measure &measure, uint time)
{
    if (measure.correct)
    {
        InterCom::Send(measure, time);

        return true;
    }

    return false;
}
