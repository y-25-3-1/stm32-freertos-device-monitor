#include "oled.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buffer[2];

    buffer[0] = 0x00;   // 0x00 表示后面这个字节是命令
    buffer[1] = cmd;

    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buffer, 2, 100);
}

static void OLED_WriteData(uint8_t data)
{
    uint8_t buffer[2];

    buffer[0] = 0x40;   // 0x40 表示后面这个字节是显示数据
    buffer[1] = data;

    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buffer, 2, 100);
}

static void OLED_SetPos(uint8_t page, uint8_t col)
{
    OLED_WriteCmd(0xB0 + page);
    OLED_WriteCmd(0x00 + (col & 0x0F));
    OLED_WriteCmd(0x10 + ((col >> 4) & 0x0F));
}

void OLED_Clear(void)
{
    uint8_t page;
    uint8_t col;

    for (page = 0; page < 8; page++)
    {
        OLED_SetPos(page, 0);

        for (col = 0; col < 128; col++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_Fill(uint8_t data)
{
    uint8_t page;
    uint8_t col;

    for (page = 0; page < 8; page++)
    {
        OLED_SetPos(page, 0);

        for (col = 0; col < 128; col++)
        {
            OLED_WriteData(data);
        }
    }
}

void OLED_Init(void)
{
    HAL_Delay(100);

    OLED_WriteCmd(0xAE); // 关闭显示

    OLED_WriteCmd(0x20); // 设置内存地址模式
    OLED_WriteCmd(0x10); // 页地址模式

    OLED_WriteCmd(0xB0); // 设置页起始地址
    OLED_WriteCmd(0xC8); // COM 扫描方向
    OLED_WriteCmd(0x00); // 低列地址
    OLED_WriteCmd(0x10); // 高列地址
    OLED_WriteCmd(0x40); // 起始行地址

    OLED_WriteCmd(0x81); // 对比度
    OLED_WriteCmd(0xFF);

    OLED_WriteCmd(0xA1); // 段重映射
    OLED_WriteCmd(0xA6); // 正常显示

    OLED_WriteCmd(0xA8); // 多路复用率
    OLED_WriteCmd(0x3F);

    OLED_WriteCmd(0xA4); // 输出遵循 RAM 内容

    OLED_WriteCmd(0xD3); // 显示偏移
    OLED_WriteCmd(0x00);

    OLED_WriteCmd(0xD5); // 显示时钟分频
    OLED_WriteCmd(0xF0);

    OLED_WriteCmd(0xD9); // 预充电周期
    OLED_WriteCmd(0x22);

    OLED_WriteCmd(0xDA); // COM 引脚配置
    OLED_WriteCmd(0x12);

    OLED_WriteCmd(0xDB); // VCOMH
    OLED_WriteCmd(0x20);

    OLED_WriteCmd(0x8D); // 电荷泵
    OLED_WriteCmd(0x14);

    OLED_WriteCmd(0xAF); // 打开显示

    OLED_Clear();
}

void OLED_Test(void)
{
    OLED_Fill(0x00);
    HAL_Delay(500);

    OLED_Fill(0xFF);
    HAL_Delay(500);

    OLED_Fill(0x00);
    HAL_Delay(500);

    OLED_Fill(0xAA);
    HAL_Delay(500);

    OLED_Fill(0x55);
    HAL_Delay(500);

    OLED_Clear();
}

static void OLED_GetFont6x8(char ch, uint8_t font[6])
{
    uint8_t i;

    for (i = 0; i < 6; i++)
    {
        font[i] = 0x00;
    }

    switch (ch)
    {
        case ' ':
            font[0]=0x00; font[1]=0x00; font[2]=0x00; font[3]=0x00; font[4]=0x00; font[5]=0x00;
            break;

        case ':':
            font[0]=0x00; font[1]=0x00; font[2]=0x36; font[3]=0x36; font[4]=0x00; font[5]=0x00;
            break;

        case '.':
            font[0]=0x00; font[1]=0x00; font[2]=0x60; font[3]=0x60; font[4]=0x00; font[5]=0x00;
            break;

        case '-':
            font[0]=0x08; font[1]=0x08; font[2]=0x08; font[3]=0x08; font[4]=0x08; font[5]=0x00;
            break;

        case '0':
            font[0]=0x3E; font[1]=0x51; font[2]=0x49; font[3]=0x45; font[4]=0x3E; font[5]=0x00;
            break;

        case '1':
            font[0]=0x00; font[1]=0x42; font[2]=0x7F; font[3]=0x40; font[4]=0x00; font[5]=0x00;
            break;

        case '2':
            font[0]=0x42; font[1]=0x61; font[2]=0x51; font[3]=0x49; font[4]=0x46; font[5]=0x00;
            break;

        case '3':
            font[0]=0x21; font[1]=0x41; font[2]=0x45; font[3]=0x4B; font[4]=0x31; font[5]=0x00;
            break;

        case '4':
            font[0]=0x18; font[1]=0x14; font[2]=0x12; font[3]=0x7F; font[4]=0x10; font[5]=0x00;
            break;

        case '5':
            font[0]=0x27; font[1]=0x45; font[2]=0x45; font[3]=0x45; font[4]=0x39; font[5]=0x00;
            break;

        case '6':
            font[0]=0x3C; font[1]=0x4A; font[2]=0x49; font[3]=0x49; font[4]=0x30; font[5]=0x00;
            break;

        case '7':
            font[0]=0x01; font[1]=0x71; font[2]=0x09; font[3]=0x05; font[4]=0x03; font[5]=0x00;
            break;

        case '8':
            font[0]=0x36; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x36; font[5]=0x00;
            break;

        case '9':
            font[0]=0x06; font[1]=0x49; font[2]=0x49; font[3]=0x29; font[4]=0x1E; font[5]=0x00;
            break;

        case 'A':
            font[0]=0x7E; font[1]=0x11; font[2]=0x11; font[3]=0x11; font[4]=0x7E; font[5]=0x00;
            break;
				
				case 'B':
            font[0]=0x7F; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x36; font[5]=0x00;
            break;
				
        case 'C':
            font[0]=0x3E; font[1]=0x41; font[2]=0x41; font[3]=0x41; font[4]=0x22; font[5]=0x00;
            break;

        case 'D':
            font[0]=0x7F; font[1]=0x41; font[2]=0x41; font[3]=0x22; font[4]=0x1C; font[5]=0x00;
            break;

        case 'E':
            font[0]=0x7F; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x41; font[5]=0x00;
            break;

        case 'H':
            font[0]=0x7F; font[1]=0x08; font[2]=0x08; font[3]=0x08; font[4]=0x7F; font[5]=0x00;
            break;
				
				case 'I':
            font[0]=0x00; font[1]=0x41; font[2]=0x7F; font[3]=0x41; font[4]=0x00; font[5]=0x00;
            break;

        case 'K':
            font[0]=0x7F; font[1]=0x08; font[2]=0x14; font[3]=0x22; font[4]=0x41; font[5]=0x00;
            break;

        case 'L':
            font[0]=0x7F; font[1]=0x40; font[2]=0x40; font[3]=0x40; font[4]=0x40; font[5]=0x00;
            break;

        case 'M':
            font[0]=0x7F; font[1]=0x02; font[2]=0x0C; font[3]=0x02; font[4]=0x7F; font[5]=0x00;
            break;

        case 'N':
            font[0]=0x7F; font[1]=0x04; font[2]=0x08; font[3]=0x10; font[4]=0x7F; font[5]=0x00;
            break;

        case 'O':
            font[0]=0x3E; font[1]=0x41; font[2]=0x41; font[3]=0x41; font[4]=0x3E; font[5]=0x00;
            break;

        case 'P':
            font[0]=0x7F; font[1]=0x09; font[2]=0x09; font[3]=0x09; font[4]=0x06; font[5]=0x00;
            break;

        case 'R':
            font[0]=0x7F; font[1]=0x09; font[2]=0x19; font[3]=0x29; font[4]=0x46; font[5]=0x00;
            break;

        case 'S':
            font[0]=0x46; font[1]=0x49; font[2]=0x49; font[3]=0x49; font[4]=0x31; font[5]=0x00;
            break;

        case 'T':
            font[0]=0x01; font[1]=0x01; font[2]=0x7F; font[3]=0x01; font[4]=0x01; font[5]=0x00;
            break;

        case 'U':
            font[0]=0x3F; font[1]=0x40; font[2]=0x40; font[3]=0x40; font[4]=0x3F; font[5]=0x00;
            break;
				
				case 'X':
            font[0]=0x63; font[1]=0x14; font[2]=0x08; font[3]=0x14; font[4]=0x63; font[5]=0x00;
            break;

        case 'Y':
            font[0]=0x07; font[1]=0x08; font[2]=0x70; font[3]=0x08; font[4]=0x07; font[5]=0x00;
            break;

        case 'Z':
            font[0]=0x61; font[1]=0x51; font[2]=0x49; font[3]=0x45; font[4]=0x43; font[5]=0x00;
            break;

        default:
            font[0]=0x00; font[1]=0x00; font[2]=0x00; font[3]=0x00; font[4]=0x00; font[5]=0x00;
            break;
    }
}

void OLED_ShowChar(uint8_t page, uint8_t col, char ch)
{
    uint8_t i;
    uint8_t font[6];

    OLED_GetFont6x8(ch, font);
    OLED_SetPos(page, col);

    for (i = 0; i < 6; i++)
    {
        OLED_WriteData(font[i]);
    }
}

void OLED_ShowString(uint8_t page, uint8_t col, char *str)
{
    while (*str != '\0')
    {
        OLED_ShowChar(page, col, *str);

        col += 6;
        str++;

        if (col > 122)
        {
            col = 0;
            page++;

            if (page >= 8)
            {
                page = 0;
            }
        }
    }
}

void OLED_ShowNum(uint8_t page, uint8_t col, int num)
{
    char buf[12];

    sprintf(buf, "%d", num);

    OLED_ShowString(page, col, buf);
}
