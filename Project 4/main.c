#include <stdio.h>
#include <stdlib.h>
#include <RTL.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"
#include <time.h>

#define BG 0xFFFF
#define FG	0xF81F
#define N 45
#define MAX_BALLS	5
#define maxRadius	45

unsigned short colours[]	= {0xF81F, 0x07E0, 0xF800, 0xFFE0, 0x001F};
unsigned short circle[N*N];
unsigned short circle2[N*N];

typedef struct {
	int x;
	int y;
	int vx;
	int vy;
	int xdir;
	int ydir;
	int rad;
	unsigned short colour;
} ball_t;

ball_t ball_array[MAX_BALLS];

const unsigned char ledPosArray[8] = { 28, 29, 31, 2, 3, 4, 5, 6 };

volatile unsigned short int ADC_Value;

// ADC hs not been read yet.
volatile unsigned char ADC_Done = 0; 

volatile unsigned char nballs = 0;
int createBall = 0;

void LEDInit( void ) {

	// LPC_SC is a general system-control register block, and PCONP referes
	// to Power CONtrol for Peripherals.
	//  - Power/clock control bit for IOCON, GPIO, and GPIO interrupts (Section 4.8.9)
	//    This can also be enabled from `system_LPC17xx.c'
	LPC_SC->PCONP     |= (1 << 15);            

	// The ports connected to p1.28, p1.29, and p1.31 are in mode 00 which
	// is functioning as GPIO (Section 8.5.5)
	LPC_PINCON->PINSEL3 &= ~(0xCF00);

	// The port connected to p2.2, p2.3, p2.4, p2.5, and p2.6 are in mode 00
	// which is functioning as GPIO (Section 8.5.5)
	LPC_PINCON->PINSEL4 &= (0xC00F);

	// LPC_GPIOx is the general control register for port x (Section 9.5)
	// FIODIR is Fast GPIO Port Direction control register. This register 
	// individually controls the direction of each port pin (Section 9.5)
	//
	// Set the LEDs connected to p1.28, p1.29, and p1.31 as output
	LPC_GPIO1->FIODIR |= 0xB0000000;           

	// Set the LEDs connected to p2.2, p2.3, p2.4, p2.5, and p2.6 as output port
	LPC_GPIO2->FIODIR |= 0x0000007C;           
}

// Turn on the LED inn a position within 0..7
void turnOnLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);

	// The first two LEDs are connedted to the port 28, 29 and 30
	if ( led < 3 ) {
		// Fast Port Output Set register controls the state of output pins.
		// Writing 1s produces highs at the corresponding port pins. Writing 0s has no effect (Section 9.5)
		LPC_GPIO1->FIOSET |= mask;
	} else {
		LPC_GPIO2->FIOSET |= mask;
	}

}

// Turn off the LED in the position within 0..7
void turnOffLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);

	// The first two LEDs are connedted to the port 28, 29 and 30
	if ( led < 3 ) {
		// Fast Port Output Clear register controls the state of output pins. 
		// Writing 1s produces lows at the corresponding port pins (Section 9.5)
		LPC_GPIO1->FIOCLR |= mask;
	} else {
		LPC_GPIO2->FIOCLR |= mask;
	}
}

void ballCount(){
unsigned temp;
int numLED = 0; 
	if (createBall > 0 && nballs < MAX_BALLS)
	{
		if (createBall == 1)
			nballs++;
		
		temp = nballs;

		while(temp > 0 && temp < 256)
		{
			if((temp % 2) == 1){
				turnOnLED(numLED);
			}
			else {
				turnOffLED(numLED);
			}
			
			numLED++; 
			temp = temp / 2; 
		 }
		temp++;
		createBall = 0;
	}
}


/******************This section shows sample API to work with INT0 button******/

