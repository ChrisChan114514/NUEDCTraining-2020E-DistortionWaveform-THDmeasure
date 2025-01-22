#include "main.h"
#include "lcd.h"
#include "stdio.h"

#define T1ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_SET);lcd_show_string(0,0,80,16,16,"T1:ON",YELLOW);
#define T1OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_RESET);lcd_show_string(0,0,80,16,16,"T1:OFF",YELLOW);

#define T2ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_SET);lcd_show_string(0,15,80,16,16,"T2:ON",YELLOW);
#define T2OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_RESET);lcd_show_string(0,15,80,16,16,"T2:OFF",YELLOW);

#define T3ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_2,GPIO_PIN_SET);lcd_show_string(0,30,80,16,16,"T3:ON",YELLOW);
#define T3OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_2,GPIO_PIN_RESET);lcd_show_string(0,30,80,16,16,"T2:OFF",YELLOW);

#define T4ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_3,GPIO_PIN_SET);lcd_show_string(0,45,80,16,16,"T4:ON",YELLOW);
#define T4OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_3,GPIO_PIN_RESET);lcd_show_string(0,45,80,16,16,"T4:OFF",YELLOW);

#define T5ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);lcd_show_string(0,60,80,16,16,"T5:ON",YELLOW);
#define T5OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_RESET);lcd_show_string(0,60,80,16,16,"T5:OFF",YELLOW);

#define T6ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_SET);lcd_show_string(0,75,80,16,16,"T6:ON",YELLOW);
#define T6OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_RESET);lcd_show_string(0,75,80,16,16,"T6:OFF",YELLOW);

#define T7ON HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);lcd_show_string(0,90,80,16,16,"T7:ON",YELLOW);
#define T7OFF HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);lcd_show_string(0,90,80,16,16,"T7:OFF",YELLOW);

#define BAOHE_NOTE lcd_show_string(0,150,80,32,16,"BAOHE    ",YELLOW);
#define NORMAL_NOTE lcd_show_string(0,150,80,32,16,"NORMAL  ",YELLOW);
#define JIEZHI_NOTE lcd_show_string(0,150,80,32,16,"JIEZHI  ",YELLOW);
#define SHUANGX_NOTE lcd_show_string(0,150,80,32,16,"SHUANGX",YELLOW);
#define JIAOYUE_NOTE lcd_show_string(0,150,80,32,16,"JIAOYUE",YELLOW);

void TFTGPIO_main(int mode)
{
	if (mode==0)  //饱和底端失真
	{
		T1ON   //22k
		T2OFF	//50K
		T3OFF	//200K
		T4ON   //20K
		T5OFF   //1K
		T6OFF
		T7OFF
		BAOHE_NOTE
	}
	else if (mode==1)  //不失真
	{
		T1OFF   //80k
		T2ON
		T3OFF
		T4ON  //0
		T5ON   //1K
		T6OFF
		T7OFF
		NORMAL_NOTE
	}
	else if (mode==2)
	{
		//顶部截止失真 初始化 
		T1OFF   //22K
		T2OFF  //30K
		T3ON  //200K
		T4OFF //0
		T5OFF 
		T6OFF
		T7OFF
		JIEZHI_NOTE

	}
	else if (mode==3)
	{
		//双向失真( 初始化 

		T1OFF   //80k
		T2ON
		T3OFF
		T4ON //20K
		T5OFF   //1K
		T6OFF
		T7OFF
		SHUANGX_NOTE
	}
	else if (mode==4)
	{
		//交越失真 初始化
		T1OFF   //80k
		T2ON
		T3OFF
		T4ON  //0
		T5ON   //1K
		T6OFF
		T7OFF
		JIAOYUE_NOTE
	}
	else if (mode==5)
	{
		//继电器初始化 测试 111_11_11
		T1ON
		T2ON
		T3ON
		T4ON
		T5ON
		T6ON
		T7ON
	}
	

	
}	
