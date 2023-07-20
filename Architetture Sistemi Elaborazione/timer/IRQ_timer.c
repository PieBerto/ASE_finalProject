/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "lpc17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../ADC/adc.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
extern int ball;
void TIMER0_IRQHandler (void)
{
	int x,y,i;
	int *interac;
	uint16_t x1,y1;
	x=LPC_getPxX(ball);
	y=LPC_getPxY(ball);
	x1=LPC_getX0(ball);
	y1=LPC_getY0(ball);
	ADC_start_conversion();
	interac=LPC_RSpritesCheckInteraction(ball,x1+x,y1+y);
	for(i=0;interac[i]!=-1 && i<10;i++);
	if(i==0){
		LCD_MoveRS(ball,x1+x,y1+y);
	} else {
		for(i=0;interac[i]!=-1 && i<10;i++){
			LPC_interaction(ball,interac[i]);
		}
	}
	LPC_freeInterac(interac);
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

uint16_t SinTable[45] =                                       /* ÕýÏÒ±í                       */
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

void TIMER1_IRQHandler (void)
{
	enable_timer(0);
	LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER2_IRQHandler (void)
{
	disable_timer(1);
	reset_timer(1);
	LPC_DAC->DACR = 0<<6;
	disable_timer(2);
	reset_timer(2);
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
