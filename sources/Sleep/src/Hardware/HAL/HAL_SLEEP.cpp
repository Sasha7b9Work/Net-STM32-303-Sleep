// 2024/10/31 10:35:18 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/HAL/HAL.h"
#include <stm32f3xx_hal.h>


void HAL_SLEEP::Init()
{

}


void HAL_SLEEP::Enter()
{
    /*#### Disable all used wakeup sources: WKUP pin ###########################*/
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

    /*#### Clear all related wakeup flags ######################################*/
    /* Clear PWR wake up Flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Enable WKUP pin */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    /* Request to enter STANDBY mode */
    HAL_PWR_EnterSTANDBYMode();

    /*
    HAL_SuspendTick();

    HAL_PWR_DisableSleepOnExit();

    // ¬ыбираем этот режим, как наименее потребл€ющий
    HAL_PWR_EnterSTANDBYMode();
    */
}
