/*
 *  video.c
 *  maptiler
 *
 *  Created by James Mardell on 12/12/2010.
 *  Copyright 2010 Imperial College London. All rights reserved.
 *
 */

#include "video.h"

static uint64_t videoSync(SDL_ffmpegFile *video_file)
{
	if (SDL_ffmpegValidVideo(video_file) ) {
		return SDL_GetTicks() % SDL_ffmpegDuration(video_file);
	} else {
		fprintf(stderr, "SDL_ffmpeg: Video Sync reports that the video is invalid.\n");
	}
	
	return 0;
}


int showVideo(void)
{
	fprintf(stderr, "Showing introductory video...\n");
	
	// Constants
	const char* video_path = RESOURCES_FILE_MOVIE;
	const unsigned int display_w = 1024;
	const unsigned int display_h = 768;
	
	// Variables
	SDL_ffmpegFile *video_file = NULL;
	SDL_ffmpegVideoFrame *video_frame = NULL;
	SDL_Rect video_location;
	int video_w, video_h;
	uint64_t video_time = 0;
	
	// Open video
	video_file = SDL_ffmpegOpen(video_path);
	if (!video_file) {
		fprintf(stderr, "SDL_ffmpeg: Error opening video '%s'.\n", video_path);
		//SDL_ffmpegPrintErrors(stderr);
		return 1;
	}
	
	// Setup initial video stream
	SDL_ffmpegSelectVideoStream(video_file, 0);
	SDL_ffmpegGetVideoSize(video_file, &video_w, &video_h);
	
	// Calculate video location
	if ((video_w > display_w) || (video_h > display_h) ) {
		fprintf(stderr, "SDL_ffmpeg: A video of size %dx%d is too large to display.\n", video_w, video_h);
		return 1;
	}
	video_location.w = video_w;
	video_location.h = video_h;
	video_location.x = (display_w - video_w) / 2;
	video_location.y = (display_h - video_h) / 2;
	
	// Open display
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	if (!display) {
		fprintf(stderr, "SDL: Could not grab video surface for video routine.\n   : Error: %s\n", SDL_GetError() );
		return 1;
	}
	
	video_frame = SDL_ffmpegCreateVideoFrame(video_file, SDL_YUY2_OVERLAY, display);
	video_frame->surface = SDL_CreateRGBSurface(0, video_w, video_h, 32, 0, 0, 0, 0);
	
	// Main display loop parameters
	SDL_Event event_handler;
	int terminate = 0;
	
	// Rendering Loop
	while (!terminate) {
		// Handle events
		while (SDL_PollEvent(&event_handler) ) {
			switch (event_handler.type) {
				case SDL_KEYDOWN:
				case SDL_QUIT:
					terminate = 1;
			} // switch
		} // SDL_PollEvent
		
		// Draw video frame
		if (video_frame) {
			if (!video_frame->ready) {
				SDL_ffmpegGetVideoFrame(video_file, video_frame);
			} else if (video_frame->pts <= videoSync(video_file)) {
				if (video_frame->overlay) {
					SDL_DisplayYUVOverlay(video_frame->overlay, &video_location);
				} else if (video_frame->surface) {
					SDL_FillRect(display, NULL, SDL_MapRGB(display->format, 128, 128, 128) );
					SDL_BlitSurface(video_frame->surface, NULL, display, &video_location);
					SDL_Flip(display);
				}
				
				video_frame->ready = 0;
			}
		}
		
		// Timekeeping
		if (SDL_GetTicks() - video_time < 5) SDL_Delay(5);
		video_time = SDL_GetTicks();
		
		// Termination
		if (video_time >= SDL_ffmpegDuration(video_file) ) terminate = 1;
	} // rendering loop
	
	SDL_ffmpegFreeVideoFrame(video_frame);
	SDL_ffmpegFree(video_file);
	
	return 0;
}