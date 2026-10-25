/*
 * ManiaDrive Android port - minimal ALUT implementation (device set-up and
 * WAV loading), replacing freealut. GPL v2 or later, like ManiaDrive.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/alut.h>

static ALCdevice  *alut_device;
static ALCcontext *alut_context;

ALboolean alutInitWithoutContext(int *argcp, char **argv)
{
    (void)argcp; (void)argv;
    return AL_TRUE;
}

ALboolean alutInit(int *argcp, char **argv)
{
    (void)argcp; (void)argv;
    if (alut_context)
        return AL_TRUE;
    alut_device = alcOpenDevice(NULL);
    if (!alut_device)
        return AL_FALSE;
    alut_context = alcCreateContext(alut_device, NULL);
    if (!alut_context) {
        alcCloseDevice(alut_device);
        alut_device = NULL;
        return AL_FALSE;
    }
    alcMakeContextCurrent(alut_context);
    return AL_TRUE;
}

ALboolean alutExit(void)
{
    alcMakeContextCurrent(NULL);
    if (alut_context) alcDestroyContext(alut_context);
    if (alut_device)  alcCloseDevice(alut_device);
    alut_context = NULL;
    alut_device = NULL;
    return AL_TRUE;
}

static unsigned rd16(const unsigned char *p) { return p[0] | (p[1] << 8); }
static unsigned rd32(const unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24); }

/* Loads a RIFF/WAVE PCM file (8 or 16 bit, mono or stereo). */
void *alutLoadMemoryFromFile(const char *filename, ALenum *format,
                             ALsizei *size, ALfloat *frequency)
{
    FILE *fp;
    long len;
    unsigned char *buf, *p, *end;
    unsigned channels = 0, bits = 0, rate = 0, fmt_tag = 0;
    unsigned char *pcm = NULL;
    unsigned pcm_len = 0;
    void *out;

    fp = fopen(filename, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (len < 44) { fclose(fp); return NULL; }
    buf = malloc(len);
    if (!buf || fread(buf, 1, len, fp) != (size_t)len) { fclose(fp); free(buf); return NULL; }
    fclose(fp);

    if (memcmp(buf, "RIFF", 4) || memcmp(buf + 8, "WAVE", 4)) { free(buf); return NULL; }
    p = buf + 12;
    end = buf + len;
    while (p + 8 <= end) {
        unsigned csize = rd32(p + 4);
        unsigned char *cdata = p + 8;
        if (cdata + csize > end) csize = (unsigned)(end - cdata);
        if (!memcmp(p, "fmt ", 4) && csize >= 16) {
            fmt_tag  = rd16(cdata);
            channels = rd16(cdata + 2);
            rate     = rd32(cdata + 4);
            bits     = rd16(cdata + 14);
        } else if (!memcmp(p, "data", 4)) {
            pcm = cdata;
            pcm_len = csize;
        }
        p = cdata + csize + (csize & 1);
    }

    if (fmt_tag != 1 || !pcm || !channels || !rate || (bits != 8 && bits != 16)) {
        free(buf);
        return NULL;
    }
    if (channels == 1) *format = (bits == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    else               *format = (bits == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    pcm_len -= pcm_len % (channels * bits / 8);

    out = malloc(pcm_len ? pcm_len : 1);
    if (out) memcpy(out, pcm, pcm_len);
    *size = (ALsizei)pcm_len;
    *frequency = (ALfloat)rate;
    free(buf);
    return out;
}

void alutLoadWAVFile(ALbyte *file, ALenum *format, void **data,
                     ALsizei *size, ALsizei *freq, ALboolean *loop)
{
    ALfloat f = 0;
    *data = alutLoadMemoryFromFile((const char *)file, format, size, &f);
    *freq = (ALsizei)f;
    if (loop) *loop = AL_FALSE;
}

void alutUnloadWAV(ALenum format, void *data, ALsizei size, ALsizei freq)
{
    (void)format; (void)size; (void)freq;
    free(data);
}
