#include "uc1705.h"
#include "font.h"

		   
volatile static uint8_t LCD_Gram[128][8];

void LCD_RefreshGram(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		LCD_W_CMD (0xb0+i);   
		LCD_W_CMD (0x10);    
		LCD_W_CMD (0x00);     
		for(n=0;n<128;n++)LCD_W_Data(LCD_Gram[n][i]); 		
	}   
} 

void LCD_W_CMD(uint8_t cmd)
{
	uint8_t i;
	LL_GPIO_ResetOutputPin(GPIOB,LCD_CS);
	LL_GPIO_ResetOutputPin(GPIOB,LCD_CD);
	for(i=0;i<8;i++)
	{
		LL_GPIO_ResetOutputPin(GPIOB,LCD_SCL);
		if(cmd&0x80)LL_GPIO_SetOutputPin(GPIOA,LCD_SDA);
		else LL_GPIO_ResetOutputPin(GPIOA,LCD_SDA);	
		LL_GPIO_SetOutputPin(GPIOB,LCD_SCL);
		cmd<<=1;	
	}
	LL_GPIO_SetOutputPin(GPIOB,LCD_CS);
}

void LCD_W_Data(uint8_t dat)
{
	uint8_t i;
	LL_GPIO_ResetOutputPin(GPIOB,LCD_CS);
	LL_GPIO_SetOutputPin(GPIOB,LCD_CD);
	for(i=0;i<8;i++)
	{
		LL_GPIO_ResetOutputPin(GPIOB,LCD_SCL);
		if(dat&0x80)LL_GPIO_SetOutputPin(GPIOA,LCD_SDA);
		else LL_GPIO_ResetOutputPin(GPIOA,LCD_SDA);	
		LL_GPIO_SetOutputPin(GPIOB,LCD_SCL);
		dat<<=1;	
	}
	LL_GPIO_SetOutputPin(GPIOB,LCD_CS);
}



void LCD_Clear(void)
{
	uint8_t i,n;  
	for(i=0;i<8;i++)
	{
		for(n=0;n<128;n++)LCD_Gram[n][i]=0X00;  	
	}
	LCD_RefreshGram();
}

void LCD_Init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Pin = LCD_CS|LCD_RST|LCD_CD|LCD_SCL;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LCD_SDA;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	LL_GPIO_SetOutputPin(GPIOB,LCD_SCL);
	LL_GPIO_SetOutputPin(GPIOB,LCD_CS);
	LL_GPIO_ResetOutputPin(GPIOB,LCD_RST);
	HAL_Delay(5);
	LL_GPIO_SetOutputPin(GPIOB,LCD_RST);
	HAL_Delay(10);
	
	LCD_W_CMD(0xE2);
	LCD_W_CMD(0xA0);
	LCD_W_CMD(0xC0);
	LCD_W_CMD(0xA2);
	LCD_W_CMD(0x2C);
	LCD_W_CMD(0x2E);
	LCD_W_CMD(0x2F);
	LCD_W_CMD(0x25);
	LCD_W_CMD(0x81);
	LCD_W_CMD(15);
	LCD_W_CMD(0xAF);
	LCD_W_CMD(0xE0);
	LCD_W_CMD(0x40);
	HAL_Delay(5);
	LCD_Clear();
	LCD_RefreshGram();
}

		   
void LCD_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t pos,bx,temp=0;
	if(x>127||y>63)return;
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)LCD_Gram[x][pos]|=temp;
	else LCD_Gram[x][pos]&=~temp;	    
}
		 
void LCD_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t f_w,uint8_t f_h,uint8_t mode)
{      			    
	uint8_t temp,t,t1;
	uint8_t y0=y;	
	uint8_t csize=(f_h/8+((f_h%8)?1:0))*f_w;
	chr=chr-' ';
	for(t=0;t<csize;t++)
	{   
		if(f_w==6&&f_h==8)temp=asc2_0608[chr][t];		
		else if(f_w==6&&f_h==12)temp=asc2_0612[chr][t];	
		else if(f_w==12&&f_h==24)temp=asc2_1224[chr][t];
		else return;	
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)LCD_DrawPoint(x,y,mode);
			else LCD_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==f_h)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}     
}


uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}				  
	  
void LCD_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t f_w,uint8_t f_h)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(f_w)*t,y,' ',f_w,f_h,1);
				continue;
			}
			else 
				enshow=1; 
		}
	 	LCD_ShowChar(x+(f_w)*t,y,temp+'0',f_w,f_h,1); 
	}
} 

void LCD_ShowLoad(uint8_t x,uint8_t y,uint8_t load)
{
	uint8_t i,j,y0=y,x0=x;

	for(i=0;i<100;i++)
	{
		for(j=0;j<2;j++)
		{
			LCD_DrawPoint(x,y++,0);
		}
		LCD_DrawPoint(x++,y,0);
		y=y0-1;
	}
	x=x0;
	y=y0;	
	for(i=0;i<100;i++)
	{
		LCD_DrawPoint(x++,y,1);
	}
	x=x0;
	y=y0-1;
	for(i=0;i<load;i++)
	{
		for(j=0;j<2;j++)
		{
			LCD_DrawPoint(x,y++,1);
		}
		LCD_DrawPoint(x++,y,1);
		y=y0-1;
	}
}

void LCD_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t f_w,uint8_t f_h)
{	
    while((*p<='~')&&(*p>=' '))
    {       
        if(x>(128-(f_w))){x=0;y+=f_h;}
        if(y>(64-f_h)){y=x=0;LCD_Clear();}
        LCD_ShowChar(x,y,*p,f_w,f_h,1);	 
        x+=f_w;
        p++;
    }  
	
}


void LCD_ShowPicture(uint8_t x,uint8_t y,const uint8_t *p,uint8_t p_w,uint8_t p_h)
{	
	uint8_t temp,i,col,row;
	uint8_t y0=y;
	uint8_t width=p_w;
	if(x+p_w>128)width=128-p_w;
	uint8_t high=p_h;
	if(y+p_h>64)high=64-p_h;
	uint8_t exp_col_bytes=(p_h/8+((p_h%8)?1:0));
	uint8_t act_col_bytes=(high/8+((high%8)?1:0));
	
	for(row=0;row<width;row++)
	{
		for(col=0;col<act_col_bytes;col++)
		{   
			temp = p[col+row*exp_col_bytes];
			for(i=0;i<8;i++)
			{
				if(temp&0x80)LCD_DrawPoint(x,y,1);
				else LCD_DrawPoint(x,y,0);
				temp<<=1;
				y++;
				if((y-y0)==high)
				{
					y=y0;
					x++;
					break;
				}		
			} 
		}
	}		
}


void LCD_Show_Logo(void)
{
	LCD_ShowPicture(2,8,bmp,32,32);
}


