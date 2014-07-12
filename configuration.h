/*
 *  configuration.h
 *  maptiler
 *
 *  Created by James Mardell on 08/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>

#ifndef _INC_LIBCONFIG
	#define _INC_LIBCONFIG
	#include "libconfig.h"
#endif // _INC_LIBCONFIG

#ifndef _INC_EXPSTRUCT
	#define _INC_EXPSTRUCT
	typedef struct experiment_type {
		char map;
		unsigned int seg;
	} experiment;
#endif // _INC_EXPSTRUCT

int parseExperimentMetadata(unsigned int *, int *, const char *);
int parseExperimentRun(const unsigned int, char *, int *, const char *);