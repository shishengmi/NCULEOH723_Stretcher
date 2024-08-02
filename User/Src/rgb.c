/***
	************************************************************************************************************************************************************************************************
	*	@version V1.2
	*  	@date    2024-5-20
	*	@author  YY
	*	@define 基于DMA2D的寄存器的屏幕驱动显示
   *************************************************************************************************
*/

#include "rgb.h"
#include "ltdc.h"
#include "gpio.h"
#include "lcd_fonts.h"
#include "lcd_image.h"
#include "stdio.h"

static pFONT *LCD_Fonts;		// 英文字体
static pFONT *LCD_CHFonts;		// 中文字体

//LCD相关参数结构体
struct
{
	uint32_t Color;  			//	LCD当前画笔颜色
	uint32_t BackColor;			//	背景色
	uint32_t ColorMode;			// 颜色格式
	uint32_t LayerMemoryAdd;	//	层显存地址
	uint8_t  Layer; 			//	当前层
	uint8_t  Direction;			//	显示方向
	uint8_t  BytesPerPixel;		// 每个像素所占字节数
	uint8_t  ShowNum_Mode;		// 数字显示模式
}LCD;


/*
 * @brief 屏幕默认配置初始化
 * @retval void
 *
 * */

void Lcd_Init(void){
/*---------------------------------- 初始化一些默认配置 --------------------------------*/

	LCD_DisplayDirection(Direction_H); 		//	设置横屏显示
	LCD_SetFont(&Font12); 				 	//	设置默认字体
	LCD_ShowNumMode(Fill_Space);			//	设置数字显示默认填充空格

	LCD_SetLayer(0);  						// 切换到 layer0
	LCD_SetBackColor(0xff121212); 				//	设置背景色0xffB9EDF8
	LCD_SetColor(LCD_WHITE);				//	设置画笔颜色
	LCD_Clear(); 								//	清屏，刷背景色

/*---------------------------------- 如果开启了两层显示 --------------------------------*/

#if LCD_NUM_LAYERS == 2

	LCD_SetLayer(1); 						// 切换到前景层 layer1，layer1始终在layer0之上
	LCD_SetBackColor(LCD_BLACK); 			//	设置背景色
	LCD_SetColor(LCD_WHITE);				//	设置画笔颜色
	LCD_Clear(); 							//	清屏，刷背景色

#endif
	HAL_Delay(200);						//延时防止闪屏
	BL_ON;						 		// 开启背光

}




/*************************************************************************************************
*	函 数 名:	LCD_SetLayer
*	入口参数:	layer - 要显示和操作的层，可以设置为0或1，即选择 layer0 或 layer1
*	返 回 值:	无
*	函数功能:	设置要显示和操作的层，切换相应的显存地址、颜色格式等
*	说    明:	750的LTDC层顺序是固定的，layer1 在 layer0之上，即开启两层显示时，
*					layer1 是前景层，通常使用带透明色的颜色格式，layer0 是背景层，
*					只开启单层时，默认只操作 layer0
*************************************************************************************************/

void LCD_SetLayer(uint8_t layer)
{
#if LCD_NUM_LAYERS == 2		// 如果开了双层

	if (layer == 0)			// 如果设置的是 layer0
	{
		LCD.LayerMemoryAdd = LCD_MemoryAdd; 	// 获取 layer0 的显存地址
		LCD.ColorMode      = ColorMode_0;		// 获取 layer0 的颜色格式
		LCD.BytesPerPixel  = BytesPerPixel_0;	// 获取 layer0 的每个像素所需字节数的大小
	}
	else if(layer == 1)	 // 如果设置的是 layer1
	{
		LCD.LayerMemoryAdd = LCD_MemoryAdd + LCD_MemoryAdd_OFFSET;	// 获取 layer1 的显存地址
		LCD.ColorMode      = ColorMode_1;                           // 获取 layer1 的颜色格式
		LCD.BytesPerPixel  = BytesPerPixel_1;		                  // 获取 layer1 的每个像素所需字节数的大小
	}
	LCD.Layer = layer;	//记录当前所在的层

#else		// 如果只开启单层，默认操作 layer0

	LCD.LayerMemoryAdd = LCD_MemoryAdd;		// 获取 layer0 的显存地址
	LCD.ColorMode      = ColorMode_0;      // 获取 layer0 的颜色格式
	LCD.BytesPerPixel  = BytesPerPixel_0;	// 获取 layer0 的每个像素所需字节数的大小
	LCD.Layer = 0;		// 层标记设置为 layer0

#endif
}




/***************************************************************************************************************
*	函 数 名:	LCD_SetColor
*
*	入口参数:	Color - 要显示的颜色，示例：0xff0000FF 表示不透明的蓝色，0xAA0000FF 表示透明度为66.66%的蓝色
*
*	函数功能:	此函数用于设置显示字符、画点画线、绘图的颜色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用32位的颜色格式，用户无需关心颜色格式的转换
*					2. 32位的颜色中，从高位到低位分别对应 A、R、G、B  4个颜色通道
*					3. 高8位的透明通道中，ff表示不透明，0表示完全透明
*					4. 除非使用ARGB1555和ARGB8888等支持透明色的颜色格式，不然透明色不起作用，其中ARGB1555仅支持一位
*						透明色，即仅有透明和不透明两种状态，ARGB8888支持255级透明度
*					5. 这里说到的透明，是指 背景层、layer0和layer1 之间的透明
*
***************************************************************************************************************/

