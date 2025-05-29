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


void Device::Init()
{
    HAL::Init();

    EnergySwitch::Init();

    Timer::Delay(25);

    HC12::TransmitString("Device enabled\r");
}


void Device::Update()
{
    HC12::TransmitString("Device asleep now\r");

    EnergySwitch::TurnOff();
}
