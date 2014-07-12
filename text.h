/*
 *  text.h
 *  maptiler
 *
 *  Created by James Mardell on 28/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#ifndef _INC_SDL
	#define _INC_SDL
	#include "SDL/SDL.h"
	#include "SDL/SDL_image.h"
	#include "SDL/SDL_ttf.h"
#endif // _INC_SDL

#include "sdl_helper.h"
#include "strings.h"
#include "constants.h"

void showStatus(const char *, SDL_Surface *, SDL_Rect *, TTF_Font *, const SDL_Color, const SDL_Color);
int showBillboard(const char *, const char *);
int showStatement(const unsigned int);
