#include "ADS1292.h"
#include "Delay.h"
#include "spi.h"
#include "stdio.h"

u8 read_Index = 0;
u8 read_data[9];
u32 ch1_data;       //ADS1292通道1数据
u32 ch2_data;       //ADS1292通道2数据
float32_t ECG_data_raw;    // 滤波前
float32_t ECG_data_filtered;  // 滤波后
uint32_t  DMA_time = 0;



void ADS1292_Write_Reg(u8 addr, u8 data)
{
	CS_L;				// 片选拉低
  ADS1292_ReadWriteByte(addr);	// 包含命令操作码和寄存器地址
  osDelay(1);
  ADS1292_ReadWriteByte(0x00);	// 要读取的寄存器数+1
  osDelay(1);
  ADS1292_ReadWriteByte(data);	// 写入的数据
	osDelay(1);
	CS_H;				// 片选置高
}


u8 ADS1292_Read_Reg(u8 addr)
{
  u8 Rxdata;
	CS_L;					// 片选拉低
  ADS1292_ReadWriteByte(addr); 			// 包含命令操作码和寄存器地址
  osDelay(1);
  ADS1292_ReadWriteByte(0x00); 			// 要读取的寄存器数+1
  osDelay(1);
  Rxdata = ADS1292_ReadWriteByte(0xFF); 	// 读取的数据
	osDelay(1);
	CS_H;					// 片选置高
  return Rxdata;
}


uint8_t ADS1292_PowerOnInit(void)
{
	u8 device_id;
	int i = 0;
  START_H;
  CS_H;
  PWDN_L; // 进入掉电模式
  HAL_Delay(1000);
  PWDN_H; // 退出掉电模式
  HAL_Delay(1000);   // 等待稳定
  PWDN_L; // 发出复位脉冲
  osDelay(1);
  PWDN_H;
  HAL_Delay(1000); // 等待稳定，可以开始使用ADS1292R

	START_L;
	CS_L;
	osDelay(1);
  ADS1292_ReadWriteByte(SDATAC); // 发送停止连续读取数据命令
	osDelay(1);
	CS_H;

	// 获取芯片ID
	device_id = ADS1292_Read_Reg(RREG | ID);
	while(device_id != 0x73 && i<3)
	{
		printf("ERROR ID:%02x\r\n",device_id);
		device_id = ADS1292_Read_Reg(RREG | ID);
		osDelay(1000);
		i++;
	}
	if(i!= 0)
	{
		return 0;
	}

	osDelay(1);
  ADS1292_Write_Reg(WREG | CONFIG2,  0XE0); // 使用内部参考电压
  HAL_Delay(10);                            // 等待内部参考电压稳定
  ADS1292_Write_Reg(WREG | CONFIG1,  0X01); // 设置转换速率为250SPS
  osDelay(1);
  ADS1292_Write_Reg(WREG | LOFF,     0XF0);	// 该寄存器配置引出检测操作
  osDelay(1);
  ADS1292_Write_Reg(WREG | CH1SET,   0X60); // 增益12，连接到电极
  osDelay(1);
  ADS1292_Write_Reg(WREG | CH2SET,   0X00); // 增益6，连接到电极
  osDelay(1);
  ADS1292_Write_Reg(WREG | RLD_SENS, 0xEF); // 右腿驱动选择
  osDelay(1);
  ADS1292_Write_Reg(WREG | LOFF_SENS,0x0F); // 引导感选择
  osDelay(1);
	ADS1292_Write_Reg(WREG | LOFF_STAT,0x00); // 导出状态
  osDelay(1);
  ADS1292_Write_Reg(WREG | RESP1,    0xEA); // 开启呼吸检测（ADS1292R特有）
  osDelay(1);
  ADS1292_Write_Reg(WREG | RESP2,    0x03); // 呼吸控制寄存器2
  osDelay(1);
  ADS1292_Write_Reg(WREG | GPIO,     0x0C);
  osDelay(1);

	CS_L;
	osDelay(1);
	ADS1292_ReadWriteByte(RDATAC);
	osDelay(1);
	CS_H;
	START_H; 				// 启动转换
	CS_L;

  printf("ADS1292 PowerOnInit OK\r\n");

  return 1;
}


void ADS1292_Read_Data(u8 *data)
{
  u8 i;
  for (i = 0; i < 9; i++)		// 连续读取9个数据
  {
    *data = ADS1292_ReadWriteByte(0xFF);
    data++;
  }
}

u8 ADS1292_ReadWriteByte(u8 data)
{
	  u8 Rxdata;
	  HAL_SPI_TransmitReceive(&SPI_HANDLE, &data, &Rxdata, 1, 1000);
	  return Rxdata; // 返回收到的数据
}

u8 ADS1292_ReadWriteByte_DMA(u8 data)
{
  u8 Rxdata;
  HAL_SPI_TransmitReceive_DMA(&SPI_HANDLE, &data, &Rxdata, 1);
  return Rxdata; // 返回收到的数据
}

void DR_EXTI_callBack(void)
{
  //开始DMA接收，接收9次？这里是开启第一次的DMA接受
  read_Index = 0;
  read_data[read_Index]=ADS1292_ReadWriteByte_DMA(0xFF);

}

void DMA_TransmitCallBack(void)
{
  read_Index++;
  if(read_Index < 9)
  {
    read_data[read_Index]=ADS1292_ReadWriteByte_DMA(0xFF);
  }else{
    read_Index = 0;
  }
  //释放一个信号量，进行滤波处理
}


