#include <stdio.h>
#include <stdlib.h>
#include <RTL.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"

#define BG	White
#define FG	Magenta
#define N 64

volatile unsigned short int ADC_Value;

// ADC hs not been read yet.
volatile unsigned char ADC_Done = 0; 

// Initializing the Potentiometer (ADC) ports
void ADCInit( void ) {

	// Enabled the Power controler in PCONP register. According the Table 46. the 12th bit is PCADC
	LPC_SC->PCONP |= (1 << 12);

	// Poti is connected to port P0.25. We have to put the port P0.25 into the AD0.2 moe for anlaoge to digital conterting.
	LPC_PINCON->PINSEL1 &= ~(0x3 << 18); // Remove all bits, Port P0.25 gets GPIO
	LPC_PINCON->PINSEL1 |=  (0x1 << 18); // Switch P0.25 to AD0.2

	// No pull-up no pull-down (function 10) on the AD0.2 pin.
	LPC_PINCON->PINMODE1 &= ~(0x3 << 18);
	LPC_PINCON->PINMODE1 |=  (0x1 << 18);

	// A/D Control Register (Section 29.5.1)
	LPC_ADC->ADCR = ( 1 <<  2)  |    // SEL=1        select channel 0~7 on AD0.2 
	                ( 4 <<  8)  |    // ADC clock is 25 MHz/5          
	                ( 0 << 16 ) |    // BURST = 0    no BURST, software controlled 
	                ( 0 << 24 ) |    // START = 0    A/D conversion stops */
	                ( 0 << 27 ) |    // EDGE = 0     CAP/MAT singal falling,trigger A/D conversion
	                ( 1 << 21);      // PDN = 1      normal operation, Enable ADC                

	// Enabling A/D Interrupt Enable Register for all channels (Section 29.5.3)
	LPC_ADC->ADINTEN = ( 1 <<  8);        

	// Registering the interrupt service for ADC
	NVIC_EnableIRQ( ADC_IRQn );                  
}

// Starting the conversion. Upon the call of this function, the ADC unit starts
// to read the connected port to its channel. The conversion takes 56 clock ticks.
// According the initialization, an intrupt will be called when the data becomes 
// ready.
void ADCConvert (void) {
	// Stop reading and converting the port channel AD0.2.
  LPC_ADC->ADCR &= ~( 7 << 24); 
	ADC_Done = 0;
	// Start reading and converting the analog input from P0.25, where Poti is connected
	//to the challen Ad0.2
  LPC_ADC->ADCR |=  ( 1 << 24) | (1 << 2);              /* start conversion              */
}

void ADC_IRQHandler( void ) {
	volatile unsigned int aDCStat;

	// Read ADC Status clears the interrupt
	aDCStat = LPC_ADC->ADSTAT;

	// Read the value and and witht a max value as 12-bit.
	ADC_Value = (LPC_ADC->ADGDR >> 4) & 0xFFF; 

	ADC_Done = 1;
}

unsigned short int ADCValue( void ) {

	// Busy wainting until the conversion is done
	while ( !ADC_Done ) {
		// Wait for IRQ handler
	}

	return ADC_Value;
}

void eraseCircle(int xc, int yc, unsigned short bmp[N])
{
	int i;
	
	for (i = 0; i < N*N; i++)
			bmp[i] = BG;
	
	GLCD_Bitmap (xc - N/2, yc - N/2, N, N, (unsigned char*)bmp);
	
}

void createCircle(int xc, int yc, unsigned short bmp[N])
{
	unsigned short chircle[N][N] = {BG};
	//unsigned short bmp[N] = {BG};
	unsigned short blankBmp[N] = {BG};
	int i, x0, y0, f, dFx, dFy, x, y, radius;
	
	x0 = N/2;
	y0 = N/2;
	radius = N/2;
	f = 1 - radius;
	dFx = 0;
	dFy = -2 * radius;
	x = 0;
	y = radius-1;

	bmp[(y - radius)*N + x0] = FG;
	bmp[(y0 + radius)*N + x0] = FG;
	bmp[y0*N + (x - radius)] = FG;
	bmp[y0*N + (x0 - radius)] = FG;
	
	for(i=0;(x0-radius+i) <= (x0+radius);i++)
		bmp[y0*N + (x0-radius+i)] = FG;

	while(x < y)
	{
		if(f >= 0)
		{
			y--;
			dFy += 2;
			f += dFy;
		}
		x++;
		dFx += 2;
		f += dFx + 1;
		
		for(i=0;(x0-x+i) <= (x0+x);i++)
			bmp[(y0 + y)*N + (x0-x+i)] = FG;
		
		for(i=0;(x0-x+i) <= (x0+x);i++)
			bmp[(y0 - y)*N + (x0-x+i)] = FG;
		
		for(i=0;(x0-y+i) <= (x0+y);i++)
			bmp[(y0 + x)*N + (x0-y+i)] = FG;
		
		for(i=0;(x0-y+i) <= (x0+y);i++)
			bmp[(y0 - x)*N + (x0-y+i)] = FG;
	}
	
	for (i = 0; i < N*N; i++){
		if (bmp[i] != FG)
			bmp[i] = BG;
	}
	
	GLCD_Bitmap (xc - N/2, yc - N/2, N, N, (unsigned char*)bmp);
}

int main( void ) {
	/*** Declare all variables ***/
// 	unsigned short circle[N][N] = {BG};
 	unsigned short circle1[N];
	unsigned short blankBmp[N];
	int xcenter, ycenter, x, y, i, count;
	char str[15];
	/*** Declare all variables ***/
	
	SystemInit();
	GLCD_Init();
	ADCInit();
	GLCD_Clear(BG); 
	
	x = 160;
	y = 120;
	
	createCircle(x, y, circle1);
	
	count = 1;
	while(1)
	{
		if(count == 25000){
			count = 0;
		
			ADCConvert();
			//ADC_IRQHandler();
			//while(!ADC_Done);
			sprintf(str, "%d", ADCValue());
			GLCD_DisplayString(1, 1, 0, str);
			
			//os_dly_wait( 1 );
			
			eraseCircle(x, y, circle1);
		
			x += 1;
			y += 1;
			
			createCircle(x, y, circle1);
			
			if (x== 320 || y == 240)
			{
				x = 0;
				y = 0;
			}
		}
		
		count ++;
	}
	
  while(1);
}	