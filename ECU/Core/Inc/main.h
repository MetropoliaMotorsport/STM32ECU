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
#define Input5_Pin GPIO_PIN_2
#define Input5_GPIO_Port GPIOE
#define Input5_EXTI_IRQn EXTI2_IRQn
#define Input8_Pin GPIO_PIN_3
#define Input8_GPIO_Port GPIOE
#define Input8_EXTI_IRQn EXTI3_IRQn
#define DET_LOCK_Pin GPIO_PIN_4
#define DET_LOCK_GPIO_Port GPIOE
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_EXTI_IRQn EXTI15_10_IRQn
#define LCD_D5_Pin GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOF
#define LCD_D4_Pin GPIO_PIN_10
#define LCD_D4_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define Input7_Pin GPIO_PIN_12
#define Input7_GPIO_Port GPIOF
#define Input7_EXTI_IRQn EXTI15_10_IRQn
#define Output2_Pin GPIO_PIN_15
#define Output2_GPIO_Port GPIOF
#define Output5_Pin GPIO_PIN_7
#define Output5_GPIO_Port GPIOE
#define Output4_Pin GPIO_PIN_8
#define Output4_GPIO_Port GPIOE
#define Output6_Pin GPIO_PIN_10
#define Output6_GPIO_Port GPIOE
#define Input6_Pin GPIO_PIN_11
#define Input6_GPIO_Port GPIOE
#define Input6_EXTI_IRQn EXTI15_10_IRQn
#define Output7_Pin GPIO_PIN_12
#define Output7_GPIO_Port GPIOE
#define Output1_Pin GPIO_PIN_13
#define Output1_GPIO_Port GPIOE
#define Output8_Pin GPIO_PIN_14
#define Output8_GPIO_Port GPIOE
#define DET_Pin GPIO_PIN_11
#define DET_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLK_RX_Pin GPIO_PIN_8
#define STLK_RX_GPIO_Port GPIOD
#define STLK_TX_Pin GPIO_PIN_9
#define STLK_TX_GPIO_Port GPIOD
#define Input3_Pin GPIO_PIN_14
#define Input3_GPIO_Port GPIOD
#define Input3_EXTI_IRQn EXTI15_10_IRQn
#define Input4_Pin GPIO_PIN_15
#define Input4_GPIO_Port GPIOD
#define Input4_EXTI_IRQn EXTI15_10_IRQn
#define LCD_E_Pin GPIO_PIN_2
#define LCD_E_GPIO_Port GPIOG
#define LCD_RS_Pin GPIO_PIN_3
#define LCD_RS_GPIO_Port GPIOG
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define LCD_D6_Pin GPIO_PIN_3
#define LCD_D6_GPIO_Port GPIOD
#define LCD_D7_Pin GPIO_PIN_4
#define LCD_D7_GPIO_Port GPIOD
#define Output3_Pin GPIO_PIN_9
#define Output3_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define Input1_Pin GPIO_PIN_8
#define Input1_GPIO_Port GPIOB
#define Input1_EXTI_IRQn EXTI9_5_IRQn
#define Input2_Pin GPIO_PIN_9
#define Input2_GPIO_Port GPIOB
#define Input2_EXTI_IRQn EXTI9_5_IRQn
#define LOCK_Pin GPIO_PIN_0
#define LOCK_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
