/*
 *  sdl_helper.h
 *  maptiler
 *
 *  Created by James Mardell on 21/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#ifndef _INC_SDL
	#define _INC_SDL
	#include "SDL.h"
	#include "SDL_image/SDL_image.h"
	#include "SDL_ttf/SDL_ttf.h"
	#include "SDL_ffmpeg.h"
#endif // _INC_SDL

#ifndef _INC_LIBCONFIG
	#define _INC_LIBCONFIG
	#include "libconfig.h"
#endif // _INC_LIBCONFIG

#include "math.h"

void initialiseSDL(void);
void destroySDL(void);
SDL_Surface *loadAlphaImage(const char *);
SDL_Surface *loadImage(const char *);
TTF_Font * loadFont(const char *, const unsigned int);
void destroyImage(SDL_Surface *);
void destroyFont(TTF_Font *);
int almostEqual(float, float);
SDL_Surface *scaleSurface(SDL_Surface *, const SDL_Rect);
SDL_Surface *scaleSurfaceBlended(SDL_Surface *, const SDL_Rect);