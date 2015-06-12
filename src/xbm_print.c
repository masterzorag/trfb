/*
	gcc -O2 -Wall -o xbm_print xbm_print.c

	2015, masterzorag@gmail.com
*/

#include "trfb.h"

/* generate with genXBMfonts, https://github.com/masterzorag/xbm_tools */
#include "xbm_font.h"

/*
	Printing a text with xbm
*/
void xbm_print(int x, const int y, const char *text, trfb_framebuffer_t *fb)
{
	short i, j;
	// 2^16 = 65536, sqrt(65536) = 256
	// 256*256 px max with a short !
	int tx = 0,	ty = 0;
	char c;

	while(*text != '\0'){
		c = *text;
		if(c < LOWER_ASCII_CODE || c > UPPER_ASCII_CODE) c = 180;
		
		// indexing font glyph by current char
		char *bit = xbmFont[c - LOWER_ASCII_CODE];
		
		// dump bits map: bytes_per_line 2, size 32 char of 8 bit
		for(i = 0; i < ((FONT_W * FONT_H) / BITS_IN_BYTE); i++) {
			for(j = 0; j < BITS_IN_BYTE; j++){
				//	printf("%c", (bit[i] & (1 << j)) ? ' ' : '0' );		// least significant bit first
				//	printf("%d", (bit[i] & (0x80 >> j)) ? 1 : 0); 			// right shifting the value will print bits in reverse.				
				if(bit[i] & (1 << j)){		// least significant bit first
					// paint FG pixel
					trfb_framebuffer_set_pixel(fb, x + tx * BITS_IN_BYTE + j, y + ty, TRFB_RGB(
						220 - ty*9 + (tx * BITS_IN_BYTE + j) *2,
						130 - ty*2 + (tx * BITS_IN_BYTE + j) *3,
						 40 - ty*3 + (tx * BITS_IN_BYTE + j) *4
						));
				}
				else // paint BG pixel (or trasparency)
				{
					trfb_framebuffer_set_pixel(fb, x + tx * BITS_IN_BYTE + j, y + ty, TRFB_RGB(0, 0, 0));
				}
			}
			tx++;
	  		if(tx == (FONT_W / BITS_IN_BYTE)) {
	  			ty++;		// use to decrease gradient
	  			tx=0;
			}
		}
		ty = 0;
		
		// glyph painted, move one char right in text
		x += FONT_W;
		++text;
	}
}