// 2023/11/09 14:19:26 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/EnergySwitch.h"
#include "Hardware/HAL/HAL_PINS.h"
#include "Hardware/Timer.h"
#include "Hardware/HAL/HAL.h"
#include <stm32f3xx_ll_rtc.h>
#include <stm32f3xx_ll_rcc.h>
#include <cstring>


namespace EnergySwitch
{
    static void RTC_Config();

    static void Enter_Standby_With_RTC_Alarm();

    static RTC_AlarmTypeDef RTC_AlarmStructure;

    static RTC_TimeTypeDef time;
}


void EnergySwitch::Init()
{
    pinPower.Init();

    pinPower.ToLow();
    pinPower.ToHi();
    
    Timer::Delay(500);
    
    pinPower.ToLow();
    pinPower.ToHi();

    RTC_Config();

//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}


void EnergySwitch::TurnOff()
{
    Enter_Standby_With_RTC_Alarm();
}


void EnergySwitch::RTC_Config()
{
    /* Allow Access to RTC Backup domaine */
    HAL_PWR_EnableBkUpAccess();

    /* Check if the system was resumed from StandBy mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)
    {       
        /* Clear StandBy flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

        /* Disable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_DISABLE((RTC_HandleTypeDef *)HAL_RTC::handle);

        /* Wait for RTC APB registers synchronisation (needed after start-up from Reset)*/
        if (HAL_RTC_WaitForSynchro((RTC_HandleTypeDef *)HAL_RTC::handle) != HAL_OK)
        {
            /* Initialization Error */
            HAL::ErrorHandler();
        }

        /* Enable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_ENABLE((RTC_HandleTypeDef *)HAL_RTC::handle);
        /* No need to configure the RTC as the RTC config(clock source, enable,
        prescaler,...) are kept after wake-up from STANDBY */
    }
    else
    {
        __HAL_RCC_BACKUPRESET_FORCE();
        __HAL_RCC_BACKUPRESET_RELEASE();
    }
}


void EnergySwitch::Enter_Standby_With_RTC_Alarm()
{
    HAL_RTC_GetTime((RTC_HandleTypeDef *)HAL_RTC::handle, &time, RTC_FORMAT_BIN);

    time.Minutes += 1;

    if (time.Minutes > 59)
    {
        time.Minutes = 0;
        time.Hours += 1;

        if (time.Hours > 23)
        {
            time.Hours = 0;
            time.Minutes = 1;
        }
    }

    RTC_AlarmStructure.Alarm = RTC_ALARM_A;
    RTC_AlarmStructure.AlarmTime.TimeFormat = time.TimeFormat;
    RTC_AlarmStructure.AlarmTime.Hours = time.Hours;
    RTC_AlarmStructure.AlarmTime.Minutes = time.Minutes;
    RTC_AlarmStructure.AlarmTime.Seconds = time.Seconds;
    RTC_AlarmStructure.AlarmDateWeekDay = 31;
    RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
    RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    if (HAL_RTC_SetAlarm_IT((RTC_HandleTypeDef *)HAL_RTC::handle, &RTC_AlarmStructure, RTC_FORMAT_BIN) != HAL_OK)
    {
        /* Initialization Error */
        HAL::ErrorHandler();
    }

    /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
       mainly  when using more than one wakeup source this is to not miss any wakeup event.
         - Disable all used wakeup sources,
         - Clear all related wakeup flags,
         - Re-enable all used wakeup sources,
         - Enter the Standby mode.
    */

    /*#### Disable all used wakeup sources: WKUP pin ###########################*/
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

    /*#### Clear all related wakeup flags ######################################*/
    /* Clear PWR wake up Flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Enable WKUP pin */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    /* Request to enter STANDBY mode */
    HAL_PWR_EnterSTANDBYMode();
}
