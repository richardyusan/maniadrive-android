/*
 * ManiaDrive Android port - entry point.
 *
 *  1. redirects stdout/stderr (Raydium's log) to logcat (tag "ManiaDrive")
 *  2. extracts the game data from the APK to internal storage on the first
 *     start and after each update (the engine reads its files with fopen)
 *  3. starts the original main() of mania_drive.c (renamed maniadrive_main)
 *
 * GPL v2 or later, like ManiaDrive.
 */
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <android/window.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define TAG "ManiaDrive"

struct android_app *port_app;

void port_handle_cmd(struct android_app *app, int32_t cmd);
int32_t port_handle_input(struct android_app *app, AInputEvent *ev);
int maniadrive_main(int argc, char **argv);

/* ------------------------------------------------------------ logcat */
static int log_pipe[2];

static void *log_thread(void *arg)
{
    char buf[1024];
    size_t used = 0;
    (void)arg;
    for (;;) {
        ssize_t n = read(log_pipe[0], buf + used, sizeof buf - 1 - used);
        char *start, *nl;
        if (n <= 0) break;
        used += n;
        buf[used] = 0;
        start = buf;
        while ((nl = strchr(start, '\n'))) {
            *nl = 0;
            __android_log_write(ANDROID_LOG_INFO, TAG, start);
            start = nl + 1;
        }
        used = strlen(start);
        memmove(buf, start, used + 1);
        if (used >= sizeof buf - 1) {            /* very long line */
            __android_log_write(ANDROID_LOG_INFO, TAG, buf);
            used = 0;
        }
    }
    return NULL;
}

static void start_logger(void)
{
    pthread_t t;
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    if (pipe(log_pipe) != 0) return;
    dup2(log_pipe[1], 1);
    dup2(log_pipe[1], 2);
    pthread_create(&t, NULL, log_thread, NULL);
    pthread_detach(t);
}

/* ------------------------------------------------------------ events */
static void pump_events(void)
{
    int events;
    struct android_poll_source *source;
    while (ALooper_pollOnce(0, NULL, &events, (void **)&source) >= 0) {
        if (source) source->process(port_app, source);
        if (port_app->destroyRequested) exit(0);
    }
}

/* ------------------------------------------------------------ assets */
static void mkdirs(const char *path)
{
    char tmp[1024];
    char *p;
    snprintf(tmp, sizeof tmp, "%s", path);
    for (p = tmp + 1; *p; p++)
        if (*p == '/') { *p = 0; mkdir(tmp, 0755); *p = '/'; }
    mkdir(tmp, 0755);
}

static char *read_asset(AAssetManager *am, const char *name, off_t *len)
{
    AAsset *a = AAssetManager_open(am, name, AASSET_MODE_BUFFER);
    char *buf;
    if (!a) return NULL;
    *len = AAsset_getLength(a);
    buf = malloc(*len + 1);
    if (buf) {
        memcpy(buf, AAsset_getBuffer(a), *len);
        buf[*len] = 0;
    }
    AAsset_close(a);
    return buf;
}

static int copy_asset(AAssetManager *am, const char *name, const char *dest)
{
    AAsset *a = AAssetManager_open(am, name, AASSET_MODE_STREAMING);
    char tmp[1024], buf[65536];
    FILE *fp;
    int n, ok = 1;
    if (!a) return 0;
    snprintf(tmp, sizeof tmp, "%s.tmp", dest);
    fp = fopen(tmp, "wb");
    if (!fp) { AAsset_close(a); return 0; }
    while ((n = AAsset_read(a, buf, sizeof buf)) > 0)
        if (fwrite(buf, 1, n, fp) != (size_t)n) { ok = 0; break; }
    if (n < 0) ok = 0;
    AAsset_close(a);
    if (fclose(fp) != 0) ok = 0;
    if (ok) ok = rename(tmp, dest) == 0;
    if (!ok) unlink(tmp);
    return ok;
}

/* manifest.txt: first line "#build <id>", then one file per line (relative to assets/game) */
static void extract_game_data(const char *gamedir)
{
    AAssetManager *am = port_app->activity->assetManager;
    char stamp_path[1024], line[1024];
    off_t len;
    char *manifest = read_asset(am, "manifest.txt", &len);
    char *build, *p, *next;
    FILE *fp;
    int count = 0, failed = 0;

    if (!manifest) {
        __android_log_write(ANDROID_LOG_ERROR, TAG, "manifest.txt missing from the APK");
        return;
    }
    build = manifest;
    p = strchr(manifest, '\n');
    if (!p) { free(manifest); return; }
    *p = 0;
    next = p + 1;

    snprintf(stamp_path, sizeof stamp_path, "%s/.data_version", gamedir);
    fp = fopen(stamp_path, "rt");
    if (fp) {
        line[0] = 0;
        if (fgets(line, sizeof line, fp)) line[strcspn(line, "\r\n")] = 0;
        fclose(fp);
        if (!strcmp(line, build)) {
            __android_log_print(ANDROID_LOG_INFO, TAG, "game data up to date (%s)", build);
            free(manifest);
            return;
        }
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "extracting game data (%s) to %s", build, gamedir);
    mkdirs(gamedir);
    unlink(stamp_path);
    for (p = next; p && *p; p = next) {
        char asset[1024], dest[1024], *slash;
        next = strchr(p, '\n');
        if (next) *next++ = 0;
        p[strcspn(p, "\r")] = 0;
        if (!*p) continue;
        snprintf(asset, sizeof asset, "game/%s", p);
        snprintf(dest, sizeof dest, "%s/%s", gamedir, p);
        slash = strrchr(dest, '/');
        if (slash) { *slash = 0; mkdirs(dest); *slash = '/'; }
        if (copy_asset(am, asset, dest)) count++;
        else { failed++; __android_log_print(ANDROID_LOG_ERROR, TAG, "cannot extract %s", asset); }
        pump_events();             /* keep the activity responsive */
    }
    if (!failed) {
        fp = fopen(stamp_path, "wt");
        if (fp) { fprintf(fp, "%s\n", build); fclose(fp); }
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "%i files extracted, %i failed", count, failed);
    free(manifest);
}

/* ------------------------------------------------------------ main */
void android_main(struct android_app *app)
{
    char gamedir[1024], exe[1100];
    char *argv[4];
    const char *files = app->activity->internalDataPath;

    port_app = app;
    app->onAppCmd = port_handle_cmd;
    app->onInputEvent = port_handle_input;
    start_logger();

    ANativeActivity_setWindowFlags(app->activity,
        AWINDOW_FLAG_KEEP_SCREEN_ON | AWINDOW_FLAG_FULLSCREEN, 0);

    if (!files) files = "/data/local/tmp/maniadrive";
    snprintf(gamedir, sizeof gamedir, "%s/game", files);
    extract_game_data(gamedir);

    /* Raydium: home dir = $HOME/.mania_drive (settings, story progress),
       working dir = directory of argv[0] (game data) */
    setenv("HOME", files, 1);
    snprintf(exe, sizeof exe, "%s/mania_drive", gamedir);
    argv[0] = exe;
    argv[1] = "--repository-disable";
    argv[2] = NULL;
    printf("ManiaDrive for Android - data in %s\n", gamedir);
    maniadrive_main(2, argv);
    exit(0);
}
