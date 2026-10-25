/*
 * ManiaDrive Android port - Raydium "myglut" backend.
 *
 * Replaces myglut-x11.c / myglut-win32.c:
 *   - __ANDROID__   : NativeActivity window + EGL (OpenGL ES 2 context, the
 *                     game's OpenGL 1.x calls are translated by gl4es),
 *                     touch / gamepad / keyboard input, pause & resume.
 *   - MANIA_HEADLESS: off-screen EGL pbuffer on a Linux PC, with scripted
 *                     input and screenshots (automated tests, see
 *                     scripts/test_linux.sh).
 *
 * Frame rate is locked to PORT_FPS (60): vsync (swap interval 1) plus a
 * sleep-based limiter for 90/120 Hz displays.
 *
 * This file is #included by raydium/myglut.c. GPL v2 or later.
 */
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "gl4esinit.h"
#include "port.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <dlfcn.h>
#include <jni.h>
extern struct android_app *port_app;          /* android_main.c */
#endif

void pwInit(int x, int y, int w, int h, int multisample,
            char *title, int border, int num_samples);
int raydium_init_cli_option(char *option, char *value);
void raydium_log(char *format, ...);
void port_overlay_draw(void);

/* ------------------------------------------------------------ state */
static EGLDisplay egl_dpy = EGL_NO_DISPLAY;
static EGLContext egl_ctx = EGL_NO_CONTEXT;
static EGLSurface egl_surf = EGL_NO_SURFACE;
static EGLConfig  egl_cfg;
static int render_w, render_h;        /* GL buffer size */
static int view_w = 1, view_h = 1;    /* size of the touch coordinate space */
static int render_max_h = 1080;       /* resolution cap (height) */
static int last_reported_w, last_reported_h;

float port_analog_x, port_analog_y;
int   port_analog_active;
int   port_mouse_cursor;

/* ------------------------------------------------------------ time */
static uint64_t mono_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + ts.tv_nsec / 1000;
}

static uint64_t time_base, time_paused_total, time_pause_start;

uint64_t port_time_us(void)
{
    uint64_t now = mono_us();
    if (!time_base) time_base = now;
    if (time_pause_start) now = time_pause_start;
    return now - time_base - time_paused_total;
}

static void time_pause(int paused)
{
    if (paused && !time_pause_start)
        time_pause_start = mono_us();
    if (!paused && time_pause_start) {
        time_paused_total += mono_us() - time_pause_start;
        time_pause_start = 0;
    }
}

static void sleep_us(int64_t us)
{
    struct timespec ts;
    if (us <= 0) return;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}

/* 60 fps lock: called after every eglSwapBuffers */
static void frame_limiter(void)
{
    static uint64_t next;
    const uint64_t period = 1000000 / PORT_FPS;
    uint64_t now = mono_us();
    if (!next || now > next + 2 * period || now + 2 * period < next) {
        next = now + period;            /* (re)synchronise */
        return;
    }
    /* keep 1 ms of margin so a 60 Hz vsync is never missed because of us */
    if (now + 1000 < next)
        sleep_us((int64_t)(next - now - 1000));
    next += period;
}

/* ------------------------------------------------------------ input -> raydium */
void port_key_special(int key, int down)
{
    if (down && glutSpecialFuncCB && !raydium_key[key]) glutSpecialFuncCB(key, 0, 0);
    if (!down && glutSpecialUpFuncCB && raydium_key[key]) glutSpecialUpFuncCB(key, 0, 0);
}

void port_key_char(int ch)
{
    if (glutKeyboardFuncCB) glutKeyboardFuncCB((unsigned char)ch, 0, 0);
}

void port_mouse_move(int x, int y)
{
    if (glutPassiveMotionFuncCB) glutPassiveMotionFuncCB(x, y);
}

void port_mouse(int down, int x, int y)
{
    port_mouse_move(x, y);
    if (glutMouseFuncCB) glutMouseFuncCB(GLUT_LEFT_BUTTON, down ? GLUT_DOWN : GLUT_UP, x, y);
}

