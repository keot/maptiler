/*
 *  tiles.h
 *  maptiler
 *
 *  Created by James Mardell on 06/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#ifndef _INC_SDL
	#define _INC_SDL
	#include "SDL.h"
	#include "SDL_image/SDL_image.h"
	#include "SDL_ttf/SDL_ttf.h"
#endif // _INC_SDL

#include "configuration.h"

// Crude but quick texture storage
SDL_Surface **tileset[6];
SDL_Surface **previews[6];

typedef struct fill_tilesets_argtype {
	experiment *experiments;
	unsigned int runs;
	float *progress;
} fill_tilesets_args;

typedef struct create_previews_argtype {
	experiment *experiments;
	unsigned int runs;
	SDL_Rect preview_size;
	float *progress;
} create_previews_args;

int fillTilesets(fill_tilesets_args *);
void blitTile(SDL_Surface *, const unsigned int);
SDL_Surface *makeTile(SDL_Surface *, const unsigned int);
int blitPreviews(experiment *, const unsigned int);