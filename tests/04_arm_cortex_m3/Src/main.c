#include "main.h"


UART_HandleTypeDef huart1;


void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
  huart1.Init.BaudRate = 115200;
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
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}




// ****************************************************************************************************************
// ****************************************************************************************************************
// ****************************************************************************************************************
// ****************************************************************************************************************


void __uprint_emit(const void *data, uint16_t length) {
    HAL_UART_Transmit(&huart1, (uint8_t*)data, length, 100);
}


/*******************************************************************************************
 *      1 byte length variable test
*******************************************************************************************/

void test_char_A() {
    char c = 'A';
    uprint("test_char_A: %c\n", c);
}

void test_uint8_min() {
    uint8_t c = 0;
    uprint("test_uint8_min: %u\n", c);
}

void test_uint8_max() {
    uint8_t c = 0xFF;
    uprint("test_uint8_max: %u\n", c);
}

void test_int8_min() {
    int8_t c = -128;
    uprint("test_int8_min: %d\n", c);
}

void test_int8_max() {
    int8_t c = 0x7F;
    uprint("test_int8_max: %d\n", c);
}

/*******************************************************************************************
 *      2 bytes length variable test
*******************************************************************************************/

void test_short_min() {
    short num = -32768;
    uprint("test_short_min: %d\n", num);
}

void test_short_max() {
    short num = 0x7FFF;
    uprint("test_short_max: %d\n", num);
}

void test_short_12345() {
    short num = 0x3039;
    uprint("test_short_12345: %d\n", num);
}

/*******************************************************************************************
 *      4 bytes length variable test
*******************************************************************************************/

void test_int32_min() {
    int32_t num = 0x80000000;
    uprint("test_int32_min: %d\n", num);
}

void test_int32_max() {
    int32_t num = 0x7FFFFFFF;
    uprint("test_int32_max: %d\n", num);
}

void test_int32_1234567890() {
    int32_t num = 0x499602D2;
    uprint("test_int32_1234567890: %d\n", num);
}

void test_uint32_min() {
    uint32_t num = 0;
    uprint("test_uint32_min: %u\n", num);
}

void test_uint32_max() {
    uint32_t num = 0xFFFFFFFF;
    uprint("test_uint32_max: %u\n", num);
}

void test_uint32_1234567890() {
    uint32_t num = 0x499602D2;
    uprint("test_uint32_1234567890: %d\n", num);
}

/*******************************************************************************************
 *      4 bytes float variable test
*******************************************************************************************/

void test_float_zero() {
    float num = 0.0f;
    uprint("test_float_zero: %f\n", num);
}

void test_float_pi() {
    float num = 3.14159265f;
    uprint("test_float_pi: %f\n", num);
}

void test_float_neg() {
    float num = -2.5f;
    uprint("test_float_neg: %f\n", num);
}

/*******************************************************************************************
 *      8 bytes double variable test
*******************************************************************************************/

void test_double_zero() {
    double num = 0.0;
    uprint("test_double_zero: %f\n", num);
}

void test_double_pi() {
    double num = 3.141592653589793;
    uprint("test_double_pi: %f\n", num);
}

void test_double_neg() {
    double num = -2.5;
    uprint("test_double_neg: %f\n", num);
}

/*******************************************************************************************
 *      8 bytes length variable test
*******************************************************************************************/

void test_int64_min() {
    int64_t num = 0x8000000000000000;
    uprint("test_int64_min: %ld\n", num);
}

void test_int64_max() {
    int64_t num = 0x7FFFFFFFFFFFFFFF;
    uprint("test_int64_max: %ld\n", num);
}

void test_int64_1to0to1() {
    int64_t num = 0xAB54A98EEE391EEA;
    uprint("test_int64_1to0to1: %ld\n", num);
}

void test_uint64_min() {
    uint64_t num = 0;
    uprint("test_uint64_min: %ld\n", num);
}

void test_uint64_max() {
    uint64_t num = 0xFFFFFFFFFFFFFFFF;
    uprint("test_uint64_max: %lu\n", num);
}

void test_uint64_1to0to1() {
    uint64_t num = 0xAB54A98EEE391EEA;
    uprint("test_uint64_1to0to1: %lu\n", num);
}

// C Promotion test
void test_promotion() {
    uprint("test_promotion: %d\n", 0);
}

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();

  HAL_Delay(100);


  test_char_A();
  test_uint8_min();
  test_uint8_max();
  test_int8_min();
  test_int8_max();

  test_short_min();
  test_short_max();
  test_short_12345();

  test_int32_min();
  test_int32_max();
  test_int32_1234567890();
  test_uint32_min();
  test_uint32_max();
  test_uint32_1234567890();

  test_float_zero();
  test_float_pi();
  test_float_neg();

  test_double_zero();
  test_double_pi();
  test_double_neg();

  test_int64_min();
  test_int64_max();
  test_int64_1to0to1();
  test_uint64_min();
  test_uint64_max();
  test_uint64_1to0to1();

  test_promotion();

  while (1)
  {
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  }
}