/* ------------------------------------------------------------ EGL */
static int egl_choose_config(void)
{
    static const EGLint depths[][3] = { {24, 8, 8}, {16, 8, 8}, {24, 0, 8}, {16, 0, 8}, {16, 0, 5} };
    unsigned i;
    for (i = 0; i < sizeof depths / sizeof depths[0]; i++) {
        EGLint n = 0;
        EGLint attr[] = {
#ifdef MANIA_HEADLESS
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
#else
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
#endif
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE, depths[i][2], EGL_GREEN_SIZE, depths[i][2] == 8 ? 8 : 6,
            EGL_BLUE_SIZE, depths[i][2],
            EGL_DEPTH_SIZE, depths[i][0], EGL_STENCIL_SIZE, depths[i][1],
            EGL_NONE };
        if (eglChooseConfig(egl_dpy, attr, &egl_cfg, 1, &n) && n > 0) {
            raydium_log("(my)glut: EGL config: depth %i, stencil %i, color %i bits",
                        depths[i][0], depths[i][1], depths[i][2] == 8 ? 24 : 16);
            return 1;
        }
    }
    return 0;
}

static void egl_init_display(void)
{
    EGLint major, minor;
    EGLint ctx_attr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_dpy == EGL_NO_DISPLAY || !eglInitialize(egl_dpy, &major, &minor)) {
        raydium_log("(my)glut: ERROR: cannot initialise EGL");
        exit(1);
    }
    raydium_log("(my)glut: EGL %i.%i", major, minor);
    if (!egl_choose_config()) {
        raydium_log("(my)glut: ERROR: no suitable EGL config");
        exit(1);
    }
    egl_ctx = eglCreateContext(egl_dpy, egl_cfg, EGL_NO_CONTEXT, ctx_attr);
    if (egl_ctx == EGL_NO_CONTEXT) {
        raydium_log("(my)glut: ERROR: cannot create an OpenGL ES 2 context");
        exit(1);
    }
}

#ifdef __ANDROID__
/* ================================================================ ANDROID */
#define LOGTAG "ManiaDrive"
static int app_resumed = 1, app_has_window = 0, app_focused = 1;

static void android_pause_audio(int pause)
{
    typedef void (*pause_fn)(ALCdevice *);
    static pause_fn p_pause, p_resume;
    ALCcontext *c = alcGetCurrentContext();
    ALCdevice *d = c ? alcGetContextsDevice(c) : NULL;
    if (!d || !alcIsExtensionPresent(d, "ALC_SOFT_pause_device")) return;
    if (!p_pause) {
        p_pause = (pause_fn)alcGetProcAddress(d, "alcDevicePauseSOFT");
        p_resume = (pause_fn)alcGetProcAddress(d, "alcDeviceResumeSOFT");
    }
    if (pause && p_pause) p_pause(d);
    if (!pause && p_resume) p_resume(d);
}

static void android_create_surface(void)
{
    ANativeWindow *win = port_app->window;
    int ww, wh;
    EGLint fmt;

    if (!win || egl_dpy == EGL_NO_DISPLAY) return;
    if (egl_surf != EGL_NO_SURFACE) return;

    /* full window size first (touch coordinates are in this space) */
    ANativeWindow_setBuffersGeometry(win, 0, 0, 0);
    ww = ANativeWindow_getWidth(win);
    wh = ANativeWindow_getHeight(win);
    view_w = ww > 0 ? ww : 1;
    view_h = wh > 0 ? wh : 1;
    render_w = ww;
    render_h = wh;
    if (render_h > render_max_h) {      /* hardware scaler does the rest */
        render_w = (int)((float)ww * render_max_h / wh + 0.5f) & ~1;
        render_h = render_max_h;
    }
    eglGetConfigAttrib(egl_dpy, egl_cfg, EGL_NATIVE_VISUAL_ID, &fmt);
    ANativeWindow_setBuffersGeometry(win, render_w, render_h, fmt);

    /* ask the system for a 60 Hz display mode (Android 11+) */
    {
        typedef int32_t (*setfr_fn)(ANativeWindow *, float, int8_t);
        setfr_fn f = (setfr_fn)dlsym(RTLD_DEFAULT, "ANativeWindow_setFrameRate");
        if (f) f(win, (float)PORT_FPS, 1 /* FIXED_SOURCE */);
    }

    egl_surf = eglCreateWindowSurface(egl_dpy, egl_cfg, win, NULL);
    if (egl_surf == EGL_NO_SURFACE || !eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
        __android_log_print(ANDROID_LOG_ERROR, LOGTAG, "eglCreateWindowSurface/eglMakeCurrent failed: 0x%x", eglGetError());
        exit(1);
    }
    eglSwapInterval(egl_dpy, 1);
    eglQuerySurface(egl_dpy, egl_surf, EGL_WIDTH, &render_w);
    eglQuerySurface(egl_dpy, egl_surf, EGL_HEIGHT, &render_h);
    __android_log_print(ANDROID_LOG_INFO, LOGTAG, "surface %ix%i (screen %ix%i)", render_w, render_h, view_w, view_h);
}

