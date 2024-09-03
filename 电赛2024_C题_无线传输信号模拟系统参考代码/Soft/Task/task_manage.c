#include "task_manage.h"
#include "delay.h"
#include "key.h"
#include "AD9954.h" 
#include <stdio.h>
#include <ctype.h>
#include <cstring>

#define OUT_KEY  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)//读取按键0
#define FLASH_SAVE_ADDR  0x0801F000 //设置FLASH 保存地址(必须为偶数)

u8 Task_Index = 0;//界面切换
u8 Param_Mode = 0;//参数类型切换
u8 fre_buf[StrMax]; //参数转换字符缓存
u8 display[StrMax]; //参数显示缓存
u8 P_Index = 0; //编辑位置
u8 Task_First_AmpAdj = 1;//调幅任务第一次进入标记
u8 Task_First_PhaseAdj = 1;//调相任务第一次进入标记
u8 _return=0;
//扫频参数
u32 SweepMinFre = 1000;
u32 SweepMaxFre = 10000;
u32 SweepStepFre = 100;
u16 SweepTime = 1;//ms
u8 SweepFlag = 0;

//把数据放到另一个缓存，显示，FloatNum小数点位数，CursorEn光标使能
void Copybuf2dis(u8 *source, u8 dis[StrMax], u8 dispoint, u8 FloatNum, u8 CursorEn)
{
    int i, len;

    len = strlen(source);
    i = len - FloatNum;//整数个数
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
    //按键判断
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
    //界面切换
    if(Key_Value == K_2_L) 
    {    
        Task_Index++;
        LCD_Clear();
    }

    if(Task_Index >= Interface) Task_Index = 0;
    switch(Task_Index)
    {
        case 0: 
            Task0_AmpAdj(Key_Value); // 调幅
            break;
        case 1: 
            Task3_PhaseAdj(Key_Value); // 调相
            break;
    }
}

//调幅
void Task0_AmpAdj(uint16_t Key_Value)
{
    static uint16_t Ampli = 16383; // 默认幅度值，范围0-16383，实际范围应根据需求调整
    u8 showstr[StrMax] = {0};

    if (Task_First_AmpAdj)
    {
        Task_First_AmpAdj = 0;
        Key_Value = K_2_S;
        sprintf(fre_buf, "%5d", Ampli); // 第一次进入，初始化幅度显示
        LCD_Show_CEStr(64 - 8 * 3, 0, "幅调节");
        _return = 1;
    }

    if (Key_Value != K_NO)
    {
        // 按键判断
        P_Index = P_Index % 5; // 数据位数，假设用户可以在屏幕上看到5位数字
        Ampli = (uint16_t)strtoul(fre_buf, NULL, 10); // 字符串转换为无符号长整型，然后强制转换为 uint16_t
        if (Ampli > 16383) Ampli = 16383; // 数据限制
        sprintf(fre_buf, "%5d", Ampli); // 更新幅度缓冲区
        sprintf(showstr, "%5d", Ampli); // 字符转换
        fre_buf_change(showstr); // fre_buf中 ' ' -> '0'
        Copybuf2dis(showstr, display, P_Index, 0, 1);
        OLED_ShowString(64 - 4 * 11, 3, display);
        LCD_Show_CEStr(64 - 4 * 11 + 9 * 8, 3, "mV"); // 显示单位为 mV

        // 数据处理写入
        AD9954_Set_Amp(Ampli);

        _return = 1;
    }
}

//调相1
void Task3_PhaseAdj(uint16_t Key_Value)
{
    static uint16_t Phase = 16383; // 初始相位，范围0-16383，对应0°到360°
    u8 showstr[StrMax] = {0};

    if (Task_First_PhaseAdj)
    {
        Task_First_PhaseAdj = 0;
        Key_Value = K_2_S;
        sprintf(fre_buf, "%5d", Phase); // 初始化显示相位值
        LCD_Show_CEStr(64 - 8 * 3, 0, "相调节");
        _return = 1;
    }

    if (Key_Value != K_NO)
    {
        // 按键判断
        P_Index = P_Index % 5; // 数据位数，假设用户可以在屏幕上看到5位数字
        Phase = (uint16_t)strtoul(fre_buf, NULL, 10); // 字符串转换为无符号长整型，然后强制转换为 uint16_t
        if (Phase > 16383) Phase = 16383; // 数据限制
        sprintf(fre_buf, "%5d", Phase); // 更新相位缓冲区
        sprintf(showstr, "%5d", Phase); // 字符转换
        fre_buf_change(showstr); // fre_buf中 ' ' -> '0'
        Copybuf2dis(showstr, display, P_Index, 0, 1);
        OLED_ShowString(64 - 4 * 11, 3, display);
        LCD_Show_CEStr(64 - 4 * 11 + 9 * 8, 3, "cC"); // 显示单位为度数

        // 数据处理写入
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
