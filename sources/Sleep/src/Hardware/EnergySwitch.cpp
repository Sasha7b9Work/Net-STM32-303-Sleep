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
    static void Config_AlarmRTC(void);

    static void MX_RTC_Init(void);

    static RTC_HandleTypeDef handleRTC;

    void *handle = &handleRTC;

    static void Enter_Standby_With_RTC_Alarm();

    static void RTC_Wakeup_Config_On_Seconds(int num_secs);
}


void EnergySwitch::Init()
{
    pinPower.Init();

    pinPower.ToLow();
    pinPower.ToHi();
    
    MX_RTC_Init();

    Config_AlarmRTC();
}


void EnergySwitch::TurnOff()
{
    Enter_Standby_With_RTC_Alarm();
}


void EnergySwitch::Config_AlarmRTC(void)
{
    RTC_AlarmTypeDef sAlarm;
    std::memset(&sAlarm, 0, sizeof(sAlarm));

    // Получаем текущее время
    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };
    HAL_RTC_GetTime(&handleRTC, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&handleRTC, &sDate, RTC_FORMAT_BIN);

    // Добавляем 1 секунду к текущему времени
    sTime.Seconds += 10;
    if (sTime.Seconds > 59 )
    {
        sTime.Seconds = 0;
        sTime.Minutes += 1;

        if (sTime.Minutes > 59)
        {
            sTime.Minutes = 0;
            sTime.Hours += 1;
        }
    }

    // Настраиваем будильник
    sAlarm.AlarmTime.Hours = sTime.Hours;
    sAlarm.AlarmTime.Minutes = sTime.Minutes;
    sAlarm.AlarmTime.Seconds = sTime.Seconds;
    sAlarm.AlarmTime.SubSeconds = 0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = RTC_ALARM_A;

    // Устанавливаем будильник с прерыванием
    if (HAL_RTC_SetAlarm_IT(&handleRTC, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    // Разрешаем пробуждение от RTC Alarm для STM32F3
    HAL_RTCEx_SetWakeUpTimer_IT(&handleRTC, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}


void EnergySwitch::MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };

    handleRTC.Instance = RTC;
    handleRTC.Init.HourFormat = RTC_HOURFORMAT_24;
    handleRTC.Init.AsynchPrediv = 127;
    handleRTC.Init.SynchPrediv = 255;
    handleRTC.Init.OutPut = RTC_OUTPUT_DISABLE;
    handleRTC.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    handleRTC.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    // Включаем тактирование LSE
    __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);

    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET)
    {
        // Ждем готовности LSE
    }

    // Настраиваем RTC на LSE
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
    __HAL_RCC_RTC_ENABLE();

    if (HAL_RTC_Init(&handleRTC) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    // Устанавливаем начальное время
    sTime.Hours = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&handleRTC, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        HAL::ErrorHandler();
    }

    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 1;
    sDate.Year = 0;

    if (HAL_RTC_SetDate(&handleRTC, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        HAL::ErrorHandler();
    }
}


void EnergySwitch::Enter_Standby_With_RTC_Alarm()
{
    RTC_Wakeup_Config_On_Seconds(10);

    // Разрешаем пробуждение от RTC
    __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_IT();
    __HAL_RTC_WAKEUPTIMER_EXTI_ENABLE_RISING_EDGE();

    // Настраиваем выводы для низкого потребления
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    // Настраиваем все GPIO порты
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // Отключаем тактирование GPIO
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();

    // Входим в Standby mode
    HAL_PWR_EnterSTANDBYMode();
}


void EnergySwitch::RTC_Wakeup_Config_On_Seconds(int num_sec)
{
    // Отключаем Wakeup Timer перед настройкой
    HAL_RTCEx_DeactivateWakeUpTimer(&handleRTC);

    // Рассчитываем значение для 1 часа
    // Для RTC_WAKEUPCLOCK_CK_SPRE_16BITS: период = (WakeUpCounter + 1) * 1 сек
    uint32_t wakeup_counter = (uint)(num_sec - 1); // 3600 секунд = 1 час

    // Настраиваем Wakeup Timer
    if (HAL_RTCEx_SetWakeUpTimer_IT(&handleRTC, wakeup_counter, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
    {
        HAL::ErrorHandler();
    }
}
