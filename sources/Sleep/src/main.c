#include "main.h"

/** @addtogroup STM32F3xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_STANDBY_RTC
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_TOGGLE_DELAY         100
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* RTC handler declaration */
RTC_HandleTypeDef RTCHandle;
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void RTC_Config(void);
static void Error_Handler(void);


static void InitPin();
static void PinToHi();
static void PinToLow();


int main(void)
{
  /* STM32F3xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  InitPin();

  PinToHi();

  /* Configure the system clock to 64 MHz */
  SystemClock_Config();

  /* Configure LED2 */
//  BSP_LED_Init(LED2);

  /* Configure RTC */
  RTC_Config();
  
  /* Insert 5 seconds delay */
  HAL_Delay(5000);

  PinToLow();

  HAL_Delay(5000);
  
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
  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 0xFFFF, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

  /* Reactivate LSI clock if it has been stopped by system reset */
  if (LL_RCC_LSI_IsReady() != 1)
  {
    LL_RCC_LSI_Enable();
    while(LL_RCC_LSI_IsReady() != 1)
    {
    }
  }

  /* Configure the system Power */
  SystemPower_Config();

  /* Check and handle if the system was resumed from StandBy mode */ 
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
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
  
  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 64000000
  *            HCLK(Hz)                       = 64000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            PLLMUL                         = RCC_PLL_MUL16 (16)
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* HSI Oscillator already ON after system reset, activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}
/**
  * @brief  System Power Configuration
  *         The system Power is configured as follow :
  *            + VREFINT OFF, with fast wakeup enabled
  *            + No IWDG
  *            + Automatic Wakeup using RTC clocked by LSI (after ~4s)
  * @param  None
  * @retval None
  */
static void SystemPower_Config(void)
{
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Enable write access to Backup domain */
  HAL_PWR_EnableBkUpAccess();
}

/**
  * @brief  RTC Configuration
  *         RTC Clocked by LSI (see HAL_RTC_MspInit)
  * @param  None
  * @retval None
  */
static void RTC_Config(void)
{
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
  if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
}


/**
  * @brief SYSTICK callback
  * @param None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  HAL_IncTick();
  if (TimingDelay != 0)
  {
    TimingDelay--;
  }
  else
  {
    /* Toggle LED2 */
//    BSP_LED_Toggle(LED2);
    TimingDelay = LED_TOGGLE_DELAY;
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
__attribute__ ((noreturn)) static void Error_Handler(void)
{
  /* Turn on the LED2 */
//  BSP_LED_On(LED2);
  while(1)
  {
  }
}


#define _PIN_ GPIO_PIN_1
#define _PORT_ GPIOA


static void InitPin()
{
    GPIO_InitTypeDef is =
    {
        _PIN_,
        GPIO_MODE_OUTPUT_PP,
        GPIO_PULLUP,
        GPIO_SPEED_FREQ_HIGH,
        0
    };

    HAL_GPIO_Init(_PORT_, &is);

    is.Pin = GPIO_PIN_2;

    HAL_GPIO_Init(GPIOA, &is);
}


static void PinToLow()
{
    HAL_GPIO_WritePin(_PORT_, _PIN_, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

static void PinToHi()
{
    HAL_GPIO_WritePin(_PORT_, _PIN_, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}
