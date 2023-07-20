/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../ADC/adc.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h" 
#include <stdio.h>


/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
#define playing 1
#define gameOver 2
#define ready 3
#define pause 4

volatile int down;
int gameCondition=ready;
int wallLeft; //external
int wallRight;
int wallTop;
int paddle[2];
int ball;
int end[2];
int xScore2=200;
int yScore2=160;
int xScore1=20;
int yScore1=160;
int xPauseLose=95;
int yPauseLose=160;
static int start=0;
extern int point1;
extern int point2;
extern int VelPad;
static int oldX=-1;
static int oldY=1;
void RIT_IRQHandler (void)
{							
	char text[10]="";
	/* button management */
	if(down>1){ 
		if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){	/* KEY1 pressed */
			switch(down){				
				case 2:				/* pay attention here: please see slides 19_ to understand value 2 */
					if(gameCondition==ready || gameCondition==pause){
						if(start==0){
							wallLeft=LCD_CreateRectangularSprite(0,0,5,320,Red);
							wallRight=LCD_CreateRectangularSprite(235,0,5,320,Red);
							paddle[0]=LCD_CreateRectangularSprite(105,278,30,10,Green);
							paddle[1]=LCD_CreateRectangularSprite(10,32,30,10,Green);
							ball=LCD_CreateRectangularSprite(230,155,5,5,Green);
							end[0]=LCD_CreateRectangularSprite(5,318,230,2,Black);
							end[1]=LCD_CreateRectangularSprite(5,0,230,1,Black);
							GUI_Text(xScore1, yScore1, (uint8_t *) "0", White, Black);
							GUI_Text(xScore2, yScore2, (uint8_t *) "0", White, Black);
							init_timer(0, 0x0004C4B4 ); 						/* 25ms * 25MHz = 0.635*10^6 = 0x00098968 *///TIMER1=>VEL PALLINA + PADDLE (da testare)
							// 25ms => 300 px in circa 7.5 sec ma velocità 1%3 (rimbalzo paddle) quindi 2.5 % 7.5 sec
							start++;
						} else {
							GUI_Text(xPauseLose, yPauseLose, (uint8_t *) "        ", White, Black);
							strcpy(text,"");
							sprintf(text,"%d    ",point1);
							GUI_Text(xScore1, yScore1, (uint8_t *)text , White, Black);
							strcpy(text,"");
							sprintf(text,"%d",point2);
							GUI_Text(xScore2, yScore2, (uint8_t *)text , White, Black);
						}
						LPC_velSet(ball,oldX,oldY);
						LPC_velSet(paddle[1],VelPad,0);
						enable_timer(0);
						gameCondition=playing;
					}
					break;
				default:
					break;
			}
			down++;
		}
		else if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){/* KEY0 pressed */
			switch(down){				
				case 2:				/* pay attention here: please see slides 19_ to understand value 2 */
					if(gameCondition==playing || gameCondition==pause || gameCondition==gameOver){
						LPC_velSet(ball,0,0);
						disable_timer(0);
						GUI_Text(xScore2, yScore2, (uint8_t *)"   " , White, Black);
						GUI_Text(xScore1, yScore1, (uint8_t *)"   " , White, Black);
						LCD_DestroySprites();
						oldX=-1;
						oldY=1;
						start=0;
						point1=0;
						point2=0;
						LCD_Clear(Black);
						gameCondition=ready;
					}
					break;
				default:
					break;
			}
			down++;
		}
		else if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){/* KEY2 pressed */
			switch(down){				
				case 2:				/* pay attention here: please see slides 19_ to understand value 2 */
					if(gameCondition==playing){
						oldX=LPC_getVX(ball);
						oldY=LPC_getVY(ball);
						LPC_velSet(ball,0,0);
						disable_timer(0);
						GUI_Text(xScore2, yScore2, (uint8_t *)"   " , White, Black);
						GUI_Text(xScore1, yScore1, (uint8_t *)"   " , White, Black);
						GUI_Text(xPauseLose, yPauseLose, (uint8_t *) "PAUSE", White, Black);
						gameCondition=pause;
					}
					break;
				default:
					break;
			}
			down++;
		}
		else {	/* button released */
			disable_RIT();
			down=0;			
			NVIC_EnableIRQ(EINT0_IRQn);
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			NVIC_EnableIRQ(EINT2_IRQn);
			LPC_PINCON->PINSEL4    |= (1 << 20);
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
			LPC_PINCON->PINSEL4    |= (1 << 24);
		}
	}
	else{
			if(down==1)
				down++;
	}
	
	reset_RIT();
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
