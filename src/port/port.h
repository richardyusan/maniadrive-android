/*
 * ManiaDrive Android port - interfaces shared by the platform layer
 * (myglut-android.c), the touch overlay and the game code.
 * GPL v2 or later, like ManiaDrive.
 */
#ifndef MANIA_PORT_H
#define MANIA_PORT_H

#include <stdint.h>

/* Target frame rate. The display is locked to this value. */
#define PORT_FPS 60

/* --- time (monotonic, microseconds, does not advance while the app is paused) */
uint64_t port_time_us(void);

/* --- game -> port */
void port_set_ingame(int driving);      /* called every frame by mania_drive.c */
void port_exit(int code);               /* clean exit (finishes the Android activity) */

/* --- analog input (gamepad sticks/triggers), merged into raydium_joy_x/y */
extern float port_analog_x, port_analog_y;
extern int   port_analog_active;
extern int   port_mouse_cursor;          /* 1 when a real mouse is used */

/* --- touch overlay (touch_overlay.c) */
#define PORT_TOUCH_DOWN   0
#define PORT_TOUCH_MOVE   1
#define PORT_TOUCH_UP     2
#define PORT_TOUCH_CANCEL 3
void port_touch(int id, float nx, float ny, int action); /* nx,ny in 0..1, origin top-left */
void port_touch_cancel_all(void);
void port_overlay_draw(void);           /* called just before each buffer swap */
void port_overlay_frame(void);          /* per-frame logic (keyboard requests, ...) */
void port_gamepad_used(void);           /* hides the overlay until the next touch */

/* --- platform services (myglut-android.c) */
void port_show_keyboard(int show);
void port_key_special(int glut_key, int down);
void port_key_char(int ch);
void port_mouse(int down, int x, int y);   /* x,y in window pixels */
void port_mouse_move(int x, int y);

/* engine functions used before their definition / PHP replacements */
signed char raydium_gui_isvisible(void);
int raydium_rayphp_repository_file_get(char *path);
signed char raydium_rayphp_http_test(void);

#endif
