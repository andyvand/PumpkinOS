// AAudio audio provider for the PumpkinOS Android port.
//
// Streams (SndStreamCreate/SndStreamStart and the tone commands) are served by
// an "AUDIO" thread that pulls PCM from the application callback and writes
// it to an AAudio output stream with blocking writes, mirroring the ALSA and
// SDL providers. The media player path (play) writes synchronously.
//
// Standard MIDI Files handed over by SndPlaySmf (mixer_play) are sequenced by
// smf.c and either sent to an external MIDI device through the MIDI provider
// (AMidi, see amidi.c) when one is connected, or rendered by a small built-in
// software synthesizer straight into an AAudio stream so that alarm and
// system melodies remain audible on devices without MIDI hardware.

#include <pthread.h>
#include <math.h>
#include <string.h>

#include <aaudio/AAudio.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "media.h"
#include "audio.h"
#include "midi.h"
#include "ptr.h"
#include "debug.h"
#include "smf.h"
#include "pitapp.h"

#define TAG_AUDIO  "audio"
#define AA "AAudio"

#define WRITE_TIMEOUT_NS  (500LL * 1000 * 1000)

typedef struct {
  char *tag;
  int pcm, rate, channels;
  int (*getaudio)(void *buffer, int len, void *data);
  void *data;
  // synchronous play() path
  AAudioStream *stream;
  int16_t *pcm16;
  int pcm16_frames;
} aaudio_audio_t;

static audio_provider_t audio_provider;
static midi_provider_t *midi_provider = NULL;

static pthread_mutex_t mixer_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile int mixer_stop_flag = 0;

static int sample_size(int pcm) {
  switch (pcm) {
    case PCM_U8:  return 1;
    case PCM_S16: return 2;
    case PCM_S32: return 4;
  }
  return 0;
}

// AAudio has no 8-bit format and 32-bit integer only since API 31, so every
// source format is converted to interleaved signed 16-bit.
static void convert_to_i16(int pcm, uint8_t *src, int samples, int16_t *dst) {
  int i;

  switch (pcm) {
    case PCM_U8:
      for (i = 0; i < samples; i++) {
        dst[i] = (int16_t)(((int)src[i] - 128) << 8);
      }
      break;
    case PCM_S16:
      sys_memcpy(dst, src, samples * 2);
      break;
    case PCM_S32: {
      int32_t *s = (int32_t *)src;
      for (i = 0; i < samples; i++) {
        dst[i] = (int16_t)(s[i] >> 16);
      }
      break;
    }
  }
}

static AAudioStream *aaudio_open(int rate, int channels) {
  AAudioStreamBuilder *builder = NULL;
  AAudioStream *stream = NULL;
  aaudio_result_t r;

  if ((r = AAudio_createStreamBuilder(&builder)) != AAUDIO_OK) {
    debug(DEBUG_ERROR, AA, "AAudio_createStreamBuilder failed: %s", AAudio_convertResultToText(r));
    return NULL;
  }

  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_NONE);
  AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
  AAudioStreamBuilder_setChannelCount(builder, channels);
  AAudioStreamBuilder_setSampleRate(builder, rate);

  if ((r = AAudioStreamBuilder_openStream(builder, &stream)) != AAUDIO_OK) {
    debug(DEBUG_ERROR, AA, "AAudioStreamBuilder_openStream(%d,%d) failed: %s", rate, channels, AAudio_convertResultToText(r));
    AAudioStreamBuilder_delete(builder);
    return NULL;
  }
  AAudioStreamBuilder_delete(builder);

  debug(DEBUG_INFO, AA, "opened stream rate=%d channels=%d format=%d burst=%d buffer=%d",
    AAudioStream_getSampleRate(stream), AAudioStream_getChannelCount(stream), AAudioStream_getFormat(stream),
    AAudioStream_getFramesPerBurst(stream), AAudioStream_getBufferCapacityInFrames(stream));

  if (AAudioStream_getSampleRate(stream) != rate || AAudioStream_getChannelCount(stream) != channels ||
      AAudioStream_getFormat(stream) != AAUDIO_FORMAT_PCM_I16) {
    debug(DEBUG_ERROR, AA, "stream format does not match request (%d,%d)", rate, channels);
    AAudioStream_close(stream);
    return NULL;
  }

  if ((r = AAudioStream_requestStart(stream)) != AAUDIO_OK) {
    debug(DEBUG_ERROR, AA, "AAudioStream_requestStart failed: %s", AAudio_convertResultToText(r));
    AAudioStream_close(stream);
    return NULL;
  }

  return stream;
}

