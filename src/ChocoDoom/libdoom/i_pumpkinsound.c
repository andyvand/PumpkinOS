//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     System interface for sound (PumpkinOS backend).
//
//     A single SndStream (see snd_pumpkin.c) pulls interleaved stereo
//     S16 audio from FillAudio() on the audio thread. Sound effects
//     from DMX-format WAD lumps are mixed there, and OPL music (see
//     opl_pumpkin.c) is added on top of the result.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "doomtype.h"

#include "deh_str.h"
#include "i_sound.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "mutex.h"

#include "snd_pumpkin.h"
#include "opl_pumpkin.h"

#define NUM_CHANNELS 16

// Converted sound effect data, attached to sfxinfo_t.driver_data.

typedef struct
{
    uint32_t length;      // number of mono samples
    uint32_t rate;        // sample rate of the lump
    int16_t data[];
} sfxsample_t;

typedef struct
{
    sfxsample_t *sample;  // NULL if channel is inactive
    uint32_t pos;         // 16.16 fixed point position
    uint32_t step;        // 16.16 fixed point step per output frame
    int leftvol;          // 0..255
    int rightvol;         // 0..255
} snd_channel_t;

static boolean use_sfx_prefix;

static snd_channel_t channels[NUM_CHANNELS];
static mutex_t *sound_mutex = NULL;

static int mixer_started = 0;
static unsigned int mixer_freq;

// Mixing accumulator, grown on demand to the size of the largest
// buffer the audio thread has requested.

static int32_t *accum = NULL;
static unsigned int accum_frames = 0;

//
// Shared mixer.
//

static void FillAudio(int16_t *buffer, unsigned int nframes)
{
    unsigned int i, n;
    int c, val;
    snd_channel_t *chan;

    if (nframes > accum_frames)
    {
        int32_t *p = realloc(accum, nframes * 2 * sizeof(int32_t));

        if (p == NULL)
        {
            memset(buffer, 0, nframes * 2 * sizeof(int16_t));
            return;
        }

        accum = p;
        accum_frames = nframes;
    }

    memset(accum, 0, nframes * 2 * sizeof(int32_t));

    mutex_lock(sound_mutex);

    for (c = 0; c < NUM_CHANNELS; ++c)
    {
        chan = &channels[c];

        if (chan->sample == NULL)
        {
            continue;
        }

        for (i = 0; i < nframes; ++i)
        {
            n = chan->pos >> 16;

            if (n >= chan->sample->length)
            {
                chan->sample = NULL;
                break;
            }

            val = chan->sample->data[n];
            accum[i * 2] += (val * chan->leftvol) >> 8;
            accum[i * 2 + 1] += (val * chan->rightvol) >> 8;

            chan->pos += chan->step;
        }
    }

    mutex_unlock(sound_mutex);

    for (i = 0; i < nframes * 2; ++i)
    {
        val = accum[i];

        if (val > 32767)
        {
            val = 32767;
        }
        else if (val < -32768)
        {
            val = -32768;
        }

        buffer[i] = val;
    }

    // Add music on top.

    OPL_Pumpkin_Render(buffer, nframes);
}

// Create and start the output stream if it isn't running yet.
// Also called from opl_pumpkin.c, so that music works with -nosfx.

int I_Pumpkin_StartMixer(void)
{
    if (mixer_started)
    {
        return 1;
    }

    if (sound_mutex == NULL)
    {
        sound_mutex = mutex_create("doomsnd");
    }

    mixer_freq = snd_samplerate;

    if (!SndPumpkin_StartStream(mixer_freq, FillAudio))
    {
        return 0;
    }

    mixer_started = 1;

    return 1;
}

void I_Pumpkin_StopMixer(void)
{
    if (mixer_started)
    {
        SndPumpkin_StopStream();
        mixer_started = 0;
    }
}

//
// Sound effect loading.
//

// Retrieve the raw data lump index for a given SFX name.

static void GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len)
{
    // Linked sfx lumps? Get the lump number for the sound linked to.

    if (sfx->link != NULL)
    {
        sfx = sfx->link;
    }

    // Doom adds a DS* prefix to sound lumps; Heretic and Hexen don't
    // do this.

    if (use_sfx_prefix)
    {
        M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
    }
    else
    {
        M_StringCopy(buf, DEH_String(sfx->name), buf_len);
    }
}

// Load and convert a DMX format sound effect lump. The converted
// sample is cached in sfxinfo->driver_data. Returns true on success.

