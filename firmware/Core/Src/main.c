/**
 * @file    main.c
 * @brief   应用入口 — 初始化 + 主循环
 *
 * 依赖链: main.c → service/protocol → bsp/uart,led → HAL
 */
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "bsp_led.h"
#include "bsp_uart.h"
#include "bsp_595.h"
#include "bsp_gpio.h"
#include "bsp_adc.h"
#include "param_store.h"
#include "protocol.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* USER CODE END PV */

void SystemClock_Config(void);

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) {
        BSP_RS485_RxCallback(huart->pRxBuffPtr[0]);
    }
}
/* USER CODE END 4 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 外设初始化 (CubeMX 生成) */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_IWDG_Init();

    /* USER CODE BEGIN 2 */
    /* BSP 层初始化 */
    BSP_LED_Init();
    BSP_RS485_Init();
    BSP_595_Init();
    BSP_NMOS_AllOff();

    /* Service 层初始化 */
    Param_Init();
    Proto_Init();

    /* 启动确认：LED 闪烁从机地址次数 */
    BSP_LED_Blink(Param_GetAddr(), 200);
    /* USER CODE END 2 */

    /* 主循环 */
    while (1)
    {
        /* USER CODE BEGIN 3 */
        Proto_Process();            /* 协议处理 (service 层) */
        HAL_IWDG_Refresh(&hiwdg);
        /* USER CODE END 3 */
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) HAL_IncTick();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
