#include "ESP8266.h"



#include "usart.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdio.h>
uint8_t ESP8266_Init(UART_HandleTypeDef *huart) {
  extern uint8_t Init_y;
  uint8_t targetReady[6] = "ready";
  uint8_t targetOK[3] = "OK";
  USART_printf(huart, "AT+RESTORE\r\n");
  if (USART_Receive_Compare(huart, targetReady, 5, 5000) == 1) {
    Init_y += 10;
  } else {
    return 2;
  }
  USART_printf(huart, "AT+CWMODE=%d\r\n", CWMODE);
  if (USART_Receive_Compare(huart, targetOK, 2, 5000) == 1) {

    Init_y += 10;
  } else {
    return 3;
  }
  USART_printf(huart, "AT+CWJAP=\"%s\",\"%s\"\r\n", pSSID, pPassWord);
  if (USART_Receive_Compare(huart, targetOK, 2, 5000) == 1) {

    Init_y += 10;
  } else {
    return 4;
  }
  USART_printf(huart, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", pClient_Id, pUserName, PassWord);
  if (USART_Receive_Compare(huart, targetOK, 2, 5000) == 1) {

    Init_y += 10;
  } else {
    return 5;
  }
  USART_printf(huart, "AT+MQTTCONN=0,\"%s\",%s,0\r\n", Ip, Port);
  if (USART_Receive_Compare(huart, targetOK, 2, 5000) == 1) {

    Init_y += 10;
  } else {
    return 6;
  }
  USART_printf(huart, "AT+MQTTSUB=0,\"%s\",1\r\n", Topic);
  if (USART_Receive_Compare(huart, targetOK, 2, 5000) == 1) {

    Init_y += 10;
  } else {
    return 7;
  }

  return 1;

}

void simpleDelay(uint32_t delay)
{
  for (volatile uint32_t i = 0; i < delay * 100000; i++);
}

uint8_t ESP8266_Init_UDP(UART_HandleTypeDef *huart)
{
  char ESP8266_Buffer[100];

// AT+RESTORE
  sprintf(ESP8266_Buffer, "AT+RESTORE\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(1000); // 用替代延迟函数代替 HAL_Delay

// AT+CWMODE
  sprintf(ESP8266_Buffer, "AT+CWMODE=%d\r\n", CWMODE);
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(1000); // 用替代延迟函数代替 HAL_Delay

// AT+CIPSTA
  sprintf(ESP8266_Buffer, "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\r\n", UDP_IP, GateWay, NetMask);
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(1000); // 用替代延迟函数代替 HAL_Delay

// AT+CWJAP
  sprintf(ESP8266_Buffer, "AT+CWJAP=\"%s\",\"%s\"\r\n", pSSID, pPassWord);
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(1000); // 用替代延迟函数代替 HAL_Delay

// AT+CIPSTART
  sprintf(ESP8266_Buffer, "AT+CIPSTART=\"UDP\",\"%s\",%d,%d\r\n", RemoteIp, RemotePort, LocalPort);
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(1000); // 用替代延迟函数代替 HAL_Delay

  sprintf(ESP8266_Buffer, "AT+CIPSEND=100\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  simpleDelay(200);
  sprintf(ESP8266_Buffer, "FUCKYOUYOUYOUYOU\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t *)ESP8266_Buffer, strlen(ESP8266_Buffer), 300);
  return 1;
}




/*
 * 函数需求：实现字符匹配算法，在规定时间内若检测到RXNE标志位非空就接收一个字符，将这个字符与目标串进行比较
 * 若接收到的字符与目标字符串的第一个相同则将接收到的下一个字符与目标字符串的第二位进行比较，否则将下一个接收
 * 到的字符串与第一个字符串比较直到超时或者匹配成功，返回不同的返回值
 *
 * 参数1：需要接收信息的串口结构体
 * 参数2：
 * */

uint8_t USART_Receive_Compare(UART_HandleTypeDef *huart,uint8_t * target,uint8_t size,uint16_t Timeout)
{
	uint16_t target_index = 0;
	uint8_t rxdata = 0;
	uint32_t Tickstart;
	uint8_t fuck[256] = {0};
	Tickstart = HAL_GetTick();

	while(1){
		if(__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET)
		{
			rxdata = huart->Instance->RDR;//接收一个数据，直接操作寄存器，高效
			if(rxdata == *(target + target_index))
			{
				target_index ++;
			}else{
				target_index =0;
			}
		}

		if(target_index == size)
		{
			HAL_UART_Receive(huart,fuck,256,200);//接收100ms内的多余数据
			return 1;//匹配数据成功
		}

	  if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))//超时处理返回0，这一段是从uart.c的超时处理抄到的
	  {
	  	HAL_UART_Receive(huart,fuck,256,200);//接收100ms内的多余数据
	    return 0;//超时

	  }
	}
	  return 0;
}



void USART_Send_Data(UART_HandleTypeDef *huart,uint8_t data)
{
	//该函数处理逻辑详见stm32h7xx_hal_uart.c,这里是对齐进行了一个简化
  if (huart->gState == HAL_UART_STATE_READY)
  {
    huart->Instance->TDR = (uint8_t)(data & 0xFFU);
    //这里&一个0xFFU 11111111是为了保证只有低八位数据进入寄存器保证安全，U代表其是一个无符号数
  }
}
//这个函数出现了一些问题，具体愿意我怀疑缺少了一些处理逻辑导致的，可能是因为函数处理太快导致发送数据的速度跟不上。

void USART_printf (UART_HandleTypeDef * huart, char * Data, ... )
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, Data);

    while ( * Data != 0 )     // 判断数据是否到达结束符
    {
        if ( * Data == 0x5c )  //'\'
        {
            switch ( *++Data )
            {
                case 'r':                                     //回车符
                USART_Send_Data(huart, 0x0d);
                Data ++;
                break;

                case 'n':                                     //换行符
                USART_Send_Data(huart, 0x0a);
                Data ++;
                break;

								case '"':
								USART_Send_Data(huart, 0x22);									//双引号符
								Data ++;

								case '\\':
								USART_Send_Data(huart, 0x5C);									//反斜杠
								Data ++;

                default:
                Data ++;
                break;
            }
        }

        else if ( * Data == '%')
        {
            switch ( *++Data )
            {
                case 's':                                         //字符串
                s = va_arg(ap, const char *);
                for ( ; *s; s++)
                {
                    USART_Send_Data(huart,*s);
                    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET);//等待发送完成，这个HAL函数及其返回值详见相关头文件、0代表空闲状态（发送完成）
                }
                Data++;
                break;

                case 'd':
                    //十进制
                d = va_arg(ap, int);
								sprintf(buf,"%d",d);
                for (s = buf; *s; s++)
                {
                    USART_Send_Data(huart,*s);
                    while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET);
                }
                     Data++;
                     break;
                default:
                     Data++;
                     break;
            }
        }
        else USART_Send_Data(huart, *Data++);
        while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE) == RESET);
    }
}

void ESP8266_MQTT_Push(UART_HandleTypeDef *huart,char *message)
{
	USART_printf(huart,"AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",Topic,message);
	//ESP8266WaiteMessage("OK",5000);
}
