/*
 *  text.c
 *  maptiler
 *
 *  Created by James Mardell on 28/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "text.h"

void showStatus(const char *text, SDL_Surface *rendered_text, SDL_Rect *text_size, TTF_Font *font, const SDL_Color foreground, const SDL_Color background)
{
	const float status_location = 3/4;
	const unsigned int display_w = 1024;
	const unsigned int display_h = 768;
	
	rendered_text = TTF_RenderText_Shaded(font, text, foreground, background);
	
	if (!rendered_text) exit(1);
	
	if (TTF_SizeText(font, text, (int *)&text_size->w, (int *)&text_size->h)) exit(1);
	
	if (text_size->w > display_w) exit(1);
	
	text_size->x = (display_w - text_size->w) / 2.0f;
	text_size->y = (display_h * status_location) - (text_size->h / 2.0f);
	
	return;
}

static int renderText(const char *text, SDL_Surface *rendered_text, SDL_Rect *text_size, TTF_Font *font, const unsigned int foreground, const unsigned int background)
{
	const unsigned int display_w = 1024;
	const unsigned int display_h = 768;
	
	SDL_Color fg, bg;
	fg.r = foreground;
	fg.g = foreground;
	fg.b = foreground;
	bg.r = background;
	bg.g = background;
	bg.b = background;
	
	rendered_text = TTF_RenderText_Shaded(font, text, fg, bg);
	
	if (!rendered_text || 
		TTF_SizeText(font, text, (int *)&text_size->w, (int *)&text_size->h) ||
		(text_size->w + text_size->x > display_w) ||
		(text_size->h + text_size->y > display_h) ) {
		fprintf(stderr, "SDL_TTF: Error rendering text.\n       : Error: %s\n", TTF_GetError() );
		return 1;
	}
	
	return 0;
}

int showBillboard(const char *title, const char *text)
{
	// State machine variables
	enum billboard_states { open, wait, close };
	enum billboard_states billboard_state = open;
	
	// Constants
	//const unsigned int display_w = 1024;
	const unsigned int display_h = 768;
	const unsigned int background = 128;
	const unsigned int foreground = 255;
	const unsigned int title_size = 64;
	const unsigned int text_size = 16;
	const unsigned int fade_frames = 30;
	const unsigned int rendering_fps = 60;
	const unsigned int idle_fps = 6;
	const char *font_path = RESOURCES_FILE_FONT;
	
	// Rendering variables
	SDL_Event event_handler;
	int terminate = 0;
	int ticks_then = -1;
	int ticks_now;
	int fade_frame = 0;
	unsigned int target_fps = 60;
	int delay = 1000 / target_fps;
	unsigned int foreground_shade = background;
	unsigned int previous_shade = 65535;
	
	// SDL variables
	TTF_Font *title_font = loadFont(font_path, title_size);
	TTF_Font *text_font = loadFont(font_path, text_size);
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	SDL_Rect title_location = { text_size, (display_h - title_size) / 2, 0, 0 };
	SDL_Rect text_location = { text_size, title_location.y + title_size + text_size, 0, 0 };
	SDL_Surface *title_surface, *text_surface;
	SDL_Color foreground_colour, background_colour;
	
	background_colour.r = background;
	background_colour.g = background;
	background_colour.b = background;
	
	// Rendering loop
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
					billboard_state = close;
					break;
				case SDL_QUIT:
					// should probably clean-up...
					exit(0);
					break;
			}
		}
		
		// State machine control
		switch (billboard_state) {
			case open:
				target_fps = rendering_fps;
				if (fade_frame >= fade_frames) billboard_state = wait;
				foreground_shade += (foreground - background) / fade_frames;
				fade_frame++;
				break;
				
			case close:
				target_fps = rendering_fps;
				if (fade_frame <= 0) terminate = 1;
				foreground_shade -= (foreground - background) / fade_frames;
				fade_frame--;
				break;
				
			case wait:
				target_fps = idle_fps;
				break;
		}
		
		// Rendering
		if (foreground_shade != previous_shade) {
			foreground_colour.r = foreground_shade;
			foreground_colour.g = foreground_shade;
			foreground_colour.b = foreground_shade;
			
			title_surface = TTF_RenderText_Shaded(title_font, title, foreground_colour, background_colour);
			text_surface = TTF_RenderText_Shaded(text_font, text, foreground_colour, background_colour);
			
			previous_shade = foreground_shade;
		}
		
		// Blitting
		SDL_FillRect(display, NULL, SDL_MapRGB(display->format, background, background, background) );
		SDL_BlitSurface(title_surface, NULL, display, &title_location);
		SDL_BlitSurface(text_surface, NULL, display, &text_location);
		
		// Flip
		SDL_Flip(display);
		SDL_Delay(delay);
		
		// Constant FPS implementation
		if (ticks_then > 0) {
			ticks_now = SDL_GetTicks();
			delay += (1000 / target_fps - (ticks_now - ticks_then) );
			ticks_then = ticks_now;
			
			if (delay < 0) delay = 1000 / target_fps; // reset delay
		} else {
			ticks_then = SDL_GetTicks();
		}
		
	} // rendering loop
	
	// Clean-up
	TTF_CloseFont(text_font);
	TTF_CloseFont(title_font);
	SDL_FreeSurface(text_surface);
	SDL_FreeSurface(title_surface);
	
	return 0;
}

int showStatement(const unsigned int statement_id)
{
	// Frame control
	int ticks_then = -1;
	int ticks_now;
	unsigned int target_fps = 60;
	int delay = 1000 / target_fps;
	enum state_machine { fade_in, fade_off, fade_out };
	enum state_machine state = fade_in;
	
	// Variables
	Uint8 fade = 0;
	int shade = 0;
		
	// Rendering variables
	SDL_Event event_handler;
	int terminate = 0;
	
	// SDL variables
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	SDL_Surface *image;
	
	// Load image
	char image_filename[255];
	sprintf(image_filename, "../Resources/statement%u.png", statement_id);
	image = loadImage(image_filename);

	// Rendering loop
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
				case SDL_QUIT:
					state = fade_out;
					break;
			}
		}
		
		// Blitting
		SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 0, 0, 0) );
		SDL_SetAlpha(image, SDL_RLEACCEL | SDL_SRCALPHA, fade);
		SDL_BlitSurface(image, NULL, display, NULL);
		
		// State machine TODO
		switch (state) {
			case fade_in:
				shade += 8;
				if (shade >= 255) {
					state = fade_off;
					fade = 255;
				} else {
					fade = shade;
				}
				break;
				
			case fade_out:
				shade -= 8;
				if (shade <= 0) {
					fade = 0;
					terminate = 1;
				} else {
					fade = shade;
				}
				break;
				
			default:
				shade = 255;
				break;
		}
		
		// Flip
		SDL_Flip(display);
		SDL_Delay(delay);
		
		// Constant FPS implementation
		if (ticks_then > 0) {
			ticks_now = SDL_GetTicks();
			delay += (1000 / target_fps - (ticks_now - ticks_then) );
			ticks_then = ticks_now;
			
			if (delay < 0) delay = 1000 / target_fps; // reset delay
		} else {
			ticks_then = SDL_GetTicks();
		}
	} // rendering loop
	
	// Clean-up
	destroyImage(image);
	
	return 0;
}