void LCD_SetColor(uint32_t Color)
{
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//将32位色转换为16位色
	{
		Red_Value   = (uint16_t)((Color&0x00F80000)>>8);
		Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//将32位色转换为ARGB1555颜色
	{
		if( (Color & 0xFF000000) == 0 )	//判断是否使用透明色
			Alpha_Value = 0x0000;
		else
			Alpha_Value = 0x8000;

		Red_Value   = (uint16_t)((Color&0x00F80000)>>9);
		Green_Value = (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//将32位色转换为ARGB4444颜色
	{

		Alpha_Value = (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   = (uint16_t)((Color&0x00F00000)>>12);
		Green_Value = (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>4);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}
	else
		LCD.Color = Color;	//24位色或32位色不需要转换
}

/***************************************************************************************************************
*	函 数 名:	LCD_SetBackColor
*
*	入口参数:	Color - 要显示的颜色，示例：0xff0000FF 表示不透明的蓝色，0xAA0000FF 表示透明度为66.66%的蓝色
*
*	函数功能:	设置背景色,此函数用于清屏以及显示字符的背景色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用32位的颜色格式，用户无需关心颜色格式的转换
*					2. 32位的颜色中，从高位到低位分别对应 A、R、G、B  4个颜色通道
*					3. 高8位的透明通道中，ff表示不透明，0表示完全透明
*					4. 除非使用ARGB1555和ARGB8888等支持透明色的颜色格式，不然透明色不起作用，其中ARGB1555仅支持一位
*						透明色，即仅有透明和不透明两种状态，ARGB8888支持255级透明度
*					5. 这里说到的透明，是指 背景层、layer0和layer1之间的透明
*
***************************************************************************************************************/

void LCD_SetBackColor(uint32_t Color)
 {
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0;  //各个颜色通道的值

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//将32位色转换为16位色
	{
		Red_Value   	= (uint16_t)((Color&0x00F80000)>>8);
		Green_Value 	= (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor	= (uint16_t)(Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//将32位色转换为ARGB1555颜色
	{
		if( (Color & 0xFF000000) == 0 )	//判断是否使用透明色
			Alpha_Value = 0x0000;
		else
			Alpha_Value = 0x8000;

		Red_Value   	= (uint16_t)((Color&0x00F80000)>>9);
		Green_Value 	= (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//将32位色转换为ARGB4444颜色
	{

		Alpha_Value 	= (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   	= (uint16_t)((Color&0x00F00000)>>12);
		Green_Value 	= (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>4);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}

	else
		LCD.BackColor = Color;	//24位色或32位色不需要转换

}

/***************************************************************************************************************
*	函 数 名:	LCD_SetFont
*
*	入口参数:	*fonts - 要设置的ASCII字体
*
*	函数功能:	设置ASCII字体，可选择使用 3216/2412/2010/1608/1206 五种大小的字体
*
*	说    明:	1. 使用示例 LCD_SetFont(&Font24) ，即设置 2412的 ASCII字体
*					2. 相关字模存放在 lcd_fonts.c
*
***************************************************************************************************************/

void LCD_SetFont(pFONT *fonts)
{
  LCD_Fonts = fonts;
}

/***************************************************************************************************************
*	函 数 名:	LCD_DisplayDirection
*
*	入口参数:	direction - 要显示的方向
*
*	函数功能:	设置要显示的方向，可输入参数 Direction_H 代表横屏显示，Direction_V 代表竖直显示
*
*	说    明:   使用示例 LCD_DisplayDirection(Direction_H) ，即设置屏幕横屏显示
*
***************************************************************************************************************/

void LCD_DisplayDirection(uint8_t direction)
{
	LCD.Direction = direction;
}

/***************************************************************************************************************
*	函 数 名:	LCD_Clear
*
*	函数功能:	清屏函数，将LCD清除为 LCD.BackColor 的颜色，使用DMA2D实现
*
*	说    明:	先用 LCD_SetBackColor() 设置要清除的背景色，再调用该函数清屏即可
*
***************************************************************************************************************/

void LCD_Clear(void)
{

	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	设置颜色格式
	DMA2D->OOR		=	0;										//	设置行偏移
	DMA2D->OMAR		=	LCD.LayerMemoryAdd ;				// 地址
	DMA2D->NLR		=	(LCD_Width<<16)|(LCD_Height);	//	设定长度和宽度
	DMA2D->OCOLR	=	LCD.BackColor;						//	颜色

/******
等待 垂直数据使能显示状态 ，即LTDC即将刷完一整屏数据的时候
因为在屏幕没有刷完一帧时进行刷屏，会有撕裂的现象
用户也可以使用 寄存器重载中断 进行判断，不过为了保证例程的简洁以及移植的方便性，这里直接使用判断寄存器的方法

如果不做判断，DMA2D刷屏速度如下

颜色格式	RGB565	 RGB888	 ARGB888
耗时	   4.3ms	    7.5ms	  11.9ms


加了之后，不管哪种格式，都需要17.6ms刷一屏，不过屏幕本身的刷新率只有60帧左右（LTDC时钟33MHz），
17.6ms的速度已经足够了，除非是对速度要求特别高的场合，不然建议加上判断垂直等待的语句，可以避免撕裂效应

******/
	while( LTDC->CDSR != 0X00000001);	// 判断 显示状态寄存器LTDC_CDSR 的第0位 VDES：垂直数据使能显示状态

	DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D

	while (DMA2D->CR & DMA2D_CR_START) ;				//	等待传输完成

}

/***************************************************************************************************************
*	函 数 名:	LCD_ClearRect
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					width  - 要清除区域的横向长度
*					height - 要清除区域的纵向宽度
*
*	函数功能:	局部清屏函数，将指定位置对应的区域清除为 LCD.BackColor 的颜色
*
*	说    明:	1. 先用 LCD_SetBackColor() 设置要清除的背景色，再调用该函数清屏即可
*					2. 使用示例 LCD_ClearRect( 10, 10, 100, 50) ，清除坐标(10,10)开始的长100宽50的区域
*
***************************************************************************************************************/

void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{

	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	设置颜色格式
	DMA2D->OCOLR	=	LCD.BackColor ;					//	颜色

	if(LCD.Direction == Direction_H)  //横屏填充
	{
		DMA2D->OOR		=	LCD_Width - width;				//	设置行偏移
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);	// 地址;
		DMA2D->NLR		=	(width<<16)|(height);			//	设定长度和宽度
	}
	else	//竖屏填充
	{
		DMA2D->OOR		=	LCD_Width - height;		//	设置行偏移
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// 地址
		DMA2D->NLR		=	(width)|(height<<16);	//	设定长度和宽度
	}

	DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D

	while (DMA2D->CR & DMA2D_CR_START) ;			//	等待传输完成

}


/***************************************************************************************************************
*	函 数 名:	LCD_DrawPoint
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					color  - 要绘制的颜色，使用32位的颜色格式，用户无需关心颜色格式的转换
*
*	函数功能:	在指定坐标绘制指定颜色的点
*
*	说    明:	1. 直接在对应的显存位置写入颜色值，即可实现画点的功能
*					2. 使用示例 LCD_DrawPoint( 10, 10, 0xff0000FF) ，在坐标(10,10)绘制蓝色的点
*
***************************************************************************************************************/

void LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{

/*----------------------- 32位色 ARGB8888 模式 ----------------------*/

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) ) = color ;
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
		}
	}
/*----------------------------- 24位色 RGB888 模式 -------------------------*/

	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) = color ;
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) + 2 ) = color>>16 ;
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) +2) = color>>16 ;
		}
	}

