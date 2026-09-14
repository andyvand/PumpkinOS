#ifndef PIT_TEST_H
#define PIT_TEST_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

int getArch(void);
int pitInit(void);
void pitFinish(int pe);
void pitRequestFinish(void);
void pitDeploy(char *path);
void pitSetSurface(JNIEnv *env, jobject surface);
void pitPause(int paused);
void pitTouch(int down, int x, int y);
void pitKey(int key);
void pitSetBattery(int level);

void window_init(int pe);
void window_set_surface(JNIEnv *env, jobject surface);

// aaudio.c / amidi.c
void audio_init(int pe);
void midi_init(int pe);
void midi_set_device(JNIEnv *env, jobject device);
void pitSetMidiDevice(JNIEnv *env, jobject device);

#ifdef __cplusplus
}
#endif

#endif
