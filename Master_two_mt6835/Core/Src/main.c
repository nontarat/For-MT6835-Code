/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "memorymap.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mt6835.h" // MT6835 驱�?�头�?件
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart3, (uint8_t*) ptr, len, HAL_MAX_DELAY);
	return len;
}

#define SPI_INSTANCE hspi3             // SPI3
//#define SPI_CS       SPI3_M1_Pin       // STM32CubeMX �?�??�?? CS 引�??
//#define SPI_CS_PORT  SPI3_M1_GPIO_Port // STM32CubeMX �?�??�?? CS 端�?�

#define SPI_M1_CS       SPI3_M1_Pin       // CS M1
#define SPI_M1_CS_PORT  SPI3_M1_GPIO_Port //
#define SPI_M2_CS       SPI3_M2_Pin       // CS M2
#define SPI_M2_CS_PORT  SPI3_M2_GPIO_Port //

static void mt6835_cs_control(mt6835_cs_state_enum_t state) {
	if (state == MT6835_CS_HIGH) {
		// 高电平
		HAL_GPIO_WritePin(SPI_M1_CS_PORT, SPI_M1_CS, GPIO_PIN_SET);
	} else {
		// 低电平
		HAL_GPIO_WritePin(SPI_M1_CS_PORT, SPI_M1_CS, GPIO_PIN_RESET);
	}
}

static void mt6835_cs2_control(mt6835_cs_state_enum_t state) {
	if (state == MT6835_CS_HIGH) {
		// 高电平
		HAL_GPIO_WritePin(SPI_M2_CS_PORT, SPI_M2_CS, GPIO_PIN_SET);
	} else {
		// 低电平
		HAL_GPIO_WritePin(SPI_M2_CS_PORT, SPI_M2_CS, GPIO_PIN_RESET);
	}
}

static void mt6835_spi_send_recv(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len) {
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_SPI_TransmitReceive_IT(&SPI_INSTANCE, tx_buf, rx_buf, len);
	if (status != HAL_OK) {
		printf("spi send_recv failed %d\n\r", status);
		return;
	}
	// wait IT
	uint32_t tickstart = HAL_GetTick();
	while (HAL_SPI_GetState(&SPI_INSTANCE) != HAL_SPI_STATE_READY) {
		if (HAL_GetTick() - tickstart > 1) {
			printf("spi send_recv timeout\n\r");
			return;
		}
	}
}

uint32_t raw_angle_1 = 0, raw_angle_2 = 0;
float radian_angle_1 = 0.0f, radian_angle_2 = 0.0f;
float degree_angle_1 = 0.0f, degree_angle_2 = 0.0f;
uint8_t id_1, id_2;


/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI3_Init();
	MX_USART3_UART_Init();
	/* USER CODE BEGIN 2 */
	mt6835_t *mt6835_1;
	mt6835_t *mt6835_2;

	mt6835_1 = mt6835_create();
	mt6835_2 = mt6835_create();

	mt6835_link_spi_cs_control(mt6835_1, mt6835_cs_control);
	mt6835_link_spi_cs_control(mt6835_2, mt6835_cs2_control);

	mt6835_link_spi_send_recv(mt6835_1, mt6835_spi_send_recv);
	mt6835_link_spi_send_recv(mt6835_2, mt6835_spi_send_recv);

	mt6835_enable_crc_check(mt6835_1);
	mt6835_enable_crc_check(mt6835_2);

	/*
	 *for change/show id
	 *mt6835_set_id(mt6835_1, 0x00); // 写入 ID 0xDA 到寄存器，只在寄存器，断电消失
	 *HAL_Delay(1);
	 *uint8_t id = mt6835_get_id(mt6835_1); // 读取 ID
	 *printf("id: 0x%x\r\n", id);
	 *		id_1 = mt6835_get_id(mt6835_1);
	 if (id_1 != 0xFF) { // ตรวจสอบว่า id_1 ไม่ใช่ค่า 0xFF ซึ่งอาจบ่งบอกถึงข้อผิดพลาด
	 printf("id 1: %2X\n", id_1);
	 } else {
	 printf("Error reading id 1\n");
	 }

	 id_2 = mt6835_get_id(mt6835_2);
	 if (id_2 != 0xFF) {
	 printf("id 2: %2X\n", id_2);
	 } else {
	 printf("Error reading id 2\n");
	 }
	 */

	// mt6835_disable_crc_check(mt6835_1);    // Disable CRC Check in 1
	// mt6835_disable_crc_check(mt6835_2);    // Disable CRC Check in 2
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		//raw_angle = mt6835_get_raw_angle(mt6835,
		//MT6835_READ_ANGLE_METHOD_BURST);
		//radian_angle = raw_angle * (M_PI * 2.0f) / MT6835_ANGLE_RESOLUTION;
		//degree_angle = raw_angle * (360.0f / MT6835_ANGLE_RESOLUTION);
		//radian_angle = mt6835_get_angle(motor1_mt6835, MT6835_READ_ANGLE_METHOD_BURST);
		//if (!mt6835->crc_res) {
		//	printf("crc error\n\r");
		//}
		//printf("raw_angle: %lu, radian_angle: %.6f, degree_angle: %.2f°\n\r",
		//raw_angle, radian_angle, degree_angle);
		//HAL_Delay(500);

		/* for Burst mode*/
		// Select CS1 for MT6835 1 and read angle
		raw_angle_1 = mt6835_get_raw_angle(mt6835_1,MT6835_READ_ANGLE_METHOD_NORMAL);
			//	MT6835_READ_ANGLE_METHOD_BURST);
		radian_angle_1 = raw_angle_1 * (M_PI * 2.0f) / MT6835_ANGLE_RESOLUTION;
		degree_angle_1 = raw_angle_1 * (360.0f / MT6835_ANGLE_RESOLUTION);

		// Select CS2 for MT6835 2 and read angle
		raw_angle_2 = mt6835_get_raw_angle(mt6835_2,MT6835_READ_ANGLE_METHOD_NORMAL);
		//MT6835_READ_ANGLE_METHOD_BURST);
		radian_angle_2 = raw_angle_2 * (M_PI * 2.0f) / MT6835_ANGLE_RESOLUTION;
		degree_angle_2 = raw_angle_2 * (360.0f / MT6835_ANGLE_RESOLUTION);

		// Check CRC for both sensors
		if (!mt6835_1->crc_res) {
			printf("CRC error on MT6835 1\n\r");
		}
		if (!mt6835_2->crc_res) {
			printf("CRC error on MT6835 2\n\r");
		}

		printf("Raw Angle 1: %lu, Radian 1: %f, degree_angle1: %.2f deg\n\r",
				raw_angle_1, radian_angle_1, degree_angle_1);

		printf("Raw Angle 2: %lu, Radian 2: %f, degree_angle2: %.2f deg\n\r",
				raw_angle_2, radian_angle_2, degree_angle_2);


		HAL_Delay(500);
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = 64;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 12;
	RCC_OscInitStruct.PLL.PLLP = 1;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
