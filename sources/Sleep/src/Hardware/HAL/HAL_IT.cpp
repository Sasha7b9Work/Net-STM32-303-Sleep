#include "defines.h"
#include "Hardware/HAL/HAL.h"
#include <stm32f3xx_hal.h>


#ifndef WIN32
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

#ifdef __cplusplus
extern "C" {
#endif 

void NMI_Handler(void)
{
}


void HardFault_Handler(void)
{
//    static int line = Debug::line[0];
//    static pchar file = Debug::file[0];
//    static int type = Debug::type;

    while (1) //-V776
    {
//        line = line; //-V570
//        file = file; //-V570
//        type = type; //-V570
    }
}


void MemManage_Handler(void)
{
    while (1)
    {
    }
}


void BusFault_Handler(void)
{
    while (1)
    {
    }
}


void UsageFault_Handler(void)
{
    while (1)
    {
    }
}


void SVC_Handler(void)
{

}


void DebugMon_Handler(void)
{

}


void PendSV_Handler(void)
{

}


void SysTick_Handler(void)
{
    HAL_IncTick();
}


void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *)
{

}

#ifdef __cplusplus
}
#endif
