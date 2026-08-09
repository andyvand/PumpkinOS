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
//     Thin wrapper around the PalmOS/PumpkinOS SndStream API. Kept
//     separate so that no Doom header has to coexist with PalmOS.h.
//

#ifndef SND_PUMPKIN_H
#define SND_PUMPKIN_H

#include <stdint.h>

// Called on the audio thread to produce nframes frames of interleaved
// stereo signed 16-bit samples. The callback must fill the entire buffer.

typedef void (*snd_pumpkin_fill_t)(int16_t *buffer, unsigned int nframes);

// Create and start the output stream. Returns 1 on success.
// Calling it again while the stream is running is a no-op returning 1.

int SndPumpkin_StartStream(unsigned int rate, snd_pumpkin_fill_t fill);

// Stop and delete the output stream.

void SndPumpkin_StopStream(void);

#endif
