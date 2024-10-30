// (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#pragma once
#include "Hardware/HAL/HAL_PINS.h"


namespace HAL
{
    void Init();

    void Delay(unsigned int timeMS);

    uint GetUID();

    void ErrorHandler();
}
