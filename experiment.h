/*
 *  experiment.h
 *  maptiler
 *
 *  Created by James Mardell on 13/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#ifndef _INC_SDL
#define _INC_SDL
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#endif // _INC_SDL

#include "configuration.h"

int runExperiment(experiment *, const unsigned int, int *);
int displayGazeAttractor(const unsigned int);
