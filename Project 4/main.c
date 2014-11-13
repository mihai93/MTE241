#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <LPC17xx.h>
#include "GLCD.h"

#define BG	White
#define FG	Magenta
#define FG2	Blue
#define N	50
#define CORNER	5

void line(int x0, int y0, int x1, int y1, unsigned short circle[][N]) {
 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
 
  for(;;){
		circle[x0][y0] = FG;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

int main( void ) {
	/*** Declare all variables ***/
	unsigned short square[N] = BG;
	unsigned short circle[N][N] = {BG};
	unsigned short circleBitmap[N];
  int i, j, k, radius, f, ddF_x, ddF_y, r, c, index, x, y, x0, y0; 
	/*** Declare all variables ***/
	
	SystemInit();
	GLCD_Init();
	GLCD_Clear(BG); 

// 	for (i = 0; i<10;i++)
// 		square[i] = FG2;
// 	for (i = 10; i< (N*N)-10; i++)
// 		square[i] = FG;		
// 	for (i = (N*N)-10;i<N*N;i++)
// 		square[i] = FG2;

	x0 = N/2;
	y0 = N/2;
	radius = N/2;
	f = 1 - radius;
	ddF_x = 0;
	ddF_y = -2 * radius;
	x = 0;
	y = radius-1;
	
	line(x0, y - radius, x0, y0 + radius, circle);
// 	circle[x0][y0 + radius] = FG;
// 	circle[x0][y0 - radius] = FG;
	line(x0 - radius, y0, x0 + radius, y0, circle);
// 	circle[x0 + radius][y0] = FG;
// 	circle[x0 - radius][y0] = FG;
	printf("Test1");
	while(x < y)
	{
		if(f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		
		line(x0 - x, y0 + y, x0 + x, y0 + y, circle);
		//circle[x0 + x][y0 + y] = FG;
	  //circle[x0 - x][y0 + y] = FG;
		line(x0 - x, y0 - y, x0 + x, y0 - y, circle);
		//circle[x0 + x][y0 - y] = FG;
		//circle[x0 - x][y0 - y] = FG;
		line(x0 - y, y0 + x, x0 + y, y0 + x, circle);
		//circle[x0 + y][y0 + x] = FG;
		//circle[x0 - y][y0 + x] = FG;
		line(x0 - y, y0 - x, x0 + y, y0 - x, circle);
		//circle[x0 + y][y0 - x] = FG;
		//circle[x0 - y][y0 - x] = FG;
	}
	

	
	index = 0;
	
	for (i = 0;i<N;i++)
	{
		for (j = 0;j<N;j++)
		{
			if (circle[j][i] == FG)
			{
				circleBitmap[index] = circle[j][i];
			}	else {
				circleBitmap[index] = BG;
			}
			
			index++;
		}
	}

	index = 0;
	
	for (i = 0;i<N;i++)
	{
		for (j = 0;j<N;j++)
		{
			if ((i < CORNER || i >= N - CORNER) && (j < CORNER || j >= N - CORNER))
				square[index] = FG;
			else
				square[index] = FG2;
			
			index++;
		}
	}
	

	GLCD_Bitmap (160-N/2, 120-N/2, N, N, (unsigned char*)circleBitmap);

  while(1);
}
	