// Lets the data that is still queued in the stream play out before closing.
static void aaudio_close(AAudioStream *stream, int drain) {
  int rate, frames;

  if (stream) {
    if (drain) {
      rate = AAudioStream_getSampleRate(stream);
      frames = AAudioStream_getBufferSizeInFrames(stream);
      if (rate > 0 && frames > 0) {
        sys_usleep((uint32_t)(((uint64_t)frames * 1000000ULL) / (uint64_t)rate));
      }
    }
    AAudioStream_requestStop(stream);
    AAudioStream_close(stream);
  }
}

// Blocking write of "frames" interleaved 16-bit frames.
// Returns 0 on success, -1 on a fatal stream error.
static int aaudio_write(AAudioStream *stream, int16_t *buf, int frames, int channels) {
  aaudio_result_t r;
  int timeouts = 0;

  while (frames > 0) {
    r = AAudioStream_write(stream, buf, frames, WRITE_TIMEOUT_NS);
    if (r < 0) {
      debug(DEBUG_ERROR, AA, "AAudioStream_write failed: %s", AAudio_convertResultToText(r));
      return -1;
    }
    if (r == 0) {
      if (++timeouts == 4) {
        debug(DEBUG_ERROR, AA, "AAudioStream_write timed out");
        return -1;
      }
      continue;
    }
    timeouts = 0;
    buf += r * channels;
    frames -= r;
  }

  return 0;
}

// --- streaming (SndStream) path -------------------------------------------

static void audio_destructor(void *p) {
  aaudio_audio_t *audio = (aaudio_audio_t *)p;

  if (audio) {
    aaudio_close(audio->stream, 1);
    if (audio->pcm16) sys_free(audio->pcm16);
    sys_free(audio);
  }
}

static audio_t libaaudio_audio_create(int pcm, int channels, int rate, void *data) {
  aaudio_audio_t *audio;
  int ptr = -1;

  debug(DEBUG_INFO, AA, "audio_create(%d,%d,%d)", pcm, channels, rate);

  if (sample_size(pcm) > 0 && (channels == 1 || channels == 2) && rate > 0) {
    if ((audio = sys_calloc(1, sizeof(aaudio_audio_t))) != NULL) {
      audio->tag = TAG_AUDIO;
      audio->pcm = pcm;
      audio->rate = rate;
      audio->channels = channels;
      if ((ptr = ptr_new(audio, audio_destructor)) == -1) {
        sys_free(audio);
      }
    }
  } else {
    debug(DEBUG_ERROR, AA, "audio_create(%d,%d,%d): invalid arguments", pcm, channels, rate);
  }

  return ptr;
}

