// AMidi MIDI provider for the PumpkinOS Android port.
//
// The Java side (PumpkinMidi.java) picks a MIDI device through MidiManager,
// opens it and hands the android.media.midi.MidiDevice object to
// midi_set_device() over JNI. From then on the provider talks to the device
// through the NDK AMidi API. AMidi only exists since API 29 while the app's
// minSdk is 26, so libamidi.so is loaded with dlopen() at runtime; on older
// systems the provider simply reports that no device is available.
//
// Terminology follows AMidi: an "input port" is a port the app sends data
// INTO (the device's input), an "output port" is one the app receives from.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#include <jni.h>
#include <android/log.h>

#include "sys.h"
#include "script.h"
#include "midi.h"
#include "debug.h"
#include "pitapp.h"

#define AM "AMidi"

// opaque AMidi types and constants (from <amidi/AMidi.h>, not linked directly)
typedef struct AMidiDevice AMidiDevice;
typedef struct AMidiInputPort AMidiInputPort;
typedef struct AMidiOutputPort AMidiOutputPort;
typedef int32_t media_status_t;
#define AMEDIA_OK 0
#define AMIDI_OPCODE_DATA 1

typedef media_status_t (*AMidiDevice_fromJava_f)(JNIEnv *env, jobject midiDeviceObj, AMidiDevice **outDevicePtrPtr);
typedef media_status_t (*AMidiDevice_release_f)(const AMidiDevice *midiDevice);
typedef int32_t (*AMidiDevice_getType_f)(const AMidiDevice *device);
typedef ssize_t (*AMidiDevice_getNumInputPorts_f)(const AMidiDevice *device);
typedef ssize_t (*AMidiDevice_getNumOutputPorts_f)(const AMidiDevice *device);
typedef media_status_t (*AMidiInputPort_open_f)(const AMidiDevice *device, int32_t portNumber, AMidiInputPort **outInputPortPtr);
typedef ssize_t (*AMidiInputPort_send_f)(const AMidiInputPort *inputPort, const uint8_t *buffer, size_t numBytes);
typedef media_status_t (*AMidiInputPort_sendFlush_f)(const AMidiInputPort *inputPort);
typedef void (*AMidiInputPort_close_f)(const AMidiInputPort *inputPort);
typedef media_status_t (*AMidiOutputPort_open_f)(const AMidiDevice *device, int32_t portNumber, AMidiOutputPort **outOutputPortPtr);
typedef void (*AMidiOutputPort_close_f)(const AMidiOutputPort *outputPort);
typedef ssize_t (*AMidiOutputPort_receive_f)(const AMidiOutputPort *outputPort, int32_t *opcodePtr, uint8_t *buffer, size_t maxBytes, size_t *numBytesReceivedPtr, int64_t *outTimestampPtr);

typedef struct {
  void *lib;
  int tried;
  AMidiDevice_fromJava_f fromJava;
  AMidiDevice_release_f release;
  AMidiDevice_getType_f getType;
  AMidiDevice_getNumInputPorts_f getNumInputPorts;
  AMidiDevice_getNumOutputPorts_f getNumOutputPorts;
  AMidiInputPort_open_f inputOpen;
  AMidiInputPort_send_f inputSend;
  AMidiInputPort_sendFlush_f inputFlush;
  AMidiInputPort_close_f inputClose;
  AMidiOutputPort_open_f outputOpen;
  AMidiOutputPort_close_f outputClose;
  AMidiOutputPort_receive_f outputReceive;
} amidi_api_t;

static amidi_api_t api;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static AMidiDevice *device = NULL;
static AMidiInputPort *in_port = NULL;    // we send here
static AMidiOutputPort *out_port = NULL;  // we receive from here
static midi_provider_t midi_provider;

// midi_set_device() can be called from the UI thread before pitInit() has run
// debug_init(), when debug() is still a no-op; log through logcat as well.
#define amidi_log(prio, ...) do { \
  __android_log_print(prio, "PumpkinOS", __VA_ARGS__); \
  debug(prio == ANDROID_LOG_ERROR ? DEBUG_ERROR : DEBUG_INFO, AM, __VA_ARGS__); \
} while (0)