static void android_destroy_surface(void)
{
    if (egl_surf == EGL_NO_SURFACE) return;
    eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(egl_dpy, egl_surf);
    egl_surf = EGL_NO_SURFACE;
}

void port_handle_cmd(struct android_app *app, int32_t cmd)
{
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        app_has_window = 1;
        android_create_surface();
        break;
    case APP_CMD_TERM_WINDOW:
        app_has_window = 0;
        android_destroy_surface();
        port_touch_cancel_all();
        break;
    case APP_CMD_RESUME:
        app_resumed = 1;
        break;
    case APP_CMD_PAUSE:
        app_resumed = 0;
        port_touch_cancel_all();
        break;
    case APP_CMD_GAINED_FOCUS:
        app_focused = 1;
        break;
    case APP_CMD_LOST_FOCUS:
        app_focused = 0;
        port_touch_cancel_all();
        break;
    case APP_CMD_WINDOW_RESIZED:
    case APP_CMD_CONFIG_CHANGED:
        if (app_has_window) {           /* rebuild the surface at the new size */
            android_destroy_surface();
            android_create_surface();
        }
        break;
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_DESTROY:
        break;
    }
    (void)app;
}

/* --- keyboard / gamepad buttons */
static int android_keycode_char(int32_t code, int32_t meta)
{
    int shift = (meta & AMETA_SHIFT_ON) != 0;
    if (code >= AKEYCODE_A && code <= AKEYCODE_Z)
        return (shift ? 'A' : 'a') + (code - AKEYCODE_A);
    if (code >= AKEYCODE_0 && code <= AKEYCODE_9 && !shift)
        return '0' + (code - AKEYCODE_0);
    switch (code) {
    case AKEYCODE_SPACE:        return ' ';
    case AKEYCODE_PERIOD:       return '.';
    case AKEYCODE_COMMA:        return ',';
    case AKEYCODE_MINUS:        return shift ? '_' : '-';
    case AKEYCODE_EQUALS:       return shift ? '+' : '=';
    case AKEYCODE_SLASH:        return '/';
    case AKEYCODE_SEMICOLON:    return shift ? ':' : ';';
    case AKEYCODE_APOSTROPHE:   return '\'';
    case AKEYCODE_AT:           return '@';
    case AKEYCODE_ENTER:
    case AKEYCODE_NUMPAD_ENTER: return 13;
    case AKEYCODE_DEL:          return 8;
    case AKEYCODE_TAB:          return 9;
    case AKEYCODE_ESCAPE:       return 27;
    }
    return 0;
}

static int android_key_special(int32_t code)
{
    switch (code) {
    case AKEYCODE_DPAD_UP:    return GLUT_KEY_UP;
    case AKEYCODE_DPAD_DOWN:  return GLUT_KEY_DOWN;
    case AKEYCODE_DPAD_LEFT:  return GLUT_KEY_LEFT;
    case AKEYCODE_DPAD_RIGHT: return GLUT_KEY_RIGHT;
    case AKEYCODE_F1: return GLUT_KEY_F1;  case AKEYCODE_F2: return GLUT_KEY_F2;
    case AKEYCODE_F3: return GLUT_KEY_F3;  case AKEYCODE_F4: return GLUT_KEY_F4;
    case AKEYCODE_F5: return GLUT_KEY_F5;  case AKEYCODE_F6: return GLUT_KEY_F6;
    case AKEYCODE_F7: return GLUT_KEY_F7;  case AKEYCODE_F8: return GLUT_KEY_F8;
    case AKEYCODE_F9: return GLUT_KEY_F9;  case AKEYCODE_F10: return GLUT_KEY_F10;
    case AKEYCODE_F11: return GLUT_KEY_F11; case AKEYCODE_F12: return GLUT_KEY_F12;
    case AKEYCODE_PAGE_UP: return GLUT_KEY_PAGE_UP;
    case AKEYCODE_PAGE_DOWN: return GLUT_KEY_PAGE_DOWN;
    case AKEYCODE_MOVE_HOME: return GLUT_KEY_HOME;
    case AKEYCODE_MOVE_END: return GLUT_KEY_END;
    case AKEYCODE_INSERT: return GLUT_KEY_INSERT;
    }
    return -1;
}

