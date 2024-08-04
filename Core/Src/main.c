/* USER CODE BEGIN Header */
/*
 * @file           main.c
 * @brief          主程序体
 *                 该文件包含各种外设的初始化和配置代码，FreeRTOS任务和应用逻辑。
 */
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
// 包含自定义头文件，用于WiFi，传感器，FreeRTOS，ARM数学库等。
#include "Wifi.h"
#include "LMT70.h"
#include "Voice_broadcast.h"
#include "Voice_task.h"
#include "MAX30102.h"
#include "blood.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "arm_math.h"
#include "ADS1292.h"
#include "circular_buffer.h"
#include "Filtering.h"
#include "ESP8266.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 私有类型定义
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 私有宏定义
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// 私有宏
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
// 私有变量

char answerString[128]; // 存储答案的字符串
uint8_t foundAnswer; // 标志是否找到答案

// 信号量和队列定义
SemaphoreHandle_t DR_Semaphore = NULL;
CircularBuffer ECG_buffer;
QueueHandle_t xQueueECGData;
SemaphoreHandle_t wifiSemaphore;
uint8_t wifiUserBuffer[1024] = "";

// 外部变量声明
extern u8 read_data[9];
extern u8 read_Index;
extern float32_t ECG_data_raw;         /* 滤波前的ECG数据 */
extern float32_t ECG_data_filtered;    /* 滤波后的ECG数据 */
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
// 私有函数原型
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
// 自定义任务函数原型
void Main_Task(void *pvParameters);
void Blood_Task(void *pvParameters);
void LMT70_Task(void *pvParameters);
void ADS1292_Task(void *pvParameters);
void Filtering_Task(void *pvParameters);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
body_condition_t body_Condition; // 定义身体状况结构体实例
/* USER CODE END 0 */

/**
  * @brief  应用程序的入口点。
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  // 用户代码开始1
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  // MCU配置

  /* 重置所有外设，初始化Flash接口和Systick。 */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // 用户初始化代码
  /* USER CODE END Init */

  /* 配置系统时钟 */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  // 用户系统初始化代码
  /* USER CODE END SysInit */

  /* 初始化所有配置的外设 */
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
  // 初始化ARM FIR滤波器
  arm_fir_init_f32(&ADS1292, NumTaps, (float32_t *)BPF_5Hz_40Hz, firState2, blockSize);

  // 初始化ECG缓冲区
  initBuffer(&ECG_buffer);
  // 创建ECG数据队列
  xQueueECGData = xQueueCreate(256, sizeof(uint32_t));

  // 创建各任务
  xTaskCreate(Blood_Task,"Blood Task",512,NULL,3,&Blood_TaskHandle);
  xTaskCreate(LMT70_Task,"LMT70_Task",512,NULL,3,&LMT70_TaskHandle);
  xTaskCreate(ADS1292_Task,"ADS1292 Task",1024,NULL,3,&ADS1292_TaskHandle);
  xTaskCreate(Filtering_Task,"Filtering Task",512,NULL,3,&Filtering_TaskHandle);

  /* USER CODE END 2 */

  /* 调用freertos对象的初始化函数（在cmsis_os2.c中） */
  MX_FREERTOS_Init();

  /* 启动调度器 */
  osKernelStart();

  /* 这里不应该到达，因为控制权现在由调度器接管 */

  /* 无限循环 */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 用户代码开始3
  }
  /* USER CODE END 3 */
}

/**
  * @brief 系统时钟配置
  * @retval 无
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** 使能电源配置更新
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** 配置主内部调节器输出电压
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** 根据RCC_OscInitTypeDef结构体中的指定参数初始化RCC振荡器
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

  /** 初始化CPU, AHB和APB总线时钟
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
// 主任务函数
void Main_Task(void *pvParameters)
{
  for (;;)
  {
    // 主任务循环
  }
}

// 血液任务函数
void Blood_Task(void *pvParameters)
{
  vTaskDelay(3000); // 延时3秒
  Max30102_reset(); // 重启max30102
  MAX30102_Config(); // 配置max30102

  for (;;)
  {
    body_Condition.blood_ox = blood_Loop(); // 获取血氧值
    vTaskDelay(2500); // 延时2.5秒
  }
}

// LMT70任务函数
void LMT70_Task(void *pvParameters)
{
  osDelay(3000); // 延时3秒
  lmt70_calibration(); // 校准LMT70传感器
  for (;;)
  {
    body_Condition.body_temperature = lmt70_get_temperature(); // 获取体温
    vTaskDelay(100); // 延时100毫秒
  }
}

// ADS1292任务函数
void ADS1292_Task(void *pvParameters)
{
  if(ADS1292_PowerOnInit() == 1)
  {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin); // 切换绿灯
  }
  for (;;)
  {
    while (HAL_GPIO_ReadPin(ADS1292_DR_GPIO_Port, ADS1292_DR_Pin) == 1)
    {
      osDelay(2); // 延时2毫秒
    }
    {
      ADS1292_Read_Data(read_data); // 读取数据
      ch1_data = 0;
      ch2_data = 0;
      ch1_data |= (uint32_t) read_data[3] << 16;
      ch1_data |= (uint32_t) read_data[4] << 8;
      ch1_data |= (uint32_t) read_data[5] << 0;
      ch2_data |= (uint32_t) read_data[6] << 16;
      ch2_data |= (uint32_t) read_data[7] << 8;
      ch2_data |= (uint32_t) read_data[8] << 0;
      ECG_data_raw = (float32_t) (ch2_data ^ 0x800000);
      if (xQueueSend(xQueueECGData, &ECG_data_raw, portMAX_DELAY) != pdPASS)
      {
        // 发送失败处理
      }
    }
  }
}

// 滤波任务函数
void Filtering_Task(void *pvParameters)
{
  int ECG_value;

  Filtering_Init(); // 初始化滤波器
  for (;;)
  {
    if(xQueueReceive(xQueueECGData, &ECG_data_raw, portMAX_DELAY) == pdTRUE)
    {
      char TEST_Buffer[128];
      arm_fir_f32(&ADS1292, &ECG_data_raw, &ECG_data_filtered, blockSize); // 执行FIR滤波
      sprintf(TEST_Buffer, "A=%d,B=%d,C=%d\r\n", (int)ECG_data_filtered, (int)(body_Condition.blood_ox * 10), (int)(body_Condition.body_temperature * 10));
      HAL_UART_Transmit(&huart1, (uint8_t*)TEST_Buffer, strlen(TEST_Buffer), 1000); // 通过UART发送数据
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  非阻塞模式下的定时器中断回调
  * @note   该函数在TIM1中断发生时调用，在HAL_TIM_IRQHandler()中调用。
  *         它直接调用HAL_IncTick()以增加全局变量"uwTick"，该变量用作应用程序时间基准。
  * @param  htim : TIM句柄
  * @retval 无
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  // 用户回调代码0
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick(); // 增加全局时间基准
  }
  /* USER CODE BEGIN Callback 1 */
  // 用户回调代码1
  /* USER CODE END Callback 1 */
}

/**
  * @brief  错误处理函数
  * @retval 无
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  // 用户可以添加自己的实现来报告HAL错误返回状态
  __disable_irq();
  while (1)
  {
    // 错误发生时进入无限循环
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  报告源文件名称和源行号发生的assert_param错误。
  * @param  file: 指向源文件名称的指针
  * @param  line: assert_param错误行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  // 用户可以添加自己的实现来报告文件名称和行号
  // 例如：printf("Wrong parameters value: file %s on line %d\r\n", file, line)
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
