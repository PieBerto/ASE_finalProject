/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		Has been tested SSD1289¡¢ILI9320¡¢R61505U¡¢SSD1298¡¢ST7781¡¢SPFD5408B¡¢ILI9325¡¢ILI9328¡¢
**						HX8346A¡¢HX8347A
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-3-10
** Version:					1.3
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			Paolo Bernardi
** Modified date:		03/01/2020
** Version:					2.0
** Descriptions:		simple arrangement for screen usage
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "AsciiLib.h"
#include "../timer/timer.h"
#include <stdio.h>
#include <stdlib.h>
#define gameOver 2
/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;
int point1=0;
int point2=0;
int VelPad=4;
extern int gameCondition;
extern int bestScore;
extern int ball;
extern int wallLeft;
extern int wallRight;
extern int wallTop;
extern int paddle[2];
extern int end[2];
extern int xScore2;
extern int yScore2;
extern int xScore1;
extern int yScore1;
extern int xPauseLose;
extern int yPauseLose;
typedef struct Move{
	int pxX;
	int pxY;
	int spin;
}Move;
typedef struct RectangularSprite{
	uint16_t x0; //sx angle
	uint16_t y0; //top angle
	uint16_t xDim; //rectangle side
	uint16_t yDim;
	uint16_t color;
	uint16_t index;
	Move Vel;
}RectangularSprite;

static RectangularSprite *myRS; //RS: rectangular sprite
static int dimRS=0;
static int posRS=0;

/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A		10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */


