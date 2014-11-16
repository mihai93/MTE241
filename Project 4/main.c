#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"

#define BG	White
#define FG	Magenta
#define N 64

void createCircle(int xcenter, int ycenter)
{
	unsigned short chircle[N][N] = {BG};
	unsigned short bmp[N] = {BG};
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
	
	for (i = 0; i < N*N; i++)
		if (bmp[i] != FG)
			bmp[i] = BG;
			
	GLCD_Bitmap (xcenter - N/2, ycenter - N/2, N, N, (unsigned char*)bmp);
	
	i = 1;
	while(1)
	{
		GLCD_Bitmap (160 - N/2 + i, 120 - N/2 + i, N, N, (unsigned char*)blankBmp);
		GLCD_Bitmap (160 - N/2 + i, 120 - N/2 + i, N, N, (unsigned char*)bmp);
		i++;
	}
}

int main( void ) {
	/*** Declare all variables ***/
// 	unsigned short circle[N][N] = {BG};
// 	unsigned short circleBitmap[N];
	/*** Declare all variables ***/
	
	SystemInit();
	GLCD_Init();
	GLCD_Clear(BG); 
	
	createCircle(160, 120);
	

  while(1);
}	