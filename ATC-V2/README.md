# STM32 AT-Command LL Library

* http://www.github.com/NimaLTD   
* https://www.instagram.com/github.nimaltd/   
* https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw   

This is the AT-Command STM32 LL Library  

How to use this Library:
* Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings.   
* Enable USART and enable interrupt.   
* Enable TX DMA is optional.
* Enable FREERTOS is optional.
* Add atc_callback() into USART IRQ callback.
* If Enabled TX DMA, Add atc_callback_txDMA() into DMA callback.
* Include Header and source into your project.   
* Config "ATCConfig.h".   
* Call atc_init( .. .. .. ).   

```
#include "atc.h"

char answerString[128];
uint8_t foundAnswer; 
int main()
{
  atc_init(ATC_Element_0, USART3, 256, 50, "WIFI", DMA1, LL_DMA_STREAM_3);
  foundAnswer = atc_sendAtCommand(ATC_Element_0, "AT\r\n", 3000, answerString, 128, 2, "OK\r\n", "ERROR\r\n");
  
  while(1)
  {
    atc_process();
  }
}
```
in stm32f4xx_it.c
```
void USART3_IRQHandler(void)
{
  atc_callback(ATC_Element_0);
}

// if use tx dma
void DMA1_Stream3_IRQHandler(void)
{
  atc_callback_txDMA(ATC_Element_0);
}


```



# STM32 AT指令LL库

* [GitHub](http://www.github.com/NimaLTD)  
* [Instagram](https://www.instagram.com/github.nimaltd/)  
* [YouTube](https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw)  

这是用于STM32的AT指令LL库  

如何使用这个库：

* 在项目设置中选择“每个外设的‘.c/.h’文件对的通用外设初始化”。  
* 启用USART并启用中断。  
* 启用TX DMA是可选的。  
* 启用FREERTOS是可选的。  
* 将atc_callback()添加到USART IRQ回调中。  
* 如果启用了TX DMA，请将atc_callback_txDMA()添加到DMA回调中。  
* 将头文件和源文件包含到您的项目中。  
* 配置"ATCConfig.h"。  
* 调用atc_init( .. .. .. )。  

```c
#include "atc.h"

char answerString[128];
uint8_t foundAnswer; 
int main()
{
  atc_init(ATC_Element_0, USART3, 256, 50, "WIFI", DMA1, LL_DMA_STREAM_3);
  foundAnswer = atc_sendAtCommand(ATC_Element_0, "AT\r\n", 3000, answerString, 128, 2, "OK\r\n", "ERROR\r\n");
  
  while(1)
  {
    atc_process();
  }
}
```

在stm32f4xx_it.c中

```c
void USART3_IRQHandler(void)
{
  atc_callback(ATC_Element_0);
}

// 如果使用tx dma
void DMA1_Stream3_IRQHandler(void)
{
  atc_callback_txDMA(ATC_Element_0);
}
```



