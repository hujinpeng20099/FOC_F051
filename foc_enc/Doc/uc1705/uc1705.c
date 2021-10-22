#include <nrfx_gpiote.h>
#include "uc1705.h"
#include "font.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"





//OLED的显存
//存放格式如下
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
volatile static uint8_t LCD_Gram[128][8];

void LCD_RefreshGram(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		LCD_WriteCMD (0xb0+i);   
		LCD_WriteCMD (0x10);    
		LCD_WriteCMD (0x00);     
		for(n=0;n<128;n++)LCD_WriteData(LCD_Gram[n][i]); 		
	}   
} 


void LCD_GPIO_Init(void)
{
	nrf_gpio_cfg_output(LCD_CS);
	nrf_gpio_cfg_output(LCD_RST);
	nrf_gpio_cfg_output(LCD_CD);
	nrf_gpio_cfg_output(LCD_SCL);
	nrf_gpio_cfg_output(LCD_SDA);
}


void LCD_WriteCMD(uint8_t cmd)
{
	uint8_t i;
	nrf_gpio_pin_write(LCD_CS,0);
	nrf_gpio_pin_write(LCD_CD,0);
	for(i=0;i<8;i++)
	{
		nrf_gpio_pin_write(LCD_SCL,0);
		if(cmd&0x80)nrf_gpio_pin_write(LCD_SDA,1);
		else nrf_gpio_pin_write(LCD_SDA,0);
		nrf_gpio_pin_write(LCD_SCL,1);
		cmd<<=1;
	}
	nrf_gpio_pin_write(LCD_CS,1);
}

void LCD_WriteData(uint8_t dat)
{
	uint8_t i;
	nrf_gpio_pin_write(LCD_CS,0);
	nrf_gpio_pin_write(LCD_CD,1);
	for(i=0;i<8;i++)
	{
		nrf_gpio_pin_write(LCD_SCL,0);
		if(dat&0x80)nrf_gpio_pin_write(LCD_SDA,1);
		else nrf_gpio_pin_write(LCD_SDA,0);
		nrf_gpio_pin_write(LCD_SCL,1);
		dat<<=1;
	}
	nrf_gpio_pin_write(LCD_CS,1);
}

void LCD_Clear(void)
{
	uint8_t i,n;  
	for(i=0;i<8;i++)
		for(n=0;n<128;n++)
			LCD_Gram[n][i]=0X00;  
	LCD_RefreshGram();
}


void LCD_Init(void)
{
    LCD_GPIO_Init();
    
    nrf_gpio_pin_write(LCD_SCL,1);
    nrf_gpio_pin_write(LCD_CS,1);
    nrf_gpio_pin_write(LCD_RST,0);
    nrf_delay_ms(5);
    nrf_gpio_pin_write(LCD_RST,1);
    nrf_delay_ms(10);
	
    LCD_WriteCMD(0xe2);         // soft reset
    LCD_WriteCMD(0xa0);        // set seg direction
    LCD_WriteCMD(0xc0);         // set com direction
    LCD_WriteCMD(0xa2|0);       // set lcd bias ratio
    LCD_WriteCMD(0x2c);         // set power control
    LCD_WriteCMD(0x2e);
    LCD_WriteCMD(0x2f);
    LCD_WriteCMD(0x25);	      
    LCD_WriteCMD(0x81);	       
    LCD_WriteCMD(15);           
    LCD_WriteCMD(0xaf);
    LCD_WriteCMD(0xe0);
    LCD_WriteCMD(0x40);
    
    nrf_delay_ms(5);
    LCD_Clear();
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

//显示图片
//x,y:起点坐标  
//p_w:图片宽（单位像素）
//p_h:图片高（单位像素）
//*p:图片起始地址 
void LCD_ShowPicture(uint8_t x,uint8_t y,const uint8_t *p,uint8_t p_w,uint8_t p_h)
{	
	uint8_t temp,i,col,row;
	uint8_t y0=y;
	uint8_t width=p_w;
	if(x+p_w>128)width=128-p_w;//实际显示宽度
	uint8_t high=p_h;
	if(y+p_h>64)high=64-p_h;//实际显示高度
	uint8_t exp_col_bytes=(p_h/8+((p_h%8)?1:0));//显示一列的字节数
	uint8_t act_col_bytes=(high/8+((high%8)?1:0));//实际显示一列的字节数
	
	for(row=0;row<width;row++)//列++
	{
		for(col=0;col<act_col_bytes;col++)//显示一列
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


//void LCD_Showdevice(void)
//{
//    char div_mac[30];
//    ble_gap_addr_t div_addr;
//    uint32_t err_code=sd_ble_gap_addr_get(&div_addr);
//    APP_ERROR_CHECK(err_code);
//    
//    sprintf(div_mac,"MAC:%02X:%02X:%02X:%02X:%02X:%02X",div_addr.addr[5],div_addr.addr[4],div_addr.addr[3],div_addr.addr[2],div_addr.addr[1],div_addr.addr[0]);
//
//    
//
//    LCD_ShowString(0,10,div_mac,6,8);
////    LCD_ShowNum(0,12,div_addr.addr[5],2,6,12);
//}