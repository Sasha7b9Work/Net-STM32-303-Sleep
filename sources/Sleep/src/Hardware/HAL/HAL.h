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


namespace HAL_SLEEP
{
    // Инициализируем
    void Init();

    // Входим в спящий режим.
    // После выхода из спящего режима выполнение продолжится с выхода из этой функции
    void Enter();
}
