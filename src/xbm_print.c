/*
	gcc -O2 -Wall -o xbm_print xbm_print.c

	2015, masterzorag@gmail.com
*/

#include <stdio.h>
#include "xbm_font.h" /* ! generate it with my genXBMfonts script ! */
#include "trfb.h"

void xbm_print(int x, const int y, const char *text, trfb_framebuffer_t *fb)
{
	int i, j;
	int tempx = 0; // scans the font bitmap, j++ 0-7: +j
	int tempy = 0;
	char c;

	while(*text != '\0'){
		c = *text;

		if(c < LOWER_ASCII_CODE || c > UPPER_ASCII_CODE) c = 180;
//D		printf("%d [%c]\n", c, c);
		
		// font indexing by current char
		char *bit = xbmFont[c - LOWER_ASCII_CODE];
		
		// dump bits map: bytes_per_line 2, size 32 char of 8 bit
		for(i = 0; i < 32; i++) {
			// scans 8 bits per char
			for(j = 0; j < 8; j++){
				//	printf("%c", (data[i] & (1 << j)) ? ' ' : '0' );		// least significant bit first
				//	printf("%d", (data[i] & (0x80 >> j)) ? 1 : 0); 			// right shifting the value will print bits in reverse.				
				if(bit[i] & (1 << j)){		// least significant bit first
				
					//if(sconsole.bgColor != FONT_COLOR_NONE) buffer[(sconsole.curY + tempy) * sconsole.screenWidth + sconsole.curX + tempx] = sconsole.bgColor;
					// paint FG pixel
					trfb_framebuffer_set_pixel(fb, x + tempx + j, y +tempy, TRFB_RGB(240 - tempy*8, 190 - tempy*10, 40));
				}
				else // paint BG pixel, (or trasparency !)
				{
					//if(sconsole.fgColor != FONT_COLOR_NONE) buffer[(sconsole.curY + tempy) * sconsole.screenWidth + sconsole.curX + tempx] = sconsole.fgColor - tempy * 0x00081515;
					trfb_framebuffer_set_pixel(fb, x + tempx + j, y +tempy, TRFB_RGB(0, 0, 0));
				}
			}
			tempx = FONT_W /2;
	  		if(i % (FONT_W /8) != 0) {
	  			tempy++;		// use decrease gradient
	  			tempx=0;
			}
		}
		tempy = 0;
		
		// move one char right		
		x += FONT_W;
		
		/* another way to iterate, scans every 16x16 256px to paint
		j = 0;
		for(i = 0; i < FONT_W * FONT_H; i++){
			//printf("%c", (xbmFont[-32+c] & (1 << j)) ? ' ' : '0' );		// least significant bit first
			
			if(bit[i] & (1 << j)){
				printf(" ");
				
		//	int consoleFont[108][8*16] =  {		//97 !!! not 108
		// static char 035_bits[] = {

				//if(sconsole.bgColor != FONT_COLOR_NONE) buffer[(sconsole.curY + tempy) * sconsole.screenWidth + sconsole.curX + tempx] = sconsole.bgColor;
			}else{
				printf("o");
				//if(sconsole.fgColor != FONT_COLOR_NONE) buffer[(sconsole.curY + tempy) * sconsole.screenWidth + sconsole.curX + tempx] = sconsole.fgColor - tempy * 0x00081515;
			}
						
			j++;
			tempx++;
			if (tempx == FONT_W){
				tempx = 0;
				tempy++;
				j = 0;
				puts("");
			}
		}
*/	
		++text;
	}
}
