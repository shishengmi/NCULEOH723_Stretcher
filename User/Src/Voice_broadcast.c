#include "Voice_broadcast.h"

/* 这个地址只要与STM32外挂的I2C器件地址不一样即可 */
#define I2Cx_OWN_ADDRESS7      0X0A

/* STM32 I2C 快速模式 */
#define I2C_Speed              100000
static uint32_t  I2CTimeout = I2CT_LONG_TIMEOUT;

void delay(unsigned long uldata)
{
  osDelay(1);
}


/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
static  uint8_t I2C_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
  //MPU_ERROR("I2C Timeout error!");

  return 0;
}



/**
  * @brief   写一个字节到I2C设备中
  * @param
  *		@arg pBuffer:写的值
  * @retval  正常返回1，异常返回0
  */
uint8_t I2C_ByteWrite(u8 pBuffer)
{

  if(HAL_I2C_Master_Transmit(&VOICE_I2C,(uint16_t)I2C_ADDR,&pBuffer,1,100)==HAL_OK)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}




void I2C_Writes_Bytes(u8 *buff,int number)
{
  HAL_I2C_Master_Transmit(&VOICE_I2C,(uint16_t)I2C_ADDR,buff,number,100);
}



/**
  * @brief   获取芯片状态值
  * @param
  * @retval  返回状态值
  */
u8 GetChipStatus(void)
{
  HAL_StatusTypeDef status;
  u16 NumByteToRead = 1;
  u8 pBuffer = 0xff;
  u8 AskState[4] = {0xFD,0x00,0x01,0x21};

  // 发送请求状态命令
  status = HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)0x60, AskState, 4, 200);

  osDelay(350); // 延时350毫秒

  // 从从设备读取1个字节的数据
  status = HAL_I2C_Master_Receive(&hi2c1, (uint16_t)0x60, &pBuffer, 1, 200);

  return pBuffer;
}



void speech_text(u8 *str,u8 encoding_format)
{
  u16 size = strlen(str) + 2;

  XFS_Protocol_TypeDef DataPack;

  DataPack.DataHead = DATAHEAD;
  DataPack.Length_HH = size >>8;
  DataPack.Length_LL = size;
  DataPack.Commond = 0x01;
  DataPack.EncodingFormat = encoding_format;
  DataPack.Text = str;

  I2C_ByteWrite(DataPack.DataHead );
  I2C_ByteWrite(DataPack.Length_HH);
  I2C_ByteWrite(DataPack.Length_LL);
  I2C_ByteWrite(DataPack.Commond);
  I2C_ByteWrite(DataPack.EncodingFormat);

  I2C_Writes_Bytes(DataPack.Text,strlen(DataPack.Text));
}

void SetBase(u8 *str)
{
  u16 size = strlen(str) + 2;

  XFS_Protocol_TypeDef DataPack;

  DataPack.DataHead = DATAHEAD;
  DataPack.Length_HH = size >>8;
  DataPack.Length_LL = size;
  DataPack.Commond = 0x01;
  DataPack.EncodingFormat = 0x00;
  DataPack.Text = str;

  I2C_ByteWrite(DataPack.DataHead );
  I2C_ByteWrite(DataPack.Length_HH);
  I2C_ByteWrite(DataPack.Length_LL);
  I2C_ByteWrite(DataPack.Commond);
  I2C_ByteWrite(DataPack.EncodingFormat);

  I2C_Writes_Bytes(DataPack.Text,strlen(DataPack.Text));
}

void TextCtrl(char c, int d)
{
  char str[10];
  if (d != -1)
    sprintf(str, "[%c%d]", c, d);
  else
    sprintf(str, "[%c]", c);
  SetBase(str);

}

void SetStyle(Style_Type style)
{
  TextCtrl('f', style);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetLanguage(Language_Type language)
{
  TextCtrl('g', language);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetArticulation(Articulation_Type articulation)
{
  TextCtrl('h', articulation);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetSpell(Spell_Type spell)
{
  TextCtrl('i', spell);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetReader(Reader_Type reader)
{
  TextCtrl('m', reader);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetNumberHandle(NumberHandle_Type numberHandle)
{
  TextCtrl('n', numberHandle);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetZeroPronunciation(ZeroPronunciation_Type zeroPronunciation)
{
  TextCtrl('o', zeroPronunciation);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}


void SetNamePronunciation(NamePronunciation_Type namePronunciation)
{
  TextCtrl('r', namePronunciation);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetSpeed(int speed)
{
  TextCtrl('s', speed);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetIntonation(int intonation)
{
  TextCtrl('t', intonation);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetVolume(int volume)
{
  TextCtrl('v', volume);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetOnePronunciation(OnePronunciation_Type onePronunciation)
{
  TextCtrl('y', onePronunciation);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetRhythm(Rhythm_Type rhythm)
{
  TextCtrl('z', rhythm);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}

void SetRestoreDefault()
{
  TextCtrl('d', -1);
  while(GetChipStatus() != ChipStatus_Idle)
  {
    delay(10);
  }
}




