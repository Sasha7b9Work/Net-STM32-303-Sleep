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
    static bool bh1750_ready = false;

    static void ProcessMeasure(const Measure &, uint time);
}


void Device::Init()
{
    HAL::Init();

    Timer::Delay(3000);
    
    EnergySwitch::Init();

    // Проверяем, было ли пробуждение от Standby
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        // Сбрасываем флаг пробуждения
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

        // Выполняем действия после пробуждения
        // 
    }

    BME280::Init();                    // Пытаемся инициализировать датчик давления

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
        bme280_ready = true;

        ProcessMeasure(temp, time);
        ProcessMeasure(pressure, time);
        ProcessMeasure(humidity, time);
        ProcessMeasure(dew_point, time);
    }

    if (BH1750::GetMeasure(&illuminace))
    {
        bh1750_ready = true;

        ProcessMeasure(illuminace, time);
    }

    if (counter++ > 20 || (bme280_ready && bh1750_ready))
    {
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
