#include "LMT70.h"


int32_t temp_array[70] = {0};

/* 电压温度转换边，电压为μv，温度从-50度开始，步进为10度，向上增加
 * 比如vol2temp_table[0] 表示-50度，vol2temp_table[1]就表示-40度。
 * 此处最好换成对应adc转换后的数字输出，这样就可以避免adc输出换算成
 * 电压导致的运算误差，同时也提高了运算速度。
 */
float v2t_tabl[VOL2TEMP_TABLE_SIZE] = {
                                1350.441,1300.593,1250.398,1199.884,1149.070,
                                1097.987,1046.647, 995.050, 943.227, 891.178,
                                 838.882, 786.360, 733.608, 680.654, 627.490,
                                 574.117, 520.551, 466.760, 412.739, 358.164,
                                 302.785
} ;

/* 斜率表，在十度区间内使用线性插值法 */
float slope_tabl[VOL2TEMP_TABLE_SIZE] = {
                                4.985,5.020,5.051,5.081,5.108,
                                5.134,5.160,5.182,5.205,5.230,
                                5.252,5.275,5.295,5.316,5.337,
                                5.357,5.379,5.402,5.458,5.538,
                                5.538
} ;

/* 折半查表实现 */
static int16_t haltserach( float vol)
{
    int16_t low = 0 ;
    int16_t up = VOL2TEMP_TABLE_SIZE ;
    int16_t mid ;

    while ( up >= low)
    {
        mid = ( low + up) >> 1 ;

        if ( v2t_tabl[mid] < vol)
        {
            up = mid - 1 ;
        }
        else if ( vol < v2t_tabl[mid])
        {
            low = mid + 1 ;
        }
        else
        {
            return mid ;
        }
    }

    /* 不在线性表里，返回-1或RT2TEMP_TABLE_SIZE表示不在表内 */
    /* 其他值，则取下边界温度并返回 */

    return up ;
}

/** @brief  初始化lmt70
  * @details 完成与lmt70相关的底层外设的初始化。例如adc，gpio。
  * @retval NONE
  */
void lmt70_init(void)
{

}

/** @brief  从lmt70获取温度数据
  * @details 使用一阶导数查表法实现电压-温度转换
  * @param[in]  vol lmt70输出电压
  * @retval 电压-温度转换结果放大100倍 -5000到+15000度
  */
int32_t lmt70_get_temp( float vol)
{
    int16_t i ;
    int32_t rev = 0 ;

    i = haltserach(vol) ;

    if ( ( i != -1) && ( i != VOL2TEMP_TABLE_SIZE))
    {
        rev = ( ( ( i * 10) - 50) + ( ( v2t_tabl[i] - vol) / slope_tabl[i])) * 100 ;
    }

    return rev ;
}

/*!
 * LMT70获取温度
 * @return 转换后的温度值
 */
float lmt70_get_temperature(void)
{
  float temperature = 0;
  float voltage = 0;
  uint32_t adc_value = 0;
  uint32_t temp_X100  =0;

  HAL_ADC_Start(&LMT70_ADC);//开始转换
  if (HAL_ADC_PollForConversion(&LMT70_ADC, 1000) == HAL_OK) // 等待转换完成
  {
    adc_value = HAL_ADC_GetValue(&LMT70_ADC);
    voltage = (float)((float)adc_value/(float)ADC_RESOLUTION*(float)REFERENCE_VOLTAGE);
    temperature = (float)lmt70_get_temp(voltage*1000)/100;
  }
  return temperature;
}


/*!
 * LMT70校准
 */
void lmt70_calibration(void)
{
  HAL_ADCEx_Calibration_Start(&LMT70_ADC,ADC_CALIB_OFFSET_LINEARITY,ADC_SINGLE_ENDED);
}


