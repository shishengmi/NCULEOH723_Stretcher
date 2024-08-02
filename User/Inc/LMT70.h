/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LMT70_H
#define __LMT70_H
#endif


/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "adc.h"
#include "stm32h7xx_hal_adc.h"

#define LMT70_ADC			        hadc1          //ADC
#define ADC_RESOLUTION        65535           //ADC分辨率，分辨率的计算公式是2^(ADC的位数)-1 常见的ADC位数有12位和16位
#define REFERENCE_VOLTAGE     3.3             //参考电压
#define VOL2TEMP_TABLE_SIZE   21              //折半查表的长度


/** @brief  从lmt70获取温度数据
  * @details 使用一阶导数查表法实现电压-温度转换
  * @param[in]  vol lmt70输出电压
  * @retval 电压-温度转换结果放大100倍 -5000到+15000度
  */
int32_t lmt70_get_temp( float vol) ;
float lmt70_get_temperature(void);
void lmt70_calibration(void);



