#ifndef GNUBOY_PUMPKIN_GLUE_H
#define GNUBOY_PUMPKIN_GLUE_H

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include <stdint.h>

/* pump PalmOS events for at most us microseconds; returns 0 when the
   emulator must stop (quit, app stop, back to ROM chooser) */
int gnuboy_pump_events(uint32_t us);

/* called from the gnuboy core (rccmds.c, menu.c) instead of exit() */
void gnuboy_request_quit(void);

/* stereo frames waiting in the audio ring, -1 when audio is not running */
int gnuboy_audio_queued(void);

#endif