void INT0Init( void ) {

	// P2.10 is related to the INT0 or the push button.
	// P2.10 is selected for the GPIO 
	LPC_PINCON->PINSEL4 &= ~(3<<20); 

	// P2.10 is an input port
	LPC_GPIO2->FIODIR   &= ~(1<<10); 

	// P2.10 reads the falling edges to generate the IRQ
	// - falling edge of P2.10
	LPC_GPIOINT->IO2IntEnF |= (1 << 10);

	// IRQ is enabled in NVIC. The name is reserved and defined in `startup_LPC17xx.s'.
	// The name is used to implemet the interrupt handler above,
	NVIC_EnableIRQ( EINT3_IRQn );

	
}

// INT0 interrupt handler
void EINT3_IRQHandler( void ) {

	
	// Check whether the interrupt is called on the falling edge. GPIO Interrupt Status for Falling edge.
	if ( LPC_GPIOINT->IO2IntStatF && (0x01 << 10) ) {
		LPC_GPIOINT->IO2IntClr |= (1 << 10); // clear interrupt condition

		// Do the stuff 
		//stopBlink = stopBlink ^ 1 ;
		//nballs++;
		//printf("%d \n", nballs);
		createBall += 1;
		ballCount();
		
	}
}

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

void eraseCircle(ball_t *ball)
{
	int i, j, xsq, ysq, rsq;
	
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			xsq = (j-ball->rad-ball->vx)*(j-ball->rad-ball->vx);
			ysq = (i-ball->rad+ball->vy)*(i-ball->rad+ball->vy);
			rsq = (ball->rad-2)*(ball->rad-2);
			
			if (xsq + ysq <= rsq)
				circle2[i*N + j] = ball->colour;
			else
				circle2[i*N + j] = BG;
		}
	}
}

void createCircle(ball_t *ball)
{
	int i, j;
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++){
			if ((j-N/2)*(j-N/2) + (i-N/2)*(i-N/2) <= (N/2 -2)*(N/2 -2))
				circle[i*N + j] = ball->colour;
			else
				circle[i*N + j] = BG;
			
		}
	}
}

__task void readPoti_task(void){
	
	while( 1 ){
		ADCConvert();
		//Now wiat for the other threads.
		os_dly_wait( 500 );
	}
}

__task void init_task( void ) {
	unsigned short int pot;
	int i, x, y, count;
	
	os_tsk_prio_self ( 2 );
	
	os_tsk_create ( readPoti_task, 1);
	
	GLCD_Init();
	GLCD_Clear(BG); 
	
	for(i = 0; i < MAX_BALLS; i++)
	{
		ball_array[i].x = i*(320/MAX_BALLS); ball_array[i].y = i*(240/MAX_BALLS);
		ball_array[i].xdir = (rand()%2)*2 - 1; ball_array[i].ydir = (rand()%2)*2 - 1;
		ball_array[i].rad = N/2;
		ball_array[i].colour = colours[i];
	}
	
	os_tsk_prio_self ( 1) ;
	
	nballs = 0;
	count = 0;
	while(1)
	{
		for (i = 0; i < nballs; i++)
		{
			ball_t *ball;
			
			if (count == nballs)
				count = 0;
			
			ball = ball_array + count;
			
			pot = ADCValue()/850;
			
			createCircle(ball);
			
			if (ball->x >= 320 - N/2)
				ball->xdir = -1*ball->xdir;
			else if (ball->x < N/2){
				ball->xdir = -1*ball->xdir;
				ball->x = N/2;
			}
			
			if (ball->y >= 240 - N/2)
				ball->ydir = -1*ball->ydir;
			else if (ball->y < N/2){
				ball->ydir = -1*ball->ydir;
				ball->y = N/2;
			}

			ball->vx = ball->xdir*pot;
			ball->vy = ball->ydir*pot;
			
			eraseCircle(ball);
			
			GLCD_Bitmap (ball->x+ball->vx - ball->rad, ball->y+ball->vy - ball->rad, N, N, (unsigned char*)circle);
			GLCD_Bitmap (ball->x - ball->rad, ball->y - ball->rad, N, N, (unsigned char*)circle2);

			ball->x += ball->vx; 
			ball->y += ball->vy;
			
			count++;
		}
	}
}	

int main( void ) {
	SystemInit();
	SystemCoreClockUpdate();
	
	ADCInit();
	LEDInit();
 	INT0Init();

	os_sys_init(init_task);
}