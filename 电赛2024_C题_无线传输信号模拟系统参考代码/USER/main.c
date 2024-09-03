#include "stm32_config.h"
#include "stdio.h"
#include "led.h"
#include "lcd.h"
#include "AD9954.h" 
#include "key.h"
#include "timer.h"
#include "task_manage.h"

char str[30];	//显示缓存
extern u8 _return;
int main(void)
{
	u16 i=0;

	MY_NVIC_PriorityGroup_Config(NVIC_PriorityGroup_2);	//设置中断分组
	delay_init(72);	//初始化延时函数
	//LED_Init();	//初始化LED接口
	key_init();
	initial_lcd();
	LCD_Clear();
	delay_ms(100);
	LCD_Refresh_Gram();
	
	//定时器
	Timerx_Init(99,71);
	LCD_Clear();
	
	AD9954_Init();
	AD9954_Set_Fre(35000000);//写频率35M	Hz
	AD9954_Set_Amp(16383);//0~16383对应峰峰值0mv~500mv(左右)
	//AD9954_Set_Phase(0);//输出相位,范围：0~16383(对应角度：0°~360°)
	while(1)
	{
		KeyRead();
		Set_PointFre(Keycode, 0);
		if(_return){_return=0;LCD_Refresh_Gram();}
		KEY_EXIT();
	}
}