static int32_t android_on_key(AInputEvent *ev)
{
    int32_t code = AKeyEvent_getKeyCode(ev);
    int32_t action = AKeyEvent_getAction(ev);
    int32_t meta = AKeyEvent_getMetaState(ev);
    int32_t src = AInputEvent_getSource(ev);
    int down = (action == AKEY_EVENT_ACTION_DOWN);
    int repeat = AKeyEvent_getRepeatCount(ev) > 0;
    int sp, ch;

    if (action == AKEY_EVENT_ACTION_MULTIPLE) return 0;

    if (code == AKEYCODE_VOLUME_UP || code == AKEYCODE_VOLUME_DOWN ||
        code == AKEYCODE_VOLUME_MUTE || code == AKEYCODE_HOME)
        return 0;                       /* let the system handle these */

    if ((src & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD ||
        (code >= AKEYCODE_BUTTON_A && code <= AKEYCODE_BUTTON_MODE))
        port_gamepad_used();

    /* gamepad buttons */
    switch (code) {
    case AKEYCODE_BACK:
    case AKEYCODE_BUTTON_SELECT:
    case AKEYCODE_BUTTON_B:
        if (code == AKEYCODE_BUTTON_B && raydium_gui_isvisible()) { if (down && !repeat) port_key_char(27); return 1; }
        if (code == AKEYCODE_BUTTON_B) { port_key_special(GLUT_KEY_DOWN, down); return 1; }
        if (down && !repeat) port_key_char(27);
        return 1;
    case AKEYCODE_BUTTON_A:
        if (raydium_gui_isvisible()) { if (down && !repeat) port_key_char(13); return 1; }
        /* fall through */
    case AKEYCODE_BUTTON_R2:
        port_key_special(GLUT_KEY_UP, down);
        return 1;
    case AKEYCODE_BUTTON_L2:
        port_key_special(GLUT_KEY_DOWN, down);
        return 1;
    case AKEYCODE_BUTTON_X:
        if (down && !repeat) port_key_char(' ');   /* restart track */
        return 1;
    case AKEYCODE_BUTTON_Y:
        if (down && !repeat) port_key_char('c');   /* change camera */
        return 1;
    case AKEYCODE_BUTTON_START:
        if (down && !repeat) port_key_char(raydium_gui_isvisible() ? 13 : 27);
        return 1;
    case AKEYCODE_BUTTON_L1:
    case AKEYCODE_BUTTON_R1:
        return 1;
    }

    sp = android_key_special(code);
    if (sp >= 0) {
        if (!repeat || !down) port_key_special(sp, down);
        if (down && (sp == GLUT_KEY_UP || sp == GLUT_KEY_DOWN) && raydium_gui_isvisible())
            port_key_char(9);           /* menus: move focus */
        return 1;
    }
    ch = android_keycode_char(code, meta);
    if (ch && down) {
        port_key_char(ch);
        return 1;
    }
    return ch ? 1 : 0;
}

/* --- gamepad sticks / triggers */
static float axis_dz(float v, float dz)
{
    if (v > dz)  return (v - dz) / (1 - dz);
    if (v < -dz) return (v + dz) / (1 - dz);
    return 0;
}

static int32_t android_on_joystick(AInputEvent *ev)
{
    float x = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_X, 0);
    float hx = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_HAT_X, 0);
    float hy = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_HAT_Y, 0);
    float gas = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_RTRIGGER, 0);
    float brk = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_LTRIGGER, 0);
    float g2 = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_GAS, 0);
    float b2 = AMotionEvent_getAxisValue(ev, AMOTION_EVENT_AXIS_BRAKE, 0);
    static int hat_l, hat_r, hat_u, hat_d;
    int l, r, u, d;

    if (g2 > gas) gas = g2;
    if (b2 > brk) brk = b2;
    port_gamepad_used();
    port_analog_x = axis_dz(x, 0.15f);
    port_analog_y = (gas > 0.05f ? gas : 0) - (brk > 0.05f ? brk : 0);
    port_analog_active = (port_analog_x != 0 || port_analog_y != 0);

    l = hx < -0.5f; r = hx > 0.5f; u = hy < -0.5f; d = hy > 0.5f;
    if (l != hat_l) port_key_special(GLUT_KEY_LEFT, l);
    if (r != hat_r) port_key_special(GLUT_KEY_RIGHT, r);
    if (u != hat_u) { port_key_special(GLUT_KEY_UP, u); if (u && raydium_gui_isvisible()) port_key_char(9); }
    if (d != hat_d) { port_key_special(GLUT_KEY_DOWN, d); if (d && raydium_gui_isvisible()) port_key_char(9); }
    hat_l = l; hat_r = r; hat_u = u; hat_d = d;
    return 1;
}

