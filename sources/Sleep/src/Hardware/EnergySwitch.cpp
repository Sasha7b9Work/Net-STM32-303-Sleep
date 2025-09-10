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

    static RTC_HandleTypeDef handleRTC;

    static RTC_AlarmTypeDef RTC_AlarmStructure;

    static RTC_TimeTypeDef RTC_TimeStructure;

    static RTC_DateTypeDef RTC_DateStructure;

    void *handle = &handleRTC;
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
    /* Enable Power Clock*/
    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_RCC_RTC_ENABLE();

    /* Allow Access to RTC Backup domaine */
    HAL_PWR_EnableBkUpAccess();

    handleRTC.Instance = RTC;

    /* Check if the system was resumed from StandBy mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
//        __HAL_RCC_BACKUPRESET_FORCE();
//        __HAL_RCC_BACKUPRESET_RELEASE();
        
        /* Clear StandBy flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

        /* Disable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_DISABLE(&handleRTC);

        /* Wait for RTC APB registers synchronisation (needed after start-up from Reset)*/
        if (HAL_RTC_WaitForSynchro(&handleRTC) != HAL_OK)
        {
            /* Initialization Error */
            HAL::ErrorHandler();
        }

        /* Enable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_ENABLE(&handleRTC);
        /* No need to configure the RTC as the RTC config(clock source, enable,
        prescaler,...) are kept after wake-up from STANDBY */
    }
    else
    {
        /* Reset Backup Domaine */
//        __HAL_RCC_BACKUPRESET_FORCE();
//        __HAL_RCC_BACKUPRESET_RELEASE();

#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0xFF

        /* Set the RTC time base to 1s */
        /* Configure RTC prescaler and RTC data registers as follows:
        - Hour Format = Format 24
        - Asynch Prediv = Value according to source clock
        - Synch Prediv = Value according to source clock
        - OutPut = Output Disable
        - OutPutPolarity = High Polarity
        - OutPutType = Open Drain */
        handleRTC.Init.HourFormat = RTC_HOURFORMAT_24;
        handleRTC.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
        handleRTC.Init.SynchPrediv = RTC_SYNCH_PREDIV;
        handleRTC.Init.OutPut = RTC_OUTPUT_DISABLE;
        handleRTC.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        handleRTC.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
        if (HAL_RTC_Init(&handleRTC) != HAL_OK)
        {
            /* Initialization Error */
            HAL::ErrorHandler();
        }

        /* Set the time to 01h 00mn 00s AM */
        RTC_TimeStructure.TimeFormat = RTC_HOURFORMAT12_AM;
        RTC_TimeStructure.Hours = 0x01;
        RTC_TimeStructure.Minutes = 0x00;
        RTC_TimeStructure.Seconds = 0x00;
        if (HAL_RTC_SetTime(&handleRTC, &RTC_TimeStructure, RTC_FORMAT_BCD) == HAL_ERROR)
        {
            /* Initialization Error */
            HAL::ErrorHandler();
        }
    }
}


void EnergySwitch::Enter_Standby_With_RTC_Alarm()
{
    HAL_RTC_GetTime(&handleRTC, &RTC_TimeStructure, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&handleRTC, &RTC_DateStructure, RTC_FORMAT_BIN);

    /* Set the alarm to current time + 5s */
    RTC_AlarmStructure.Alarm = RTC_ALARM_A;
    RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_TimeStructure.TimeFormat;
    RTC_AlarmStructure.AlarmTime.Hours = RTC_TimeStructure.Hours;
    RTC_AlarmStructure.AlarmTime.Minutes = RTC_TimeStructure.Minutes;
    RTC_AlarmStructure.AlarmTime.Seconds = (uint8)((RTC_TimeStructure.Seconds + 10) % 60);
    RTC_AlarmStructure.AlarmDateWeekDay = 31;
    RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
    RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    if (HAL_RTC_SetAlarm_IT(&handleRTC, &RTC_AlarmStructure, RTC_FORMAT_BIN) != HAL_OK)
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
