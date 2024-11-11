// 2022/04/27 11:48:13 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Device.h"
#include "Hardware/Timer.h"
#include "Hardware/Keyboard.h"
#include "Hardware/HAL/HAL.h"


void Device::Init()
{
    HAL::Init();

    Keyboard::Init();

    HAL_SLEEP::Init();

    HAL_RTC::ConfigForSleep();
}


void Device::Update()
{
    HAL_SLEEP::Enter();

    HAL_RTC::ConfigForSleep();

    Keyboard::Update();
}
