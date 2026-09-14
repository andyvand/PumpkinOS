#ifndef PIT_SMF_H
#define PIT_SMF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Minimal Standard MIDI File (format 0/1) parser and sequencer, used to feed
// SndPlaySmf data either to an external MIDI device or to a software synth.

typedef struct smf_t smf_t;

// Called for every channel message or SysEx message, in playback order.
// msg holds a complete MIDI message (status byte first). Return non-zero to
// abort playback.
typedef int (*smf_event_f)(uint8_t *msg, int len, void *data);

// Called to let "us" microseconds of playback time elapse before the next
// event. Return non-zero to abort playback.
typedef int (*smf_wait_f)(uint32_t us, void *data);

// Parses a SMF held in memory. The buffer is copied. Returns NULL on error.
smf_t *smf_load(const uint8_t *buf, uint32_t len);
void smf_free(smf_t *smf);

uint32_t smf_num_events(smf_t *smf);

// Total playback length in microseconds (following tempo changes).
uint64_t smf_duration(smf_t *smf);

// Runs through the file, calling wait() and event() in order.
// Returns 0 when the file finished, 1 when a callback aborted, -1 on error.
int smf_play(smf_t *smf, smf_event_f event, smf_wait_f wait, void *data);

#ifdef __cplusplus
}
#endif

#endif
