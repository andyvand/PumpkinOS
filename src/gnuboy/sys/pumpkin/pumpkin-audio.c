/*
 * pumpkin-audio.c
 * PumpkinOS (PalmOS SndStream) audio interface for gnuboy.
 *
 * gnuboy mixes unsigned 8-bit PCM into pcm.buf and calls pcm_submit()
 * whenever the buffer is full (and once per frame). We convert the
 * samples to signed 16-bit and push them into a ring buffer that the
 * SndStream callback drains on the host audio thread.
 *
 * PumpkinOS audio contract (see the VBA-M PumpkinOS port): the audio
 * thread calls the stream callback in an unthrottled loop and treats a
 * short or zero return as end-of-stream, permanently. Therefore the
 * callback always returns a full buffer (silence on underrun), paces
 * itself to real time with a sleep, and pcm_pause() never stops the
 * stream, it only mutes it.
 *
 * The ring fill level is also the frame pacing clock: pcm_submit()
 * blocks (pumping UI events) while more than AUDIO_HIGH_WATER frames
 * are queued, and returns non-zero so emu_run() skips its own sleep.
 *
 * Licensed under the GPLv2, or later.
 */

#include <PalmOS.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef ESP_PLATFORM
#include "../../libpit/sys.h"
#include "../../libpit/mutex.h"
#include "../../libpit/debug.h"
#include "../../libpumpkin/pumpkin.h"

#include "esp_attr.h"

#include "rc.h"
#include "pcm.h"
#else
#define EXT_RAM_BSS_ATTR

#include "sys.h"
#include "mutex.h"
#include "debug.h"
#include "pumpkin.h"

#include "../../rc.h"
#include "../../pcm.h"
#endif

#include "pumpkin-glue.h"

#define TAG "GNUBOY"

/* ring of interleaved stereo frames (~0.37s at 44100 Hz), power of two */
#define RING_FRAMES 16384
#define RING_MASK (RING_FRAMES - 1)

/* frames queued above which the emulator is throttled (~50ms at 44100 Hz) */
#define AUDIO_HIGH_WATER 2200

struct pcm pcm;

static int sound = 1;
static int samplerate = 44100;
static int stereo = 1;

rcvar_t pcm_exports[] =
{
	RCV_BOOL("sound", &sound, "enable sound"),
	RCV_INT("stereo", &stereo, "enable stereo"),
	RCV_INT("samplerate", &samplerate, "samplerate (44100 recommended on PumpkinOS)"),
	RCV_END
};

static EXT_RAM_BSS_ATTR int16_t ring[RING_FRAMES * 2];
static uint32_t ringRead;   /* frame index, only advanced by the audio thread */
static uint32_t ringWrite;  /* frame index, only advanced by the emu thread */
static mutex_t *ringMutex;

static SndStreamRef stream;
static int streamActive;
static int paused;
static int audioDead;
static int64_t pullDue;

static Err stream_callback(void *userdata, SndStreamRef channel, void *buffer, UInt32 numberofframes)
{
	int16_t *out = (int16_t *)buffer;
	UInt32 filled = 0, i;
	uint32_t avail;
	int64_t now;

	(void)userdata;
	(void)channel;

	/* pace the unthrottled pull loop to real time */
	now = sys_get_clock();
	if (pullDue == 0 || pullDue < now - 500000) pullDue = now;
	if (pullDue > now) sys_usleep((uint32_t)(pullDue - now));
	pullDue += (int64_t)numberofframes * 1000000 / pcm.hz;

	if (!paused && ringMutex && mutex_lock(ringMutex) == 0) {
		avail = ringWrite - ringRead;
		if (avail > numberofframes) avail = numberofframes;
		for (i = 0; i < avail; i++, ringRead++) {
			out[i * 2] = ring[(ringRead & RING_MASK) * 2];
			out[i * 2 + 1] = ring[(ringRead & RING_MASK) * 2 + 1];
		}
		filled = avail;
		mutex_unlock(ringMutex);
	}

	if (filled < numberofframes)
		memset(out + filled * 2, 0, (numberofframes - filled) * 2 * sizeof(int16_t));

	return errNone;
}