/*----------------------- 16位色 ARGB1555、RGB565或者ARGB4444 模式 ----------------------*/
	else
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) ) = color ;
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
		}
	}
}

/***************************************************************************************************************
*	函 数 名:	LCD_ReadPoint
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*
*	返 回 值：  读取到的颜色
*
*	函数功能:	读取指定坐标点的颜色，在使用16或24位色模式时，读出来的颜色数据对应为16位或24位
*
*	说    明:	1. 直接读取对应的显存值，即可实现读点的功能
*					2. 使用示例 color = LCD_ReadPoint( 10, 10) ，color 为读取到的坐标点(10,10)的颜色
*
***************************************************************************************************************/

uint32_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
	uint32_t color = 0;

/*----------------------- 32位色 ARGB8888 模式 ----------------------*/
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) );
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) );
		}
	}

/*----------------------------- 24位色 RGB888 模式 -------------------------*/
	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) &0x00ffffff;
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) &0x00ffffff;
		}
	}

/*----------------------- 16位色 ARGB1555、RGB565或者ARGB4444 模式 ----------------------*/
	else
	{
		if (LCD.Direction == Direction_H) //水平方向
		{
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) );
		}
		else if(LCD.Direction == Direction_V)	//垂直方向
		{
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) );
		}
	}
	return color;
}

/***************************************************************************************************************
*	函 数 名:	LCD_DisplayChar
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					c  - ASCII字符
*
*	函数功能:	在指定坐标显示指定的字符
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetFont(&Font24) 设置为 2412的ASCII字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayChar( 10, 10, 'a') ，在坐标(10,10)显示字符 'a'
*
***************************************************************************************************************/

void LCD_DisplayChar(uint16_t x, uint16_t y,uint8_t c)
{
	uint16_t  index = 0, counter = 0;		// 计数变量
   uint8_t   disChar;		//存储字符的地址
	uint16_t  Xaddress = x; //水平坐标

	c = c - 32; 	// 计算ASCII字符的偏移

	for(index = 0; index < LCD_Fonts->Sizes; index++)
	{
		disChar = LCD_Fonts->pTable[c*LCD_Fonts->Sizes + index]; //获取字符的模值
		for(counter = 0; counter < 8; counter++)
		{
			if(disChar & 0x01)
			{
				LCD_DrawPoint(Xaddress,y,LCD.Color);		//当前模值不为0时，使用画笔色绘点
			}
			else
			{
				LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//否则使用背景色绘制点
			}
			disChar >>= 1;
			Xaddress++;  //水平坐标自加

			if( (Xaddress - x)==LCD_Fonts->Width ) //如果水平坐标达到了字符宽度，则退出当前循环
			{													//进入下一行的绘制
				Xaddress = x;
				y++;
				break;
			}
		}
	}
}

/***************************************************************************************************************
*	函 数 名:	LCD_DisplayString
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					p - ASCII字符串的首地址
*
*	函数功能:	在指定坐标显示指定的字符串
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetFont(&Font24) 设置为 2412的ASCII字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayString( 10, 10, "FANKE") ，在起始坐标为(10,10)的地方显示字符串"FANKE"
*
***************************************************************************************************************/

