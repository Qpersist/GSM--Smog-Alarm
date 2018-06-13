#include <STC12.H>
#include<intrins.h>



#define uchar unsigned char;
#define uint unsigned int;

#define RS_CLR RS=0 
#define RS_SET RS=1

#define RW_CLR RW=0 
#define RW_SET RW=1 

#define EN_CLR EN=0
#define EN_SET EN=1

#define DataPort P0 //1602数据口              
sbit RS = P3^5;   
sbit RW = P3^6;
sbit EN = P3^7;

sbit beep= P2^6;//蜂鸣器
sbit key1= P2^0;
sbit key2= P2^2;
sbit led= P2^5;
uchar da1=0,da2=0,da3=0;

double Data,c;
uint warnnum=2;
char a[5]="";   
char temp;  
uint key_num;
uchar ADC_Chanul_Turn=0;
char code str[] = "ATD18895702575;\n";

void DelayUs2x(unsigned char t)
{   
	while(--t);
}

void DelayMs(unsigned char t)
{     
	while(t--)
	{
     
		DelayUs2x(245);
		DelayUs2x(245);
	}
}

bit LCD_Check_Busy(void) 
{ 
	DataPort= 0xFF; 
	RS_CLR; 
	RW_SET; 
	EN_CLR; 
	_nop_(); 
	EN_SET;
	return (bit)(DataPort & 0x80);    
}

void LCD_Write_Com(unsigned char com) 
{  
	while(LCD_Check_Busy());
	RS_CLR; 
	RW_CLR; 
	EN_SET; 
	DataPort= com;         
	_nop_(); 
	EN_CLR;
}

void LCD_Write_Data(unsigned char Data) 
{ 
	while(LCD_Check_Busy()); 
	RS_SET; 
	RW_CLR; 
	EN_SET; 
	DataPort= Data; 
	_nop_();
	EN_CLR;
}


void LCD_Clear(void) 
{ 
	LCD_Write_Com(0x01); 
	DelayMs(5);
}

void LCD_Write_String(unsigned char x,unsigned char y,unsigned char*s)
{     
	if (y == 0) 
	{     
		LCD_Write_Com(0x80 + x);    
	}
	else 
	{      
		LCD_Write_Com(0xC0 + x);     
	}        
	while (*s) 
	{     
		LCD_Write_Data( *s);     
		s ++;     
	}
}

void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data) 
{     
	if (y == 0) 
	{     
		LCD_Write_Com(0x80 + x);     
	}    
	else 
	{     
		LCD_Write_Com(0xC0 + x);     
	}        
	LCD_Write_Data( Data);  
}

void LCD_Init(void) 
{
	LCD_Write_Com(0x38);    
	DelayMs(100);
	LCD_Write_Com(0x0C);
	DelayMs(200);
	LCD_Write_Com(0x06);    
    DelayMs(200);
	LCD_Write_Com(0x01); 
	DelayMs(150);    
//   LCD_Write_Com(0x38); 
//   DelayMs(5); 
//   LCD_Write_Com(0x38); 
//   DelayMs(5); 
//   LCD_Write_Com(0x38);  
//	LCD_Write_Com(0x08);            
}
   
 

void InitADC()
{
	P1ASF=0X07;     
	ADC_RES=0X00;    
	ADC_CONTR=0xF0; 
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ADC_CONTR=0xE8;                                
} 
    
void timer0() interrupt 1      
{
    TH0=(65536-20000)/256;       
    TL0=(65536-20000)%256;     
    InitADC();
}
void adc_isr() interrupt 5     
{  
	da1=ADC_RES;                                
	Data=((double)da1/256)*5;     
	c =Data;
      
	a[0]=((int)c%10+0x30);
	a[1]=0x2e;                    
	a[2]=((int)(c*10)%10+0x30); 
	a[3]=((int)(c*100)%10+0x30);
	a[4]='\0';                  

	LCD_Write_String(8,0,a);
	temp=((int)warnnum%10+0x30);
	LCD_Write_Char(12,1,temp);      //显示警报设定电压值
 		
	ADC_CONTR&=0xEF;               
	ADC_Chanul_Turn++;
	if(ADC_Chanul_Turn==252) 
		ADC_Chanul_Turn=0;    
}

void key_scan()
{
	if(key1==0)
	{
		DelayMs(10);
		if(key1==0)
		{
			key_num++;
			if(key_num==1)
			{  	
				LCD_Write_Com(0x0f);	
				LCD_Write_Com(0x80+0x40+12);
			}
			if(key_num==2)
			{
				key_num=0;
				LCD_Write_Char(12,1,warnnum);
				LCD_Write_Com(0x0c);
			}
			while(!key1);		
		}	
	}

	if(key2==0&&key_num!=0)
	{
		DelayMs(10);
		if(key2==0)
		{
			warnnum++;
			if(warnnum==9)
				warnnum=0;
			LCD_Write_Com(0x80+0x40+12);
			LCD_Write_Data(0x30+warnnum);
			while(!key2);								
		}
	}
}
void time_init()
{
		TMOD = 0x20;			// 定时器1工作于8位自动重载模式, 用于产生波特率
		TH1 = 0xFD;				// 波特率9600
		TL1 = 0xFD;
	
		SCON = 0x50;			// 设定串行口工作方式
		PCON &= 0xef;			// 波特率不倍增
	
    TH0=(65536-20000)/256;           //????0
    TL0=(65536-20000)%256;
    EA=1;                             //?????
    ET0=1;                             //????????
    EADC=1;                          //??ADC?? 
 		TR1 = 1;				
	  TR0=1;
}
void send_str()
// 传送字串
{
	unsigned char i = 0;
	beep=0;          //蜂鸣器报警
	IE=0x00;
	while(str[i] != '\0')
	{
		SBUF = str[i];
		while(!TI);				// 等特数据传送
		TI = 0;					// 清除数据传送标志
		i++;					// 下一个字符
	}	
	time_init();

}
void main()
{
    LCD_Init(); 
    LCD_Clear();                     
	LCD_Write_String(0,2,"Warning num:");
	LCD_Write_String(0,0,"Ammonia:");//氨气浓度
	LCD_Write_String(12,0,"%");
    DelayMs(255);
		
	  TMOD = 0x20;			// 定时器1工作于8位自动重载模式, 用于产生波特率
		TH1 = 0xFD;				// 波特率9600
		TL1 = 0xFD;
	
		SCON = 0x50;			// 设定串行口工作方式
		PCON &= 0xef;			// 波特率不倍增
    TH0=(65536-20000)/256;           
    TL0=(65536-20000)%256;
    EA=1;                            
    ET0=1;                             
    EADC=1;                          
    TR0=1; 
		TR1 = 1;	
    while(1)
	{
		key_scan();
		if(Data>=warnnum&&key_num==0)
		{
//			beep=0;
			send_str();
			led=0;
			DelayMs(200);           //超过警戒值报警	
		}
		else {beep=1;led=1;}
	}
}