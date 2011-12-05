//
//  video.c
//  maptiler2
//
//  Created by James Mardell on 04/12/2011.
//  Copyright 2011 Imperial College London. All rights reserved.
//

#include "video.h"

int showVideo(void)
{
    // Open display
	SDL_Surface *display;
	display = SDL_GetVideoSurface();
	
	if (!display) {
		fprintf(stderr, "SDL: Could not grab video surface for video routine.\n   : Error: %s\n", SDL_GetError() );
		return 1;
	}
    
    av_register_all(); // initialise libavcodec
    
    
    AVFormatContext *video_file_pointer;
    AVCodecContext *video_codec_pointer;
    AVCodec *video_codec;
    
    // Open video file
    if (av_open_input_file(&video_file_pointer, INTRODUCTION_MOVIE_PATH, NULL, 0, NULL) != 0) {
        fprintf(stderr, "Error opening introduction movie file '%s'\n", INTRODUCTION_MOVIE_PATH);
        return -1;
    }
    
    // Investigate video file
    if (av_find_stream_info(video_file_pointer) < 0) {
        fprintf(stderr, "Unable to find video stream in movie file '%s'.\n", INTRODUCTION_MOVIE_PATH);
        return -1;
    }
    
    // Obtain first video stream
    int video_stream = -1;
    
    for (int i = 0; i < video_file_pointer->nb_streams; i++) {
        if (video_file_pointer->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = i;
            break;
        }
    }
    
    if (video_stream == -1) {
        fprintf(stderr, "Could not find a video stream in file '%s'.\n", INTRODUCTION_MOVIE_PATH);
        return -1;
    }
    
    video_codec_pointer = video_file_pointer->streams[video_stream]->codec;
    
    // Find relevant video codec for decoding
    video_codec = avcodec_find_decoder(video_codec_pointer->codec_id);
    
    if (video_codec == NULL) {
        fprintf(stderr, "The video format of '%s' is unsupported.\n", INTRODUCTION_MOVIE_PATH);
        return -1;
    }
    
    if (avcodec_open(video_codec_pointer, video_codec) < 0) {
        fprintf(stderr, "Unable to open the codec for the video file '%s'\n", INTRODUCTION_MOVIE_PATH);
        return -1;
    }
    
    // Variables for video display
    AVFrame *video_frame;
    int flag_frame_finished;
    AVPacket video_packet;
    SDL_Overlay *video_overlay;
    SDL_Rect video_coords;
    
    video_coords.x = 0;
    video_coords.y = 0;
    video_coords.w = video_codec_pointer->width;
    video_coords.h = video_codec_pointer->height;
    
    video_overlay = SDL_CreateYUVOverlay(video_codec_pointer->width, video_codec_pointer->height, SDL_YV12_OVERLAY, display);
    
    video_frame = avcodec_alloc_frame();
    if (video_frame == NULL) {
        fprintf(stderr, "Error allocating memory for the video.\n");
        return -1;
    }
    
    // Initialise buffers
    uint8_t *video_buffer;
    int bytes_received;
    
    bytes_received = avpicture_get_size(video_codec_pointer->pix_fmt, video_codec_pointer->width, video_codec_pointer->height);
    video_buffer = (uint8_t *)av_malloc(bytes_received * sizeof(uint8_t) );
    
    avpicture_fill((AVPicture *)video_frame, video_buffer, video_codec_pointer->pix_fmt, video_codec_pointer->width, video_codec_pointer->height);
    
    while (av_read_frame(video_file_pointer, &video_packet) >= 0) {
        // check for video stream
        if (video_packet.stream_index == video_stream) {
            // decode frame
            avcodec_decode_video2(video_codec_pointer, video_frame, &flag_frame_finished, &video_packet);
            
            // have we got a complete frame yet?
            if (flag_frame_finished) {
                SDL_LockYUVOverlay(video_overlay);
                
                AVPicture video_image;
                
                video_image.data[0] = video_overlay->pixels[0];
                video_image.data[1] = video_overlay->pixels[2];
                video_image.data[2] = video_overlay->pixels[1];
                
                video_image.linesize[0] = video_overlay->pitches[0];
                video_image.linesize[1] = video_overlay->pitches[2];
                video_image.linesize[2] = video_overlay->pitches[1];
                
                img_convert(&video_image, PIX_FMT_YUV420P, video_codec_pointer->pix_fmt, video_codec_pointer->width, video_codec_pointer->height);
                
                SDL_UnlockYUVOverlay(video_overlay);
                
                SDL_DisplayYUVOverlay(video_overlay, &video_coords);
            }
        }
        
        // free the packet
        av_free_packet(&video_packet);
    }
    
    return 0;
}