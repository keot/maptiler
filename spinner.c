/*
 *  spinner.c
 *  maptiler
 *
 *  Created by James Mardell on 21/10/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "spinner.h"
#include "sdl_helper.h"
#include "eyeserver.h"

static void blitSpinner(SDL_Surface *spinner)
{
	static int frame = 0;
	
	// Get the the current display surface
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	// Constants TODO: Replace with libconfig input
	const int spinner_frames = 24;
	const int spinner_size = 64;
	
	// Establish cropping and layout co-ordinates
	SDL_Rect spinner_crop, spinner_location;
	
	spinner_crop.w = spinner_size;
	spinner_crop.h = spinner_size;
	spinner_crop.x = 0;
	spinner_crop.y = frame * spinner_crop.h;
	
	spinner_location.w = display->w;
	spinner_location.h = display->h;
	spinner_location.x = (spinner_location.w - spinner_crop.w) / 2;
	spinner_location.y = (spinner_location.h - spinner_crop.h) / 2;
	
	// Clear
	SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 128, 128, 128) ); // clear
	
	// Blit
	SDL_BlitSurface(spinner, &spinner_crop, display, &spinner_location);
	
	// Flip
	SDL_Flip(display);
	
	// Update frame number
	frame++;
	frame %= spinner_frames; // stay within the image
	
	return;
}

static void blitProgress(SDL_Surface *spinner, SDL_Surface *progress, TTF_Font *progress_font, float *percentage)
{
	static int frame = 0;
	
	// Get the the current display surface
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	// Constants TODO: Replace with libconfig input
	const int spinner_frames = 24;
	const int spinner_size = 64;
	const int progress_frames = 100;
	const int progress_size = 32;
	
	// Establish cropping and layout co-ordinates
	SDL_Rect spinner_crop, spinner_location, progress_crop, progress_location;
	
	progress_crop.w = progress_size;
	progress_crop.h = progress_size;
	progress_crop.x = 0;
	progress_crop.y = lrint(*percentage * progress_frames) * progress_crop.h;
	
	progress_location.w = display->w;
	progress_location.h = display->h;
	progress_location.x = (progress_location.w - progress_crop.w) / 2;
	progress_location.y = (progress_location.h - progress_crop.h) / 2;;
	
	spinner_crop.w = spinner_size;
	spinner_crop.h = spinner_size;
	spinner_crop.x = 0;
	spinner_crop.y = frame * spinner_crop.h;
	
	spinner_location.w = display->w;
	spinner_location.h = display->h;
	spinner_location.x = (spinner_location.w - spinner_crop.w) / 2;
	spinner_location.y = (spinner_location.h - spinner_crop.h) / 2;
	
	// Determine text
	char progress_text[255];
	int progress_text_w, progress_text_h;
	sprintf(progress_text, "%ld%%", lrint(*percentage * 100) );
	
	if (TTF_SizeText(progress_font, progress_text, &progress_text_w, &progress_text_h) ) {
		// error in font rendering
	}
	
	SDL_Rect progress_text_location;
	progress_text_location.x = (display->w - progress_text_w) / 2;
	progress_text_location.y = ((display->h) / 2) + 64;
	progress_text_location.w = progress_text_w;
	progress_text_location.h = progress_text_h;
	
	SDL_Color foreground, background;
	foreground.r = 255;
	foreground.g = 255;
	foreground.b = 255;
	background.r = 128;
	background.g = 128;
	background.b = 128;
	
	
	SDL_Surface *progress_text_surface;
	progress_text_surface = TTF_RenderText_Shaded(progress_font, progress_text, foreground, background);
	
	// Clear
	SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 128, 128, 128) ); // clear
	
	// Blit
	SDL_BlitSurface(spinner, &spinner_crop, display, &spinner_location);
	SDL_BlitSurface(progress, &progress_crop, display, &progress_location);
	SDL_BlitSurface(progress_text_surface, NULL, display, &progress_text_location);
	
	// Flip
	SDL_Flip(display);
	
	// Update frame number
	frame++;
	frame %= spinner_frames; // stay within the image
	
	return;
}

void displayCountdownScreen(float *percentage)
{
	// Constants TODO: Replace with libconfig input
	const char *spinner_image_path = "../Resources/spinner.png";
	const char *progress_image_path = "../Resources/progress.png";
	const char *progress_font_path = "../Resources/Helvetica.ttf";
	const unsigned int progress_font_size = 11;
	const int fps = 60;

	// Load spinner resources
	SDL_Surface *spinner_image, *progress_image;
	spinner_image = loadImage(spinner_image_path);
	progress_image = loadImage(progress_image_path);
	
	// Setup font rendering
	TTF_Font *progress_font = loadFont(progress_font_path, progress_font_size);
	
	// Main display loop
	SDL_Event event_handler;
	int terminate = 0;
	int ticks_then = -1;
	int ticks_now;
	int frame = 0;
	int delay = 1000 / fps;
	
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			// noop required to purge events
		} // SDL_PollEvent
		
		// Draw spinner
		blitProgress(spinner_image, progress_image, progress_font, percentage);
		
		// Constant FPS implementation
		if (ticks_then > 0) {
			ticks_now = SDL_GetTicks();
			delay += (1000 / fps - (ticks_now - ticks_then) );
			ticks_then = ticks_now;
			
			if (delay < 0) delay = 1000 / fps; // reset delay
		} else {
			ticks_then = SDL_GetTicks();
		}
		
		frame++;
		
		if (almostEqual(*percentage, 1.0f) ) terminate = 1;
		
		SDL_Delay(delay);
		
	} // while
	
	// Destroy spinner resources
	destroyImage(spinner_image);
	
	return;
}

static int gazepointValid(const	int x, const int y, const int eye_found)
{
	const int display_w = 1024;
	const int display_h = 768;
	
	if ( (x > display_w) ||
		(y > display_h) ||
		(x < 0) || (y < 0) ||
		(!eye_found) )
		return 0;
	return 1;
}

void displayGazeTrace(eye_data *eye)
{
	const char *gazetrace_image_path = "../Resources/gazetrace.png";
	const unsigned int gazetrace_image_size = 32;
	const unsigned int frames = 60;
	SDL_Surface *gazetrace_image;
	SDL_Rect gazetrace_image_crop, gazetrace_image_location;
	gazetrace_image_crop.x = 0;
	gazetrace_image_crop.y = 0;
	gazetrace_image_crop.w = gazetrace_image_size;
	gazetrace_image_crop.h = gazetrace_image_size;
	gazetrace_image = loadAlphaImage(gazetrace_image_path);
	
	// Main display loop variables
	SDL_Event event_handler;
	int terminate = 0;
	const unsigned int delay = 4;
	unsigned long previous_field = 0;
	
	// Rendering variables
	typedef struct gaze_history_struct {
		int x;
		int y;
		int valid;
	} gaze_history_type;
	
	gaze_history_type gaze_history[frames];
	int gaze_history_offset = 0;
	
	// Initially avoid rendering anything
	for (unsigned int i = 0; i < frames; i++) {
		gaze_history[i].valid = 0;
	}
	
	// Get the the current display surface
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	
	// Latched eye_data rendering loop
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
			} // switch
		} // SDL_PollEvent
		
		if (eye->ulCameraFieldCount != previous_field) {
			gaze_history[gaze_history_offset].x = eye->iIGaze;
			gaze_history[gaze_history_offset].y = eye->iJGaze;
			gaze_history[gaze_history_offset].valid = gazepointValid(eye->iIGaze, eye->iJGaze, eye->bGazeVectorFound);
			
			SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 128, 128, 128) ); // clear
			
			// Draw all gazepoints
			for (unsigned int i = 0; i < frames; i++) {
				if (gaze_history[i].valid) {
					gazetrace_image_crop.y = ((i + gaze_history_offset) % frames) * gazetrace_image_size;
					gazetrace_image_location.x = gaze_history[i].x - (gazetrace_image_size / 2);
					gazetrace_image_location.y = gaze_history[i].y - (gazetrace_image_size / 2);
					gazetrace_image_location.w = gazetrace_image_size;
					gazetrace_image_location.h = gazetrace_image_size;
					
					SDL_BlitSurface(gazetrace_image, &gazetrace_image_crop, display, &gazetrace_image_location);
				}
			}
			
			gaze_history_offset = (gaze_history_offset + 1) % frames;
			previous_field = eye->ulCameraFieldCount;
			
			SDL_Flip(display);
			
		} else {
			SDL_Delay(delay);
		}
		
	} // while

	destroyImage(gazetrace_image);
	
	return;
}