#ifndef __UC1705_H
#define __UC1705_H


#include "main.h"

void LCD_W_CMD(uint8_t cmd);
void LCD_W_Data(uint8_t dat);
void LCD_Init(void);
void LCD_RefreshGram(void);
void LCD_Show_Logo(void);
void LCD_ShowLoad(uint8_t x,uint8_t y,uint8_t load);
void LCD_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t f_w,uint8_t f_h);
void LCD_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t f_w,uint8_t f_h,uint8_t mode);
void LCD_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t f_w,uint8_t f_h);
#endif

