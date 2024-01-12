/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOE
#define LD4_Pin GPIO_PIN_4
#define LD4_GPIO_Port GPIOE
#define EEPROMWC_Pin GPIO_PIN_2
#define EEPROMWC_GPIO_Port GPIOF
#define DO3_Pin GPIO_PIN_3
#define DO3_GPIO_Port GPIOA
#define DO4_Pin GPIO_PIN_4
#define DO4_GPIO_Port GPIOA
#define DO5_Pin GPIO_PIN_5
#define DO5_GPIO_Port GPIOA
#define DO6_Pin GPIO_PIN_6
#define DO6_GPIO_Port GPIOA
#define DO7_Pin GPIO_PIN_7
#define DO7_GPIO_Port GPIOA
#define DO1_Pin GPIO_PIN_1
#define DO1_GPIO_Port GPIOB
#define DO2_Pin GPIO_PIN_2
#define DO2_GPIO_Port GPIOB
#define DO11_Pin GPIO_PIN_11
#define DO11_GPIO_Port GPIOF
#define DO12_Pin GPIO_PIN_12
#define DO12_GPIO_Port GPIOF
#define DO13_Pin GPIO_PIN_13
#define DO13_GPIO_Port GPIOF
#define DO14_Pin GPIO_PIN_14
#define DO14_GPIO_Port GPIOF
#define DO15_Pin GPIO_PIN_15
#define DO15_GPIO_Port GPIOF
#define ShutdownMon_Pin GPIO_PIN_9
#define ShutdownMon_GPIO_Port GPIOE
#define DI13_Pin GPIO_PIN_13
#define DI13_GPIO_Port GPIOE
#define DI13_EXTI_IRQn EXTI15_10_IRQn
#define Shutdown_Pin GPIO_PIN_11
#define Shutdown_GPIO_Port GPIOB
#define DI8_Pin GPIO_PIN_8
#define DI8_GPIO_Port GPIOD
#define DI8_EXTI_IRQn EXTI9_5_IRQn
#define DI10_Pin GPIO_PIN_10
#define DI10_GPIO_Port GPIOD
#define DI10_EXTI_IRQn EXTI15_10_IRQn
#define DI11_Pin GPIO_PIN_11
#define DI11_GPIO_Port GPIOD
#define DI11_EXTI_IRQn EXTI15_10_IRQn
#define DI14_Pin GPIO_PIN_14
#define DI14_GPIO_Port GPIOD
#define DI14_EXTI_IRQn EXTI15_10_IRQn
#define DI15_Pin GPIO_PIN_15
#define DI15_GPIO_Port GPIOD
#define DI15_EXTI_IRQn EXTI15_10_IRQn
#define DI2_Pin GPIO_PIN_2
#define DI2_GPIO_Port GPIOG
#define DI2_EXTI_IRQn EXTI2_IRQn
#define DI3_Pin GPIO_PIN_3
#define DI3_GPIO_Port GPIOG
#define DI3_EXTI_IRQn EXTI3_IRQn
#define DI4_Pin GPIO_PIN_4
#define DI4_GPIO_Port GPIOG
#define DI4_EXTI_IRQn EXTI4_IRQn
#define DI5_Pin GPIO_PIN_5
#define DI5_GPIO_Port GPIOG
#define DI5_EXTI_IRQn EXTI9_5_IRQn
#define DI6_Pin GPIO_PIN_6
#define DI6_GPIO_Port GPIOG
#define DI6_EXTI_IRQn EXTI9_5_IRQn
#define DI7_Pin GPIO_PIN_7
#define DI7_GPIO_Port GPIOC
#define DI7_EXTI_IRQn EXTI9_5_IRQn
#define LD7_Pin GPIO_PIN_7
#define LD7_GPIO_Port GPIOB
#define LD8_Pin GPIO_PIN_8
#define LD8_GPIO_Port GPIOB
#define LD9_Pin GPIO_PIN_9
#define LD9_GPIO_Port GPIOB
#define LD0_Pin GPIO_PIN_0
#define LD0_GPIO_Port GPIOE
#define LD1_Pin GPIO_PIN_1
#define LD1_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
