/*
 *  tiles.c
 *  maptiler
 *
 *  Created by James Mardell on 06/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "tiles.h"
#include "spinner.h"
#include "sdl_helper.h"

int fillTilesets(fill_tilesets_args *args)
{
	char map;
	unsigned int seg, tiles;
	
	unsigned int tile_count = 0;
	unsigned int tile_total = 0;
	double progress_total;
	
	for (int i = 0; i < args->runs; i++) tile_total += (args->experiments[i].seg * args->experiments[i].seg * 15);
	progress_total = tile_total;
	for (int run = 0; run < args->runs; run++) {
		
		// Fill parameters
		map = args->experiments[run].map;
		seg = args->experiments[run].seg;
		
		// Calculate parameters
		tiles = seg * seg * 15;
		
		// Allocate memory for tile images
		tileset[run] = malloc(tiles * sizeof(SDL_Surface*) );
	
		if (tileset[run] == NULL) {
			// malloc(...) error
			fprintf(stderr, "Error allocating tileset memory for run %d, map %c segmentation %d.\n", run, map, seg);
			exit(1);
		}
		
		// Load tiles
		char filename[128];
		for (unsigned int tile = 0; tile < tiles; tile++) {
			sprintf(filename, "../Resources/%c-%u-tile-%04u.png", map, seg, tile);
			
			tileset[run][tile] = loadImage(filename);
			
			if (!tileset[run][tile]) {
				fprintf(stderr, "SDL: IMG_Load: Error: %s\n", IMG_GetError() );
				*args->progress = 1.0f; // we're done!
				return 1;
			}
			
			tile_count++;
			*args->progress = tile_count / progress_total;
		}
		
		fprintf(stderr, "Loaded %u tiles for run %i (Map %c, Seg %u).\n", tiles, run, map, seg);
	} // for run
	
	return 0;
}

SDL_Surface *makeTile(SDL_Surface *image, const unsigned int seg)
{
	SDL_Surface *surface;
    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, 1024, 768,32,0,0,0,0);
	
	// Establish layout co-ordinates
	SDL_Rect location;
	
	location.w = surface->w;
	location.h = surface->h;
	
	switch (seg) {
		case 1:
			location.x = 0;
			location.y = 0;
			break;
		case 2:
			location.x = 256;
			location.y = 192;
			break;
		case 3:
			location.x = 341;
			location.y = 256;
			break;
		case 4:
			location.x = 384;
			location.y = 288;
			break;
		case 5:
			location.x = 410;
			location.y = 307;
			break;
		case 6:
			location.x = 426;
			location.y = 320;
			break;
		default:
			location.x = 0;
			location.y = 0;
			break;
	}
	
	// Clear
	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 128, 128, 128) ); // clear
	
	// Blit
	SDL_BlitSurface(image, NULL, surface, &location);
	
	return surface;
}

void blitTile(SDL_Surface *image, const unsigned int seg)
{
	// Get the the current display surface
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	SDL_Surface *tile = makeTile(image, seg);
	
	// Blit
	SDL_BlitSurface(tile, NULL, display, NULL);
	
	// Flip
	SDL_Flip(display);
	
	SDL_FreeSurface(image);
	
	return;
}

SDL_Surface *addCorners(SDL_Surface *image)
{
	const char* tl_path = "../Resources/corner_tl.png";
	const char* tr_path = "../Resources/corner_tr.png";
	const char* bl_path = "../Resources/corner_bl.png";
	const char* br_path = "../Resources/corner_br.png";
	
	const unsigned int size = 8;
	
	static SDL_Surface *tl, *tr, *bl, *br;
	
	// No need to reload if already in memory...
	if (!tl || !tr || !bl || !br) {
		tl = loadAlphaImage(tl_path);
		tr = loadAlphaImage(tr_path);
		bl = loadAlphaImage(bl_path);
		br = loadAlphaImage(br_path);
		
		SDL_SetAlpha(tl, SDL_SRCALPHA, 0);
		SDL_SetAlpha(tr, SDL_SRCALPHA, 0);
		SDL_SetAlpha(bl, SDL_SRCALPHA, 0);
		SDL_SetAlpha(br, SDL_SRCALPHA, 0);
	}
	
	SDL_Rect tl_loc = {0, 0, size, size};
	SDL_Rect tr_loc = {image->w - size, 0, size, size};
	SDL_Rect bl_loc = {0, image->h - size, size, size};
	SDL_Rect br_loc = {image->w - size, image->h - size, size, size};
	
	SDL_BlitSurface(tl, NULL, image, &tl_loc);
	SDL_BlitSurface(tr, NULL, image, &tr_loc);
	SDL_BlitSurface(bl, NULL, image, &bl_loc);
	SDL_BlitSurface(br, NULL, image, &br_loc);
	
	return image;
}

SDL_Surface *addOverlayLetter(SDL_Surface *image, const char *text)
{
	const char *font_path = "../Resources/Helvetica.ttf";
	const unsigned int font_size = 128;
	TTF_Font *overlay_font = loadFont(font_path, font_size); // TODO: check this succeeded!
	//TTF_SetFontOutline(overlay_font, 1);
	SDL_Rect text_loc;
	SDL_Color text_color = { 0xFF, 0xFF, 0xFF };
	int text_minx, text_maxx, text_miny, text_maxy, text_advance;
	TTF_GlyphMetrics(overlay_font, text[0], &text_minx, &text_maxx, &text_miny, &text_maxy, &text_advance);
	
	// Center the letter in the image
	text_loc.w = image->w;
	text_loc.h = image->h;
	text_loc.x = text_minx - 8;
	text_loc.y = text_miny;
	
	// Blitting
	SDL_Surface *overlay = TTF_RenderText_Blended(overlay_font, text, text_color);
	overlay = SDL_DisplayFormatAlpha(overlay);
	SDL_SetAlpha(overlay, SDL_SRCALPHA | SDL_RLEACCEL, 10);
	SDL_BlitSurface(overlay, &text_loc, image, NULL);
	
	SDL_FreeSurface(overlay);
	TTF_CloseFont(overlay_font);
	
	return image;
}

static int createPreviews(create_previews_args *args)
{
	unsigned int tile_count = 0;
	unsigned int tile_total = 0;
	double progress_total;
	char overlay_text[50];
	
	for (int i = 0; i < args->runs; i++) tile_total += (args->experiments[i].seg * args->experiments[i].seg * 15);
	progress_total = tile_total;
	
	for (unsigned int run = 0; run < args->runs; run++) {
		unsigned int seg = args->experiments[run].seg;
		unsigned int tiles = seg * seg * 15;
		
		previews[run] = malloc(tiles * sizeof(SDL_Surface*) );
		
		if (previews[run] == NULL) {
			// malloc(...) error
			fprintf(stderr, "Error allocating preview memory for run %d.\n", run);
			return 1;
		}
		
		sprintf(overlay_text, "%c", run + 65); // +65 for A..F
		
		for (unsigned int tile = 0; tile < tiles; tile++) {
			previews[run][tile] = addOverlayLetter(
												   addCorners(
															  scaleSurface(
																		   makeTile(tileset[run][tile], seg),
																		   args->preview_size
																		   )
															  ), // scaleSurfaceBlended takes too long!
												   overlay_text
												   );
			tile_count++;
			*args->progress = tile_count / progress_total;
		}
	}
	
	return 0;
}

int blitPreviews(experiment *experiments, unsigned int runs)
{
	// Viewport variables
	const unsigned int viewport_w = 340;
	const unsigned int viewport_h = 255;
	const float sequence_length = 58.0f + (2.0f / 11.0f);
	
	// Viewport data
	typedef struct viewport_data_t {
		SDL_Rect viewport;
		unsigned int tile;
		unsigned int tiles;
		unsigned int seg;
		float cumulative_error;
		float tile_error;
		unsigned int field;
		unsigned int base_fields;
		unsigned int cur_fields;
	} viewport_data;
	
	viewport_data *viewports;
	viewports = malloc(runs * sizeof(viewport_data) );
	
	if (!viewports) {
		fprintf(stderr, "Error allocating memory for viewport data.\n");
		return 1;
	}
	
	if (runs > 6) {
		fprintf(stderr, "Non-fatal: Cannot display more than six runs! Truncating output to six screens.\n");
		runs = 5; // starting from zero
	}
	
	for (int i = 0; i < runs; i++) {
		// Viewport dimensions
		viewports[i].viewport.w = viewport_w;
		viewports[i].viewport.h = viewport_h;
		
		switch (i) {
			case 0:
				viewports[i].viewport.x = 1;
				viewports[i].viewport.y = 1;
				break;
			case 1:
				viewports[i].viewport.x = 342;
				viewports[i].viewport.y = 1;
				break;
			case 2:
				viewports[i].viewport.x = 683;
				viewports[i].viewport.y = 1;
				break;
			case 3:
				viewports[i].viewport.x = 1;
				viewports[i].viewport.y = 512;
				break;
			case 4:
				viewports[i].viewport.x = 342;
				viewports[i].viewport.y = 512;
				break;
			case 5:
				viewports[i].viewport.x = 683;
				viewports[i].viewport.y = 512;
				break;
		}
		
		// Viewport tiles and scaling
		viewports[i].tile = 0; // start at the beginning
		viewports[i].tiles = experiments[i].seg * experiments[i].seg * 15;
		viewports[i].seg = experiments[i].seg;
		
		// Viewport tile jitter
		viewports[i].cumulative_error = 0.0f;
		viewports[i].tile_error = fmod((60.0f * sequence_length) / viewports[i].tiles, 1.0f);
		viewports[i].field = 0; // beginning again
		viewports[i].base_fields = lrint(floor( ((60.0f * sequence_length) / viewports[i].tiles) ) );
		viewports[i].cur_fields = viewports[i].base_fields;
	}
	
	// Display variables
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	const SDL_Rect preview_size = { 0, 0, viewport_w, viewport_h };
	const int target_fps = 60;
	SDL_Event event_handler;
	int terminate = 0;
	int ticks_then = -1;
	int ticks_now = 0;
	int field = 0;
	int delay = 1000 / target_fps;
	
	// Create previews
	SDL_Thread *create_previews;
	int create_previews_return;
	float progress = 0;
	create_previews_args create_previews_arg = { experiments, runs, preview_size, &progress };
	create_previews = SDL_CreateThread((int (*)(void *))createPreviews, &create_previews_arg);
	
	if (create_previews == NULL) {
		fprintf(stderr, "SDL: Unable to create preview tile creation thread: %s\n", SDL_GetError() );
		return 1;
	}
	
	displayCountdownScreen(&progress);
	
	SDL_WaitThread(create_previews, &create_previews_return);
	
	if (create_previews_return != 0) {
		fprintf(stderr, "SDL: Preview tile creation thread failed.\n");
		return 1;
	}
	
	// Questions display
	const char *question_1_image_filename = "../Resources/question1.png";
	const char *question_2_image_filename = "../Resources/question2.png";
	SDL_Surface *question_1_image, *question_2_image;
	question_1_image = loadImage(question_1_image_filename);
	question_2_image = loadImage(question_2_image_filename);
	SDL_Rect question_image_location;
	question_image_location.w = 1022;
	question_image_location.h = 254;
	question_image_location.x = 1;
	question_image_location.y = 257;
	int question_image_fade = 0;
	const Uint8 question_image_fade_increment = 8;
	enum question_state_machine {
		q1_fadein,
		q1_static,
		q1_fadeout,
		q2_fadein,
		q2_static,
		q2_fadeout
	} question_state = q1_fadein;
	int question_keypress = 0;
	
	
	// Display routine
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
					question_keypress = 1;
					break;
				case SDL_QUIT:
					terminate = 1;
					break;
				default:
					break;
			} // switch event_handler
		} // while SDL_PollEvent
		
		// State machine
		switch (question_state) {
			case q1_fadein:
				question_image_fade += question_image_fade_increment;
				if (question_image_fade >= 255) {
					question_image_fade = 255;
					question_state = q1_static;
				}
				break;
				
			case q1_static:
				if (question_keypress) {
					question_keypress = 0;
					question_state = q1_fadeout;
				}
				break;
			
			case q1_fadeout:
				question_image_fade -= question_image_fade_increment;
				if (question_image_fade <= 0) {
					question_image_fade = 0;
					question_state = q2_fadein;
				}
				break;
				
			case q2_fadein:
				question_image_fade += question_image_fade_increment;
				if (question_image_fade >= 255) {
					question_image_fade = 255;
					question_state = q2_static;
				}
				break;
				
			case q2_static:
				if (question_keypress) {
					question_keypress = 0;
					question_state = q2_fadeout;
				}
				break;
				
			case q2_fadeout:
				question_image_fade -= question_image_fade_increment;
				if (question_image_fade <= 0) {
					question_image_fade = 0;
					terminate = 1;
				}
				break;
				
			default:
				printf("Invalid state for blitPreviews.\n");
				break;
		} // switch question_state
		
		// Calculations
		for (int i = 0; i < runs; i++) {
			// Field updating
			viewports[i].field++;
			
			if (viewports[i].field == viewports[i].cur_fields) {
				// Reset tile
				viewports[i].field = 0;
				viewports[i].tile++;
				
				// Check we haven't gone beyond our bounds
				if (viewports[i].tile == viewports[i].tiles) {
					viewports[i].tile = 0;
				}
				
				// Calculate tile display duration
				viewports[i].cumulative_error += viewports[i].tile_error;
				if (viewports[i].cumulative_error >= 1.0f) {
					// Standard duration frame
					viewports[i].cur_fields = viewports[i].base_fields;
					viewports[i].cumulative_error -= 1.0f;
				} else {
					// Extended duration frame
					viewports[i].cur_fields = viewports[i].base_fields + 1;
				}
			}
		}
		
		// Rendering
		SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 0, 0, 0) ); // clear with black
		for (int i = 0; i < runs; i++) {
			SDL_BlitSurface(previews[i][viewports[i].tile], NULL, display, &viewports[i].viewport);
		}
		
		// Question display
		switch (question_state) {
			case q1_fadein:
			case q1_static:
			case q1_fadeout:
				SDL_SetAlpha(question_1_image, SDL_RLEACCEL | SDL_SRCALPHA, question_image_fade);
				SDL_BlitSurface(question_1_image, NULL, display, &question_image_location);
				break;
				
			case q2_fadein:
			case q2_static:
			case q2_fadeout:
				SDL_SetAlpha(question_2_image, SDL_RLEACCEL | SDL_SRCALPHA, question_image_fade);
				SDL_BlitSurface(question_2_image, NULL, display, &question_image_location);
				break;
		} // switch question_state
	
		// Flipping
		SDL_Flip(display);
	
		// Delay calculation
		if (ticks_then > 0) {
			ticks_now = SDL_GetTicks();
			delay += (1000 / target_fps - (ticks_now - ticks_then) ); // adjust delay
			ticks_then = ticks_now;
			
			if (delay < 0) delay = 1000 / target_fps; // reset delay
		} else {
			ticks_then = SDL_GetTicks();
		}
		
		field++;
		
		SDL_Delay(delay);
	} // while !terminate
	
	// Clean-up
	destroyImage(question_1_image);
	destroyImage(question_2_image);
	free(viewports);
	
	return 0;
}