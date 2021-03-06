#include <stdio.h>
#include <stdlib.h>
#include <RTL.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"
#include <time.h>
#include <math.h>

#define BG 0xFFFF
#define FG	0xF81F
#define MAX_DIAM 50
#define MAX_BALLS	7

unsigned short colours[MAX_BALLS]	= {Magenta, Green, Red, DarkCyan, Blue, DarkGreen, Purple};
unsigned short radii[MAX_BALLS] = {22.5, 15, 10, 25, 17.5, 7.5, 12.5};
unsigned short circle[MAX_DIAM*MAX_DIAM];
unsigned short circle2[MAX_DIAM*MAX_DIAM];

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
int count;
int prevCount;
int prevJ;

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
	
	for (i = 0; i < 2*ball->rad; i++){
		for (j = 0; j < 2*ball->rad; j++){
			xsq = (j-ball->rad-ball->vx)*(j-ball->rad-ball->vx);
			ysq = (i-ball->rad+ball->vy)*(i-ball->rad+ball->vy);
			rsq = (ball->rad-2)*(ball->rad-2);
			
			if (xsq + ysq <= rsq)
				circle2[i*2*ball->rad + j] = ball->colour;
			else
				circle2[i*2*ball->rad + j] = BG;
		}
	}
}

void createCircle(ball_t *ball)
{
	int i, j;
	for (i = 0; i < 2*ball->rad; i++){
		for (j = 0; j < 2*ball->rad; j++){
			if ((j-ball->rad)*(j-ball->rad) + (i-ball->rad)*(i-ball->rad) <= (ball->rad -2)*(ball->rad -2))
				circle[i*2*ball->rad + j] = ball->colour;
			else
				circle[i*2*ball->rad + j] = BG;
			
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

int colliding(ball_t *ball, int j)
{
	int xd, yd, radSum, radSqr, distSqr;
	
	xd = (ball->x + ball->rad/2) - (ball_array[j].x + ball_array[j].rad/2);
	yd = (ball->y + ball->rad/2) - (ball_array[j].y + ball_array[j].rad/2);
	
	radSum = ball->rad + ball_array[j].rad;
	radSqr = radSum * radSum;
	
	distSqr = (xd*xd) + (yd*yd);
	
	if (distSqr < radSqr && !(count == prevCount && prevJ == j))
	{
		prevCount = count;
		prevJ = j;
		return 1;
	}
	
	return 0;
}

void trajectory(ball_t *ball, int j)
{
	double x1, y1, x2, y2, vx1, vy1, vx2, vy2, r1, r2, nx, ny, n,
				 unx, uny, utx, uty, vn1, vt1, vn2, vt2,
				 vtf1, vtf2, vnf1, vnf2, vnfx1, vnfy1,
				 vnfx2, vnfy2, xd, yd, d, xtd, ytd, im1, im2;
	
	x1 = ball->x + ball->rad;
	y1 = ball->y + ball->rad;
	x2 = ball_array[j].x + ball_array[j].rad;
	y2 = ball_array[j].y + ball_array[j].rad;
	vx1 = ball->vx*ball->xdir;
	vy1 = ball->vy*ball->ydir;
	vx2 = ball_array[j].vx*ball_array[j].xdir;
	vy2 = ball_array[j].vy*ball_array[j].ydir;
	r1 = ball->rad;
	r2 = ball_array[j].rad;
	
	nx = x2 - x1;
	ny = y2 - y1;
	
	n = sqrt((nx*nx)+(ny*ny));
	unx = nx/n;
	uny = ny/n;
	
	utx = -uny;
	uty = unx;
	
	vn1 = unx*vx1 + uny*vy1;
	vt1 = utx*vx1 + uty*vy1;
	
	vn2 = unx*vx2 + uny*vy2;
	vt2 = utx*vx2 + uty*vy2;
	
	vtf1 = vt1;
	vtf2 = vt2;
	
	vnf1 = (vn1*(r1-r2)+(2*r2*vn2))/(r1+r2);
	vnf2 = (vn2*(r2-r1)+(2*r1*vn1))/(r1+r2);
	
	vnfx1 = vnf1*unx;
	vnfy1 = vnf1*uny;
	
	vnfx2 = vnf2*unx;
	vnfy2 = vnf2*uny;
	
	vx1 = vnfx1 + vtf1;
	vy1 = vnfy1 + vtf1;
	
	vx2 = vnfx2 + vtf2;
	vy2 = vnfy2 + vtf2;
	
	ball->xdir = vx1/abs(vx1);
	ball->vx = vx1/ball->xdir;
	
	ball->ydir = vy1/abs(vy1);
	ball->vy = vy1/ball->ydir;
	
	ball_array[j].xdir = vx2/abs(vx2);
	ball_array[j].vx = vx2/ball_array[j].xdir;
	
	ball_array[j].ydir = vy2/abs(vy2);
	ball_array[j].vy = vy2/ball_array[j].ydir;
	
// 	xd = x1 - x2;
// 	yd = y1 - y2;
// 	d = sqrt((xd*xd) + (yd*yd));
// 	xtd = xd*(((r1+r2)-d)/d);
// 	ytd = yd*(((r1+r2)-d)/d);
// 	
// 	im1 = 1/r1;
// 	im2 = 1/r2;
// 	
// 	ball->x += xtd;
// 	ball->y += ytd;
// 	
// 	ball_array[j].x -= xtd;
// 	ball_array[j].y -= ytd;
}

__task void init_task( void ) {
	unsigned short int pot;
	int i, j, x, y, numCol;
	
	os_tsk_prio_self ( 2 );
	
	os_tsk_create ( readPoti_task, 1);
	
	GLCD_Init();
	GLCD_Clear(BG); 
	
	for(i = 0; i < MAX_BALLS; i++)
	{
		ball_array[i].x = i*(320/MAX_BALLS); ball_array[i].y = i*(240/MAX_BALLS);
		ball_array[i].xdir = (rand()%2)*2 - 1; ball_array[i].ydir = (rand()%2)*2 - 1;
		ball_array[i].rad = radii[i];
		ball_array[i].colour = colours[i];
	}
	
	os_tsk_prio_self ( 1) ;
	
	nballs = 0;
	count = 0;
	numCol = 0;
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
			
			if (ball->x >= 320 - ball->rad)
				ball->xdir = -1*ball->xdir;
			else if (ball->x < ball->rad){
				ball->xdir = -1*ball->xdir;
				ball->x = ball->rad;
			}
			
			if (ball->y >= 240 - ball->rad)
				ball->ydir = -1*ball->ydir;
			else if (ball->y < ball->rad){
				ball->ydir = -1*ball->ydir;
				ball->y = ball->rad;
			}

			for (j = i+1; j < nballs; j++)
			{
				if (colliding(ball, j))
				{
					ball->xdir = ball_array[j].xdir;
					ball_array[j].xdir = -1*ball_array[j].xdir;
					
					ball->ydir = ball_array[j].ydir;
					ball_array[j].ydir = -1*ball_array[j].ydir;
				}
			}
			
			ball->vx = ball->xdir*pot;
			ball->vy = ball->ydir*pot;
			
			eraseCircle(ball);
			
			GLCD_Bitmap (ball->x+ball->vx - ball->rad, ball->y+ball->vy - ball->rad, 2*ball->rad, 2*ball->rad, (unsigned char*)circle);
			GLCD_Bitmap (ball->x - ball->rad, ball->y - ball->rad, 2*ball->rad, 2*ball->rad, (unsigned char*)circle2);

			ball->x += ball->vx; 
			ball->y += ball->vy;
			
			count++;
		}
	}
}	

int main( void ) {
	SystemInit();
	SystemCoreClockUpdate();
	
	printf(" ");
	
	ADCInit();
	LEDInit();
 	INT0Init();

	os_sys_init(init_task);
	
	while ( 1 ) {
		// Endless loop
	}
}