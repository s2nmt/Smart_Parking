/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pn532_stm32f1.h"
#include <stdio.h>
#include <string.h>
#include "bw16AT.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define use_HEXADECIMAL
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void Bw16_WiFi_Init(void);
void Stm32_Clearing_Buffer(uint8_t *ptrBuffer, uint16_t sizeBuffer);
void Stm32_Check_BW16_Response(uint8_t *ptrBuffer, uint8_t *ptrContent);
void SmartConfigWifi(void);
void BW16_MQTT_Broker_Init();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t wirelessBuffer[WIRELESS_MAX_BYTES];
uint8_t wirelessData = 0;
uint16_t wirelessCounter = 0;

uint8_t modbusData[WIRELESS_MAX_BYTES];
uint8_t modbusDataChar = 0;
uint16_t modbusDataCount = 0;
uint8_t buffUartWifi = 0;
uint8_t displayStatusWiFi = 0;
MQTT_StructuresTypeDef broker;
uint8_t maincheck = 0;
uint8_t statusWiFiCounter = 0;
uint8_t statusClose = 0;
uint8_t Flag_counter = 0;
//use for print debug log
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UNUSED(huart);// tránh cảnh báo biến chưa được sử dụng từ trình biên dịc

	if(huart->Instance == USART1)
	{

		HAL_UART_Receive_IT(&huart1, &modbusDataChar, 1);
		modbusData[modbusDataCount] = modbusDataChar;
		modbusDataCount++;
	}
	if(huart->Instance == USART2)
	{
//		if(buffUartWifi == 0){
//			buffUartWifi = 1;
//			HAL_TIM_Base_Start(&htim2);
//		}
		HAL_UART_Receive_IT(&huart2, &wirelessData, 1);
		if(wirelessData == 0) wirelessData = 64;
		wirelessBuffer[wirelessCounter++] = wirelessData;
		if(wirelessCounter > WIRELESS_MAX_BYTES - 1) wirelessCounter = 0;
//		__HAL_TIM_SET_COUNTER(&htim2, 0);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  UNUSED(htim);

  if(htim->Instance == TIM1)
  {
	statusWiFiCounter++;
	if(maincheck == 0){
		if(statusWiFiCounter == displayStatusWiFi)
		{
			if(displayStatusWiFi != SMARTCONFIG_WIFI){
			  if(displayStatusWiFi == SUCCESSFULLY_CONNECTED_WIFIAP) HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin, GPIO_PIN_RESET);
			  else HAL_GPIO_TogglePin(GreenLed_GPIO_Port, GreenLed_Pin);
			}
			else{
			  HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin, GPIO_PIN_RESET);
			  if(displayStatusWiFi == SUCCESSFULLY_CONNECTED_WIFIAP) HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin, GPIO_PIN_RESET);
			  else HAL_GPIO_TogglePin(RedLed_GPIO_Port, RedLed_Pin);
			}

		  statusWiFiCounter = 0;
		}
	}
	else{
		if(statusWiFiCounter == 120){
			Flag_counter = 1;
			statusWiFiCounter = 0;

		}
	}
  }
}
/* USER CODE END 0 */
uint16_t countertest;
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  HAL_Delay(2000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive_IT(&huart1, &modbusDataChar, 1);
  HAL_UART_Receive_IT(&huart2, &wirelessData, 1);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  uint8_t buff[255];
  uint8_t uid[10];
  int32_t uid_len = 0;
  printf("init systems\r\n");
//  while(1){
//		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,950);
//		HAL_Delay(1000);
//		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,450);
//		HAL_Delay(1000);
//  }


  Bw16_WiFi_Init();
  BW16_MQTT_Broker_Init();

//  HAL_TIM_Base_Stop(&htim1);
  maincheck = 1;
  HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin,RESET);
  __HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,950);

  PN532 pn532;
  PN532_SPI_Init(&pn532);
  printf("init PN532\r\n");
  if (PN532_GetFirmwareVersion(&pn532, buff) != PN532_STATUS_OK)
  {	  uint8_t timeout = 10;
	  while(timeout--){
		  HAL_GPIO_TogglePin(RedLed_GPIO_Port, RedLed_Pin);
		  HAL_Delay(200);
	  }HAL_TIM_Base_Stop(&htim2);
	  HAL_NVIC_SystemReset();
  }
  printf("PN532_GetFirmwareVersion success\r\n");

  PN532_SamConfiguration(&pn532);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	memset(uid, 0, sizeof(uid));

	uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
	if (uid_len != PN532_STATUS_ERROR)
	{
		char stringForHex[6 * uid_len + 6];
		memset(stringForHex, 0, sizeof(stringForHex));
		strcat(stringForHex, "CODE:");
		for(uint8_t i=0; i<uid_len; i++)
		{
//			printf("%02X\r\n",uid[i]);
			sprintf(stringForHex + strlen(stringForHex), " 0x%02X", uid[i]);
		}

		printf("%s\r\n",stringForHex);
		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, SET);
		Bw16_MQTT_Publish(huart2,broker.topic.pub,(uint8_t*)"0",(uint8_t*)"0",(uint8_t*)stringForHex);
		Stm32_Clearing_Buffer(wirelessBuffer, WIRELESS_MAX_BYTES);
		HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin,RESET);
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,950);
		HAL_Delay(400);
	}
	for (uint8_t i = 0; i < 50;i++){
		if(wirelessBuffer[i] == 'O' && wirelessBuffer[i+1] == 'P' && wirelessBuffer[i+2] == 'E' &&
		   wirelessBuffer[i+3] == 'N'){
				HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin,SET);
				HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin,RESET);
				Stm32_Clearing_Buffer(wirelessBuffer, WIRELESS_MAX_BYTES);
				statusWiFiCounter = 0;
				statusClose = 1;
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,450);
			printf("Open\r\n");
		}
		else if(wirelessBuffer[i] == 'C' && wirelessBuffer[i+1] == 'L' && wirelessBuffer[i+2] == 'O' &&
		   wirelessBuffer[i+3] == 'S' && wirelessBuffer[i+4] == 'E'){
				HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin,RESET);
				HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin,SET);
				Stm32_Clearing_Buffer(wirelessBuffer, WIRELESS_MAX_BYTES);
				statusWiFiCounter = 0;
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,950);
				printf("Close\r\n");
		}
	}
	if(statusClose == 1 && Flag_counter == 1){
		statusClose = 0;
		Flag_counter = 0;
		HAL_GPIO_WritePin(RedLed_GPIO_Port, RedLed_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GreenLed_GPIO_Port, GreenLed_Pin, GPIO_PIN_RESET);
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,950);
	}
	else if(statusClose == 0 && Flag_counter == 1){
		Flag_counter = 0;
	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 200;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8799;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 6;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 15;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RST_BW16_GPIO_Port, RST_BW16_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RFID_SS_GPIO_Port, RFID_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RFID_Reset_Pin|GreenLed_Pin|RedLed_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : RST_BW16_Pin */
  GPIO_InitStruct.Pin = RST_BW16_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RST_BW16_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RFID_SS_Pin */
  GPIO_InitStruct.Pin = RFID_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RFID_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RFID_Reset_Pin */
  GPIO_InitStruct.Pin = RFID_Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RFID_Reset_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RFID_IRQ_Pin */
  GPIO_InitStruct.Pin = RFID_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RFID_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Buzzer_Pin GreenLed_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin|GreenLed_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RedLed_Pin */
  GPIO_InitStruct.Pin = RedLed_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RedLed_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void Stm32_Clearing_Buffer(uint8_t *ptrBuffer, uint16_t sizeBuffer)
{
  if(ptrBuffer == &wirelessBuffer[0]) wirelessCounter = 0;

	for(uint16_t i = 0; i < sizeBuffer; i++) ptrBuffer[i] = '\0';
}
void Bw16_WiFi_Init(void){
	displayStatusWiFi = CHECK_CONNECTING_WIFIAP;
	HAL_GPIO_WritePin(RST_BW16_GPIO_Port, RST_BW16_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(RST_BW16_GPIO_Port, RST_BW16_Pin, GPIO_PIN_SET);
//	HAL_Delay(10000);
//	Bw16_Setup_Default(huart2);
//	HAL_Delay(5000);
    if(Bw16_Response(wirelessBuffer, (uint8_t*)"WIFI_GOT_IP", 10000)) Stm32_Clearing_Buffer(wirelessBuffer, WIRELESS_MAX_BYTES);
    else SmartConfigWifi();
    HAL_Delay(200);
	Bw16_Echo(huart2, DISABLE);
	Stm32_Check_BW16_Response(wirelessBuffer, (uint8_t*)"OK");
    HAL_Delay(200);
	Bw16_Configure_Mode(huart2, Flash_STA_Mode);
	Stm32_Check_BW16_Response(wirelessBuffer, (uint8_t*)"OK");
    HAL_Delay(200);
}
void Stm32_Check_BW16_Response(uint8_t *ptrBuffer, uint8_t *ptrContent)
{
	if(!Bw16_Response(ptrBuffer, ptrContent, 10000)) HAL_NVIC_SystemReset();
	else Stm32_Clearing_Buffer(ptrBuffer, WIRELESS_MAX_BYTES);
}
void SmartConfigWifi(void)
{
	displayStatusWiFi = SMARTCONFIG_WIFI;
	HAL_Delay(100);
	BW16_Start_SmartConfig(huart2,Wifi_Config);
	HAL_Delay(100);
	if(Bw16_Response(wirelessBuffer, (uint8_t*)"WIFI_GOT_IP", 60000)) Stm32_Clearing_Buffer(wirelessBuffer, WIRELESS_MAX_BYTES);
	else HAL_NVIC_SystemReset();

	BW16_Start_SmartConfig(huart2,OFF);
	HAL_Delay(100);
	Stm32_Check_BW16_Response(wirelessBuffer, (uint8_t*)"OK");
}
void BW16_MQTT_Broker_Init()
{
	strcpy((char*)broker.host, "");
	strcpy((char*)broker.port, "");
	strcpy((char*)broker.user, "");
	strcpy((char*)broker.password, "");
	strcpy((char*)broker.id, DeviceID);
	strcpy((char*)broker.topic.sub, "device/NFC");
	strcpy((char*)broker.topic.pub, "server/NFC");
	displayStatusWiFi = SUCCESSFULLY_CONNECTED_WIFIAP;
	Bw16_Set_Info_MQTT(huart2, (uint8_t *)"1", broker.host);

	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"2",broker.port);
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"3",(uint8_t *)"1");
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"4",(uint8_t *) DeviceID);
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"5",broker.user);
	HAL_Delay(100);
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"6",broker.password);
	HAL_Delay(100);
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Set_Info_MQTT(huart2,(uint8_t *)"7",(uint8_t *)"\"\",0,0,\"\"");
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");

	Bw16_Connect_MQTT(huart2);
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");
	HAL_Delay(500);
	Bw16_MQTT_Publish(huart2,broker.topic.pub,(uint8_t*)"0",(uint8_t*)"0",(uint8_t*)"<!> Successfully connected");
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");
	HAL_Delay(500);
	Bw16_MQTT_Subscribe(huart2,broker.topic.sub,(uint8_t*)"0");
	Stm32_Check_BW16_Response(wirelessBuffer,(uint8_t*)"OK");
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
