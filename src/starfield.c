/* Copyright (C) 2002 W.P. van Paassen - peter@paassen.tmfweb.nl

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* note that the code has not been fully optimized */

#include <math.h>
#include "trfb.h"

#define NUMBER_OF_STARS 200

/* star struct */
typedef struct 
{
	float xpos, ypos;
	short zpos, speed;
	unsigned int color;
} STAR;

static STAR stars[NUMBER_OF_STARS];

void init_star(STAR *star, int i)
{
	/* randomly init stars, generate them around the center of the screen */
	star->xpos =  -10.0 + (20.0 * (rand()/(RAND_MAX+1.0)));
	star->ypos =  -10.0 + (20.0 * (rand()/(RAND_MAX+1.0)));

	/* change viewpoint */
	star->xpos *= 3141;
	star->ypos *= 3141;

	star->zpos =  i;
	star->speed =  2 + (int)(2.0 * (rand()/(RAND_MAX+1.0)));

	star->color = 
		(uint32_t)(i^2) << 24 | (uint32_t)(i^2) << 16 |
		(uint32_t)(i^2) << 8  | (uint32_t)(i^2);
}

void init(/* stars */)
{
	short i;
	for (i = 0; i < NUMBER_OF_STARS; i++) 
  		init_star(stars + i, i + 1);
}

void move(trfb_framebuffer_t *fb)
{
	short i, tx, ty;
	for (i = 0; i < NUMBER_OF_STARS; i++){
		stars[i].zpos -= stars[i].speed;
		  
		if (stars[i].zpos <= 0) init_star(stars + i, i + 1);
		
		/* compute 3D position */
		tx = (stars[i].xpos / stars[i].zpos) + (fb->width >> 1);
		ty = (stars[i].ypos / stars[i].zpos) + (fb->height >> 1);

		/* check if a star leaves the screen */  
		if (tx < 0 || tx > fb->width - 1 || ty < 0 || ty > fb->height - 1) {
			init_star(stars + i, i + 1);
			continue;
		}
		
		trfb_framebuffer_set_pixel(fb, tx, ty, stars[i].color);
	}
}