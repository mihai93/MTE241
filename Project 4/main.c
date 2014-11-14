#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"

#define BG	White
#define FG	Magenta
#define N	50

void line(int x0, int y0, int x1, int y1, unsigned short dimArray[][N]) {
 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
		dimArray[x0][y0] = FG;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void convToBitmap(unsigned short dimArray[][N], unsigned short bitmap[N])
{
	int i, j, index;
	index = 0;
	
	for (i = 0;i<N;i++)
	{
		for (j = 0;j<N;j++)
		{
			if (dimArray[j][i] == FG)
				bitmap[index] = dimArray[j][i];
			else
				bitmap[index] = BG;
			
			index++;
		}
	}
}

void createCircle(int xcenter, int ycenter)
{
	unsigned short circle[N][N] = {BG};
	unsigned short circleBitmap[N];
	int x0, y0, f, dFx, dFy, x, y, radius;
	
	x0 = N/2;
	y0 = N/2;
	radius = N/2;
	f = 1 - radius;
	dFx = 0;
	dFy = -2 * radius;
	x = 0;
	y = radius-1;
	
	line(x0, y - radius, x0, y0 + radius, circle);
	line(x0 - radius, y0, x0 + radius, y0, circle);

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
		
		line(x0 - x, y0 + y, x0 + x, y0 + y, circle);
		line(x0 - x, y0 - y, x0 + x, y0 - y, circle);
		line(x0 - y, y0 + x, x0 + y, y0 + x, circle);
		line(x0 - y, y0 - x, x0 + y, y0 - x, circle);
	}
	convToBitmap(circle, circleBitmap);
	GLCD_Bitmap (xcenter - N/2, ycenter - N/2, N, N, (unsigned char*)circleBitmap);
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