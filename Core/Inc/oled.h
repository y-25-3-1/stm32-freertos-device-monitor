#ifndef __OLED_H
#define __OLED_H

#include "main.h"

#define OLED_ADDR 0x78

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Fill(uint8_t data);
void OLED_Test(void);

void OLED_ShowChar(uint8_t page, uint8_t col, char ch);
void OLED_ShowString(uint8_t page, uint8_t col, char *str);
void OLED_ShowNum(uint8_t page, uint8_t col, int num);

#endif
