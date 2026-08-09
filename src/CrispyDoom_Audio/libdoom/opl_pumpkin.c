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
//     OPL PumpkinOS interface. Software OPL3 emulation rendered on
//     demand into the shared audio mixer stream (see i_pumpkinsound.c).
//     Callback timing is derived from the number of samples rendered,
//     like the upstream SDL driver, but without any SDL dependency.
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mutex.h"

#include "opl3.h"

#include "opl.h"
#include "opl_internal.h"
#include "opl_pumpkin.h"

#include "opl_queue.h"

typedef struct
{
    unsigned int rate;        // Number of times the timer is advanced per sec.
    unsigned int enabled;     // Non-zero if timer is enabled.
    unsigned int value;       // Last value that was set.
    uint64_t expire_time;     // Calculated time that timer will expire.
} opl_timer_t;

// When the callback mutex is locked using OPL_Lock, callback functions
// are not invoked.

static mutex_t *callback_mutex = NULL;

// Queue of callbacks waiting to be invoked.

static opl_callback_queue_t *callback_queue;

// Mutex used to control access to the callback queue.

static mutex_t *callback_queue_mutex = NULL;

// Current time, in us since startup:

static uint64_t current_time;

// If non-zero, playback is currently paused.

static int opl_pumpkin_paused;

// Time offset (in us) due to the fact that callbacks
// were previously paused.

static uint64_t pause_offset;

// OPL software emulator structure.

static opl3_chip opl_chip;
static int opl_opl3mode;

// Temporary mixing buffer used by the mixing callback.

static int16_t *mix_buffer = NULL;

// Register number that was written.

static int register_num = 0;

// Timers; OPL3 emulator does not do timer stuff itself.

static opl_timer_t timer1 = { 12500, 0, 0, 0 };
static opl_timer_t timer2 = { 3125, 0, 0, 0 };

static int opl_pumpkin_initialized = 0;
static unsigned int mixing_freq;

// Provided by i_pumpkinsound.c: makes sure the shared output stream
// exists (music must be able to play even when sfx are disabled).

int I_Pumpkin_StartMixer(void);

// Advance time by the specified number of samples, invoking any
// callback functions as appropriate.

static void AdvanceTime(unsigned int nsamples)
{
    opl_callback_t callback;
    void *callback_data;
    uint64_t us;

    mutex_lock(callback_queue_mutex);

    // Advance time.

    us = ((uint64_t) nsamples * OPL_SECOND) / mixing_freq;
    current_time += us;

    if (opl_pumpkin_paused)
    {
        pause_offset += us;
    }

    // Are there callbacks to invoke now?  Keep invoking them
    // until there are no more left.

    while (!OPL_Queue_IsEmpty(callback_queue)
        && current_time >= OPL_Queue_Peek(callback_queue) + pause_offset)
    {
        // Pop the callback from the queue to invoke it.

        if (!OPL_Queue_Pop(callback_queue, &callback, &callback_data))
        {
            break;
        }

        // The mutex stuff here is a bit complicated.  We must
        // hold callback_mutex when we invoke the callback (so that
        // the control thread can use OPL_Lock() to prevent callbacks
        // from being invoked), but we must not be holding
        // callback_queue_mutex, as the callback must be able to
        // call OPL_SetCallback to schedule new callbacks.

        mutex_unlock(callback_queue_mutex);

        mutex_lock(callback_mutex);
        callback(callback_data);
        mutex_unlock(callback_mutex);

        mutex_lock(callback_queue_mutex);
    }

    mutex_unlock(callback_queue_mutex);
}

// Call the OPL emulator code to generate nsamples samples, then add
// them into the output buffer with saturation.

