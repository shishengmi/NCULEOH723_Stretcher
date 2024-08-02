/*作者：石胜米
 *邮箱：shishengmi@outlook.com
 *功能：在HAL库+STM32CubeMX配置的工程中实现ESP8266发送MQTT到指定的服务器
 *时间：2024年5月20日
 *使用说明：将.c和.h放入工程中，在STM32cubeMX中配置好USART，
 * */
#ifndef __ESP8266_H
#define __ESP8266_H
#endif


#include "stm32h7xx_hal.h"


#define ESP8266_huart huart2 //定义在usart.h中的一个UART_HandleTypeDef
#define CWMODE 1
#define pSSID "407"
#define pPassWord "22222222"
#define pClient_Id "ESP8266"
#define pUserName ""
#define PassWord ""
#define Ip "shishengmi.com"
#define Port "1883"
#define Topic "/sub"
#define UDP_IP "192.168.2.202"
#define GateWay "192.168.2.1"
#define NetMask "255.255.255.0"
#define RemoteIp "192.168.2.110"
#define RemotePort 9999
#define LocalPort 100

void USART_Send_Data(UART_HandleTypeDef *huart,uint8_t data);
void USART_printf (UART_HandleTypeDef *huart, char * Data, ... );
uint8_t USART_Receive_Compare(UART_HandleTypeDef *huart,uint8_t * target,uint8_t size,uint16_t Timeout);
uint8_t  ESP8266_Init(UART_HandleTypeDef *huart);
void ESP8266_MQTT_Push(UART_HandleTypeDef *huart,char *message);
uint8_t ESP8266_Init_UDP(UART_HandleTypeDef *huart);