static int amidi_load(void) {
  if (api.tried) return api.lib != NULL;
  api.tried = 1;

  if ((api.lib = dlopen("libamidi.so", RTLD_NOW | RTLD_LOCAL)) == NULL) {
    amidi_log(ANDROID_LOG_INFO, "libamidi.so not available (%s); MIDI output disabled", dlerror());
    return 0;
  }

#define SYM(field, name) \
  if ((api.field = (name##_f)dlsym(api.lib, #name)) == NULL) { \
    amidi_log(ANDROID_LOG_ERROR, "symbol %s missing from libamidi.so", #name); \
    dlclose(api.lib); api.lib = NULL; return 0; \
  }

  SYM(fromJava, AMidiDevice_fromJava);
  SYM(release, AMidiDevice_release);
  SYM(getType, AMidiDevice_getType);
  SYM(getNumInputPorts, AMidiDevice_getNumInputPorts);
  SYM(getNumOutputPorts, AMidiDevice_getNumOutputPorts);
  SYM(inputOpen, AMidiInputPort_open);
  SYM(inputSend, AMidiInputPort_send);
  SYM(inputFlush, AMidiInputPort_sendFlush);
  SYM(inputClose, AMidiInputPort_close);
  SYM(outputOpen, AMidiOutputPort_open);
  SYM(outputClose, AMidiOutputPort_close);
  SYM(outputReceive, AMidiOutputPort_receive);
#undef SYM

  amidi_log(ANDROID_LOG_INFO, "libamidi.so loaded");
  return 1;
}

static void amidi_close_locked(void) {
  if (in_port) {
    api.inputClose(in_port);
    in_port = NULL;
  }
  if (out_port) {
    api.outputClose(out_port);
    out_port = NULL;
  }
  if (device) {
    api.release(device);
    device = NULL;
    amidi_log(ANDROID_LOG_INFO, "MIDI device released");
  }
}

// Called from JNI (Java_com_pit_pit_Pumpkin_pitSetMidiDevice) with the
// android.media.midi.MidiDevice to use, or NULL to disconnect.
void midi_set_device(JNIEnv *env, jobject jdevice) {
  media_status_t st;
  ssize_t nin, nout;

  pthread_mutex_lock(&mutex);

  if (api.lib) {
    amidi_close_locked();
  }

  if (jdevice != NULL) {
    if (!amidi_load()) {
      pthread_mutex_unlock(&mutex);
      return;
    }

    if ((st = api.fromJava(env, jdevice, &device)) != AMEDIA_OK || device == NULL) {
      amidi_log(ANDROID_LOG_ERROR, "AMidiDevice_fromJava failed (%d)", (int)st);
      device = NULL;
      pthread_mutex_unlock(&mutex);
      return;
    }

    nin = api.getNumInputPorts(device);
    nout = api.getNumOutputPorts(device);
    amidi_log(ANDROID_LOG_INFO, "MIDI device type %d, %d input port(s), %d output port(s)", (int)api.getType(device), (int)nin, (int)nout);

    if (nin > 0) {
      if ((st = api.inputOpen(device, 0, &in_port)) != AMEDIA_OK) {
        amidi_log(ANDROID_LOG_ERROR, "AMidiInputPort_open failed (%d)", (int)st);
        in_port = NULL;
      }
    }
    if (nout > 0) {
      if ((st = api.outputOpen(device, 0, &out_port)) != AMEDIA_OK) {
        amidi_log(ANDROID_LOG_ERROR, "AMidiOutputPort_open failed (%d)", (int)st);
        out_port = NULL;
      }
    }

    if (in_port == NULL && out_port == NULL) {
      amidi_close_locked();
    }
  }

  pthread_mutex_unlock(&mutex);
}

// --- provider -------------------------------------------------------------

static int amidi_available(void *data) {
  int r;

  pthread_mutex_lock(&mutex);
  r = in_port != NULL;
  pthread_mutex_unlock(&mutex);

  return r;
}

static int amidi_send(uint8_t *msg, int len, void *data) {
  ssize_t r = -1;

  if (msg == NULL || len <= 0) return -1;

  pthread_mutex_lock(&mutex);
  if (in_port) {
    r = api.inputSend(in_port, msg, (size_t)len);
    if (r < 0) {
      debug(DEBUG_ERROR, AM, "AMidiInputPort_send failed (%d)", (int)r);
    }
  }
  pthread_mutex_unlock(&mutex);

  return (int)r;
}

static int amidi_flush(void *data) {
  int r = -1;

  pthread_mutex_lock(&mutex);
  if (in_port) {
    r = api.inputFlush(in_port) == AMEDIA_OK ? 0 : -1;
  }
  pthread_mutex_unlock(&mutex);

  return r;
}

static int amidi_reset(void *data) {
  uint8_t msg[3];
  int ch, r = -1;

  pthread_mutex_lock(&mutex);
  if (in_port) {
    r = 0;
    for (ch = 0; ch < 16; ch++) {
      msg[0] = 0xB0 | ch;
      msg[1] = 120; msg[2] = 0; // all sound off
      if (api.inputSend(in_port, msg, 3) < 0) r = -1;
      msg[1] = 123; msg[2] = 0; // all notes off
      if (api.inputSend(in_port, msg, 3) < 0) r = -1;
      msg[1] = 121; msg[2] = 0; // reset all controllers
      if (api.inputSend(in_port, msg, 3) < 0) r = -1;
    }
    api.inputFlush(in_port);
  }
  pthread_mutex_unlock(&mutex);

  return r;
}

static int amidi_receive(uint8_t *buf, int len, void *data) {
  int32_t opcode = 0;
  size_t received = 0;
  int64_t timestamp = 0;
  ssize_t n;
  int r = 0;

  if (buf == NULL || len <= 0) return -1;

  pthread_mutex_lock(&mutex);
  if (out_port) {
    n = api.outputReceive(out_port, &opcode, buf, (size_t)len, &received, &timestamp);
    if (n < 0) {
      r = -1;
    } else if (n > 0 && opcode == AMIDI_OPCODE_DATA) {
      r = (int)received;
    }
  }
  pthread_mutex_unlock(&mutex);

  return r;
}

// Registers the AMidi provider. Call before audio_init().
void midi_init(int pe) {
  sys_memset(&midi_provider, 0, sizeof(midi_provider));
  midi_provider.available = amidi_available;
  midi_provider.send = amidi_send;
  midi_provider.flush = amidi_flush;
  midi_provider.reset = amidi_reset;
  midi_provider.receive = amidi_receive;

  amidi_load();

  debug(DEBUG_INFO, AM, "registering provider %s", MIDI_PROVIDER);
  script_set_pointer(pe, MIDI_PROVIDER, &midi_provider);
}
