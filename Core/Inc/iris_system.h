/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef IRIS_SYSTEM_H
#define IRIS_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define UART_DEBUG
//#define SPI_DEBUG

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USART2_CS1_Pin GPIO_PIN_0
#define USART2_CS1_GPIO_Port GPIOA
#define USART2_CS2_Pin GPIO_PIN_1
#define USART2_CS2_GPIO_Port GPIOA
#define USART2_MOSI_Pin GPIO_PIN_2
#define USART2_MOSI_GPIO_Port GPIOA
#define USART2_MISO_Pin GPIO_PIN_3
#define USART2_MISO_GPIO_Port GPIOA
#define USART2_CLK_Pin GPIO_PIN_4
#define USART2_CLK_GPIO_Port GPIOA
#define TEST_OUT1_Pin GPIO_PIN_0
#define TEST_OUT1_GPIO_Port GPIOB
#define NAND_CS1_Pin GPIO_PIN_12
#define NAND_CS1_GPIO_Port GPIOB
#define WP__Pin GPIO_PIN_8
#define WP__GPIO_Port GPIOA
#define CAM_EN_Pin GPIO_PIN_11
#define CAM_EN_GPIO_Port GPIOA
#define NAND_CS2_Pin GPIO_PIN_12
#define NAND_CS2_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */
#define software_ver 0x13
#define VIS_SENSOR 0x3C
#define NIR_SENSOR 0x3D


//#define IRIS_EM
#define IRIS_FM

#ifdef IRIS_FM
#define CURRENTSENSE_5V 0x40
#define CURRENTSENSE_3V3 0x45
#endif

#ifdef IRIS_EM
#define CURRENTSENSE_5V 0x40
#endif


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
