/*
 *  main.c
 *  maptiler
 *
 *  Created by James Mardell on 18/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _INC_SDL
	#define _INC_SDL
	#include "SDL/SDL.h"
	#include "SDL/SDL_image.h"
	#include "SDL/SDL_ttf.h"
#endif // _INC_SDL

#ifndef _INC_LIBCONFIG
	#define _INC_LIBCONFIG
	#include "libconfig.h"
#endif // _INC_LIBCONFIG

#include "sdl_helper.h"
#include "spinner.h"
#include "configuration.h"
#include "tiles.h"
#include "text.h"
#include "eyeserver.h"
#include "experiment.h"
#include "strings.h"
#include "movie.h"

	
int main(int argc, char *argv[])
{
	// Parse argument
	if (argc != 1 + 1) {
		fprintf(stdout, "Usage: %s [experiment configuration]\n", argv[0]);
		return 0;
	}
    
    fprintf(stdout, "Using configuration '%s'.\n", argv[1]);
	
	
	
	// Initialse experiments
	experiment *experiments;
	unsigned int runs;
	int participant;
	
	parseExperimentMetadata(&runs, &participant, argv[1]);
	
	if ((runs <= 0) || (runs > 6) ) {
		fprintf(stderr, "Fatal error: the number of experiment runs cannot be %d, it must be between 1 and 6.\n", runs);
		return 1;
	}

	
	fprintf(stdout, "Found %d runs for participant %d.\n", runs, participant);
	
	// Allocate memory for experiment configuration
	experiments = malloc(runs * sizeof(experiment) );
	if (experiments == NULL) {
		// malloc(...) error
		fprintf(stderr, "Fatal error allocating enough memory for experiment configuration.\n");
		return 1;
	}
	
	// Parse experiment run configurations
	char temp_map;
	int temp_seg;
	for (int i = 0; i < runs; i++) {
		parseExperimentRun(i, &temp_map, &temp_seg, argv[1]);
		
		experiments[i].map = temp_map;
		experiments[i].seg = temp_seg;
		
		fprintf(stdout, "Experiment run %d: Map %c, segmentation degree %d.\n", i, experiments[i].map, experiments[i].seg);
	}
	
	// Initialise graphics
	initialiseSDL();
		
	showBillboard(ENTRANCE_TITLE, ENTRANCE_TEXT);
	
	showMovie();
	
	SDL_Thread *load_images;
	int load_images_return;
	float progress = 0;
	fill_tilesets_args load_images_args = { experiments, runs, &progress };
	load_images = SDL_CreateThread((int (*)(void *))fillTilesets, &load_images_args);

	if (load_images == NULL) {
		fprintf(stderr, "SDL: Unable to create image loading thread: %s\n", SDL_GetError() );
		return 1;
	}

	displayCountdownScreen(&progress);

	SDL_WaitThread(load_images, &load_images_return);

	if (load_images_return != 0) {
		fprintf(stderr, "SDL: Image loading thread failed.\n");
		return 1;
	}
	
	// Setup and spin-off server thread
	eye_server_args eye_server;
	const char *client_hostname = "igaze.ee.ic.ac.uk";
	const unsigned int port = 4444;
	int terminate = 0;
	int logfile_ready = 0;
	FILE logfile;
	int tile = -1;
	unsigned int spacebar = 0;
	eye_data eye;
		
	eye_server.client_hostname = malloc((strlen(client_hostname) + 1) * sizeof(char) );
	eye_server.port = port;
	eye_server.terminate = &terminate;
	eye_server.logfile_ready = &logfile_ready;
	eye_server.logfile = &logfile;
	eye_server.tile = &tile;
	eye_server.spacebar = &spacebar;
	eye_server.data = &eye;
	
	SDL_Thread *eye_server_thread;
	eye_server_thread = SDL_CreateThread((int (*)(void *))eyeServer, &eye_server);
	
	if (eye_server_thread == NULL) {
		fprintf(stderr, "SDL: Unable to launch eye-tracking server thread: %s.\n", SDL_GetError() );
		return 1;
	}
	
	showBillboard(CALIBRATION_TITLE, CALIBRATION_TEXT);
	
	// Logfile filename
	const char *logfile_prefix = "../Logfiles/rsvp-ms";
	const char *logfile_postfix = ".log";
	char logfile_path[255];
	
	// Run all experiments
	char experiment_title[255];
	int string_buffer_size;
	for (unsigned int i = 0; i < runs; i++) {
		sprintf(experiment_title, "Experiment run %u of %u.", i + 1, runs);
		
		string_buffer_size = snprintf(logfile_path, 255, "%s-%02d-%c-%u%s", logfile_prefix, participant, experiments[i].map, experiments[i].seg, logfile_postfix);
		if (string_buffer_size > 255) {
			fprintf(stderr, "Path for logfile larger than buffer. Execution aborted.\n");
			return 1;
		}
		
		showBillboard(READY_TITLE, experiment_title);
		
		/* Strangest bug ever. If you substitle the temp.log string for the logfile_path char* then tile++ breaks... */
		if (openLogfile(&eye_server.logfile, logfile_path) ) {
			fprintf(stderr, "Error opening logfile '%s' for writing.\n", logfile_path);
			terminate = 1;
			destroySDL();
			return 1;
		}
		
		logfile_ready = 1;
		
		runExperiment(experiments, i, &tile);
		   
		showStatement(1); // confidence
		showStatement(2); // challenge
		showStatement(3); // image size
		showStatement(4); // image rate
        showStatement(5); // fatigue
		
		logfile_ready = 0;
		
		closeLogfile(&eye_server.logfile);
	}
	
	showBillboard(QUESTION_TITLE, QUESTION_TEXT);
	
	blitPreviews(experiments, runs);
	
	showBillboard(EXIT_TITLE, EXIT_TEXT);
	
	// Close threads
	terminate = 1;
	
	//fprintf(stderr, "Continuing application exit...\n");

	//destroySDL();
	
    return 0;
}