void LCD_DisplayString( uint16_t x, uint16_t y, char *p)
{
	while ((x < LCD_Width) && (*p != 0))	//判断显示坐标是否超出显示区域并且字符是否为空字符
	{
		 LCD_DisplayChar( x,y,*p);
		 x += LCD_Fonts->Width; //显示下一个字符
		 p++;	//取下一个字符地址
	}
}

/***************************************************************************************************************
*	函 数 名:	LCD_SetTextFont
*
*	入口参数:	*fonts - 要设置的文本字体
*
*	函数功能:	设置文本字体，包括中文和ASCII字符，
*
*	说    明:	1. 可选择使用 3232/2424/2020/1616/1212 五种大小的中文字体，
*						并且对应的设置ASCII字体为 3216/2412/2010/1608/1206
*					2. 相关字模存放在 lcd_fonts.c
*					3. 中文字库使用的是小字库，即用到了对应的汉字再去取模
*					4. 使用示例 LCD_SetTextFont(&CH_Font24) ，即设置 2424的中文字体以及2412的ASCII字符字体
*
***************************************************************************************************************/

void LCD_SetTextFont(pFONT *fonts)
{
	LCD_CHFonts = fonts;		// 设置中文字体
	switch(fonts->Width )
	{
		case 12:	LCD_Fonts = &Font12;	break;	// 设置ASCII字符的字体为 1206
		case 16:	LCD_Fonts = &Font16;	break;	// 设置ASCII字符的字体为 1608
		case 20:	LCD_Fonts = &Font20;	break;	// 设置ASCII字符的字体为 2010
		case 24:	LCD_Fonts = &Font24;	break;	// 设置ASCII字符的字体为 2412
		case 32:	LCD_Fonts = &Font32;	break;	// 设置ASCII字符的字体为 3216
		default: break;
	}

}
/***************************************************************************************************************
*	函 数 名:	LCD_DisplayChinese
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					pText - 中文字符
*
*	函数功能:	在指定坐标显示指定的单个中文字符
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetTextFont(&CH_Font24) 设置为 2424的中文字体以及2412的ASCII字符字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayChinese( 10, 10, "反") ，在坐标(10,10)显示中文字符"反"
*
***************************************************************************************************************/

void LCD_DisplayChinese(uint16_t x, uint16_t y, char *pText)
{
	uint16_t  i=0,index = 0, counter = 0;	// 计数变量
	uint16_t  addr;	// 字模地址
   uint8_t   disChar;	//字模的值
	uint16_t  Xaddress = x; //水平坐标

	while(1)
	{
		// 对比数组中的汉字编码，用以定位该汉字字模的地址
		if ( *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 0)==*pText && *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 1)==*(pText+1) )
		{
			addr=i;	// 字模地址偏移
			break;
		}
		i+=2;	// 每个中文字符编码占两字节

		if(i >= LCD_CHFonts->Table_Rows)	break;	// 字模列表中无相应的汉字
	}


	for(index = 0; index <LCD_CHFonts->Sizes; index++)
	{
		disChar = *(LCD_CHFonts->pTable + (addr)*LCD_CHFonts->Sizes + index);	// 获取相应的字模地址

		for(counter = 0; counter < 8; counter++)
		{
			if(disChar & 0x01)
			{
				LCD_DrawPoint(Xaddress,y,LCD.Color);	//当前模值不为0时，使用画笔色绘点
			}
			else
			{
				LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//否则使用背景色绘制点
			}
			disChar >>= 1;
			Xaddress++;  //水平坐标自加

			if( (Xaddress - x)==LCD_CHFonts->Width ) 	//	如果水平坐标达到了字符宽度，则退出当前循环
			{														//	进入下一行的绘制
				Xaddress = x;
				y++;
				break;
			}
		}
	}

}

/***************************************************************************************************************
*	函 数 名:	LCD_DisplayText
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					pText - 字符串，可以显示中文或者ASCII字符
*
*	函数功能:	在指定坐标显示指定的字符串
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetTextFont(&CH_Font24) 设置为 2424的中文字体以及2412的ASCII字符字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayChinese( 10, 10, "反客科技STM32") ，在坐标(10,10)显示字符串"反客科技STM32"
*
***************************************************************************************************************/

void LCD_DisplayText(uint16_t x, uint16_t y, char *pText)
{

	while(*pText != 0)	// 判断是否为空字符
	{
		if(*pText<=0x7F)	// 判断是否为ASCII码
		{
			LCD_DisplayChar(x,y,*pText);	// 显示ASCII
			x+=LCD_Fonts->Width;				// 水平坐标调到下一个字符处
			pText++;								// 字符串地址+1
		}
		else					// 若字符为汉字
		{
			LCD_DisplayChinese(x,y,pText);	// 显示汉字
			x+=LCD_CHFonts->Width;				// 水平坐标调到下一个字符处
			pText+=2;								// 字符串地址+2，汉字的编码要2字节
		}
	}
}


/***************************************************************************************************************
*	函 数 名:	LCD_ShowNumMode
*
*	入口参数:	mode - 设置变量的显示模式
*
*	函数功能:	设置变量显示时多余位补0还是补空格，可输入参数 Fill_Space 填充空格，Fill_Zero 填充零
*
*	说    明:   1. 只有 LCD_DisplayNumber() 显示整数 和 LCD_DisplayDecimals()显示小数 这两个函数用到
*					2. 使用示例 LCD_ShowNumMode(Fill_Zero) 设置多余位填充0，例如 123 可以显示为 000123
*
***************************************************************************************************************/