static void FillBuffer(int16_t *buffer, unsigned int nsamples)
{
    unsigned int i;
    int val;

    // OPL output is generated into a temporary buffer and then mixed
    // (to avoid overflows etc.)

    OPL3_GenerateStream(&opl_chip, (Bit16s *) mix_buffer, nsamples);

    for (i = 0; i < nsamples * 2; ++i)
    {
        val = buffer[i] + mix_buffer[i];

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
}

// Mix OPL music into a stereo S16 buffer; called by the shared audio
// mixer in i_pumpkinsound.c on the audio thread.

void OPL_Pumpkin_Render(int16_t *buffer, unsigned int nframes)
{
    unsigned int filled = 0;

    if (!opl_pumpkin_initialized)
    {
        return;
    }

    // Repeatedly call the OPL emulator update function until the buffer is
    // full.

    while (filled < nframes)
    {
        uint64_t next_callback_time;
        uint64_t nsamples;

        mutex_lock(callback_queue_mutex);

        // Work out the time until the next callback waiting in
        // the callback queue must be invoked.  We can then fill the
        // buffer with this many samples.

        if (opl_pumpkin_paused || OPL_Queue_IsEmpty(callback_queue))
        {
            nsamples = nframes - filled;
        }
        else
        {
            next_callback_time = OPL_Queue_Peek(callback_queue) + pause_offset;

            nsamples = (next_callback_time - current_time) * mixing_freq;
            nsamples = (nsamples + OPL_SECOND - 1) / OPL_SECOND;

            if (nsamples > nframes - filled)
            {
                nsamples = nframes - filled;
            }
        }

        mutex_unlock(callback_queue_mutex);

        // Add emulator output to buffer.

        FillBuffer(buffer + filled * 2, nsamples);
        filled += nsamples;

        // Invoke callbacks for this point in time.

        AdvanceTime(nsamples);
    }
}

static void OPL_Pumpkin_Shutdown(void)
{
    if (!opl_pumpkin_initialized)
    {
        return;
    }

    opl_pumpkin_initialized = 0;

    OPL_Queue_Destroy(callback_queue);
    callback_queue = NULL;

    free(mix_buffer);
    mix_buffer = NULL;

    if (callback_mutex != NULL)
    {
        mutex_destroy(callback_mutex);
        callback_mutex = NULL;
    }

    if (callback_queue_mutex != NULL)
    {
        mutex_destroy(callback_queue_mutex);
        callback_queue_mutex = NULL;
    }
}

static int OPL_Pumpkin_Init(unsigned int port_base)
{
    // The shared output stream must be running for music to be heard
    // (and for emulated time to advance).

    if (!I_Pumpkin_StartMixer())
    {
        fprintf(stderr, "OPL_Pumpkin_Init: Unable to set up sound.\n");
        return 0;
    }

    opl_pumpkin_paused = 0;
    pause_offset = 0;

    // Queue structure of callbacks to invoke.

    callback_queue = OPL_Queue_Create();
    current_time = 0;

    mixing_freq = opl_sample_rate;

    // Mix buffer: four bytes per sample (16 bits * 2 channels):
    mix_buffer = malloc(mixing_freq * 4);

    if (mix_buffer == NULL)
    {
        OPL_Queue_Destroy(callback_queue);
        callback_queue = NULL;
        return 0;
    }

    // Create the emulator structure:

    OPL3_Reset(&opl_chip, mixing_freq);
    opl_opl3mode = 0;

    callback_mutex = mutex_create("opl_cb");
    callback_queue_mutex = mutex_create("opl_q");

    opl_pumpkin_initialized = 1;

    return 1;
}

static unsigned int OPL_Pumpkin_PortRead(opl_port_t port)
{
    unsigned int result = 0;

    if (port == OPL_REGISTER_PORT_OPL3)
    {
        return 0xff;
    }

    if (timer1.enabled && current_time > timer1.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x40;   // Timer 1 has expired
    }

    if (timer2.enabled && current_time > timer2.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x20;   // Timer 2 has expired
    }

    return result;
}

static void OPLTimer_CalculateEndTime(opl_timer_t *timer)
{
    int tics;

    // If the timer is enabled, calculate the time when the timer
    // will expire.

    if (timer->enabled)
    {
        tics = 0x100 - timer->value;
        timer->expire_time = current_time
                           + ((uint64_t) tics * OPL_SECOND) / timer->rate;
    }
}

static void WriteRegister(unsigned int reg_num, unsigned int value)
{
    switch (reg_num)
    {
        case OPL_REG_TIMER1:
            timer1.value = value;
            OPLTimer_CalculateEndTime(&timer1);
            break;

        case OPL_REG_TIMER2:
            timer2.value = value;
            OPLTimer_CalculateEndTime(&timer2);
            break;

        case OPL_REG_TIMER_CTRL:
            if (value & 0x80)
            {
                timer1.enabled = 0;
                timer2.enabled = 0;
            }
            else
            {
                if ((value & 0x40) == 0)
                {
                    timer1.enabled = (value & 0x01) != 0;
                    OPLTimer_CalculateEndTime(&timer1);
                }

                if ((value & 0x20) == 0)
                {
                    timer1.enabled = (value & 0x02) != 0;
                    OPLTimer_CalculateEndTime(&timer2);
                }
            }

            break;

        case OPL_REG_NEW:
            opl_opl3mode = value & 0x01;

        default:
            OPL3_WriteRegBuffered(&opl_chip, reg_num, value);
            break;
    }
}

static void OPL_Pumpkin_PortWrite(opl_port_t port, unsigned int value)
{
    if (port == OPL_REGISTER_PORT)
    {
        register_num = value;
    }
    else if (port == OPL_REGISTER_PORT_OPL3)
    {
        register_num = value | 0x100;
    }
    else if (port == OPL_DATA_PORT)
    {
        WriteRegister(register_num, value);
    }
}

static void OPL_Pumpkin_SetCallback(uint64_t us, opl_callback_t callback,
                                    void *data)
{
    mutex_lock(callback_queue_mutex);
    OPL_Queue_Push(callback_queue, callback, data,
                   current_time - pause_offset + us);
    mutex_unlock(callback_queue_mutex);
}

static void OPL_Pumpkin_ClearCallbacks(void)
{
    mutex_lock(callback_queue_mutex);
    OPL_Queue_Clear(callback_queue);
    mutex_unlock(callback_queue_mutex);
}

static void OPL_Pumpkin_Lock(void)
{
    mutex_lock(callback_mutex);
}

static void OPL_Pumpkin_Unlock(void)
{
    mutex_unlock(callback_mutex);
}

static void OPL_Pumpkin_SetPaused(int paused)
{
    opl_pumpkin_paused = paused;
}

static void OPL_Pumpkin_AdjustCallbacks(float factor)
{
    mutex_lock(callback_queue_mutex);
    OPL_Queue_AdjustCallbacks(callback_queue, current_time, factor);
    mutex_unlock(callback_queue_mutex);
}

opl_driver_t opl_pumpkin_driver =
{
    "Pumpkin",
    OPL_Pumpkin_Init,
    OPL_Pumpkin_Shutdown,
    OPL_Pumpkin_PortRead,
    OPL_Pumpkin_PortWrite,
    OPL_Pumpkin_SetCallback,
    OPL_Pumpkin_ClearCallbacks,
    OPL_Pumpkin_Lock,
    OPL_Pumpkin_Unlock,
    OPL_Pumpkin_SetPaused,
    OPL_Pumpkin_AdjustCallbacks,
};
