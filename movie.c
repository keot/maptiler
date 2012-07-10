//
//  movie.c
//  maptiler2
//
//  Created by James Mardell on 05/12/2011.
//  Copyright 2011 Imperial College London. All rights reserved.
//

#include "movie.h"

struct ctx
{
    SDL_Surface *surf;
    SDL_mutex *mutex;
};

static void *lock(void *data, void **p_pixels)
{
    struct ctx *ctx = data;
    
    SDL_LockMutex(ctx->mutex);
    SDL_LockSurface(ctx->surf);
    *p_pixels = ctx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
    struct ctx *ctx = data;
    
    SDL_UnlockSurface(ctx->surf);
    SDL_UnlockMutex(ctx->mutex);
    
    assert(id == NULL); /* picture identifier, not needed here */
}

static void view(void *data, void *id)
{
    /* VLC wants to display the video */
    (void) data;
    assert(id == NULL);
}

int showMovie(void)
{
    // VLC Structures
    libvlc_instance_t *libvlc_instance;
    libvlc_media_t *libvlc_media;
    libvlc_media_player_t *libvlc_media_player;
    char const *vlc_argv[] =
    {
        "--no-audio", /* skip any audio track */
        "--no-xlib", /* tell VLC to not use Xlib */
        "--no-video-title-show", /* Don't overlay title at beginning */
        "--ignore-config"
        //"--no-spu",
        //"-vvv",
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    struct ctx ctx;
    
    // SDL Structures
    SDL_Surface *display, *empty;
	display = SDL_GetVideoSurface();
    empty = SDL_CreateRGBSurface(SDL_HWSURFACE, 960, 540,
                                 32, 0, 0, 0, 0);
    SDL_Rect movie_location;
    
    
    ctx.surf = SDL_CreateRGBSurface(SDL_HWSURFACE, 960, 540,
                                    24, 0xff0000, 0x00ff00, 0x0000ff, 0); // rgb
    
    ctx.mutex = SDL_CreateMutex();
    
    // Miscellaneous Variables
    int done = 0;
    
    // Initialise VLC
    libvlc_instance = libvlc_new(vlc_argc, vlc_argv);
    
    libvlc_media = libvlc_media_new_path(libvlc_instance, "../Resources/introduction.m4v");
    libvlc_media_player = libvlc_media_player_new_from_media(libvlc_media);
    libvlc_media_release(libvlc_media);
    
    // Play the media
    libvlc_video_set_callbacks(libvlc_media_player, lock, unlock, view, &ctx);
    libvlc_video_set_format(libvlc_media_player, "RV24", 960, 540, 960*3); // YUYV
    libvlc_media_player_play(libvlc_media_player);
    
    movie_location.w = 960;
    movie_location.h = 540;
    movie_location.x = (1024 - movie_location.w) / 2;
    movie_location.y = (768 - movie_location.h) / 2;
    
    while (!done) {
        SDL_LockMutex(ctx.mutex);
        SDL_BlitSurface(ctx.surf, NULL, display, &movie_location);
        SDL_UnlockMutex(ctx.mutex);
        
        SDL_Flip(display);
        SDL_Delay(10);
        
        if (libvlc_media_player_get_state(libvlc_media_player) == 6) {
            done = 1;
        }
    }
    
    libvlc_media_player_release(libvlc_media_player);
    libvlc_release(libvlc_instance);
    SDL_DestroyMutex(ctx.mutex);
    SDL_FreeSurface(ctx.surf);
    SDL_FreeSurface(empty);
    
    return 0;
}