void LCD_ShowNumMode(uint8_t mode)
{
	LCD.ShowNum_Mode = mode;
}

/*****************************************************************************************************************************************
*	函 数 名:	LCD_DisplayNumber
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					number - 要显示的数字,范围在 -2147483648~2147483647 之间
*					len - 数字的位数，如果位数超过len，将按其实际长度输出，不包括正负号。但如果需要显示负数，请预留一个位的符号显示空间
*
*	函数功能:	在指定坐标显示指定的整数变量
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetTextFont(&CH_Font24) 设置为 2424的中文字体以及2412的ASCII字符字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayNumber( 10, 10, a, 5) ，在坐标(10,10)显示指定变量a,总共5位，多余位补0或空格，
*						例如 a=123 时，会根据 LCD_ShowNumMode()的设置来显示  123(前面两个空格位) 或者00123
*
*****************************************************************************************************************************************/

void  LCD_DisplayNumber( uint16_t x, uint16_t y, int32_t number, uint8_t len)
{
	char   Number_Buffer[15];				// 用于存储转换后的字符串

	if( LCD.ShowNum_Mode == Fill_Zero)	// 多余位补0
	{
		sprintf( Number_Buffer , "%0.*d",len, number );	// 将 number 转换成字符串，便于显示
	}
	else			// 多余位补空格
	{
		sprintf( Number_Buffer , "%*d",len, number );	// 将 number 转换成字符串，便于显示
	}

	LCD_DisplayString( x, y,(char *)Number_Buffer) ;  // 将转换得到的字符串显示出来

}

/***************************************************************************************************************************************
*	函 数 名:	LCD_DisplayDecimals
*
*	入口参数:	x - 起始水平坐标，取值范围0~799
*					y - 起始垂直坐标，取值范围0~479
*					decimals - 要显示的数字, double型取值1.7 x 10^（-308）~ 1.7 x 10^（+308），但是能确保准确的有效位数为15~16位
*
*       			len - 整个变量的总位数（包括小数点和负号），若实际的总位数超过了指定的总位数，将按实际的总长度位输出，
*							示例1：小数 -123.123 ，指定 len <=8 的话，则实际照常输出 -123.123
*							示例2：小数 -123.123 ，指定 len =10 的话，则实际输出   -123.123(负号前面会有两个空格位)
*							示例3：小数 -123.123 ，指定 len =10 的话，当调用函数 LCD_ShowNumMode() 设置为填充0模式时，实际输出 -00123.123
*
*					decs - 要保留的小数位数，若小数的实际位数超过了指定的小数位，则按指定的宽度四舍五入输出
*							 示例：1.12345 ，指定 decs 为4位的话，则输出结果为1.1235
*
*	函数功能:	在指定坐标显示指定的变量，包括小数
*
*	说    明:	1. 可设置要显示的字体，例如使用 LCD_SetTextFont(&CH_Font24) 设置为 2424的中文字体以及2412的ASCII字符字体
*					2.	可设置要显示的颜色，例如使用 LCD_SetColor(0xff0000FF) 设置为蓝色
*					3. 可设置对应的背景色，例如使用 LCD_SetBackColor(0xff000000) 设置为黑色的背景色
*					4. 使用示例 LCD_DisplayDecimals( 10, 10, a, 5, 3) ，在坐标(10,10)显示字变量a,总长度为5位，其中保留3位小数
*
*****************************************************************************************************************************************/

void  LCD_DisplayDecimals( uint16_t x, uint16_t y, double decimals, uint8_t len, uint8_t decs)
{
	char  Number_Buffer[20];				// 用于存储转换后的字符串

	if( LCD.ShowNum_Mode == Fill_Zero)	// 多余位填充0模式
	{
		sprintf( Number_Buffer , "%0*.*lf",len,decs, decimals );	// 将 number 转换成字符串，便于显示
	}
	else		// 多余位填充空格
	{
		sprintf( Number_Buffer , "%*.*lf",len,decs, decimals );	// 将 number 转换成字符串，便于显示
	}

	LCD_DisplayString( x, y,(char *)Number_Buffer) ;	// 将转换得到的字符串显示出来
}


/***************************************************************************************************************************************
*	函 数 名: LCD_DrawImage
*
*	入口参数: x - 起始水平坐标，取值范围0~799
*				 y - 起始垂直坐标，取值范围0~479
*			 	 width  - 图片的水平宽度，最大取值800
*				 height - 图片的垂直宽度，最大取值480
*				*pImage - 图片数据存储区的首地址
*
*	函数功能: 在指定坐标处显示图片
*
*	说    明: 要显示的图片需要事先进行取模，且只能显示一种颜色，使用 LCD_SetColor() 函数设置画笔色
*
*****************************************************************************************************************************************/

void 	LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage)
{
   uint8_t   disChar;	//字模的值
	uint16_t  Xaddress = x; //水平坐标
	uint16_t  i=0,j=0,m=0;

	for(i = 0; i <height; i++)
	{
		for(j = 0; j <(float)width/8; j++)
		{
			disChar = *pImage;

			for(m = 0; m < 8; m++)
			{
				if(disChar & 0x01)
				{
					LCD_DrawPoint(Xaddress,y,LCD.Color);	//当前模值不为0时，使用画笔色绘点
				}
				else
				{
					LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//否则使用背景色绘制点
				}
				disChar >>= 1;
				Xaddress++;  //水平坐标自加

				if( (Xaddress - x)==width ) //如果水平坐标达到了字符宽度，则退出当前循环
				{													//进入下一行的绘制
					Xaddress = x;
					y++;
					break;
				}
			}
			pImage++;
		}
	}
}


