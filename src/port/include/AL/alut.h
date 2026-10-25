/*
 * ManiaDrive Android port - minimal ALUT replacement.
 *
 * freealut is not available on Android. Raydium only needs device/context
 * set-up and a WAV loader, implemented in port/alut_mini.c.
 */
#ifndef MANIA_PORT_ALUT_H
#define MANIA_PORT_ALUT_H

#include <AL/al.h>
#include <AL/alc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Raydium checks this to choose the "freealut 1.x" code path */
#define ALUT_API_MAJOR_VERSION 1
#define ALUT_API_MINOR_VERSION 1

ALboolean alutInit(int *argcp, char **argv);
ALboolean alutInitWithoutContext(int *argcp, char **argv);
ALboolean alutExit(void);
void *alutLoadMemoryFromFile(const char *filename, ALenum *format,
                             ALsizei *size, ALfloat *frequency);
/* legacy API (kept for completeness) */
void alutLoadWAVFile(ALbyte *file, ALenum *format, void **data,
                     ALsizei *size, ALsizei *freq, ALboolean *loop);
void alutUnloadWAV(ALenum format, void *data, ALsizei size, ALsizei freq);

#ifdef __cplusplus
}
#endif
#endif