static int32_t android_on_touch(AInputEvent *ev)
{
    port_mouse_cursor = 0;
    int32_t action = AMotionEvent_getAction(ev);
    int32_t masked = action & AMOTION_EVENT_ACTION_MASK;
    int idx = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
    size_t i, n = AMotionEvent_getPointerCount(ev);

#define NX(k) (AMotionEvent_getX(ev, k) / (float)view_w)
#define NY(k) (AMotionEvent_getY(ev, k) / (float)view_h)
    switch (masked) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
        port_touch(AMotionEvent_getPointerId(ev, idx), NX(idx), NY(idx), PORT_TOUCH_DOWN);
        break;
    case AMOTION_EVENT_ACTION_MOVE:
        for (i = 0; i < n; i++)
            port_touch(AMotionEvent_getPointerId(ev, i), NX(i), NY(i), PORT_TOUCH_MOVE);
        break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP:
        port_touch(AMotionEvent_getPointerId(ev, idx), NX(idx), NY(idx), PORT_TOUCH_UP);
        break;
    case AMOTION_EVENT_ACTION_CANCEL:
        port_touch_cancel_all();
        break;
    }
#undef NX
#undef NY
    return 1;
}

int32_t port_handle_input(struct android_app *app, AInputEvent *ev)
{
    int32_t type = AInputEvent_getType(ev);
    int32_t src = AInputEvent_getSource(ev);
    (void)app;
    if (type == AINPUT_EVENT_TYPE_KEY)
        return android_on_key(ev);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        if ((src & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK)
            return android_on_joystick(ev);
        if ((src & AINPUT_SOURCE_TOUCHSCREEN) == AINPUT_SOURCE_TOUCHSCREEN)
            return android_on_touch(ev);
        if ((src & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE) {
            int x = (int)(AMotionEvent_getX(ev, 0) * render_w / view_w);
            int y = (int)(AMotionEvent_getY(ev, 0) * render_h / view_h);
            int32_t a = AMotionEvent_getAction(ev) & AMOTION_EVENT_ACTION_MASK;
            port_mouse_cursor = 1;
            if (a == AMOTION_EVENT_ACTION_DOWN) port_mouse(1, x, y);
            else if (a == AMOTION_EVENT_ACTION_UP) port_mouse(0, x, y);
            else port_mouse_move(x, y);
            return 1;
        }
    }
    return 0;
}

/* characters typed on the soft keyboard (sent by MainActivity.java) */
static pthread_mutex_t char_lock = PTHREAD_MUTEX_INITIALIZER;
static int char_queue[64], char_count;

JNIEXPORT void JNICALL
Java_org_maniadrive_android_MainActivity_nativeChar(JNIEnv *env, jclass cls, jint ch)
{
    (void)env; (void)cls;
    pthread_mutex_lock(&char_lock);
    if (char_count < 64) char_queue[char_count++] = ch;
    pthread_mutex_unlock(&char_lock);
}

static void android_flush_chars(void)
{
    int q[64], n, i;
    pthread_mutex_lock(&char_lock);
    n = char_count;
    memcpy(q, char_queue, n * sizeof(int));
    char_count = 0;
    pthread_mutex_unlock(&char_lock);
    for (i = 0; i < n; i++) {
        if (q[i] == '\n') q[i] = 13;
        if (q[i] == 8 || q[i] == 13 || (q[i] >= 32 && q[i] < 127))
            port_key_char(q[i]);
    }
}

/* calls MainActivity.showKeyboard(boolean) through JNI */
void port_show_keyboard(int show)
{
    JavaVM *vm = port_app->activity->vm;
    JNIEnv *env = NULL;
    jclass cls;
    jmethodID mid;
    if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK || !env) return;
    cls = (*env)->GetObjectClass(env, port_app->activity->clazz);
    mid = (*env)->GetMethodID(env, cls, "showKeyboard", "(Z)V");
    if (mid) (*env)->CallVoidMethod(env, port_app->activity->clazz, mid, (jboolean)(show != 0));
    if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
    (*env)->DeleteLocalRef(env, cls);
}