/***************************************************************************************************************************************
*	函 数 名: LCD_DrawLine
*
*	入口参数: x1 - 起点 水平坐标，取值范围 0~799
*			 	 y1 - 起点 垂直坐标，取值范围 0~479
*
*				 x2 - 终点 水平坐标，取值范围 0~799
*            y2 - 终点 垂直坐标，取值范围 0~479
*
*	函数功能: 在两点之间画线
*
*	说    明: 该函数移植于ST官方评估板的例程
*
*****************************************************************************************************************************************/

#define ABS(X)  ((X) > 0 ? (X) : -(X))

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if (x2 >= x1)                 /* The x-values are increasing */
	{
	 xinc1 = 1;
	 xinc2 = 1;
	}
	else                          /* The x-values are decreasing */
	{
	 xinc1 = -1;
	 xinc2 = -1;
	}

	if (y2 >= y1)                 /* The y-values are increasing */
	{
	 yinc1 = 1;
	 yinc2 = 1;
	}
	else                          /* The y-values are decreasing */
	{
	 yinc1 = -1;
	 yinc2 = -1;
	}

	if (deltax >= deltay)         /* There is at least one x-value for every y-value */
	{
	 xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
	 yinc2 = 0;                  /* Don't change the y for every iteration */
	 den = deltax;
	 num = deltax / 2;
	 numadd = deltay;
	 numpixels = deltax;         /* There are more x-values than y-values */
	}
	else                          /* There is at least one y-value for every x-value */
	{
	 xinc2 = 0;                  /* Don't change the x for every iteration */
	 yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
	 den = deltay;
	 num = deltay / 2;
	 numadd = deltax;
	 numpixels = deltay;         /* There are more y-values than x-values */
	}
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	 LCD_DrawPoint(x,y,LCD.Color);             /* Draw the current pixel */
	 num += numadd;              /* Increase the numerator by the top of the fraction */
	 if (num >= den)             /* Check if numerator >= denominator */
	 {
		num -= den;               /* Calculate the new numerator value */
		x += xinc1;               /* Change the x as appropriate */
		y += yinc1;               /* Change the y as appropriate */
	 }
	 x += xinc2;                 /* Change the x as appropriate */
	 y += yinc2;                 /* Change the y as appropriate */
	}
}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawRect
*
*	入口参数: x - 起始水平坐标，取值范围0~799
*				 y - 起始垂直坐标，取值范围0~479
*			 	 width  - 图片的水平宽度，最大取值800
*				 height - 图片的垂直宽度，最大取值480
*
*	函数功能: 在指点位置绘制指定长宽的矩形线条
*
*	说    明: 该函数移植于ST官方评估板的例程
*
*****************************************************************************************************************************************/

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	/* draw horizontal lines */
	LCD_DrawLine(x, y, x+width, y);
	LCD_DrawLine(x, y+height, x+width, y+height);

	/* draw vertical lines */
	LCD_DrawLine(x, y, x, y+height);
	LCD_DrawLine(x+width, y, x+width, y+height);
}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawCircle
*
*	入口参数: x - 圆心 水平坐标，取值范围 0~799
*			 	 y - 圆心 垂直坐标，取值范围 0~479
*			 	 r  - 半径
*
*	函数功能: 在坐标 (x,y) 绘制半径为 r 的圆形线条
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r)
{
	int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
	do {

		LCD_DrawPoint(x-Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y-Yadd,LCD.Color);
		LCD_DrawPoint(x-Xadd,y-Yadd,LCD.Color);

		e2 = err;
		if (e2 <= Yadd) {
			err += ++Yadd*2+1;
			if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
		}
		if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);

}

