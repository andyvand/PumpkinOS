/*
 * sdl-audio.c
 * sdl2 audio interface
 *
 * (C) 2001 Laguna
 * (C) 2021 rofl0r
 * (C) 2025 Andy Vandijck
 *
 * Licensed under the GPLv2, or later.
 */

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "../../rc.h"
#include "../../pcm.h"


struct pcm pcm;

static int sound = 1;
static int samplerate = 44100;
static int stereo = 1;
static SDL_AudioDeviceID device = 0;
static SDL_AudioStream *dev_stream = NULL;
static int paused = 0;
static int threaded = 0;
static volatile int audio_done = 0;

rcvar_t pcm_exports[] =
{
	RCV_BOOL("sound", &sound, "enable sound"),
	RCV_BOOL("sound_threaded", &threaded, "use threaded sound"),
	RCV_INT("stereo", &stereo, "enable stereo"),
	RCV_INT("samplerate", &samplerate, "samplerate, recommended: 32768"),
	RCV_END
};

static void audio_callback(void *blah, SDL_AudioStream *stream, int add_len, int len)
{
	SDL_PutAudioStreamData(stream, pcm.buf, add_len);
	audio_done = 1;
}

void pcm_init()
{
	SDL_AudioSpec as = {0}, ob;

	if (!sound) return;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == false)
    {
        sound = 0;
        return;
    }

	as.freq = samplerate;
	as.format = SDL_AUDIO_U8;
	as.channels = 1 + stereo;
    dev_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &as, threaded ? audio_callback : NULL, NULL);

    if(dev_stream == NULL)
    {
        printf("WARNING: SDL could not open audio stream: %s\n", SDL_GetError());
        sound = 0;
        return;
    }

    device = SDL_GetAudioStreamDevice(dev_stream);
    
	if (!device) {
		sound = 0;
		return;
	}

    SDL_GetAudioStreamFormat(dev_stream, &as, &ob);

	pcm.hz = ob.freq;
	pcm.stereo = ob.channels - 1;
#ifdef _WIN32
	pcm.len = 128 * 1024;
#else
	pcm.len = 64 * 1024;
#endif
	pcm.buf = malloc(pcm.len);
	pcm.pos = 0;
	memset(pcm.buf, 0, pcm.len);
    SDL_ResumeAudioStreamDevice(dev_stream);
	SDL_ResumeAudioDevice(device);
}

int pcm_submit()
{
	int res, min;
	if (!sound || !pcm.buf || paused) {
		pcm.pos = 0;
		return 0;
	}
	if(threaded) {
		if(pcm.pos < pcm.len) return 1;
		while(!audio_done) SDL_Delay(4);
		audio_done = 0;
		pcm.pos = 0;
		return 1;
	}
	min = pcm.len * 2;
	res = (int)(SDL_PutAudioStreamData(dev_stream, pcm.buf, pcm.pos) == true);
	pcm.pos = 0;
	while (res == true && SDL_GetAudioStreamQueued(dev_stream) > min)
		SDL_Delay(1);
	return res;
}

void pcm_close()
{
	if (sound) SDL_CloseAudioDevice(device);
}

void pcm_pause(int dopause)
{
	paused = dopause;
    if (paused == false)
    {
        SDL_ResumeAudioStreamDevice(dev_stream);
        SDL_ResumeAudioDevice(device);
    } else {
        SDL_PauseAudioStreamDevice(dev_stream);
        SDL_PauseAudioDevice(device);
    }
}
