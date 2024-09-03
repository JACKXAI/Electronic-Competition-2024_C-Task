#include "task_manage.h"
#include "delay.h"
#include "key.h"
#include "AD9954.h" 
#include <stdio.h>
#include <ctype.h>
#include <cstring>

#define OUT_KEY  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)//��ȡ����0
#define FLASH_SAVE_ADDR  0x0801F000 //����FLASH �����ַ(����Ϊż��)

u8 Task_Index = 0;//�����л�
u8 Param_Mode = 0;//���������л�
u8 fre_buf[StrMax]; //����ת���ַ�����
u8 display[StrMax]; //������ʾ����
u8 P_Index = 0; //�༭λ��
u8 Task_First_AmpAdj = 1;//���������һ�ν�����
u8 Task_First_PhaseAdj = 1;//���������һ�ν�����
u8 _return=0;
//ɨƵ����
u32 SweepMinFre = 1000;
u32 SweepMaxFre = 10000;
u32 SweepStepFre = 100;
u16 SweepTime = 1;//ms
u8 SweepFlag = 0;

//�����ݷŵ���һ�����棬��ʾ��FloatNumС����λ����CursorEn���ʹ��
void Copybuf2dis(u8 *source, u8 dis[StrMax], u8 dispoint, u8 FloatNum, u8 CursorEn)
{
    int i, len;

    len = strlen(source);
    i = len - FloatNum;//��������
    if(FloatNum > 0) dis[i] = '.';
    for (i = 0; i < len; i++)
    {
        if(i < (len - FloatNum)) dis[i] = source[i];
        else 
        { dis[i + 1] = source[i]; }
    }

    if(CursorEn)
    {
        if(dispoint < (len - FloatNum)) dis[dispoint] += 128;
        else dis[dispoint + 1] += 128;    
    }
}

void Set_PointFre(u32 Key_Value, u8* Task_ID)
{
    //�����ж�
    switch(Key_Value)
    {
        case K_4_S: fre_buf[P_Index]++; break;
        case K_4_L: fre_buf[P_Index]++; break;
        case K_5_L: P_Index++; break;
        case K_5_S: P_Index++; break;
        case K_1_L: P_Index--; break;
        case K_1_S: P_Index--; break;
        case K_3_S: fre_buf[P_Index]--; break;
        case K_3_L: fre_buf[P_Index]--; break;
        case K_2_S: Param_Mode++; break;
    }
    if(fre_buf[P_Index] == '/') fre_buf[P_Index] = '9'; //< '0'
    if(fre_buf[P_Index] == ':') fre_buf[P_Index] = '0'; // > '9'
    //�����л�
    if(Key_Value == K_2_L) 
    {    
        Task_Index++;
        LCD_Clear();
    }

    if(Task_Index >= Interface) Task_Index = 0;
    switch(Task_Index)
    {
        case 0: 
            Task0_AmpAdj(Key_Value); // ����
            break;
        case 1: 
            Task3_PhaseAdj(Key_Value); // ����
            break;
    }
}

//����
void Task0_AmpAdj(uint16_t Key_Value)
{
    static uint16_t Ampli = 16383; // Ĭ�Ϸ���ֵ����Χ0-16383��ʵ�ʷ�ΧӦ�����������
    u8 showstr[StrMax] = {0};

    if (Task_First_AmpAdj)
    {
        Task_First_AmpAdj = 0;
        Key_Value = K_2_S;
        sprintf(fre_buf, "%5d", Ampli); // ��һ�ν��룬��ʼ��������ʾ
        LCD_Show_CEStr(64 - 8 * 3, 0, "������");
        _return = 1;
    }

    if (Key_Value != K_NO)
    {
        // �����ж�
        P_Index = P_Index % 5; // ����λ���������û���������Ļ�Ͽ���5λ����
        Ampli = (uint16_t)strtoul(fre_buf, NULL, 10); // �ַ���ת��Ϊ�޷��ų����ͣ�Ȼ��ǿ��ת��Ϊ uint16_t
        if (Ampli > 16383) Ampli = 16383; // ��������
        sprintf(fre_buf, "%5d", Ampli); // ���·��Ȼ�����
        sprintf(showstr, "%5d", Ampli); // �ַ�ת��
        fre_buf_change(showstr); // fre_buf�� ' ' -> '0'
        Copybuf2dis(showstr, display, P_Index, 0, 1);
        OLED_ShowString(64 - 4 * 11, 3, display);
        LCD_Show_CEStr(64 - 4 * 11 + 9 * 8, 3, "mV"); // ��ʾ��λΪ mV

        // ���ݴ���д��
        AD9954_Set_Amp(Ampli);

        _return = 1;
    }
}

//����1
void Task3_PhaseAdj(uint16_t Key_Value)
{
    static uint16_t Phase = 16383; // ��ʼ��λ����Χ0-16383����Ӧ0�㵽360��
    u8 showstr[StrMax] = {0};

    if (Task_First_PhaseAdj)
    {
        Task_First_PhaseAdj = 0;
        Key_Value = K_2_S;
        sprintf(fre_buf, "%5d", Phase); // ��ʼ����ʾ��λֵ
        LCD_Show_CEStr(64 - 8 * 3, 0, "�����");
        _return = 1;
    }

    if (Key_Value != K_NO)
    {
        // �����ж�
        P_Index = P_Index % 5; // ����λ���������û���������Ļ�Ͽ���5λ����
        Phase = (uint16_t)strtoul(fre_buf, NULL, 10); // �ַ���ת��Ϊ�޷��ų����ͣ�Ȼ��ǿ��ת��Ϊ uint16_t
        if (Phase > 16383) Phase = 16383; // ��������
        sprintf(fre_buf, "%5d", Phase); // ������λ������
        sprintf(showstr, "%5d", Phase); // �ַ�ת��
        fre_buf_change(showstr); // fre_buf�� ' ' -> '0'
        Copybuf2dis(showstr, display, P_Index, 0, 1);
        OLED_ShowString(64 - 4 * 11, 3, display);
        LCD_Show_CEStr(64 - 4 * 11 + 9 * 8, 3, "cC"); // ��ʾ��λΪ����

        // ���ݴ���д��
        AD9954_Set_Phase(Phase);

        _return = 1;
    }
}

void fre_buf_change(u8 *strbuf)
{
    int i;
    for (i = 0 ; i < strlen(strbuf); i++)
        if(strbuf[i] == 0x20) strbuf[i] = '0';
    for (i = 0 ; i < strlen(fre_buf); i++)
        if(fre_buf[i] == 0x20) fre_buf[i] = '0';
}