/***************************************************************************************************************************************
*	函 数 名: LCD_DrawEllipse
*
*	入口参数: x - 圆心 水平坐标，取值范围 0~799
*			 	 y - 圆心 垂直坐标，取值范围 0~479
*			 	 r1  - 水平半轴的长度
*				 r2  - 垂直半轴的长度
*
*	函数功能: 在坐标 (x,y) 绘制水平半轴为 r1 垂直半轴为 r2 的椭圆线条
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_DrawEllipse(int x, int y, int r1, int r2)
{
  int Xadd = -r1, Yadd = 0, err = 2-2*r1, e2;
  float K = 0, rad1 = 0, rad2 = 0;

  rad1 = r1;
  rad2 = r2;

  if (r1 > r2)
  {
    do {
      K = (float)(rad1/rad2);

		LCD_DrawPoint(x-Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x+Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x+Xadd,y-(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x-Xadd,y-(uint16_t)(Yadd/K),LCD.Color);

      e2 = err;
      if (e2 <= Yadd) {
        err += ++Yadd*2+1;
        if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
      }
      if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
  }
  else
  {
    Yadd = -r2;
    Xadd = 0;
    do {
      K = (float)(rad2/rad1);

		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y-Yadd,LCD.Color);
		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y-Yadd,LCD.Color);

      e2 = err;
      if (e2 <= Xadd) {
        err += ++Xadd*3+1;
        if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
      }
      if (e2 > Yadd) err += ++Yadd*3+1;
    }
    while (Yadd <= 0);
  }
}
/***************************************************************************************************************************************
*	函 数 名: LCD_FillRect
*
*	入口参数: x - 水平坐标，取值范围 0~799
*			 	 y - 垂直坐标，取值范围 0~479
*			 	 width  - 图片的水平宽度，最大取值800
*				 height - 图片的垂直宽度，最大取值480
*
*	函数功能: 在坐标 (x,y) 填充指定长宽的实心矩形
*
*	说    明: 1. 使用DMA2D实现
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{

	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	停止DMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	寄存器到SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	设置颜色格式
	DMA2D->OCOLR	=	LCD.Color;							//	颜色

	if(LCD.Direction == Direction_H)  //横屏填充
	{
		DMA2D->OOR		=	LCD_Width - width;				//	设置行偏移
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);	// 地址;
		DMA2D->NLR		=	(width<<16)|(height);			//	设定长度和宽度
	}
	else	//竖屏填充
	{
		DMA2D->OOR		=	LCD_Width - height;		//	设置行偏移
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// 地址
		DMA2D->NLR		=	(width)|(height<<16);	//	设定长度和宽度
	}

	DMA2D->CR	  |=	DMA2D_CR_START;					//	启动DMA2D

	while (DMA2D->CR & DMA2D_CR_START) ;			//	等待传输完成

}

/***************************************************************************************************************************************
*	函 数 名: LCD_FillCircle
*
*	入口参数: x - 圆心 水平坐标，取值范围 0~799
*			 	 y - 圆心 垂直坐标，取值范围 0~479
*			 	 r  - 半径
*
*	函数功能: 在坐标 (x,y) 填充半径为 r 的圆形区域
*
*	说    明: 1. 该函数移植于ST官方评估板的例程
*				 2. 要绘制的区域不能超过屏幕的显示区域
*
*****************************************************************************************************************************************/

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r)
{
  int32_t  D;    /* Decision Variable */
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */

  D = 3 - (r << 1);

  CurX = 0;
  CurY = r;

  while (CurX <= CurY)
  {
    if(CurY > 0)
    {
		LCD_DrawLine(x - CurX, y - CurY,x - CurX,y - CurY + 2*CurY);
		LCD_DrawLine(x + CurX, y - CurY,x + CurX,y - CurY + 2*CurY);
    }

    if(CurX > 0)
    {
		LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
		LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX);
    }
    if (D < 0)
    {
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }

  LCD_DrawCircle(x, y, r);
}
/**************************************************************************************************************************************************************************************************************************************************************************FANKE***/
// ：test函数

void LCD_Test_Clear(void)
{
	uint16_t time = 1000;	// 延时时间
	uint8_t	i = 0;			// 计数变量

// 绘制初始界面，包括标题、LOGO以及进度条>>>>>

	LCD_SetBackColor(0xffB9EDF8); 			//	设置背景色，使用自定义颜色
	LCD_Clear(); 									//	清屏，刷背景色

	LCD_SetTextFont(&CH_Font32);				// 设置3232中文字体,ASCII字体对应为3216
	LCD_SetColor(0xff333333);					//	设置画笔色，使用自定义颜色
	LCD_DisplayText(334, 160,"刷屏测试");	// 显示文本

	LCD_SetColor(0xfffd7923);					//	设置画笔色，使用自定义颜色
	LCD_DrawImage(  280, 218, 240, 83, Image_FANKE_240x83) ;		// 显示LOGO图片

	LCD_SetColor(LIGHT_YELLOW);		//	设置画笔色
	for(i=0;i<150;i++)
   {
		LCD_FillRect(100,330,4*i,6);	// 绘制矩形，实现简易进度条的效果
		HAL_Delay(10);
   }



	// 刷屏测试>>>>>

		LCD_SetTextFont(&CH_Font32);			// 设置2424中文字体,ASCII字体对应为2412
		LCD_SetColor(LCD_BLACK);				// 设置画笔颜色

		for(i=0;i<8;i++)
		{
			switch (i)		// 切换背景色
			{
				case 0: LCD_SetBackColor(LIGHT_RED); 		break;
				case 1: LCD_SetBackColor(LIGHT_GREEN); 	break;
				case 2: LCD_SetBackColor(LIGHT_BLUE); 		break;
				case 3: LCD_SetBackColor(LIGHT_YELLOW); 	break;
				case 4: LCD_SetBackColor(LIGHT_CYAN);		break;
				case 5: LCD_SetBackColor(LIGHT_GREY); 		break;
				case 6: LCD_SetBackColor(LIGHT_MAGENTA); 	break;
				case 7: LCD_SetBackColor(LCD_WHITE); 		break;
				default:	break;
			}
			LCD_Clear();		// 清屏
			LCD_DisplayText(112, 84,"STM32H743 LTDC 刷屏测试");
			LCD_DisplayText(112,134,"核心板型号：FK743M5");
			LCD_DisplayText(112,184,"屏幕分辨率：800*480");
			HAL_Delay(time);	// 延时
		}
}






