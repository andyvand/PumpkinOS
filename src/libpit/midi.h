#ifndef PIT_MIDI_H
#define PIT_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

// MIDI output provider. A platform plug-in (for example the Android AMidi
// provider) registers a midi_provider_t under MIDI_PROVIDER in the script
// engine. The audio provider's mixer_play implementation can then hand
// Standard MIDI File events to an external MIDI device instead of, or in
// addition to, rendering them itself.

#define MIDI_PROVIDER "midi_provider"

typedef struct {
  // Returns 1 when a MIDI output device is currently connected and open.
  int (*available)(void *data);

  // Sends one complete MIDI message (status byte followed by its data bytes,
  // or a complete SysEx message). Returns the number of bytes sent or -1.
  int (*send)(uint8_t *msg, int len, void *data);

  // Flushes any queued messages to the device. Returns 0 on success.
  int (*flush)(void *data);

  // Sends "all sound off" / "all notes off" and "reset all controllers" on
  // every channel. Called when playback is interrupted. Returns 0 on success.
  int (*reset)(void *data);

  // Receives one incoming MIDI message from the device, if any. Returns the
  // number of bytes stored in buf (0 when nothing is pending, -1 on error).
  int (*receive)(uint8_t *buf, int len, void *data);

  void *data;
} midi_provider_t;

#ifdef __cplusplus
}
#endif

#endif
