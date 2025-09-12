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
    return;
    /* Allow Access to RTC Backup domaine */
    HAL_PWR_EnableBkUpAccess();

    /* Check if the system was resumed from StandBy mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
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
    IWDG_HandleTypeDef hiwdg;

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    RTC_AlarmTypeDef sAlarm;

    std::memset(&sTime, 0, sizeof(sTime));
    std::memset(&sDate, 0, sizeof(sDate));
    std::memset(&sAlarm, 0, sizeof(sAlarm));

    RTC_HandleTypeDef *hrtc = (RTC_HandleTypeDef *)(HAL_RTC::handle);

    hrtc->Instance = RTC;
    hrtc->Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc->Init.AsynchPrediv = 127;
    hrtc->Init.SynchPrediv = 255;
    hrtc->Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(hrtc) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    /* USER CODE BEGIN Check_RTC_BKUP */

    /* USER CODE END Check_RTC_BKUP */

    /** Initialize RTC and set the Time and Date
    */
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        HAL::ErrorHandler();
    }
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 0x1;
    sDate.Year = 0x0;

    if (HAL_RTC_SetDate(hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    /** Enable the Alarm A
    */
    sAlarm.AlarmTime.Hours = 0x0;
    sAlarm.AlarmTime.Minutes = 0x1;
    sAlarm.AlarmTime.Seconds = 0x0;
    sAlarm.AlarmTime.SubSeconds = 0x0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES
        | RTC_ALARMMASK_SECONDS;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 0x1;
    sAlarm.Alarm = RTC_ALARM_A;
    if (HAL_RTC_SetAlarm(hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    /** Enable the WakeUp
    */
    if (HAL_RTCEx_SetWakeUpTimer(hrtc, 100, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
    {
        HAL::ErrorHandler();
    }
}
