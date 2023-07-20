/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "adc.h"
#include "../GLCD/GLCD.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/
extern int paddle[2];
unsigned long AD_current = 0;   
static unsigned long AD_last = 0;     /* Last converted value               */
void ADC_IRQHandler(void) {
	static int tries=0;
  AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);/* Read Conversion Result             */
	AD_current=AD_current*(235-30-5);
	AD_current=AD_current/4095;
	AD_current=AD_current+5;
	 //normalizzo i valori del potenziometro rispetto i pixel
  if(AD_current > AD_last+4 || AD_current < AD_last-4){
		if(tries>5){
			LCD_MoveRS(paddle[0],AD_current,LPC_getY0(paddle[0]));
			AD_last = AD_current;
			tries=0;
		}else{
			tries++;
		}
	}
}