static int android_running(void)
{
    return app_resumed && app_has_window && egl_surf != EGL_NO_SURFACE;
}

static void android_pump(int block_while_paused)
{
    int paused = 0;
    for (;;) {
        int events, ret, timeout;
        struct android_poll_source *source = NULL;
        int running = android_running();
        if (block_while_paused && !running && !paused) {
            /* app in background / no window: freeze game time and sound */
            time_pause(1);
            android_pause_audio(1);
            port_touch_cancel_all();
            paused = 1;
        }
        timeout = (!block_while_paused || running) ? 0 : -1;
        ret = ALooper_pollOnce(timeout, NULL, &events, (void **)&source);
        if (ret == ALOOPER_POLL_TIMEOUT || ret == ALOOPER_POLL_ERROR) {
            if (timeout == 0) break;
            continue;
        }
        if (ret >= 0 && source)
            source->process(port_app, source);
        if (port_app->destroyRequested) {
            raydium_log("(my)glut: activity destroyed, exiting");
            exit(0);
        }
    }
    if (paused) {
        android_pause_audio(0);
        time_pause(0);
    }
    android_flush_chars();
}

/* exit cleanly: finish the activity first, so Android does not restart it */
void port_exit(int code)
{
    static int exiting;
    if (exiting) exit(code);
    exiting = 1;
    raydium_log("(my)glut: quit requested (%i)", code);
    ANativeActivity_finish(port_app->activity);
    {
        uint64_t until = mono_us() + 1500000;     /* give the activity time to finish */
        while (!port_app->destroyRequested && mono_us() < until) {
            int events; struct android_poll_source *source = NULL;
            int ret = ALooper_pollOnce(50, NULL, &events, (void **)&source);
            if (ret >= 0 && source) source->process(port_app, source);
        }
    }
    exit(code);
}

#else
/* ================================================================ HEADLESS */
/* Scripted input: MANIA_SCRIPT=file with lines "<frame> <command> [args]"
 *   tap X Y          touch down at frame, up 3 frames later (0..1 coords)
 *   down ID X Y | move ID X Y | up ID X Y
 *   special KEY 0|1  (up, down, left, right, f1..f12)
 *   char C           type character C
 *   key N            type ascii code N (8 backspace, 13 enter, 27 escape)
 *   shot FILE.ppm    screenshot
 *   quit
 * MANIA_SIZE=WxH (default 1280x720), MANIA_FRAMES=N (exit after N frames),
 * MANIA_SHOTS=dir (where relative screenshot names are written)
 */
typedef struct { int frame; char cmd[16]; char a[256]; float x, y; int id, v; } script_ev;
static script_ev script[1024];
static int script_n, frame_no, max_frames;

