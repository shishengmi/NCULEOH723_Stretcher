#ifndef __ADS1292_H
#define __ADS1292_H
#include "stm32h7xx_hal.h"
#include "Delay.h"
#include "main.h"
#include "gpio.h"
#include "arm_math.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#ifndef INC_FREERTOS_H
#define
#endif


#define SPI_HANDLE hspi1 //若修改了spi外设需要修改spi句柄


#define Delay_1us  delay_us
#define u8         uint8_t
#define u16        uint16_t
#define u32        uint32_t
//-----------------------------------------------------------------
// 位操作
//-----------------------------------------------------------------
#define PWDN_L      HAL_GPIO_WritePin(ADS1292_PW_GPIO_Port, ADS1292_PW_Pin, GPIO_PIN_RESET)
#define PWDN_H      HAL_GPIO_WritePin(ADS1292_PW_GPIO_Port, ADS1292_PW_Pin, GPIO_PIN_SET)
#define START_L     HAL_GPIO_WritePin(ADS1292_ST_GPIO_Port, ADS1292_ST_Pin, GPIO_PIN_RESET)
#define START_H     HAL_GPIO_WritePin(ADS1292_ST_GPIO_Port, ADS1292_ST_Pin, GPIO_PIN_SET)
#define CS_L        HAL_GPIO_WritePin(ADS1292_CS_GPIO_Port, ADS1292_CS_Pin, GPIO_PIN_RESET)
#define CS_H        HAL_GPIO_WritePin(ADS1292_CS_GPIO_Port, ADS1292_CS_Pin, GPIO_PIN_SET)

//-----------------------------------------------------------------
// ADS1292命令定义
//-----------------------------------------------------------------
// 系统命令
#define WAKEUP    0X02// 从待机模式唤醒
#define STANDBY   0X04// 进入待机模式
#define RESET     0X06// 复位ADS1292R
#define START     0X08// 启动或转换
#define STOP      0X0A// 停止转换
#define OFFSETCAL 0X1A// 通道偏移校准
// 数据读取命令
#define RDATAC    0X10// 启用连续的数据读取模式,默认使用此模式
#define SDATAC    0X11// 停止连续的数据读取模式
#define RDATA     0X12// 通过命令读取数据;支持多种读回。
// 寄存器读取命令
#define RREG      0X20// 读取001r rrrr 000n nnnn  其中r rrrr是起始寄存器地址
#define WREG      0X40// 写入010r rrrr 000n nnnn  其中r rrrr是起始寄存器地址 n nnnn=是要写的数据*/

// ADS1292R内部寄存器地址定义
#define ID        0X00// ID控制寄存器
#define CONFIG1   0X01// 配置寄存器1
#define CONFIG2   0X02// 配置寄存器2
#define LOFF      0X03// 导联脱落控制寄存器
#define CH1SET    0X04// 通道1设置寄存器
#define CH2SET    0X05// 通道2设置寄存器
#define RLD_SENS  0X06// 右腿驱动选择寄存器
#define LOFF_SENS 0X07// 导联脱落检测选择寄存器
#define LOFF_STAT 0X08// 导联脱落检测状态寄存器
#define RESP1     0X09// 呼吸检测控制寄存器1
#define RESP2     0X0A// 呼吸检测控制寄存器2
#define GPIO      0X0B// GPIO控制寄存器

//-----------------------------------------------------------------
// 外部函数声明
//-----------------------------------------------------------------
void GPIO_ADS1292_Configuration(void);
uint8_t ADS1292_PowerOnInit(void);
void ADS1292_Write_Reg(u8 addr, u8 data);
u8 ADS1292_Read_Reg(u8 addr);
void ADS1292_Read_Data(u8 *data);
u8 ADS1292_ReadWriteByte(u8 data);

extern u32 ch1_data;       //ADS1292通道1数据
extern u32 ch2_data;       //ADS1292通道2数据


#endif
