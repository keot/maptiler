/*
 *  sdl_helper.c
 *  maptiler
 *
 *  Created by James Mardell on 21/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "sdl_helper.h"

void initialiseSDL(void)
{
	// Get the the current display surface
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	// Constants TODO: Replace with libconfig input
	const int display_w = 1024;
	const int display_h = 768;
	const int display_bpp = 32;
	
	// Library setting constants
	const Uint32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
	const Uint32 sdl_display_flags = SDL_HWSURFACE | SDL_DOUBLEBUF;// | SDL_FULLSCREEN;
	
	// SDL synchronisation
	SDL_putenv("__GL_SYNC_TO_VBLANK=1");
	
	// Initialise SDL
	if (SDL_Init(sdl_init_flags) < 0) {
		fprintf(stderr, "SDL: Error initialising: %s\n", SDL_GetError() );
		exit(1);
	}
	
	// Initialise SDL_image
	const int sdl_image_flags = IMG_INIT_PNG;
	
	if (IMG_Init(sdl_image_flags) != sdl_image_flags) {
		fprintf(stderr, "SDL_image: Error initialising PNG library: %s\n", IMG_GetError() );
		exit(1);
	}
	
	// Initialise SDL_ttf
	if (TTF_Init() == -1) {
		fprintf(stderr, "SDL_ttf: Error initialising: %s\n", TTF_GetError() );
		exit(1);
	}
	
	// OpenGL synchronisation
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	
	// Open display
	display = SDL_SetVideoMode(display_w, display_h, display_bpp, sdl_display_flags);
	
	if (!display) {
		fprintf(stderr, "SDL: Error setting video mode of %dx%dx%d: %s\n",
				display_w, display_h, display_bpp, SDL_GetError() );
		SDL_Quit();
		exit(1);
	}
	
	// Final SDL settings
	SDL_ShowCursor(0);
	
	return;
}

void destroySDL(void)
{
	TTF_Quit(); // SDL_ttf
	IMG_Quit(); // SDL_image
	SDL_Quit(); // SDL
	
	return;
}

SDL_Surface *loadAlphaImage(const char *path)
{
	SDL_Surface *input, *output;
	
	input = IMG_Load(path);
	
	if (!input) {
		fprintf(stderr, "SDL_image: Unable to load image from '%s'.\n         : Error: %s\n", path, IMG_GetError() );
		exit(1);
	}
	
	output = SDL_DisplayFormatAlpha(input);
	
	SDL_FreeSurface(input);
	
	return output;
}

SDL_Surface *loadImage(const char *path)
{
	SDL_Surface *input, *output;
	
	input = IMG_Load(path);
	
	if (!input) {
		fprintf(stderr, "SDL_image: Unable to load image from '%s'.\n         : Error: %s\n", path, IMG_GetError() );
		exit(1);
	}
	
	output = SDL_DisplayFormat(input);
	
	SDL_FreeSurface(input);
	
	return output;
}

TTF_Font * loadFont(const char *path, const unsigned int size)
{
	TTF_Font *font;
	
	font = TTF_OpenFont(path, size);
	
	if (!font) {
		fprintf(stderr, "SDL_ttf: Unable to load font from '%s'.\n       : Error: %s\n", path, TTF_GetError() );
		exit(1);
	}
	
	return font;
}

void destroyImage(SDL_Surface *image)
{
	SDL_FreeSurface(image);
	
	return;
}

void destroyFont(TTF_Font *font)
{

	TTF_CloseFont(font);
	font = NULL;

	return;
}

int almostEqual(float a, float b)
{
	const float epsilon = 0.0005f;
	
	if (a > b) {
		if ((a - b) < epsilon) return 1;
	} else {
		if ((b - a) < epsilon) return 1;
	}

	return 0;
}

static inline Uint32 getPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	
    switch(bpp) {
		case 1:
			return *p;
			break;
			
		case 2:
			return *(Uint16 *)p;
			break;
			
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
			break;
			
		case 4:
			return *(Uint32 *)p;
			break;
			
		default:
			return 0;       /* shouldn't happen, but avoids warnings */
    }
}

static inline void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	
    switch(bpp) {
		case 1:
			*p = pixel;
			break;
			
		case 2:
			*(Uint16 *)p = pixel;
			break;
			
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			} else {
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;
			
		case 4:
			*(Uint32 *)p = pixel;
			break;
    }
}

SDL_Surface *scaleSurface(SDL_Surface *input, const SDL_Rect size)
{
	// TODO: Check inputs
	
	// Create output surface
	SDL_Surface *output;
    output = SDL_CreateRGBSurface(SDL_HWSURFACE, size.w, size.h,32,0,0,0,0);

	const float step_x = input->w / size.w;
	const float step_y = input->h / size.h;
	
	for (unsigned int x = 0; x < size.w; x++) for (unsigned int y = 0; y < size.h; y++) {
		putPixel(output, x, y, getPixel(input, lrint(x * step_x), lrint(y * step_y) ) );
	}
	
	SDL_FreeSurface(input);
	
	return output;
}

static inline Uint32 getBlendedPixel(SDL_Surface *input,
								 const unsigned int x0,
								 const unsigned int y0,
								 const unsigned int x1,
								 const unsigned int y1)
{
	// Simple average blending - doesn't acknowledge gamma!
	
	long unsigned int cu_r = 0;
	long unsigned int cu_g = 0;
	long unsigned int cu_b = 0;
	
	const unsigned int pixels = (x1+1 - x0) * (y1+1 - y0);
	
	Uint8 temp_r, temp_g, temp_b;
	Uint32 temp_pixel;
	
	for (unsigned int x = x0; x <= x1; x++) for (unsigned int y = y0; y <= y1; y++) {
		SDL_GetRGB(getPixel(input, x, y), input->format, &temp_r, &temp_g, &temp_b);
		cu_r += temp_r;
		cu_g += temp_g;
		cu_b += temp_b;
	}
	
	temp_r = (double) cu_r / pixels;
	temp_g = (double) cu_g / pixels;
	temp_b = (double) cu_b / pixels;
	
	temp_pixel = SDL_MapRGB(input->format, temp_r, temp_g, temp_b);
	
	return temp_pixel;
}

SDL_Surface *scaleSurfaceBlended(SDL_Surface *input, const SDL_Rect size)
{
	// TODO: Check inputs
	
	// Create output surface
	SDL_Surface *output;
    output = SDL_CreateRGBSurface(SDL_HWSURFACE, size.w, size.h,32,0,0,0,0);
	
	const float step_x = input->w / size.w;
	const float step_y = input->h / size.h;
	
	for (unsigned int x = 0; x < size.w; x++) for (unsigned int y = 0; y < size.h; y++) {
		putPixel(output, x, y, getBlendedPixel(input, lrint(x * step_x), lrint(y * step_y), lrint( (x+1) * step_x), lrint( (y+1) * step_y) ) );
	}
	
	SDL_FreeSurface(input);
	
	return output;
}