static void headless_load_script(void)
{
    const char *path = getenv("MANIA_SCRIPT");
    char line[512];
    FILE *fp;
    if (!path || !(fp = fopen(path, "rt"))) return;
    while (fgets(line, sizeof line, fp) && script_n < 1024) {
        script_ev *e = &script[script_n];
        memset(e, 0, sizeof *e);
        if (line[0] == '#' || sscanf(line, "%d %15s", &e->frame, e->cmd) < 2) continue;
        if (!strcmp(e->cmd, "tap")) sscanf(line, "%*d %*s %f %f", &e->x, &e->y);
        else if (!strcmp(e->cmd, "down") || !strcmp(e->cmd, "move") || !strcmp(e->cmd, "up"))
            sscanf(line, "%*d %*s %d %f %f", &e->id, &e->x, &e->y);
        else if (!strcmp(e->cmd, "special")) sscanf(line, "%*d %*s %255s %d", e->a, &e->v);
        else sscanf(line, "%*d %*s %255s", e->a);
        script_n++;
        if (!strcmp(e->cmd, "tap") && script_n < 1024) {      /* auto "up" */
            script_ev *u = &script[script_n++];
            *u = *e;
            u->frame += 3;
            strcpy(u->cmd, "up");
            u->id = 9;
            strcpy(e->cmd, "down");
            e->id = 9;
        }
    }
    fclose(fp);
    raydium_log("headless: %i scripted events", script_n);
}

static void headless_shot(const char *name)
{
    unsigned char *px = malloc(render_w * render_h * 4);
    char file[1024];
    const char *dir = getenv("MANIA_SHOTS");
    FILE *fp;
    int y;
    if (name[0] != '/' && dir) snprintf(file, sizeof file, "%s/%s", dir, name);
    else snprintf(file, sizeof file, "%s", name);
    glReadPixels(0, 0, render_w, render_h, GL_RGBA, GL_UNSIGNED_BYTE, px);
    fp = fopen(file, "wb");
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", render_w, render_h);
        for (y = render_h - 1; y >= 0; y--) {
            int x;
            for (x = 0; x < render_w; x++) fwrite(px + (y * render_w + x) * 4, 1, 3, fp);
        }
        fclose(fp);
        raydium_log("headless: screenshot %s (frame %i)", file, frame_no);
    }
    free(px);
}

static int special_from_name(const char *s)
{
    if (!strcmp(s, "up")) return GLUT_KEY_UP;
    if (!strcmp(s, "down")) return GLUT_KEY_DOWN;
    if (!strcmp(s, "left")) return GLUT_KEY_LEFT;
    if (!strcmp(s, "right")) return GLUT_KEY_RIGHT;
    if (s[0] == 'f') return atoi(s + 1);
    return atoi(s);
}

static void headless_events(int after_swap)
{
    int i;
    for (i = 0; i < script_n; i++) {
        script_ev *e = &script[i];
        int shot = !strcmp(e->cmd, "shot");
        if (e->frame != frame_no || shot != after_swap) continue;
        if (!strcmp(e->cmd, "down")) port_touch(e->id, e->x, e->y, PORT_TOUCH_DOWN);
        else if (!strcmp(e->cmd, "move")) port_touch(e->id, e->x, e->y, PORT_TOUCH_MOVE);
        else if (!strcmp(e->cmd, "up")) port_touch(e->id, e->x, e->y, PORT_TOUCH_UP);
        else if (!strcmp(e->cmd, "special")) port_key_special(special_from_name(e->a), e->v);
        else if (!strcmp(e->cmd, "char")) port_key_char(e->a[0]);
        else if (!strcmp(e->cmd, "key")) port_key_char(atoi(e->a));
        else if (shot) headless_shot(e->a);
        else if (!strcmp(e->cmd, "quit")) exit(0);
    }
}

void port_show_keyboard(int show) { raydium_log("headless: soft keyboard %s", show ? "shown" : "hidden"); }
void port_exit(int code) { exit(code); }
#endif

/* ================================================================ myglut API */
void glutInit(int *argc, char **argv)
{
    (void)argc; (void)argv;
    glutReshapeFuncCB = NULL;
    glutKeyboardFuncCB = NULL;
    glutSpecialUpFuncCB = NULL;
    glutSpecialFuncCB = NULL;
    glutMotionFuncCB = NULL;
    glutPassiveMotionFuncCB = NULL;
    glutMouseFuncCB = NULL;
    glutDisplayFuncCB = NULL;
    glutIdleFuncCB = NULL;
    _glutMouseVisible = 1;
}

void mylgutCloseWindow(void)
{
}

void glutSetCursor(int cursor)
{
    _glutMouseVisible = (cursor == GLUT_CURSOR_NONE) ? 0 : 1;
}

