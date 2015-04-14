#include <trfb.h>
#include <string.h>
#include <stdlib.h>

static int isBE(void)
{
	union {
		unsigned value;
		unsigned char data[sizeof(unsigned)];
	} test;

	test.value = 1;

	return !test.data[0];
}

#define BALL(r, g) \
	TRFB_RGB(r, g, 0x00), \
	TRFB_RGB(r, g, 0x33), \
	TRFB_RGB(r, g, 0x66), \
	TRFB_RGB(r, g, 0x99), \
	TRFB_RGB(r, g, 0xCC), \
	TRFB_RGB(r, g, 0xFF)

#define GBALL(r) \
	BALL(r, 0x00), \
	BALL(r, 0x33), \
	BALL(r, 0x66), \
	BALL(r, 0x99), \
	BALL(r, 0xCC), \
	BALL(r, 0xFF)

const trfb_color_t trfb_framebuffer_pallete[256] = {
	GBALL(0x00),
	GBALL(0x33),
	GBALL(0x66),
	GBALL(0x99),
	GBALL(0xCC),
	GBALL(0xFF), /* 216 - we need 40 zeros: */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 256 / 6 is 42 and 42 * 6 = 252. */
#define SIX(some) \
	some, some, some, some, some, some
#define SEVEN(some) \
	SIX(some), some
const uint8_t trfb_framebuffer_revert_pallete[256] = {
	SIX(SEVEN(0x00)), 0x00, 0x00,
	SIX(SEVEN(0x33)),
	SIX(SEVEN(0x66)),
	SIX(SEVEN(0x99)),
	SIX(SEVEN(0xCC)),
	SIX(SEVEN(0xFF)), 0xFF, 0xFF
};

/* Framebuffer support */
trfb_framebuffer_t* trfb_framebuffer_create(unsigned width, unsigned height, unsigned char bpp)
{
	unsigned sz = width * height;
	trfb_framebuffer_t *fb;

	if (width > 0xffff || height > 0xffff) {
		trfb_msg("Trying to create framebuffer of size %ux%u", width, height);
		return NULL;
	}

	fb = calloc(1, sizeof(trfb_framebuffer_t));
	if (!fb) {
		trfb_msg("Not enought memory");
		return NULL;
	}

	fb->bpp = bpp;
	fb->width = width;
	fb->height = height;
	if (bpp == 8) {
		fb->pixels = calloc(1, sz);
		/* Mask and shift does not matter */
	} else if (bpp == 16) {
		fb->pixels = calloc(2, sz);
		fb->rmask = TRFB_FB16_RMASK;
		fb->gmask = TRFB_FB16_GMASK;
		fb->bmask = TRFB_FB16_BMASK;
		fb->rshift = TRFB_FB16_RSHIFT;
		fb->gshift = TRFB_FB16_GSHIFT;
		fb->bshift = TRFB_FB16_BSHIFT;
	} else if (bpp == 32) {
		fb->pixels = calloc(4, sz);
		fb->rmask = TRFB_FB32_RMASK;
		fb->gmask = TRFB_FB32_GMASK;
		fb->bmask = TRFB_FB32_BMASK;
		fb->rshift = TRFB_FB32_RSHIFT;
		fb->gshift = TRFB_FB32_GSHIFT;
		fb->bshift = TRFB_FB32_BSHIFT;
	} else {
		trfb_msg("Only 8, 16 and 32 bits per pixel is supported");
		free(fb);
		return NULL;
	}

	if (!fb->pixels) {
		trfb_msg("Not enought memory");
		free(fb);
		return NULL;
	}

	return fb;
}

trfb_framebuffer_t* trfb_framebuffer_copy(trfb_framebuffer_t *fb)
{
	trfb_framebuffer_t *c;
	size_t len;

	c = malloc(sizeof(trfb_framebuffer_t));
	if (!c) {
		return NULL;
	}

	memcpy(c, fb, sizeof(trfb_framebuffer_t));
	if (fb->bpp != 8 && fb->bpp != 16 && fb->bpp != 32) {
		free(c);
		return NULL;
	}

	len = fb->width * fb->height * (fb->bpp / 8);
	c->pixels = malloc(len);
	if (!c->pixels) {
		free(c);
		return NULL;
	}

	memcpy(c->pixels, fb->pixels, len);

	return c;
}

void trfb_framebuffer_free(trfb_framebuffer_t *fb)
{
	if (fb) {
		free(fb->pixels);
		free(fb);
	}
}

int trfb_framebuffer_resize(trfb_framebuffer_t *fb, unsigned width, unsigned height)
{
	unsigned W, H;
	unsigned y;

	if (!fb || !width || !height || width > 0xffff || height > 0xffff) {
		trfb_msg("Invalid params to resize");
		return -1;
	}

	W = width < fb->width? width : fb->width;
	H = height < fb->height? height : fb->height;

#define FB_COPY(tp) \
	do { \
		tp *p = fb->pixels; \
		tp *np; \
		np = calloc(width * height, sizeof(tp)); \
		if (!np) { \
			trfb_msg("Not enought memory!"); \
			return -1; \
		} \
		for (y = 0; y < H; y++) \
			memcpy(np + y * width, p + y * fb->width, W * sizeof(tp)); \
	} while (0)

	if (fb->bpp == 8) {
		FB_COPY(uint8_t);
	} else if (fb->bpp == 16) {
		FB_COPY(uint16_t);
	} else if (fb->bpp == 32) {
		FB_COPY(uint32_t);
	} else {
		trfb_msg("Invalid framebuffer: bpp = %d", fb->bpp);
		return -1;
	}

	fb->width = width;
	fb->height = height;

	return 0;
}

/* This is the main function for us */
int trfb_framebuffer_convert(trfb_framebuffer_t *dst, trfb_framebuffer_t *src)
{
	unsigned y, x;

	if (!dst || !src) {
		trfb_msg("Invalid arguments");
		return -1;
	}

	if (src->bpp != 8 && src->bpp != 16 && src->bpp != 32) {
		trfb_msg("Invalid framebuffer: BPP = %d", src->bpp);
		return -1;
	}

	if (dst->bpp != 8 && dst->bpp != 16 && dst->bpp != 32) {
		trfb_msg("Invalid framebuffer: BPP = %d", dst->bpp);
		return -1;
	}

	if (!src->width || !src->height || !dst->width || !dst->height ||
			src->width > 0xffff || src->height > 0xffff ||
			dst->width > 0xffff || dst->height > 0xffff) {
		trfb_msg("Framebuffer of invalid size");
		return -1;
	}

	/* If image has another size we need to change it: */
	if (dst->width != src->width || dst->height != src->height) {
		void *p = realloc(dst->pixels, src->width * src->height * (dst->bpp / 8));
		if (!p) {
			trfb_msg("Not enought memory!");
			return -1;
		}
		dst->pixels = p;
		dst->width = src->width;
		dst->height = src->height;
	}

	if ((dst->bpp == 8 && src->bpp == 8) ||
			(
			 dst->bpp == src->bpp &&
			 dst->rmask == src->rmask &&
			 dst->gmask == src->gmask &&
			 dst->bmask == src->bmask &&
			 dst->rshift == src->rshift &&
			 dst->gshift == src->gshift &&
			 dst->bshift == src->bshift
			)
	   ) { /* Format is the same! */
		memcpy(dst->pixels, src->pixels, src->width * src->height * (dst->bpp / 8));
		return 0;
	}

	/* Ok, so we need to do it slow way... */
	/* TODO: it is too slow */
	for (y = 0; y < src->height; y++) {
		for (x = 0; x < src->width; x++) {
			trfb_framebuffer_set_pixel(dst, x, y, trfb_framebuffer_get_pixel(src, x, y));
		}
	}
}

int trfb_framebuffer_format(trfb_framebuffer_t *fb, trfb_format_t *fmt)
{
	if (!fb || !fmt) {
		trfb_msg("Invalid arguments");
		return -1;
	}

	fmt->bpp = fb->bpp;
	fmt->big_endian = isBE();
	fmt->depth = fb->bpp > 8? 3: 1;
	fmt->true_color = fb->bpp > 8? 1: 0;
	fmt->rmax = fb->rmask;
	fmt->gmax = fb->gmask;
	fmt->bmax = fb->bmask;
	fmt->rshift = fb->rshift;
	fmt->gshift = fb->gshift;
	fmt->bshift = fb->bshift;

	return 0;
}

trfb_framebuffer_t* trfb_framebuffer_create_of_format(unsigned width, unsigned height, trfb_format_t *fmt)
{
	trfb_framebuffer_t *fb;

	if (!fmt) {
		trfb_msg("Invalid arguments");
		return NULL;
	}

	if (fmt->bpp != 8 && fmt->bpp != 16 && fmt->bpp != 32) {
		trfb_msg("Invalid format: BPP = %d", fmt->bpp);
		return NULL;
	}

	fb = trfb_framebuffer_create(width, height, fmt->bpp);
	if (!fb) {
		return NULL;
	}

	fb->rmask = fmt->rmax;
	fb->gmask = fmt->gmax;
	fb->bmask = fmt->bmax;
	fb->rshift = fmt->rshift;
	fb->gshift = fmt->gshift;
	fb->bshift = fmt->bshift;

	return fb;
}

void trfb_framebuffer_endian(trfb_framebuffer_t *fb, int is_be)
{
	size_t i;
	size_t len;
	unsigned char tmp;
	unsigned char *p;

#define cswap(x, y) \
	do { \
		tmp = x; \
		x = y; \
		y = tmp; \
	} while (0)

	if (!fb)
		return;
	if (fb->bpp == 8)
		return;
	if (isBE() && is_be)
		return;
	if (!isBE() && !is_be)
		return;

	p = fb->pixels;
	if (fb->bpp == 16) {
		len = fb->width * fb->height * 2;
		for (i = 0; i < len; i += 2) {
			cswap(p[i], p[i + 1]);
		}
	} else if (fb->bpp == 32) {
		len = fb->width * fb->height * 4;
		for (i = 0; i < len; i += 4) {
			cswap(p[i], p[i + 3]);
			cswap(p[i + 1], p[i + 2]);
		}
	}
}
