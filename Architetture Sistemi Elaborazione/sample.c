/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "ADC/adc.h"
#include "button_EXINT/button.h"
#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif



int main(void)
{
  SystemInit();  												/* System Initialization (i.e., PLL)  */
  LCD_Initialization();
  BUTTON_init();												/* BUTTON Initialization              */
	init_RIT(0x002DC6C0);									/* RIT Initialization 30 msec       */ //RIT => BUTTON
	init_timer(1,0xBEBC20*2);								/* 0.5 sec*2 */
  ADC_init();														/* ADC Initialization									*/
	//240x320 dimensione schermo
	LCD_Clear(Black);
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	LPC_PINCON->PINSEL1 |= (1<<21);				/* DAC on */
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
