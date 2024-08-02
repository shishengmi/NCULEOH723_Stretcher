#include "Wifi.h"
#include "main.h"



//#######################################################################################
void	Wifi_UserInit(void)
{
  /*!
   * 重启初始化
   * 设置wifi模式为sta
   * 设置ip地址
   * 连接到ap
   *
   * 与TCP服务器建立TCP连接，接受来自控制信息和体温血氧气等信息
   * 同时需要发送ECG心电的信息
   */
  Wifi_Restart();
  Wifi_TcpIp_GetMultiConnection();
  Wifi_TcpIp_SetMultiConnection(false);
  Wifi_GetMode();
  Wifi_SetMode(WifiMode_Station);
  Wifi_Station_SetIp("192.168.2.202","192.168.2.1","255.255.255.0");
  while (Wifi_Station_ConnectToAp("407","22222222",NULL) == false);
  Wifi_TcpIp_StartUdpConnection(1,"192.168.2.110",9999,100);
}
//#######################################################################################
void  Wifi_UserProcess(void)
{
//  static uint8_t last=0;
//  if(strstr(Wifi.MyIP,"0.0.0.0")!=NULL)
//  {
//    if(last!=1)
//	{
//
//	}
//    last=1;
//  }
//  else
//  {
//    if(last!=2)
//    {
//      Wifi_TcpIp_StartTcpConnection(0,Wifi.MyGateWay,33333,10);
//    }
//    last=2;
//  }

}

//#######################################################################################
void  Wifi_UserGetUdpData(uint8_t LinkId,uint16_t DataLen,uint8_t *Data)
{
  Wifi_TcpIp_SendDataUdp(LinkId,2,(uint8_t*)"OK");
}
//#######################################################################################
void  Wifi_UserGetTcpData(uint8_t LinkId,uint16_t DataLen,uint8_t *Data)
{
  Wifi_TcpIp_SendDataTcp(LinkId,2,(uint8_t*)"OK");
}
//#######################################################################################
