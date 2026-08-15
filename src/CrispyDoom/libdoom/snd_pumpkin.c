//
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
//     PalmOS/PumpkinOS SndStream output for the Doom audio mixer.
//     This file deliberately includes only PalmOS headers; the game
//     side of the mixer lives in i_pumpkinsound.c.
//

#include <PalmOS.h>

#include <stdint.h>

#include "snd_pumpkin.h"

static SndStreamRef stream = 0;
static snd_pumpkin_fill_t fill_func = NULL;

static Err StreamCallback(void *userdata, SndStreamRef channel,
                          void *buffer, UInt32 numberofframes)
{
    if (fill_func != NULL && numberofframes > 0)
    {
        fill_func((int16_t *) buffer, numberofframes);
    }

    return errNone;
}

int SndPumpkin_StartStream(unsigned int rate, snd_pumpkin_fill_t fill)
{
    if (stream != 0)
    {
        return 1;
    }

    fill_func = fill;

    if (SndStreamCreate(&stream, sndOutput, rate, sndInt16, sndStereo,
                        StreamCallback, NULL, 0, false) != errNone)
    {
        stream = 0;
        fill_func = NULL;
        return 0;
    }

    if (SndStreamStart(stream) != errNone)
    {
        SndStreamDelete(stream);
        stream = 0;
        fill_func = NULL;
        return 0;
    }

    return 1;
}

void SndPumpkin_StopStream(void)
{
    if (stream != 0)
    {
        SndStreamStop(stream);
        SndStreamDelete(stream);
        stream = 0;
    }

    fill_func = NULL;
}