/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCDÐ´Êý¾Ý
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) 
{
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    				/* Interface A->B */
	LCD_EN(0)	                        	/* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)														/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}
/* Private prototipe ----------------------------------------------------------*/
void drawRS(RectangularSprite RS); 
/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None 
*******************************************************************************/
static void wait_delay(int count)
{
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD¶ÁÊý¾Ý
* Input          : - byte: byte to be read
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) 
{
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           				 /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   						 /* delay some times */
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   						 /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCDÐ´¼Ä´æÆ÷µØÖ·
* Input          : - index: ¼Ä´æÆ÷µØÖ·
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index)
{
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCDÐ´¼Ä´æÆ÷Êý¾Ý
* Input          : - index: ¼Ä´æÆ÷Êý¾Ý
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data)
{				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : ¶ÁÈ¡¿ØÖÆÆ÷Êý¾Ý
* Input          : None
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void)
{ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    
{ 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void)
{
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* ¶ÁÈ¡ÆÁID	*/	
	
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* ÐÐÊ×Ö·0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* ÁÐÊ×Ö·0 */     
	}

    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : ½«ÆÁÄ»Ìî³ä³ÉÖ¸¶¨µÄÑÕÉ«£¬ÈçÇåÆÁ£¬ÔòÌî³ä 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB ¸ÄÎª BBBBBGGGGGGRRRRR ¸ñÊ½
* Input          : - color: BRG ÑÕÉ«Öµ  
* Output         : None
* Return         : RGB ÑÕÉ«Öµ
* Attention		 : ÄÚ²¿º¯Êýµ÷ÓÃ
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : »ñÈ¡Ö¸¶¨×ù±êµÄÑÕÉ«Öµ
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : ÔÚÖ¸¶¨×ù±ê»­µã
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: AµãÐÐ×ù±ê
*                  - y1: AµãÁÐ×ù±ê 
*				   - x2: BµãÐÐ×ù±ê
*				   - y2: BµãÁÐ×ù±ê 
*				   - color: ÏßÑÕÉ«
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color )
{
    short dx,dy;      /* ¶¨ÒåX YÖáÉÏÔö¼ÓµÄ±äÁ¿Öµ */
    short temp;       /* Æðµã ÖÕµã´óÐ¡±È½Ï ½»»»Êý¾ÝÊ±µÄÖÐ¼ä±äÁ¿ */

    if( x0 > x1 )     /* XÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )     /* YÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;       /* XÖá·½ÏòÉÏµÄÔöÁ¿ */
	dy = y1-y0;       /* YÖá·½ÏòÉÏµÄÔöÁ¿ */

    if( dx == 0 )     /* XÖáÉÏÃ»ÓÐÔöÁ¿ »­´¹Ö±Ïß */ 
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ Ãè´¹Ö±Ïß */
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )     /* YÖáÉÏÃ»ÓÐÔöÁ¿ »­Ë®Æ½Ö±Ïß */ 
    {
        do
        {
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ ÃèË®Æ½Ïß */
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }
	/* ²¼À¼É­ººÄ·(Bresenham)Ëã·¨»­Ïß */
    if( dx > dy )                         /* ¿¿½üXÖá */
    {
	    temp = 2 * dy - dx;               /* ¼ÆËãÏÂ¸öµãµÄÎ»ÖÃ */         
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    /* »­Æðµã */ 
	        x0++;                         /* XÖáÉÏ¼Ó1 */
	        if( temp > 0 )                /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */
	        {
	            y0++;                     /* ÎªÓÒÉÏÏàÁÚµã£¬¼´£¨x0+1,y0+1£© */ 
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;           /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */  
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                      /* ¿¿½üYÖá */       
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
} 

/******************************************************************************
* Function Name  : PutChar
* Description    : ½«LcdÆÁÉÏÈÎÒâÎ»ÖÃÏÔÊ¾Ò»¸ö×Ö·û
* Input          : - Xpos: Ë®Æ½×ø±ê 
*                  - Ypos: ´¹Ö±×ø±ê  
*				   - ASCI: ÏÔÊ¾µÄ×Ö·û
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* È¡×ÖÄ£Êý¾Ý */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* ×Ö·ûÑÕÉ« */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* ±³¾°ÑÕÉ« */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : ÔÚÖ¸¶¨×ù±êÏÔÊ¾×Ö·û´®
* Input          : - Xpos: ÐÐ×ù±ê
*                  - Ypos: ÁÐ×ù±ê 
*				   - str: ×Ö·û´®
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}

int LCD_CreateRectangularSprite(uint16_t x0,uint16_t y0,uint16_t xDim,uint16_t yDim,uint16_t color){ //ritorna l'indice dello sprite
	if(posRS==dimRS && dimRS!=0){
		myRS=(RectangularSprite*)realloc(myRS,sizeof(RectangularSprite)*dimRS*2);
		if(myRS == NULL){
				printf("Memoria esaurita\n");
				exit(1);
		}
		dimRS=dimRS*2;
	} else if(dimRS==0){
		myRS=malloc(sizeof(RectangularSprite)*2);
		dimRS=2;
		if(myRS == NULL){
				printf("Memoria esaurita\n");
				exit(1);
		}
	}
	myRS[posRS].x0=x0;
	myRS[posRS].y0=y0;
	myRS[posRS].xDim=xDim;
	myRS[posRS].yDim=yDim;
	myRS[posRS].color=color;
	myRS[posRS].index=posRS;
	myRS[posRS].Vel.spin=0;
	myRS[posRS].Vel.pxX=0;
	myRS[posRS].Vel.pxY=0;
	drawRS(myRS[posRS]);
	posRS++;
	return (posRS-1);
}

void LCD_DestroySprites(void){
	int i;
	for(i=myRS[ball].x0;i<myRS[ball].x0+myRS[ball].xDim;i++){
		LCD_DrawLine(i,myRS[ball].y0,i,myRS[ball].y0+myRS[ball].yDim,Black);
	}
	if(myRS!=NULL){
		free(myRS);
	}
	posRS=0;
	dimRS=0;
}
void drawRS(RectangularSprite RS){
	int i;
	for(i=0;i<RS.yDim;i++){
		LCD_DrawLine(RS.x0,RS.y0+i,RS.x0+RS.xDim-1,RS.y0+i,RS.color);
	}
	return;
}
static int dontSpin=0;
void LCD_MoveRS(int indexRS,uint16_t xf,uint16_t yf){
	static int spinning=0;
	RectangularSprite RS=myRS[indexRS];
	int dx;
	int dy;
	uint16_t sideup=RS.y0;
	uint16_t sidedown=sideup+RS.yDim-1;
	uint16_t sideleft=RS.x0;
	uint16_t sideright=sideleft+RS.xDim-1;
	uint16_t sidex;
	uint16_t sidey;
	int i;
	//effetto spin
	if(myRS[indexRS].Vel.spin!=0 && spinning>1 && dontSpin!=1){
		spinning=0;
		xf=xf+myRS[indexRS].Vel.spin;
	} else{
		spinning++;
	}
	dx=xf-RS.x0;
	dy=yf-RS.y0;
	//delete
	if(dx>0){
		sidex=sideleft;
	}
	else{
		sidex=sideright;
	}
	if(dy>0){
		sidey=sideup;
	}
	else{
		sidey=sidedown;
	}
	for(i=0;i<abs(dx) && i<RS.xDim;i++){
		if(dx>0){
			LCD_DrawLine(sidex+i,sideup,sidex+i,sidedown,Black);
		}
		else{
			LCD_DrawLine(sidex-i,sideup,sidex-i,sidedown,Black);
		}
	}
	for(i=0;i<abs(dy) && i<RS.yDim;i++){
		if(dy>0){
			LCD_DrawLine(sideleft,sidey+i,sideright,sidey+i,Black);
		}
		else{
			LCD_DrawLine(sideleft,sidey-i,sideright,sidey-i,Black);
		}
	}
	//draw
	sideup=yf;
	sidedown=sideup+RS.yDim-1;
	sideleft=xf;
	sideright=sideleft+RS.xDim-1;
	if(!(abs(dx)>RS.xDim || abs(dy)>RS.yDim)){ //if i've not deleted all the sprite
		if(dx>0){
			sidex=sideright;
		}
		else{
			sidex=sideleft;
		}
		if(dy>0){
			sidey=sidedown;
		}
		else{
			sidey=sideup;
		}
		for(i=0;i<abs(dx);i++){
			if(dx>0){
				LCD_DrawLine(sidex-i,sideup,sidex-i,sidedown,myRS[indexRS].color);
			}
			else{
				LCD_DrawLine(sidex+i,sideup,sidex+i,sidedown,myRS[indexRS].color);
			}
		}
		for(i=0;i<abs(dy);i++){
			if(dy>0){
				LCD_DrawLine(sideleft,sidey-i,sideright,sidey-i,myRS[indexRS].color);
			}
			else{
				LCD_DrawLine(sideleft,sidey+i,sideright,sidey+i,myRS[indexRS].color);
			}
		}	
	}else{
		RS.x0=xf;
		RS.y0=yf;
		drawRS(RS);
	}
	//update struct
	myRS[indexRS].x0=xf;
	myRS[indexRS].y0=yf;
	return;
}

void LPC_velSet(int indexRS,int vx,int vy){
	myRS[indexRS].Vel.pxX=vx;
	myRS[indexRS].Vel.pxY=vy;
	return;
}
int LPC_getPxX(int indexRS){
	return myRS[indexRS].Vel.pxX;
}
int LPC_getPxY(int indexRS){
	return myRS[indexRS].Vel.pxY;
}
uint16_t LPC_getX0(int indexRS){
	return myRS[indexRS].x0;
}
uint16_t LPC_getY0(int indexRS){
	return myRS[indexRS].y0;
}

int* LPC_RSpritesCheckInteraction(int indexRS,int newPosX,int newPosY){
	static int rewrite1=0;
	static int rewrite2=0;
	static int rewrite1Last;
	static int rewrite2Last;
	int z,ri=0;//dim result accessiva
	char text[7];
	static int* result;
	result=malloc(sizeof(int)*4);
	if(myRS[paddle[1]].x0+myRS[paddle[1]].xDim>=234-VelPad)
		myRS[paddle[1]].Vel.pxX=-VelPad;
	if(myRS[paddle[1]].x0<=6+VelPad)
		myRS[paddle[1]].Vel.pxX=VelPad;
	LCD_MoveRS(paddle[1],myRS[paddle[1]].x0+myRS[paddle[1]].Vel.pxX,myRS[paddle[1]].y0);
	for(z=0;z<4;z++)result[z]=-1;
	if(newPosX>239)newPosX=239;
	if(newPosY>319)newPosY=319;
	if(newPosX<1)newPosX=0;
	if(newPosY<1)newPosY=0;
	for(z=0;z<posRS;z++){
		if(z==indexRS)continue;
		if((myRS[z].x0<=newPosX && newPosX<=myRS[z].x0+myRS[z].xDim && myRS[z].y0<=newPosY && newPosY<=myRS[z].y0+myRS[z].yDim)||
			(myRS[z].x0<=newPosX+myRS[indexRS].xDim && newPosX+myRS[indexRS].xDim<=myRS[z].x0+myRS[z].xDim && myRS[z].y0<=newPosY+myRS[indexRS].yDim && newPosY+myRS[indexRS].yDim<=myRS[z].y0+myRS[z].yDim)||
			(myRS[z].x0<=newPosX && newPosX<=myRS[z].x0+myRS[z].xDim && myRS[z].y0<=newPosY+myRS[indexRS].yDim && newPosY+myRS[indexRS].yDim<=myRS[z].y0+myRS[z].yDim)||
			(myRS[z].x0<=newPosX+myRS[indexRS].xDim && newPosX+myRS[indexRS].xDim<=myRS[z].x0+myRS[z].xDim && myRS[z].y0<=newPosY && newPosY<=myRS[z].y0+myRS[z].yDim)){
			result[ri++]=z;
		}
	}
	if(indexRS==ball && xScore1-myRS[ball].xDim-1<myRS[ball].x0 && myRS[ball].x0<xScore1+30 && yScore1-myRS[ball].yDim-1<myRS[ball].y0 && myRS[ball].y0<yScore1+20){
		rewrite1++;
	}
	if(indexRS==ball && xScore2-myRS[ball].xDim-1<myRS[ball].x0 && myRS[ball].x0<xScore2+30 && yScore2-myRS[ball].yDim-1<myRS[ball].y0 && myRS[ball].y0<yScore2+20){
		rewrite2++;
	}
	if(rewrite1==rewrite1Last && rewrite1>0){
		sprintf(text,"%d",point1);
		GUI_Text(xScore1, yScore1, (uint8_t *)text, White, Black);
		rewrite1=0;
	}	
	rewrite1Last=rewrite1;
	if(rewrite2==rewrite2Last && rewrite2>0){
		sprintf(text,"%d",point2);
		GUI_Text(xScore2, yScore2, (uint8_t *)text, White, Black);
		rewrite2=0;
	}	
	rewrite2Last=rewrite2;
	return result;
}
void LPC_freeInterac(int *result){
	free(result);
	return;
}
void LPC_interaction(int indexRS1,int indexRS2){
	char text[6];
	int pad=-1;
	int en=-1;
	int willRedraw=0;
	if((indexRS1==ball || indexRS2==ball) && (indexRS1==wallLeft || indexRS2==wallLeft)){
		if(myRS[ball].x0<myRS[wallLeft].x0+myRS[wallLeft].xDim){
			willRedraw=1;
		}
		if(myRS[ball].Vel.spin==-1)dontSpin=1;
		LCD_MoveRS(ball,myRS[wallLeft].x0+myRS[wallLeft].xDim,myRS[ball].y0+myRS[ball].Vel.pxY);
		if(myRS[ball].Vel.spin==-1)dontSpin=0;
		if(willRedraw==1){
			willRedraw=0;
			drawRS(myRS[wallLeft]);
		}
		myRS[ball].Vel.pxX=-myRS[ball].Vel.pxX;
		//disable_timer(1);
		//reset_timer(1);
		//init_timer(1,0x426);//TIMER 1 RIPRODUCE LE NOTE (DA PROVARE DAL VIVO)
		//enable_timer(1);
		//enable_timer(2);
	}
	if((indexRS1==ball || indexRS2==ball) && (indexRS1==wallRight || indexRS2==wallRight)){
		if(myRS[ball].x0+myRS[ball].xDim>=myRS[wallRight].x0){
			willRedraw=1;
		}
		if(myRS[ball].Vel.spin==1)dontSpin=1;
		LCD_MoveRS(ball,myRS[wallRight].x0-myRS[ball].xDim,myRS[ball].y0+myRS[ball].Vel.pxY);
		if(myRS[ball].Vel.spin==1)dontSpin=0;
		if(willRedraw==1){
			willRedraw=0;
			drawRS(myRS[wallRight]);
		}
		myRS[ball].Vel.pxX=-myRS[ball].Vel.pxX;
		//disable_timer(1);
		//reset_timer(1);
		//init_timer(1,0x426);//TIMER 1 RIPRODUCE LE NOTE (DA PROVARE DAL VIVO)
		//enable_timer(1);
		//enable_timer(2);
	}
/*	if((indexRS1==ball || indexRS2==ball) && (indexRS1==wallTop || indexRS2==wallTop)){
		if(myRS[ball].y0<myRS[wallTop].y0+myRS[wallTop].yDim){
			willRedraw=1;
		}
		LCD_MoveRS(ball,myRS[ball].x0+myRS[ball].Vel.pxX,myRS[wallTop].y0+myRS[wallTop].yDim);
		if(willRedraw==1){
			willRedraw=0;
			drawRS(myRS[wallTop]);
		}
		myRS[ball].Vel.pxY=-myRS[ball].Vel.pxY;
		//disable_timer(1);
		//reset_timer(1);
		//init_timer(1,0x426);//TIMER 1 RIPRODUCE LE NOTE (DA PROVARE DAL VIVO)
		//enable_timer(1);
		//enable_timer(2);
	}*/
	if(indexRS1==paddle[0] || indexRS2==paddle[0]){
		pad=paddle[0];
	} else if (indexRS1==paddle[1] || indexRS2==paddle[1]){
		pad=paddle[1];
	}
	if((indexRS1==ball || indexRS2==ball) && (indexRS1==pad || indexRS2==pad)){
		if(pad==paddle[0]){
			if(myRS[ball].y0+myRS[ball].yDim>myRS[pad].y0){
				willRedraw=1;
			}
			LCD_MoveRS(ball,myRS[ball].x0+myRS[ball].Vel.pxX,myRS[pad].y0-myRS[ball].yDim);
			if(willRedraw==1){
				willRedraw=0;
				drawRS(myRS[pad]);
			}
		} else if(pad==paddle[1]){
			LCD_MoveRS(ball,myRS[ball].x0+myRS[ball].Vel.pxX,myRS[pad].y0+myRS[pad].yDim);
			drawRS(myRS[pad]);
		}
		//spin
		if(myRS[ball].Vel.pxX<0){
			myRS[ball].Vel.spin=-1; //angolo >90 gradi rispetto al angolo sx del paddle
		} else if(myRS[ball].Vel.pxX>0){
			myRS[ball].Vel.spin=1; //angolo <90 gradi rispetto al angolo sx del paddle
		} else{
			myRS[ball].Vel.spin=0; //angolo =90 gradi rispetto al angolo sx del paddle
		}
		//velocità e direzione di uscita
		if(myRS[pad].x0<=myRS[ball].x0+myRS[ball].xDim && myRS[ball].x0+myRS[ball].xDim<myRS[pad].x0+4){
			myRS[ball].Vel.pxY=-1*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));																														// ALPHA = ARCTG (-1/-3);
			myRS[ball].Vel.pxX=-3;
		}else if(myRS[pad].x0-1<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+3){
			myRS[ball].Vel.pxY=-1*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=-2;
		}else if(myRS[pad].x0+3<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+6){
			myRS[ball].Vel.pxY=-2*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=-2;
		}else if(myRS[pad].x0+6<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+9){
			myRS[ball].Vel.pxY=-2*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=-1;
		}else if(myRS[pad].x0+9<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+12){
			myRS[ball].Vel.pxY=-3*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=-1;
		}else if(myRS[pad].x0+12<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+15){
			myRS[ball].Vel.pxY=-3*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=0;
		}else if(myRS[pad].x0+15<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+18){
			myRS[ball].Vel.pxY=-3*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=1;
		}else if(myRS[pad].x0+18<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+21){
			myRS[ball].Vel.pxY=-2*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=1;
		}else if(myRS[pad].x0+21<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+25){
			myRS[ball].Vel.pxY=-2*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=2;
		}else if(myRS[pad].x0+25<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+29){
			myRS[ball].Vel.pxY=-1*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=2;
		}else if(myRS[pad].x0+29<=myRS[ball].x0 && myRS[ball].x0<myRS[pad].x0+33){
			myRS[ball].Vel.pxY=-1*(myRS[ball].Vel.pxY/abs(myRS[ball].Vel.pxY));
			myRS[ball].Vel.pxX=3;
		} else {
			myRS[ball].Vel.pxY=-myRS[ball].Vel.pxY;
		}
		/*if(pad==paddle[0]){
			if(points1>=100)points1+=5;
			points1+=5;
		} else if(pad==paddle[1]){
			if(points2>=100)points2+=5;
			points2+=5;
		}
		if(points>bestScore){
			bestScore=points;
			strcpy(text,"");
			sprintf(text,"%d",bestScore);
			GUI_Text(xBest, yBest, (uint8_t *)text , White, Black);
		}
		sprintf(text,"%d",points);
		GUI_Text(xScore, yScore, (uint8_t *)text, White, Black);
		disable_timer(1);
		reset_timer(1);
		init_timer(1,0x848); //TIMER 1 RIPRODUCE LE NOTE (DA PROVARE DAL VIVO)
		enable_timer(1);
		enable_timer(2);*/
	}
	if(indexRS1==end[1] || indexRS2==end[1]){
		en=end[1];
		point1+=1;
	}else if(indexRS1==end[0] || indexRS2==end[0]){
		en=end[0];
		point2+=1;
	}
	if((indexRS1==ball || indexRS2==ball) && (indexRS1==en || indexRS2==en)){
		myRS[ball].Vel.pxX=0;
		myRS[ball].Vel.pxY=0;
		GUI_Text(xScore2, yScore2, (uint8_t *)"   " , White, Black);
		GUI_Text(xScore1, yScore1, (uint8_t *)"   " , White, Black);
		if(point2>=5){
			strcpy(text,"YOU LOSE");
			GUI_Text(xPauseLose, yPauseLose, (uint8_t *)text , White, Black);
			LCD_DestroySprites();
			gameCondition=gameOver;
		}
		else if(point1>=5){
			strcpy(text,"YOU WIN");
			GUI_Text(xPauseLose, yPauseLose, (uint8_t *)text , White, Black);
			LCD_DestroySprites();
			gameCondition=gameOver;
		} else{
			LCD_MoveRS(ball,230,155);
			LPC_velSet(ball,-1,1);
			myRS[ball].Vel.spin=0;
			strcpy(text,"");
			sprintf(text,"%d",point1);
			GUI_Text(xScore1, yScore1, (uint8_t *)text , White, Black);
			strcpy(text,"");
			sprintf(text,"%d",point2);
			GUI_Text(xScore2, yScore2, (uint8_t *)text , White, Black);
			enable_timer(1);
		}
		disable_timer(0);
	}
	return;
}
uint16_t LPC_getXDim(int indexRS){
	return myRS[indexRS].xDim;
}
int LPC_getVX(int indexRS){
	return myRS[indexRS].Vel.pxX;
}
int LPC_getVY(int indexRS){
	return myRS[indexRS].Vel.pxY;
}
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
