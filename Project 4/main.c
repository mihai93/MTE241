#include <stdio.h>
#include <stdlib.h>
#include <RTL.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"

#define BG	White
#define FG	Magenta
#define N 30

unsigned short circle[N*N];

typedef struct {
	int x;
	int vx;
	int y;
	int vy;
	int rad;
} ball_t;

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

void eraseCircle(int xc, int yc, int vx, int vy, int rad)
{
	int i, j;
	
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
// 			if (j < vx && vx > 0)
// 				circle[i*N + j] = BG;
// 			if (j > ((2*rad - 1) + vx - 1)&& vx < 0)
// 				circle[i*N + j] = BG;
// 			if (i > ((2*rad - 1) - vy - 1) && vy > 0)
// 				circle[i*N + j] = BG;
// 			if (i < abs(vy - 1) && vy < 0)
// 				circle[i*N + j] = BG;
			
			//if (vx > 0 && vy > 0){
				if ((j-rad-vx)*(j-rad-vx) + (i-rad+vy)*(i-rad+vy) <= (rad-1)*(rad-1))
					circle[i*N + j] = FG;
				else
					circle[i*N + j] = BG;
			//}
			if (j == 1 || j == N-1)
					circle[i*N + j] = BG;
		}
	}
	
	GLCD_Bitmap (xc - N/2, yc - N/2, N, N, (unsigned char*)circle);
	
}

void createCircle(int xc, int yc)
{
	int i, j;
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
				if ((j-N/2)*(j-N/2) + (i-N/2)*(i-N/2) <= (N/2 -1)*(N/2 -1))
					circle[i*N + j] = FG;
				else
					circle[i*N + j] = BG;
				
				if (j == 1 || j == N-1)
					circle[i*N + j] = BG;
		}
	}
	
// 	x0 = N/2;
// 	y0 = N/2;
// 	radius = N/2;
// 	f = 1 - radius;
// 	dFx = 0;
// 	dFy = -2 * radius;
// 	x = 0;
// 	y = radius-1;

// 	circle[(y - radius)*N + x0] = FG;
// 	circle[(y0 + radius)*N + x0] = FG;
// 	circle[y0*N + (x - radius)] = FG;
// 	circle[y0*N + (x0 - radius)] = FG;
// 	
// 	for(i=0;(x0-radius+i) <= (x0+radius);i++)
// 		circle[y0*N + (x0-radius+i)] = FG;

// 	while(x < y){
// 		if(f >= 0){
// 			y--;
// 			dFy += 2;
// 			f += dFy;
// 		}
// 		
// 		x++;
// 		dFx += 2;
// 		f += dFx + 1;
// 		
// 		for(i=0;(x0-x+i) <= (x0+x);i++)
// 			circle[(y0 + y)*N + (x0-x+i)] = FG;
// 		
// 		for(i=0;(x0-x+i) <= (x0+x);i++)
// 			circle[(y0 - y)*N + (x0-x+i)] = FG;
// 		
// 		for(i=0;(x0-y+i) <= (x0+y);i++)
// 			circle[(y0 + x)*N + (x0-y+i)] = FG;
// 		
// 		for(i=0;(x0-y+i) <= (x0+y);i++)
// 			circle[(y0 - x)*N + (x0-y+i)] = FG;
// 	}
// 	
// 	for (i = 0; i < N*N; i++){
// 		if (circle[i] != FG)
// 			circle[i] = BG;
// 	}
	
	GLCD_Bitmap (xc - N/2, yc - N/2, N, N, (unsigned char*)circle);
}

__task void readPoti_task(void){
	
	while( 1 ){
		ADCConvert();
		//Now wiat for the other threads.
		os_dly_wait( 100 );
	}
}

__task void init_task( void ) {
//int main( void ) {
	unsigned short int potval;
	int x, y, count;
	ball_t ball;
	
	os_tsk_prio_self ( 2 );
	
	os_tsk_create ( readPoti_task, 1);
	
	GLCD_Init();
	GLCD_Clear(BG); 
	
	ball.x = 160; ball.y = 120;
	ball.vx = 5; ball.vy = 5;
	ball.rad = 15;
	
	createCircle(ball.x, ball.y);
	
	//count = 1;
	
	os_tsk_prio_self ( 1) ;
	while(1)
	{
		//os_dly_wait(1000);
 		potval = ADCValue();
		
		if (potval == 0)
			potval = 1;
		
// 	if(count == 10){
// 		count = 1;
		//os_dly_wait(1000);
		createCircle(ball.x, ball.y);
		//os_dly_wait(1000);
		eraseCircle(ball.x, ball.y, ball.vx*potval/250, ball.vy*potval/250, ball.rad);
		//os_dly_wait(1000);

		
		ball.x += ball.vx*potval/250; 
		ball.y += ball.vy*potval/250;
		
		
		if (ball.x > 320)
			ball.x = 320 - N/2;
		else if (ball.x < 0)
			ball.x = N/2;
		
		if (ball.y > 240)
			ball.y = 240 - N/2;
		else if (ball.y < 0)
			ball.y = N/2;
		
		if (ball.x >= 320 - N/2 || ball.x <= N/2)
			ball.vx = -1*ball.vx;
		
		if (ball.y >= 240 - N/2 || ball.y <= N/2)
			ball.vy = -1*ball.vy;
// 		}
		//count ++;
	}
}	

int main( void ) {
	SystemInit();
	SystemCoreClockUpdate();
	
	ADCInit();

	os_sys_init(init_task);
}