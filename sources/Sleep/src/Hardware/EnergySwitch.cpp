// 2023/11/09 14:19:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL_PINS.h"
#include "Hardware/Timer.h"


void EnergySwitch::Init()
{
    pinPower.Init();

    pinPower.ToHi();        // ѕодаЄм напр€жение на этот вывод, чтобы источник питани€ запитал все узлы устройства
}
