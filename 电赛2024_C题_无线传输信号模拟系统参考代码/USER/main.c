#include "stm32_config.h"
#include "stdio.h"
#include "led.h"
#include "lcd.h"
#include "AD9954.h" 
#include "key.h"
#include "timer.h"
#include "task_manage.h"

char str[30];	//��ʾ����
extern u8 _return;
int main(void)
{
	u16 i=0;

	MY_NVIC_PriorityGroup_Config(NVIC_PriorityGroup_2);	//�����жϷ���
	delay_init(72);	//��ʼ����ʱ����
	//LED_Init();	//��ʼ��LED�ӿ�
	key_init();
	initial_lcd();
	LCD_Clear();
	delay_ms(100);
	LCD_Refresh_Gram();
	
	//��ʱ��
	Timerx_Init(99,71);
	LCD_Clear();
	
	AD9954_Init();
	AD9954_Set_Fre(35000000);//дƵ��35M	Hz
	AD9954_Set_Amp(16383);//0~16383��Ӧ���ֵ0mv~500mv(����)
	//AD9954_Set_Phase(0);//�����λ,��Χ��0~16383(��Ӧ�Ƕȣ�0��~360��)
	while(1)
	{
		KeyRead();
		Set_PointFre(Keycode, 0);
		if(_return){_return=0;LCD_Refresh_Gram();}
		KEY_EXIT();
	}
}