static int queued_frames(void)
{
	int queued = -1;

	if (streamActive && ringMutex && mutex_lock(ringMutex) == 0) {
		queued = (int)(ringWrite - ringRead);
		mutex_unlock(ringMutex);
	}
	return queued;
}

void pcm_init(void)
{
	Err err;

	pcm.hz = samplerate;
	pcm.stereo = stereo ? 1 : 0;
	pcm.len = 4096;
	pcm.pos = 0;
	pcm.buf = NULL;
	streamActive = 0;
	audioDead = 0;
	paused = 0;
	pullDue = 0;
	ringRead = ringWrite = 0;

	if (!sound) return;

	if (!ringMutex) ringMutex = mutex_create("gnuboysnd");

	err = SndStreamCreate(&stream, sndOutput, (UInt32)pcm.hz, sndInt16, sndStereo,
			stream_callback, NULL, 0, false);
	if (err != errNone) {
		debug(DEBUG_ERROR, TAG, "SndStreamCreate failed: %d", err);
		sound = 0;
		return;
	}
	if ((err = SndStreamStart(stream)) != errNone) {
		debug(DEBUG_ERROR, TAG, "SndStreamStart failed: %d (sound enabled %d, audio handle %d)",
			err, pumpkin_sound_enabled(), pumpkin_audio_get(NULL, NULL, NULL));
		SndStreamDelete(stream);
		stream = 0;
		sound = 0;
		return;
	}

	streamActive = 1;
	pcm.buf = malloc(pcm.len);
	memset(pcm.buf, 0, pcm.len);
	debug(DEBUG_INFO, TAG, "audio stream started (%d Hz, %s)", pcm.hz, pcm.stereo ? "stereo" : "mono");
}

int pcm_submit(void)
{
	int i, n, queued;
	int16_t l, r;
	int64_t waitStart;

	if (!streamActive || !pcm.buf || paused) {
		pcm.pos = 0;
		return 0;
	}

	if (ringMutex && mutex_lock(ringMutex) == 0) {
		if (pcm.stereo) {
			n = pcm.pos / 2;
			for (i = 0; i < n; i++, ringWrite++) {
				if (ringWrite - ringRead >= RING_FRAMES) break;
				l = (int16_t)(((int)pcm.buf[i * 2] - 128) << 8);
				r = (int16_t)(((int)pcm.buf[i * 2 + 1] - 128) << 8);
				ring[(ringWrite & RING_MASK) * 2] = l;
				ring[(ringWrite & RING_MASK) * 2 + 1] = r;
			}
		} else {
			n = pcm.pos;
			for (i = 0; i < n; i++, ringWrite++) {
				if (ringWrite - ringRead >= RING_FRAMES) break;
				l = (int16_t)(((int)pcm.buf[i] - 128) << 8);
				ring[(ringWrite & RING_MASK) * 2] = l;
				ring[(ringWrite & RING_MASK) * 2 + 1] = l;
			}
		}
		mutex_unlock(ringMutex);
	}
	pcm.pos = 0;

	if (audioDead) return 0;

	/* throttle: wait for the audio thread to drain the ring */
	waitStart = sys_get_clock();
	while ((queued = queued_frames()) > AUDIO_HIGH_WATER) {
		if (!gnuboy_pump_events(2000)) break;
		if (sys_get_clock() - waitStart > 400000) {
			/* nobody is draining the ring: fall back to wall clock pacing */
			debug(DEBUG_ERROR, TAG, "audio ring not draining, using clock pacing");
			audioDead = 1;
			return 0;
		}
	}
	return 1;
}

/* stereo frames waiting in the ring, -1 when audio is not running */
int gnuboy_audio_queued(void)
{
	if (audioDead || paused) return -1;
	return queued_frames();
}

void pcm_close(void)
{
	if (streamActive) {
		SndStreamDelete(stream);
		stream = 0;
		streamActive = 0;
	}
	if (pcm.buf) {
		free(pcm.buf);
		pcm.buf = NULL;
	}
}

void pcm_pause(int dopause)
{
	paused = dopause;
	if (!dopause && ringMutex && mutex_lock(ringMutex) == 0) {
		/* drop stale audio so the game does not resume with a burst */
		ringRead = ringWrite = 0;
		mutex_unlock(ringMutex);
	}
}