static int libaaudio_audio_start(int handle, audio_t _audio, int (*getaudio)(void *buffer, int len, void *data), void *data) {
  aaudio_audio_t *audio;
  uint32_t ptr;
  int r = -1;

  if (handle && (audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    audio->getaudio = getaudio;
    audio->data = data;
    ptr_unlock(_audio, TAG_AUDIO);
    ptr = _audio;
    r = thread_client_write(handle, (uint8_t *)&ptr, sizeof(uint32_t)) == sizeof(uint32_t) ? 0 : -1;
  }

  return r;
}

// Synchronous playback used by the media player (media.c).
static int libaaudio_audio_play(audio_t _audio, uint8_t *raw, int len) {
  aaudio_audio_t *audio;
  int frame_size, frames, r = -1;

  if ((audio = ptr_lock(_audio, TAG_AUDIO)) != NULL) {
    frame_size = sample_size(audio->pcm) * audio->channels;
    frames = frame_size ? len / frame_size : 0;

    if (frames > 0) {
      if (audio->stream == NULL) {
        audio->stream = aaudio_open(audio->rate, audio->channels);
      }
      if (audio->stream) {
        if (audio->pcm16_frames < frames) {
          if (audio->pcm16) sys_free(audio->pcm16);
          audio->pcm16 = sys_calloc(frames * audio->channels, sizeof(int16_t));
          audio->pcm16_frames = audio->pcm16 ? frames : 0;
        }
        if (audio->pcm16) {
          convert_to_i16(audio->pcm, raw, frames * audio->channels, audio->pcm16);
          if (aaudio_write(audio->stream, audio->pcm16, frames, audio->channels) == 0) {
            r = 0;
          } else {
            // the device went away (headset unplugged etc); reopen on the next call
            aaudio_close(audio->stream, 0);
            audio->stream = NULL;
          }
        }
      }
    } else {
      r = 0;
    }
    ptr_unlock(_audio, TAG_AUDIO);
  }

  return r;
}

static int libaaudio_audio_destroy(audio_t audio) {
  return ptr_free(audio, TAG_AUDIO);
}

static int audio_action(void *arg) {
  aaudio_audio_t *audio;
  AAudioStream *stream = NULL;
  unsigned char *msg;
  unsigned int msglen;
  uint32_t ptr;
  uint8_t *buffer;
  int16_t *pcm16;
  int stream_rate = 0, stream_channels = 0;
  int frame_size, frames, chunk, len, n, r;

  debug(DEBUG_INFO, AA, "audio thread starting");

  for (; !thread_must_end();) {
    if ((r = thread_server_read_timeout(2000, &msg, &msglen)) == -1) {
      break;
    }

    if (r == 1 && msg) {
      if (msglen == sizeof(uint32_t)) {
        ptr = *((uint32_t *)msg);
        debug(DEBUG_TRACE, AA, "received ptr %d", ptr);

        if ((audio = ptr_lock(ptr, TAG_AUDIO)) != NULL) {
          debug(DEBUG_TRACE, AA, "audio pcm=%d channels=%d rate=%d", audio->pcm, audio->channels, audio->rate);
          frame_size = sample_size(audio->pcm) * audio->channels;

          if (frame_size > 0) {
            // keep the stream open between sources of the same format: a tone
            // sequence would otherwise pay the stream open latency per note
            if (stream && (stream_rate != audio->rate || stream_channels != audio->channels)) {
              aaudio_close(stream, 1);
              stream = NULL;
            }
            if (stream == NULL) {
              if ((stream = aaudio_open(audio->rate, audio->channels)) != NULL) {
                stream_rate = audio->rate;
                stream_channels = audio->channels;
              }
            }
          }

          if (stream) {
            frames = audio->rate / 20; // 50 ms per callback
            if (frames < 64) frames = 64;
            chunk = frames * frame_size;
            buffer = sys_calloc(1, chunk);
            pcm16 = sys_calloc(frames * audio->channels, sizeof(int16_t));

            if (buffer && pcm16) {
              for (; !thread_must_end();) {
                len = audio->getaudio(buffer, chunk, audio->data);
                debug(DEBUG_TRACE, AA, "get audio len=%d bytes", len);
                if (len <= 0) break;
                n = len / frame_size;
                convert_to_i16(audio->pcm, buffer, n * audio->channels, pcm16);
                if (aaudio_write(stream, pcm16, n, audio->channels) != 0) {
                  aaudio_close(stream, 0);
                  stream = NULL;
                  break;
                }
                if (len != chunk) break;
              }
            }

            if (buffer) sys_free(buffer);
            if (pcm16) sys_free(pcm16);
          }
          ptr_unlock(ptr, TAG_AUDIO);
        }
        debug(DEBUG_TRACE, AA, "handled ptr %d", ptr);
        ptr_free(ptr, TAG_AUDIO);
      } else {
        debug(DEBUG_ERROR, AA, "invalid message size %d", msglen);
      }
      sys_free(msg);
    }
  }

  if (stream) {
    debug(DEBUG_INFO, AA, "close stream");
    aaudio_close(stream, 1);
  }

  debug(DEBUG_INFO, AA, "audio thread exiting");

  return 0;
}

static int libaaudio_audio_init(int pcm, int channels, int rate) {
  return thread_begin("AUDIO", audio_action, NULL);
}

static int libaaudio_audio_finish(int handle) {
  return thread_end("AUDIO", handle);
}

// --- MIDI (SndPlaySmf) path -----------------------------------------------

static int mixer_should_stop(void) {
  return mixer_stop_flag || thread_must_end();
}

// External MIDI device through the MIDI provider.

typedef struct {
  int volume; // 0..128
} midi_play_t;

static int midi_event(uint8_t *msg, int len, void *data) {
  midi_play_t *mp = (midi_play_t *)data;
  uint8_t scaled[3];

  if (len == 3 && (msg[0] & 0xF0) == 0x90 && msg[2] > 0 && mp->volume < 128) {
    // apply the requested amplitude to note-on velocities
    scaled[0] = msg[0];
    scaled[1] = msg[1];
    scaled[2] = (uint8_t)((msg[2] * mp->volume) / 128);
    if (scaled[2] == 0) scaled[2] = 1;
    msg = scaled;
  }

  midi_provider->send(msg, len, midi_provider->data);
  return 0;
}

static int midi_wait(uint32_t us, void *data) {
  uint32_t slice;

  if (midi_provider->flush) midi_provider->flush(midi_provider->data);

  while (us > 0) {
    if (mixer_should_stop()) return 1;
    slice = us > 5000 ? 5000 : us;
    sys_usleep(slice);
    us -= slice;
  }

  return 0;
}

static int play_midi(smf_t *smf, int volume) {
  midi_play_t mp;
  int r;

  mp.volume = volume;
  debug(DEBUG_INFO, AA, "playing %u MIDI events on external device", smf_num_events(smf));
  r = smf_play(smf, midi_event, midi_wait, &mp);
  if (midi_provider->reset) midi_provider->reset(midi_provider->data);
  if (midi_provider->flush) midi_provider->flush(midi_provider->data);

  return r < 0 ? -1 : 0;
}

// Built-in software synthesizer (fallback when no MIDI device is connected).

#define SYNTH_RATE     22050
#define SYNTH_VOICES   32
#define SYNTH_BLOCK    128    // frames per render block (~5.8 ms)
#define SYNTH_TAIL_US  400000 // release tail rendered after the last event

enum { ENV_OFF = 0, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE };

typedef struct {
  int state;
  int channel, note, velocity;
  float phase, inc;
  float env;
  uint32_t age;
} synth_voice_t;

typedef struct {
  AAudioStream *stream;
  synth_voice_t voices[SYNTH_VOICES];
  float chan_volume[16];
  float chan_expression[16];
  float chan_bend[16];   // pitch bend as a frequency multiplier
  float master;
  int16_t block[SYNTH_BLOCK];
  double pending_frames;
  uint32_t noise;
  uint32_t clock;
  int failed;
} synth_t;

static float note_frequency(int note) {
  return 440.0f * powf(2.0f, (float)(note - 69) / 12.0f);
}

static void synth_voice_retune(synth_t *s, synth_voice_t *v) {
  v->inc = (2.0f * (float)M_PI * note_frequency(v->note) * s->chan_bend[v->channel]) / (float)SYNTH_RATE;
}

static void synth_note_on(synth_t *s, int channel, int note, int velocity) {
  synth_voice_t *v, *oldest = NULL;
  int i;

  // retrigger a voice already playing this note, otherwise take a free one,
  // otherwise steal the oldest
  for (i = 0; i < SYNTH_VOICES; i++) {
    v = &s->voices[i];
    if (v->state != ENV_OFF && v->channel == channel && v->note == note) {
      oldest = v;
      break;
    }
    if (v->state == ENV_OFF) {
      if (oldest == NULL || oldest->state != ENV_OFF) oldest = v;
    } else if (oldest == NULL || (oldest->state != ENV_OFF && v->age < oldest->age)) {
      oldest = v;
    }
  }

  v = oldest;
  v->state = ENV_ATTACK;
  v->channel = channel;
  v->note = note;
  v->velocity = velocity;
  v->phase = 0.0f;
  v->env = 0.0f;
  v->age = s->clock;
  synth_voice_retune(s, v);
}

static void synth_note_off(synth_t *s, int channel, int note) {
  int i;

  for (i = 0; i < SYNTH_VOICES; i++) {
    if (s->voices[i].state != ENV_OFF && s->voices[i].state != ENV_RELEASE &&
        s->voices[i].channel == channel && s->voices[i].note == note) {
      s->voices[i].state = ENV_RELEASE;
    }
  }
}

static void synth_all_off(synth_t *s, int channel, int hard) {
  int i;

  for (i = 0; i < SYNTH_VOICES; i++) {
    if (s->voices[i].state != ENV_OFF && (channel < 0 || s->voices[i].channel == channel)) {
      s->voices[i].state = hard ? ENV_OFF : ENV_RELEASE;
    }
  }
}

static int synth_event(uint8_t *msg, int len, void *data) {
  synth_t *s = (synth_t *)data;
  int channel = msg[0] & 0x0F;

  switch (msg[0] & 0xF0) {
    case 0x90:
      if (msg[2] > 0) synth_note_on(s, channel, msg[1], msg[2]);
      else synth_note_off(s, channel, msg[1]);
      break;
    case 0x80:
      synth_note_off(s, channel, msg[1]);
      break;
    case 0xB0:
      switch (msg[1]) {
        case 7:   s->chan_volume[channel] = (float)msg[2] / 127.0f; break;
        case 11:  s->chan_expression[channel] = (float)msg[2] / 127.0f; break;
        case 120: synth_all_off(s, channel, 1); break;
        case 121:
          s->chan_expression[channel] = 1.0f;
          s->chan_bend[channel] = 1.0f;
          break;
        case 123: synth_all_off(s, channel, 0); break;
      }
      break;
    case 0xE0: {
      // 14-bit bend, +/- 2 semitones
      int bend = ((msg[2] << 7) | msg[1]) - 8192;
      int i;
      s->chan_bend[channel] = powf(2.0f, ((float)bend / 8192.0f) * 2.0f / 12.0f);
      for (i = 0; i < SYNTH_VOICES; i++) {
        if (s->voices[i].state != ENV_OFF && s->voices[i].channel == channel) {
          synth_voice_retune(s, &s->voices[i]);
        }
      }
      break;
    }
    default:
      break;
  }

  return 0;
}

static int synth_active(synth_t *s) {
  int i;

  for (i = 0; i < SYNTH_VOICES; i++) {
    if (s->voices[i].state != ENV_OFF) return 1;
  }

  return 0;
}

static void synth_render_block(synth_t *s, int frames) {
  const float attack  = 1.0f / (0.005f * SYNTH_RATE);
  const float decay   = 1.0f / (0.400f * SYNTH_RATE);
  const float release = 1.0f / (0.150f * SYNTH_RATE);
  const float drum_decay = 1.0f / (0.120f * SYNTH_RATE);
  const float sustain = 0.55f;
  synth_voice_t *v;
  float mix, sample, gain;
  int i, j;

  for (i = 0; i < frames; i++) {
    mix = 0.0f;

    for (j = 0; j < SYNTH_VOICES; j++) {
      v = &s->voices[j];
      if (v->state == ENV_OFF) continue;

      if (v->channel == 9) {
        // percussion: decaying noise burst
        if (v->state == ENV_ATTACK) { v->env = 1.0f; v->state = ENV_DECAY; }
        v->env -= drum_decay;
        if (v->env <= 0.0f) { v->state = ENV_OFF; continue; }
        s->noise = s->noise * 1664525u + 1013904223u;
        sample = ((float)(int32_t)s->noise / 2147483648.0f) * 0.6f;
      } else {
        switch (v->state) {
          case ENV_ATTACK:
            v->env += attack;
            if (v->env >= 1.0f) { v->env = 1.0f; v->state = ENV_DECAY; }
            break;
          case ENV_DECAY:
            v->env -= decay;
            if (v->env <= sustain) { v->env = sustain; v->state = ENV_SUSTAIN; }
            break;
          case ENV_RELEASE:
            v->env -= release;
            if (v->env <= 0.0f) { v->state = ENV_OFF; continue; }
            break;
          default:
            break;
        }
        // a few harmonics keep plain sine tones from sounding too dull
        sample = (sinf(v->phase) + 0.35f * sinf(2.0f * v->phase) + 0.15f * sinf(3.0f * v->phase)) / 1.5f;
        v->phase += v->inc;
        if (v->phase >= 2.0f * (float)M_PI) v->phase -= 2.0f * (float)M_PI;
      }

      gain = ((float)v->velocity / 127.0f) * s->chan_volume[v->channel] * s->chan_expression[v->channel];
      mix += sample * v->env * gain;
    }

    mix *= s->master;
    if (mix > 1.0f) mix = 1.0f;
    if (mix < -1.0f) mix = -1.0f;
    s->block[i] = (int16_t)(mix * 32767.0f);
  }

  s->clock += frames;
}

static int synth_render(synth_t *s, uint32_t us) {
  int frames;

  s->pending_frames += ((double)us * SYNTH_RATE) / 1000000.0;

  while (s->pending_frames >= 1.0) {
    if (mixer_should_stop()) return 1;
    frames = s->pending_frames > SYNTH_BLOCK ? SYNTH_BLOCK : (int)s->pending_frames;
    synth_render_block(s, frames);
    if (aaudio_write(s->stream, s->block, frames, 1) != 0) {
      s->failed = 1;
      return 1;
    }
    s->pending_frames -= frames;
  }

  return 0;
}

static int synth_wait(uint32_t us, void *data) {
  return synth_render((synth_t *)data, us);
}

static int play_synth(smf_t *smf, int volume) {
  synth_t *s;
  int i, r;

  if ((s = sys_calloc(1, sizeof(synth_t))) == NULL) return -1;

  for (i = 0; i < 16; i++) {
    s->chan_volume[i] = 100.0f / 127.0f;
    s->chan_expression[i] = 1.0f;
    s->chan_bend[i] = 1.0f;
  }
  s->master = ((float)volume / 128.0f) * 0.30f;
  s->noise = 0x12345678;

  if ((s->stream = aaudio_open(SYNTH_RATE, 1)) == NULL) {
    sys_free(s);
    return -1;
  }

  debug(DEBUG_INFO, AA, "playing %u MIDI events on built-in synth (%llu us)", smf_num_events(smf), (unsigned long long)smf_duration(smf));
  r = smf_play(smf, synth_event, synth_wait, s);

  if (r == 0) {
    // let hanging notes ring out
    synth_all_off(s, -1, 0);
    for (i = 0; i < SYNTH_TAIL_US / 10000 && synth_active(s); i++) {
      if (synth_render(s, 10000) != 0) break;
    }
  }

  aaudio_close(s->stream, !s->failed);
  sys_free(s);

  return r < 0 ? -1 : 0;
}

static int libaaudio_mixer_init(void) {
  debug(DEBUG_INFO, AA, "mixer init (MIDI provider %s)", midi_provider ? "present" : "absent");
  return 0;
}

static int libaaudio_mixer_play(uint8_t *buf, uint32_t len, int volume) {
  smf_t *smf;
  int r;

  if (buf == NULL || len == 0) return -1;
  if (volume < 0) volume = 128;
  if (volume > 128) volume = 128;

  if ((smf = smf_load(buf, len)) == NULL) {
    return -1;
  }

  // one melody at a time; a concurrent caller waits for the current one
  pthread_mutex_lock(&mixer_mutex);
  mixer_stop_flag = 0;

  if (midi_provider && midi_provider->send && midi_provider->available &&
      midi_provider->available(midi_provider->data)) {
    r = play_midi(smf, volume);
  } else {
    r = play_synth(smf, volume);
  }

  pthread_mutex_unlock(&mixer_mutex);
  smf_free(smf);

  debug(DEBUG_INFO, AA, "mixer play done (%d)", r);
  return r;
}

static int libaaudio_mixer_stop(void) {
  debug(DEBUG_INFO, AA, "mixer stop");
  mixer_stop_flag = 1;
  return 0;
}

// Registers the AAudio provider. Call after midi_init() so that the MIDI
// provider can be picked up for SndPlaySmf playback.
void audio_init(int pe) {
  midi_provider = (midi_provider_t *)script_get_pointer(pe, MIDI_PROVIDER);

  sys_memset(&audio_provider, 0, sizeof(audio_provider));
  audio_provider.init = libaaudio_audio_init;
  audio_provider.finish = libaaudio_audio_finish;
  audio_provider.create = libaaudio_audio_create;
  audio_provider.start = libaaudio_audio_start;
  audio_provider.play = libaaudio_audio_play;
  audio_provider.destroy = libaaudio_audio_destroy;
  audio_provider.mixer_init = libaaudio_mixer_init;
  audio_provider.mixer_play = libaaudio_mixer_play;
  audio_provider.mixer_stop = libaaudio_mixer_stop;

  debug(DEBUG_INFO, AA, "registering provider %s", AUDIO_PROVIDER);
  script_set_pointer(pe, AUDIO_PROVIDER, &audio_provider);
}
