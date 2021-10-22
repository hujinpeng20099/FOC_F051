#ifndef __UC1705_H
#define __UC1705_H




#define LCD_CS  2
#define LCD_RST	3
#define LCD_CD	4
#define LCD_SCL	5
#define LCD_SDA	7

void LCD_RefreshGram(void);
void LCD_GPIO_Init(void);
void LCD_WriteCMD(uint8_t cmd);
void LCD_WriteData(uint8_t dat);
void LCD_Clear(void);
void LCD_Init(void);

void LCD_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void LCD_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t f_w,uint8_t f_h,uint8_t mode);
uint32_t mypow(uint8_t m,uint8_t n);
void LCD_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t f_w,uint8_t f_h);
void LCD_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t f_w,uint8_t f_h);
void LCD_ShowPicture(uint8_t x,uint8_t y,const uint8_t *p,uint8_t p_w,uint8_t p_h);
//void LCD_Showdevice(void);


#endif
