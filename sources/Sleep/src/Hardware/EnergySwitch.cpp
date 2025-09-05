// 2023/11/09 14:19:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL_PINS.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include <stm32f3xx_ll_rtc.h>
#include <stm32f3xx_ll_rcc.h>


namespace EnergySwitch
{
    static void RTC_Config(void);

    static void SystemPower_Config(void);

    static RTC_HandleTypeDef RTCHandle;
}


void EnergySwitch::Init()
{
    pinPower.Init();

    pinPower.ToHi();        // ѕодаЄм напр€жение на этот вывод, чтобы источник питани€ запитал все узлы устройства
}


void EnergySwitch::TurnOff()
{
    /* Configure RTC */
    RTC_Config();

    /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
      mainly  when using more than one wakeup source this is to not miss any wakeup event.
      - Disable all used wakeup sources,
      - Clear all related wakeup flags,
      - Re-enable all used wakeup sources,
      - Enter the Standby mode.
    */
    /* Disable all used wakeup sources*/
    HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

    /* Re-enable all used wakeup sources*/
    /* ## Setting the Wake up time ############################################*/
    /* RTC Wakeup Interrupt Generation:
      the wake-up counter is set to its maximum value to yield the longest
      stand-by time to let the current reach its lowest operating point.
      The maximum value is 0xFFFF, corresponding to about 26 sec. when
      RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16

      Wakeup Time Base = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI))
      Wakeup Time = Wakeup Time Base * WakeUpCounter
        = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI)) * WakeUpCounter
        ==> WakeUpCounter = Wakeup Time / Wakeup Time Base

      To configure the wake up timer to 26s the WakeUpCounter is set to 0xFFFF:
      Wakeup Time Base = 16 /(~40 kHz RC) = ~0.4 ms
      Wakeup Time = 0.4 ms  * WakeUpCounter
      Therefore, with wake-up counter =  0xFFFF  = 65,535
         Wakeup Time =  0.4 ms *  65,535 = ~ 26 sec. */
    HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 0xFFFF / 5, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

    /* Reactivate LSI clock if it has been stopped by system reset */
    if (LL_RCC_LSI_IsReady() != 1)
    {
        LL_RCC_LSI_Enable();
        while (LL_RCC_LSI_IsReady() != 1)
        {
        }
    }

    /* Configure the system Power */
    SystemPower_Config();

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

    HAL_Delay(5000);

    pinPower.ToLow();

    /* Enter the Standby mode */
    HAL_PWR_EnterSTANDBYMode();
}


void EnergySwitch::SystemPower_Config(void)
{
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enable write access to Backup domain */
    HAL_PWR_EnableBkUpAccess();
}



void EnergySwitch::RTC_Config(void)
{
    const uint RTC_ASYNCH_PREDIV = 0x7F;
    const uint RTC_SYNCH_PREDIV = 0x137;    // 40 kHz RC/127 - 1

    /* Configure RTC */
    RTCHandle.Instance = RTC;
    /* Set the RTC time base to 1s */
    /* Configure RTC prescaler and RTC data registers as follow:
      - Hour Format = Format 24
      - Asynch Prediv = Value according to source clock
      - Synch Prediv = Value according to source clock
      - OutPut = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType = Open Drain */
    RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
    RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
    RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&RTCHandle) != HAL_OK)
    {
        /* Initialization Error */
        HAL::ErrorHandler();
    }
}