void draw_ecg(uint16_t* data_buffer)
{
    uint16_t x, y, prev_x, prev_y;

    LCD_SetColor(LCD_RED);
    // 绘制基线
    LCD_DrawLine(0, LCD_Height / 2, LCD_Width, LCD_Height / 2); // X轴
    LCD_DrawLine(10, 0, 10, LCD_Height); // Y轴（左边）

    prev_x = 0;
    prev_y = LCD_Height / 2 - data_buffer[0]; // 初始值设置为数据数组的第一个值

    for (x = 10; x < LCD_Width; x+=10)
    {
        y = LCD_Height / 2 - data_buffer[x]; // 将数据值映射到Y轴

        // 限制y坐标在LCD范围内
        if (y < 0)
            y = 0;
        else if (y >= LCD_Height)
            y = LCD_Height - 1;

        // 画线段连接前一点和当前点
        LCD_DrawLine(prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }
}



/*@brief 开始界面绘制进度条*/
void Progress(void)
{
	uint8_t i;
	for(i=0;i<150;i++)
	{
		LCD_FillRect(100,330,4*i,15);
		HAL_Delay(20);
	}


}





/*
 * @breif 在两点之间画虚线,该函数用于在LCD屏幕上绘制两点之间的虚线。虚线的长度由参数dashLength指定。
*      函数采用Bresenham直线算法，通过控制虚线的绘制逻辑，实现了虚线的绘制效果。
*  @param	x1 - 起点 水平坐标，取值范围 0~799
*           y1 - 起点 垂直坐标，取值范围 0~479
*           x2 - 终点 水平坐标，取值范围 0~799
*           y2 - 终点 垂直坐标，取值范围 0~479
*           dashLength - 虚线的长度
*           gapLength - 虚线间隔的长度*/
void LCD_DrawDashedLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t dashLength, uint16_t gapLength)
{
    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
            yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
            curpixel = 0, onLength = 0, offLength = 0, currentLength = 0;

    // 计算直线的长度和方向
    deltax = ABS(x2 - x1);
    deltay = ABS(y2 - y1);

    // 初始化起点坐标
    x = x1;
    y = y1;

    // 根据直线方向确定 x 和 y 的增量方向
    if (x2 >= x1)
    {
        xinc1 = 1;
        xinc2 = 1;
    }
    else
    {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1)
    {
        yinc1 = 1;
        yinc2 = 1;
    }
    else
    {
        yinc1 = -1;
        yinc2 = -1;
    }

    // 确定主方向和次方向以及主次方向上的像素数
    if (deltax >= deltay)
    {
        xinc1 = 0;
        yinc2 = 0;
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax;
    }
    else
    {
        xinc2 = 0;
        yinc1 = 0;
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay;
    }

    // 初始化虚线状态
    onLength = dashLength;
    offLength = gapLength;
    currentLength = 0;

    // 遍历直线上的像素并绘制虚线
    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
        // 检查当前是否在虚线段内
        if (currentLength < onLength)
        {
            LCD_DrawPoint(x, y, LCD.Color);  // 绘制当前像素点
            currentLength++;
        }
        else if (currentLength < onLength + offLength)
        {
            // 在虚线间隔内，不绘制像素点
            currentLength++;
        }
        else
        {
            // 重新开始下一个虚线段
            currentLength = 0;
        }

        num += numadd;              // 更新分子
        if (num >= den)             // 检查是否需要更新 x 或 y 坐标
        {
            num -= den;
            x += xinc1;
            y += yinc1;
        }
        x += xinc2;
        y += yinc2;
    }
}



/*
 * @brief  在背景上显示虚线网格
 *
 * 该函数在屏幕上绘制一个虚线网格。网格的水平线和垂直线的间隔由函数参数决定。
 *
 * @param full_line 每段虚线的长度
 * @param gap 虚线段之间的间隔长度
 *
 * 该函数将调用 LCD_DrawDashedLine 函数来绘制虚线。虚线的起点和终点由函数内部定义。
*/
void Draw_background_ECG(int full_line, int gap)
{
	LCD_SetBackColor(0xffB9EDF8); 				//	设置背景色0xffB9EDF8
	LCD_Clear();
	int i;
	for (i = 0; i <= 479; i += 100)
	{
			LCD_DrawDashedLine(0, i, 800, i, full_line, gap); // x 轴虚线
	}
	int y;
	for (y = 0; y <= 800; y += 100)
	{
			LCD_DrawDashedLine(y, 0, y, 400, full_line, gap); // y 轴虚线

	}

	LCD_DisplayString(50, 420,"HR: ");

	LCD_DisplayString(250,420,"RS:");

	LCD_DisplayString(450,420,"BT: ");


}




/*@brief	根据实时采集的信号值绘制在屏幕上
 *@param	当前信号值的采集值
 * */
// 全局变量，存储当前x坐标
uint16_t current_x = 0;

// 采集信号并绘制ECG曲线的函数
void Draw_ECG(int32_t new_signal)
{
    static int32_t previous_signal = 0; // 静态变量，保存上一次的信号值
    static uint16_t previous_x = 0;     // 静态变量，保存上一次的x坐标
    uint16_t y1, y2;
    if (new_signal >= 120000 && new_signal <= 160000)
    {
			// 计算y坐标，将信号值映射到屏幕坐标
			y1 = (((float)previous_signal - 105000.0)/55000.0)*480;
			y2 = (((float)new_signal - 105000.0) / 55000.0)*480;
			y1 =LCD_Height-y1;
			y2 =LCD_Height-y2;



			if (previous_x < 799)
			    {
			            LCD_DrawLine(previous_x, y1, current_x, y2);
			     }

			// 更新静态变量的值
			previous_signal = new_signal;
			previous_x = current_x;

			// 更新当前x坐标
			current_x++;

			// 如果到达屏幕的右边界，则重置为0（实现滚动效果）
			if (current_x >= LCD_Width)
			{
				current_x = 0;
				LCD_Clear();  // 清屏刷新
				Draw_background_ECG(20, 10);


			}
    }
}


