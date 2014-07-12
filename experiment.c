/*
 *  experiment.c
 *  maptiler
 *
 *  Created by James Mardell on 13/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "experiment.h"
#include "sdl_helper.h"
#include "tiles.h"
#include "configuration.h"
#include "math.h"

int runExperiment(experiment *exp, const unsigned int run, int *tile)
{
	// Viewport variables
	const unsigned int viewport_w = 1024;
	const unsigned int viewport_h = 768;
	const float sequence_length = (58.0f + (2.0f / 11.0f)) * 1000.0f;
	unsigned int tiles;
	float cumulative_error;
	float tile_error;
	unsigned int field;
	unsigned int base_fields;
	unsigned int cur_fields;
		
	// Viewport tiles
	tiles = exp[run].seg * exp[run].seg * 15;
	
	// Viewport tile jitter
	cumulative_error = 0.0f;
	tile_error = fmod((60.0f * sequence_length) / tiles, 1.0f);
	field = 0; // beginning again
	base_fields = (unsigned int) lrint(floor( ((60.0f * sequence_length) / tiles) ) );
	cur_fields = base_fields;
	
	// Display variables
	SDL_Surface *display, *tile_image;
	display = SDL_GetVideoSurface();
	SDL_Rect viewport_size = { 0, 0, viewport_w, viewport_h };
	SDL_Event event_handler;
	int terminate = 0;
	
	fprintf(stderr, "Beginning experiment run %u: map %c, seg %u, %u tiles.\n", run, exp[run].map, exp[run].seg, tiles);
	
	// Check that the first and last tile exists
	if ((tileset[run][0] == NULL) || (tileset[run][tiles - 1] == NULL) ) {
		// Error loading tilesets. Abort!
		fprintf(stderr, "Fatal error retrieving tileset images. Aborting.\n");
		return 1;
	}
	
	// Tile display duration calculations
	Uint32 tile_update_ticks;
	Uint32 tile_duration_error_incrementor = 0;
	const Uint32 tile_duration_base_ticks = (Uint32) lrint(floor(sequence_length / tiles) );
	Uint32 tile_duration_ticks = tile_duration_base_ticks;
	const float tile_duration_error = (sequence_length / tiles) - tile_duration_base_ticks;
	float tile_duration_error_accumulator = tile_duration_error;
	
	*tile = -1; // start at the beginning
	
	displayGazeAttractor(3);
    tile_update_ticks = SDL_GetTicks(); // why was this uninitialised?

	// Display routine
	while (!terminate) {
		
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
				case SDL_QUIT:
					terminate = 1;
					break;
				default:
					break;
			} // switch event_handler
		} // while SDL_PollEvent
		
		// Calculations
        
        //fprintf(stderr, "SDL_GetTicks(): % 8d, tile_update_ticks: % 8d, tile_duration_tics: % 8d, sum: % 8d.\n", SDL_GetTicks(), tile_update_ticks, tile_duration_ticks, tile_update_ticks + tile_duration_ticks);
        
        
		if (SDL_GetTicks() >= tile_update_ticks + tile_duration_ticks) {
			// Reset tile
			*tile = *tile + 1;
			
			// Check we haven't gone beyond our bounds
			if (*tile >= tiles) {
				fprintf(stderr, "Finished stimuli. Attempted to display tile %u of %u.\n", *tile, tiles);
				*tile = -1; // reset
				return 0;
			}
			
			// Render tile
			tile_image = makeTile(tileset[run][*tile], exp[run].seg);
			SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 128, 128, 128) ); // clear with grey
			SDL_BlitSurface(tile_image, NULL, display, &viewport_size);
			
			SDL_Flip(display);
			tile_update_ticks = SDL_GetTicks();
			
			SDL_FreeSurface(tile_image);
			
			// Calculate next tile display duration
			tile_duration_error_accumulator += tile_duration_error;
			
			if (tile_duration_error_accumulator >= 1.0f) {
				tile_duration_error_incrementor = (Uint32) lrint(floor(tile_duration_error_accumulator));
				tile_duration_error_accumulator -= tile_duration_error_incrementor;
			} else {
				tile_duration_error_incrementor = 0;
			}
			
			tile_duration_ticks = tile_duration_base_ticks + tile_duration_error_incrementor;
			
			//fprintf(stderr, "Tile %03u: %03u ms (error %4.2f).\n", *tile, tile_duration_ticks, tile_duration_error_accumulator);
		}
		
	} // while !terminate
	
	return 0;
}

int displayGazeAttractor(const unsigned int duration)
{
	// Rendering variables
	const char *attractor_image_path = "../Resources/gazetrace.png";
	const unsigned int attractor_image_w = 32;
	const unsigned int attractor_image_h = 32;
	const unsigned int attractor_image_f = 60;
	SDL_Surface *attractor_image;
	SDL_Rect attractor_image_crop, attractor_image_location;
	attractor_image_crop.x = 0;
	attractor_image_crop.y = 0;
	attractor_image_crop.w = attractor_image_w;
	attractor_image_crop.h = attractor_image_h;
	attractor_image = loadAlphaImage(attractor_image_path);
	attractor_image_location.x = (1024 - attractor_image_w) / 2;
	attractor_image_location.y = (768 - attractor_image_h) / 2;
	attractor_image_location.w = attractor_image_w;
	attractor_image_location.h = attractor_image_h;
	const Uint8 grey = 128;
	SDL_Event event_handler;
	
	int terminate = 0;
	
	// Rate control
	Uint32 previous_frame = SDL_GetTicks();
	unsigned int cycles = 0;
	const Uint32 frame_duration = 1000 / attractor_image_f;
	
	// Configure environment
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	
	// Rendering loop
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
				case SDL_QUIT:
					terminate = 1;
					break;
				default:
					break;
			} // switch event_handler
		} // while SDL_PollEvent
		
		// Perform rendering
		if (SDL_GetTicks() >= previous_frame + frame_duration) {
			attractor_image_crop.y += attractor_image_h;
			attractor_image_crop.y %= attractor_image_h * attractor_image_f;
			if (attractor_image_crop.y == attractor_image_h * (attractor_image_f - 1) )
				cycles++;
			
			SDL_FillRect(display, NULL, SDL_MapRGB(display->format, grey, grey, grey) );
			SDL_BlitSurface(attractor_image, &attractor_image_crop, display, &attractor_image_location);
			SDL_Flip(display);
			previous_frame = SDL_GetTicks();
		}
		
		if (cycles >= duration)
			terminate = 1;
		
	} // while !terminate
	
	// Clean-up
	destroyImage(attractor_image);
	
	return 0;
}