/*
 *  spinner.h
 *  maptiler
 *
 *  Created by James Mardell on 21/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#ifndef _INC_SDL
	#define _INC_SDL
	#include "SDL/SDL.h"
	#include "SDL/SDL_image.h"
	#include "SDL/SDL_ttf.h"
#endif // _INC_SDL

#include "math.h"
#include "eyeserver.h"

void addMessage(char *);

void displayCountdownScreen(float *);

void displayGazeTrace(eye_data *);
