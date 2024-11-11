// 2024/10/31 10:40:47 (c) Aleksandr Shevchenko e-mail : Sasha7b9@tut.by
#include "defines.h"
#include "Hardware/HAL/HAL.h"


namespace HAL_RTC
{
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0130

    static RTC_HandleTypeDef handle;
    static RTC_TimeTypeDef   time;
    static RTC_DateTypeDef   date;
    static RTC_AlarmTypeDef  alarm;
}


void HAL_RTC::Init()
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    __HAL_RCC_RTC_ENABLE();
}


void HAL_RTC::ConfigForSleep()
{
    /* Enable Power Clock*/
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Allow Access to RTC Backup domaine */
    HAL_PWR_EnableBkUpAccess();

    handle.Instance = RTC;

    /* Check if the system was resumed from StandBy mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        /* Clear StandBy flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

        /* Disable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_DISABLE(&handle);

        /* Wait for RTC APB registers synchronisation (needed after start-up from Reset)*/
        if (HAL_RTC_WaitForSynchro(&handle) != HAL_OK)
        {
            /* Initialization Error */
            HAL::ErrorHandler();
        }

        /* Enable the write protection for RTC registers */
        __HAL_RTC_WRITEPROTECTION_ENABLE(&handle);
        /* No need to configure the RTC as the RTC config(clock source, enable,
        prescaler,...) are kept after wake-up from STANDBY */
    }
    else
    {
        /* Reset Backup Domaine */
        __HAL_RCC_BACKUPRESET_FORCE();
        __HAL_RCC_BACKUPRESET_RELEASE();

        /* Set the RTC time base to 1s */
        /* Configure RTC prescaler and RTC data registers as follows:
        - Hour Format = Format 24
        - Asynch Prediv = Value according to source clock
        - Synch Prediv = Value according to source clock
        - OutPut = Output Disable
        - OutPutPolarity = High Polarity
        - OutPutType = Open Drain */
        handle.Init.HourFormat = RTC_HOURFORMAT_24;
        handle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
        handle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
        handle.Init.OutPut = RTC_OUTPUT_DISABLE;
        handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
        handle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

        if (HAL_RTC_Init(&handle) != HAL_OK)
        {
            HAL::ErrorHandler();
        }

        /* Set the time to 01h 00mn 00s AM */
        time.TimeFormat = RTC_HOURFORMAT12_AM;
        time.Hours = 0x01;
        time.Minutes = 0x00;
        time.Seconds = 0x00;

        if (HAL_RTC_SetTime(&handle, &time, RTC_FORMAT_BCD) == HAL_ERROR)
        {
            HAL::ErrorHandler();
        }
    }

    HAL_RTC_GetTime(&handle, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&handle, &date, RTC_FORMAT_BIN);

    /* Set the alarm to current time + 5s */
    alarm.Alarm = RTC_ALARM_A;
    alarm.AlarmTime.TimeFormat = time.TimeFormat;
    alarm.AlarmTime.Hours = time.Hours;
    alarm.AlarmTime.Minutes = time.Minutes;
    alarm.AlarmTime.Seconds = (uint8)((time.Seconds + 5) % 60);
    alarm.AlarmDateWeekDay = 31;
    alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
    alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;

    if (HAL_RTC_SetAlarm_IT(&handle, &alarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        HAL::ErrorHandler();
    }
}
