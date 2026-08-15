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
//     OPL PumpkinOS driver: render hook for the shared audio mixer.
//

#ifndef OPL_PUMPKIN_H
#define OPL_PUMPKIN_H

#include <stdint.h>

// Mix nframes of OPL music into the given interleaved stereo signed
// 16-bit buffer (saturating add). Safe to call when the OPL driver
// is not initialized; it does nothing in that case.

void OPL_Pumpkin_Render(int16_t *buffer, unsigned int nframes);

#endif
