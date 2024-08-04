/* USER CODE BEGIN Header */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "eth.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "Wifi.h"
#include "LMT70.h"
#include "Voice_broadcast.h"
#include "MAX30102.h"
#include "blood.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "arm_math.h"
#include "ADS1292.h"
#include "circular_buffer.h"
#include "Filtering.h"


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

char answerString[128];
uint8_t foundAnswer;


SemaphoreHandle_t DR_Semaphore = NULL;
CircularBuffer ECG_buffer;
QueueHandle_t xQueueECGData;
SemaphoreHandle_t wifiSemaphore;
uint8_t wifiUserBuffer[1024] = "";

extern u8 read_data[9];
extern u8 read_Index;
extern float32_t ECG_data_raw;         /*滤波前的ECG数据*/
extern float32_t ECG_data_filtered;    /*滤波后的ECG数据*/
int i = 0;

TaskHandle_t Main_TaskHandle;
TaskHandle_t Blood_TaskHandle;
TaskHandle_t LMT70_TaskHandle;
TaskHandle_t ADS1292_TaskHandle;
TaskHandle_t Filtering_TaskHandle;

extern uint32_t blockSize;
extern uint32_t numBlocks;
extern float32_t firState2[Block_Size + NumTaps - 1];
extern const float32_t BPF_5Hz_40Hz[NumTaps];

extern arm_fir_instance_f32 ADS1292;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

void Main_Task(void *pvParameters);
void Blood_Task(void *pvParameters);
void LMT70_Task(void *pvParameters);
void ADS1292_Task(void *pvParameters);
void Filtering_Task(void *pvParameters);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
body_condition_t body_Condition;
/* USER CODE END 0 */

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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_HS_USB_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_ETH_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  arm_fir_init_f32(&ADS1292, NumTaps, (float32_t *)BPF_5Hz_40Hz, firState2, blockSize);


  initBuffer(&ECG_buffer);
  xQueueECGData = xQueueCreate(256, sizeof(uint32_t));

  xTaskCreate(Blood_Task,"Blood Task",512,NULL,3,&Blood_TaskHandle);
  xTaskCreate(LMT70_Task,"LMT70_Task",512,NULL,3,&LMT70_TaskHandle);
  xTaskCreate(ADS1292_Task,"ADS1292 Task",1024,NULL,3,&ADS1292_TaskHandle);
  xTaskCreate(Filtering_Task,"Filtering Task",512,NULL,3,&Filtering_TaskHandle);


  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



void Main_Task(void *pvParameters)
{

  for (;;) {

  }
}

void Blood_Task(void *pvParameters)
{
  vTaskDelay(3000);
  Max30102_reset();     //重启max30102
  MAX30102_Config();    //配置max30102

  for (;;)
  {
      body_Condition.blood_ox = blood_Loop();
    vTaskDelay(2500);
  }
}

void LMT70_Task(void *pvParameters)
{
  osDelay(3000);
  lmt70_calibration();
  for (;;)
  {
    body_Condition.body_temperature = lmt70_get_temperature();
    vTaskDelay(100);
  }

}


void ADS1292_Task(void *pvParameters)
{

  if(ADS1292_PowerOnInit() == 1)
  {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port,LED_GREEN_Pin);
  }
  for (;;) {

    while (HAL_GPIO_ReadPin(ADS1292_DR_GPIO_Port, ADS1292_DR_Pin) == 1) {
      osDelay(2);
    }
    {
    ADS1292_Read_Data(read_data);
    ch1_data = 0;
    ch2_data = 0;
    ch1_data |= (uint32_t) read_data[3] << 16;
    ch1_data |= (uint32_t) read_data[4] << 8;
    ch1_data |= (uint32_t) read_data[5] << 0;
    ch2_data |= (uint32_t) read_data[6] << 16;
    ch2_data |= (uint32_t) read_data[7] << 8;
    ch2_data |= (uint32_t) read_data[8] << 0;
    ECG_data_raw = (float32_t) (ch2_data ^ 0x800000);
    if (xQueueSend(xQueueECGData, &ECG_data_raw, portMAX_DELAY) != pdPASS) {
    }
    }
  }
}

void Filtering_Task(void *pvParameters)
{
  int ECG_value;

  Filtering_Init();
  for(;;) {
    if(xQueueReceive(xQueueECGData, &ECG_data_raw, portMAX_DELAY) == pdTRUE) {

      char TEST_Buffer[128];
      arm_fir_f32(&ADS1292, &ECG_data_raw, &ECG_data_filtered, blockSize);
      sprintf(TEST_Buffer,"A=%d,B=%d,C=%d\r\n",(int)ECG_data_filtered,(int)(body_Condition.blood_ox*10),(int)(body_Condition.body_temperature*10));
      HAL_UART_Transmit(&huart1,(uint8_t*)TEST_Buffer,strlen(TEST_Buffer),1000);

    }
  }
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {

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
