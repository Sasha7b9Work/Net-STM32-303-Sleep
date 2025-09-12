// 2023/11/09 14:19:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL_PINS.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include <stm32f3xx_ll_rtc.h>
#include <stm32f3xx_ll_rcc.h>
#include <cstring>


void EnergySwitch::Init()
{
    pinPower.Init();

    pinPower.ToLow();
    pinPower.ToHi();
    
    Timer::Delay(500);
    
    pinPower.ToLow();
    pinPower.ToHi();

}


void EnergySwitch::TurnOff()
{
    HAL_RTCEx_DeactivateWakeUpTimer((RTC_HandleTypeDef *)HAL_RTC::handle);

    HAL_RTCEx_SetWakeUpTimer_IT((RTC_HandleTypeDef *)HAL_RTC::handle, 20, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

    /* Reactivate LSI clock if it has been stopped by system reset */
    if (LL_RCC_LSI_IsReady() != 1)
    {
        LL_RCC_LSI_Enable();
        while (LL_RCC_LSI_IsReady() != 1)
        {
        }
    }

    /* Check and handle if the system was resumed from StandBy mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        /* Clear Standby flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    }

    /* Clear Wake-up timer flag if it is set    */
    /* Flag will set after exiting from Standby */
    if (LL_RTC_IsActiveFlag_WUT(RTC) == 1)
    {
        LL_RTC_ClearFlag_WUT(RTC);
    }

    /* Clear all related wakeup flags */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Enter the Standby mode */
    HAL_PWR_EnterSTANDBYMode();
}