static boolean CacheSFX(sfxinfo_t *sfxinfo)
{
    int lumpnum;
    unsigned int lumplen;
    unsigned int samplerate;
    unsigned int length;
    unsigned int i;
    byte *data;
    sfxsample_t *sample;

    if (sfxinfo->driver_data != NULL)
    {
        return true;
    }

    lumpnum = sfxinfo->lumpnum;

    if (lumpnum < 0)
    {
        char namebuf[9];

        GetSfxLumpName(sfxinfo, namebuf, sizeof(namebuf));
        lumpnum = W_CheckNumForName(namebuf);

        if (lumpnum < 0)
        {
            return false;
        }

        sfxinfo->lumpnum = lumpnum;
    }

    data = W_CacheLumpNum(lumpnum, PU_STATIC);
    lumplen = W_LumpLength(lumpnum);

    // Check the header, and ensure this is a valid sound.

    if (lumplen < 8 || data[0] != 0x03 || data[1] != 0x00)
    {
        W_ReleaseLumpNum(lumpnum);
        return false;
    }

    // 16 bit sample rate field, 32 bit length field

    samplerate = (data[3] << 8) | data[2];
    length = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4];

    // If the header specifies that the length of the sound is greater
    // than the length of the lump itself, this is an invalid sound lump.
    // Also reject any sound shorter than the DMX padding.

    if (length > lumplen - 8 || length <= 48)
    {
        W_ReleaseLumpNum(lumpnum);
        return false;
    }

    // The DMX sound library seems to skip the first 16 and last 16
    // bytes of the lump - reason unknown.

    data += 16;
    length -= 32;

    // Convert the 8-bit unsigned mono data to signed 16-bit.

    sample = malloc(sizeof(sfxsample_t) + length * sizeof(int16_t));

    if (sample == NULL)
    {
        W_ReleaseLumpNum(lumpnum);
        return false;
    }

    sample->length = length;
    sample->rate = samplerate;

    for (i = 0; i < length; ++i)
    {
        sample->data[i] = ((int) data[i] - 128) << 8;
    }

    sfxinfo->driver_data = sample;

    // The WAD lump is no longer needed.

    W_ReleaseLumpNum(lumpnum);

    return true;
}

//
// The sound module.
//

static boolean I_Pumpkin_InitSound(boolean _use_sfx_prefix)
{
    use_sfx_prefix = _use_sfx_prefix;

    if (!I_Pumpkin_StartMixer())
    {
        fprintf(stderr, "I_Pumpkin_InitSound: Unable to start audio.\n");
        return false;
    }

    memset(channels, 0, sizeof(channels));

    return true;
}

static void I_Pumpkin_ShutdownSound(void)
{
    int i;

    mutex_lock(sound_mutex);

    for (i = 0; i < NUM_CHANNELS; ++i)
    {
        channels[i].sample = NULL;
    }

    mutex_unlock(sound_mutex);

    // The stream itself is stopped in I_ShutdownSound, after the
    // music module has also shut down.
}

static int I_Pumpkin_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];

    GetSfxLumpName(sfx, namebuf, sizeof(namebuf));

    return W_CheckNumForName(namebuf);
}

static void I_Pumpkin_UpdateSound(void)
{
    // Mixing happens on the audio thread; nothing to do here.
}

static void I_Pumpkin_UpdateSoundParams(int handle, int vol, int sep)
{
    int left, right;

    if (!mixer_started || handle < 0 || handle >= NUM_CHANNELS)
    {
        return;
    }

    left = ((254 - sep) * vol) / 127;
    right = (sep * vol) / 127;

    if (left < 0) left = 0;
    else if (left > 255) left = 255;
    if (right < 0) right = 0;
    else if (right > 255) right = 255;

    mutex_lock(sound_mutex);
    channels[handle].leftvol = left;
    channels[handle].rightvol = right;
    mutex_unlock(sound_mutex);
}

//
// Starting a sound means loading (or fetching the cached copy of) the
// sample and latching it onto the given channel for the mixer thread.
//

static int I_Pumpkin_StartSound(sfxinfo_t *sfxinfo, int channel,
                                int vol, int sep, int pitch)
{
    snd_channel_t *chan;
    sfxsample_t *sample;

    if (!mixer_started || channel < 0 || channel >= NUM_CHANNELS)
    {
        return -1;
    }

    // Linked sounds use the sample data of the sound they link to.

    if (sfxinfo->link != NULL)
    {
        sfxinfo = sfxinfo->link;
    }

    if (!CacheSFX(sfxinfo))
    {
        return -1;
    }

    sample = sfxinfo->driver_data;

    if (pitch <= 0)
    {
        pitch = NORM_PITCH;
    }

    mutex_lock(sound_mutex);

    chan = &channels[channel];
    chan->sample = NULL;
    chan->pos = 0;
    chan->step = (uint32_t)(((uint64_t) sample->rate * 65536 * pitch)
                            / ((uint64_t) NORM_PITCH * mixer_freq));
    chan->sample = sample;

    mutex_unlock(sound_mutex);

    I_Pumpkin_UpdateSoundParams(channel, vol, sep);

    return channel;
}

static void I_Pumpkin_StopSound(int handle)
{
    if (!mixer_started || handle < 0 || handle >= NUM_CHANNELS)
    {
        return;
    }

    mutex_lock(sound_mutex);
    channels[handle].sample = NULL;
    mutex_unlock(sound_mutex);
}

static boolean I_Pumpkin_SoundIsPlaying(int handle)
{
    boolean playing;

    if (!mixer_started || handle < 0 || handle >= NUM_CHANNELS)
    {
        return false;
    }

    mutex_lock(sound_mutex);
    playing = channels[handle].sample != NULL;
    mutex_unlock(sound_mutex);

    return playing;
}

static void I_Pumpkin_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    // Sounds are loaded and cached on first use.
}

static const snddevice_t sound_pumpkin_devices[] =
{
    SNDDEVICE_SB,
    SNDDEVICE_PAS,
    SNDDEVICE_GUS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

const sound_module_t sound_pumpkin_module =
{
    sound_pumpkin_devices,
    arrlen(sound_pumpkin_devices),
    I_Pumpkin_InitSound,
    I_Pumpkin_ShutdownSound,
    I_Pumpkin_GetSfxLumpNum,
    I_Pumpkin_UpdateSound,
    I_Pumpkin_UpdateSoundParams,
    I_Pumpkin_StartSound,
    I_Pumpkin_StopSound,
    I_Pumpkin_SoundIsPlaying,
    I_Pumpkin_PrecacheSounds,
};