void glutWarpPointer(int x, int y)
{
    port_mouse_move(x, y);
}

void glutSwapBuffers(void)
{
    port_overlay_frame();
    port_overlay_draw();
    glFlush();                          /* flushes gl4es' batched commands */
#ifdef __ANDROID__
    if (egl_surf != EGL_NO_SURFACE && !eglSwapBuffers(egl_dpy, egl_surf)) {
        EGLint err = eglGetError();
        __android_log_print(ANDROID_LOG_WARN, LOGTAG, "eglSwapBuffers failed: 0x%x", err);
        if (err == EGL_CONTEXT_LOST) {
            raydium_log("(my)glut: OpenGL context lost, restarting");
            exit(0);
        }
    }
#else
    eglSwapBuffers(egl_dpy, egl_surf);
    headless_events(1);
    frame_no++;
    if (max_frames && frame_no >= max_frames) {
        raydium_log("headless: %i frames done", frame_no);
        exit(0);
    }
#endif
    frame_limiter();
}

void pwInit(int x, int y, int w, int h, int multisample,
            char *title, int border, int num_samples)
{
    (void)x; (void)y; (void)multisample; (void)title; (void)border; (void)num_samples;

    /* "windowed" mode (Options menu) is used as a resolution limit on Android */
    if (w > 0 && h > 0 && h < render_max_h)
        render_max_h = h;
    if (render_max_h < 240) render_max_h = 240;

    /* gl4es is a static library here and must be initialised by hand.
       It probes the GPU with its own temporary EGL context (and terminates
       the display afterwards), so this is done before our context exists. */
    set_getprocaddress((void *(*)(const char *))eglGetProcAddress);
    initialize_gl4es();

#ifdef __ANDROID__
    egl_init_display();
    /* wait for the window */
    while (!port_app->window) {
        int events; struct android_poll_source *source = NULL;
        int ret = ALooper_pollOnce(-1, NULL, &events, (void **)&source);
        if (ret >= 0 && source) source->process(port_app, source);
        if (port_app->destroyRequested) exit(0);
    }
    app_has_window = 1;
    android_create_surface();
#else
    {
        const char *size = getenv("MANIA_SIZE");
        EGLint pb[] = { EGL_WIDTH, 1280, EGL_HEIGHT, 720, EGL_NONE };
        render_w = 1280; render_h = 720;
        if (size) sscanf(size, "%dx%d", &render_w, &render_h);
        pb[1] = render_w; pb[3] = render_h;
        view_w = render_w; view_h = render_h;
        if (getenv("MANIA_FRAMES")) max_frames = atoi(getenv("MANIA_FRAMES"));
        egl_init_display();
        egl_surf = eglCreatePbufferSurface(egl_dpy, egl_cfg, pb);
        if (egl_surf == EGL_NO_SURFACE || !eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
            raydium_log("(my)glut: ERROR: cannot create pbuffer (0x%x)", eglGetError());
            exit(1);
        }
        headless_load_script();
    }
#endif

    /* gl4es is a static library here: it must be initialised by hand,
       after a context is current */

    _glutWindowSize[0] = render_w;
    _glutWindowSize[1] = render_h;
    last_reported_w = render_w;
    last_reported_h = render_h;
    glViewport(0, 0, render_w, render_h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    raydium_log("(my)glut: %ix%i OpenGL ES 2 surface (gl4es), locked to %i fps",
                render_w, render_h, PORT_FPS);
}

void myglutGetEvents(void)
{
#ifdef __ANDROID__
    android_pump(1);
    if (render_w != last_reported_w || render_h != last_reported_h) {
        last_reported_w = render_w;
        last_reported_h = render_h;
        _glutWindowSize[0] = render_w;
        _glutWindowSize[1] = render_h;
        if (glutReshapeFuncCB) glutReshapeFuncCB(render_w, render_h);
    }
#else
    headless_events(0);
#endif
}

/* touch coordinates (0..1) -> GL window pixels */
int port_window_x(float nx) { return (int)(nx * render_w); }
int port_window_y(float ny) { return (int)(ny * render_h); }
float port_aspect(void) { return render_h ? (float)render_w / render_h : 1.f; }
