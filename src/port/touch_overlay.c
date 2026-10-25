/*
 * ManiaDrive Android port - touch controls.
 *
 * In menus a finger works like the mouse (tap = left click, drag = move).
 * While driving, semi-transparent buttons are drawn over the game:
 *
 *   [MENU]                                         [RESET] [CAM]
 *
 *
 *   [ < ] [ > ]                                [BRAKE] [ GAS ]
 *   (steering, slide between them)                 (pedals)
 *
 * The buttons send the same keys as the PC version (arrows, space, C, Esc),
 * so the original game logic is untouched. Multi-touch: every finger owns
 * the button it pressed; sliding a finger moves it to the neighbour button.
 *
 * This file is #included by raydium/index.c (MANIA_PORT). GPL v2 or later.
 */

enum { B_NONE = -1, B_LEFT, B_RIGHT, B_BRAKE, B_GAS, B_MENU, B_RESET, B_CAM, B_COUNT };

typedef struct { float x0, y0, x1, y1; } port_rect;   /* OSD units: 0..100, origin bottom-left */

static port_rect port_btn[B_COUNT];
static int   port_btn_down[B_COUNT];
static int   port_ingame_mode;           /* 0 menu, 1 driving, 2 credits */
static int   port_overlay_hidden;        /* gamepad in use */
static int   port_kbd_shown, port_kbd_tap;

#define PORT_MAX_TOUCH 10
static struct { int id, button, mouse; } port_finger[PORT_MAX_TOUCH];
static int port_fingers_init;

int   port_window_x(float nx);
int   port_window_y(float ny);
float port_aspect(void);

void port_set_ingame(int mode) { port_ingame_mode = mode; }
void port_gamepad_used(void)   { port_overlay_hidden = 1; }

static int port_overlay_active(void)
{
    return port_ingame_mode == 1 && !raydium_gui_isvisible();
}

/* layout, recomputed every frame (screen aspect can change) */
static void port_layout(void)
{
    float a = port_aspect();             /* width / height */
    float s = 20.f;                      /* big button size, % of height */
    float sx = s / a;                    /* same size in % of width */
    float m = 2.5f, mx = m / a;          /* margins */
    float gap = 1.5f / a;
    float bottom = 18.f;                 /* above the HUD bar (0..16%) */
    float t = 11.f, tx = 15.f / a;       /* small buttons */

    port_btn[B_LEFT]  = (port_rect){ mx, bottom, mx + sx, bottom + s };
    port_btn[B_RIGHT] = (port_rect){ mx + sx + gap, bottom, mx + 2 * sx + gap, bottom + s };
    port_btn[B_GAS]   = (port_rect){ 100 - mx - sx, bottom, 100 - mx, bottom + s * 1.25f };
    port_btn[B_BRAKE] = (port_rect){ 100 - mx - 2 * sx - gap, bottom, 100 - mx - sx - gap, bottom + s };
    port_btn[B_MENU]  = (port_rect){ mx, 100 - 8 - t, mx + tx, 100 - 8 };
    port_btn[B_CAM]   = (port_rect){ 100 - mx - tx, 100 - 8 - t, 100 - mx, 100 - 8 };
    port_btn[B_RESET] = (port_rect){ 100 - mx - 2 * tx - gap, 100 - 8 - t, 100 - mx - tx - gap, 100 - 8 };
}

static int port_in(const port_rect *r, float x, float y, float grow)
{
    float gx = (r->x1 - r->x0) * grow, gy = (r->y1 - r->y0) * grow;
    return x >= r->x0 - gx && x <= r->x1 + gx && y >= r->y0 - gy && y <= r->y1 + gy;
}

/* x,y in OSD units */
static int port_hit(float x, float y)
{
    int i;
    for (i = B_MENU; i <= B_CAM; i++)
        if (port_in(&port_btn[i], x, y, 0.25f)) return i;
    if (y < 62) {
        /* generous zones: whole lower-left area steers, lower-right area = pedals */
        if (x < 45) {
            float mid = (port_btn[B_LEFT].x1 + port_btn[B_RIGHT].x0) / 2;
            return x < mid ? B_LEFT : B_RIGHT;
        }
        if (x > 55) {
            float mid = (port_btn[B_BRAKE].x1 + port_btn[B_GAS].x0) / 2;
            return x < mid ? B_BRAKE : B_GAS;
        }
    }
    return B_NONE;
}

static void port_button(int b, int down)
{
    int was = port_btn_down[b] > 0;
    port_btn_down[b] += down ? 1 : -1;
    if (port_btn_down[b] < 0) port_btn_down[b] = 0;
    if (was == (port_btn_down[b] > 0)) return;
    down = port_btn_down[b] > 0;
    switch (b) {
    case B_LEFT:  port_key_special(GLUT_KEY_LEFT, down); break;
    case B_RIGHT: port_key_special(GLUT_KEY_RIGHT, down); break;
    case B_GAS:   port_key_special(GLUT_KEY_UP, down); break;
    case B_BRAKE: port_key_special(GLUT_KEY_DOWN, down); break;
    case B_MENU:  if (down) port_key_char(27); break;
    case B_RESET: if (down) port_key_char(' '); break;
    case B_CAM:   if (down) port_key_char('c'); break;
    }
}

static int port_finger_slot(int id, int create)
{
    int i, free_slot = -1;
    if (!port_fingers_init) {
        for (i = 0; i < PORT_MAX_TOUCH; i++) port_finger[i].id = -1;
        port_fingers_init = 1;
    }
    for (i = 0; i < PORT_MAX_TOUCH; i++) {
        if (port_finger[i].id == id) return i;
        if (port_finger[i].id < 0 && free_slot < 0) free_slot = i;
    }
    if (!create || free_slot < 0) return -1;
    port_finger[free_slot].id = id;
    port_finger[free_slot].button = B_NONE;
    port_finger[free_slot].mouse = 0;
    return free_slot;
}

static int port_mouse_owner = -1;

void port_touch(int id, float nx, float ny, int action)
{
    int s, b;
    float x = nx * 100.f, y = (1.f - ny) * 100.f;   /* OSD coordinates */

    if (action == PORT_TOUCH_DOWN) port_overlay_hidden = 0;
    s = port_finger_slot(id, action == PORT_TOUCH_DOWN);
    if (s < 0) return;
    port_layout();

    if (action == PORT_TOUCH_DOWN) {
        if (port_overlay_active()) {
            b = port_hit(x, y);
            port_finger[s].button = b;
            if (b != B_NONE) port_button(b, 1);
        } else if (port_ingame_mode == 2) {
            port_key_char(27);                          /* credits: tap to leave */
        } else if (port_mouse_owner < 0) {
            port_mouse_owner = id;
            port_finger[s].mouse = 1;
            port_mouse(1, port_window_x(nx), port_window_y(ny));
        }
        return;
    }

    if (action == PORT_TOUCH_MOVE) {
        if (port_finger[s].mouse) {
            port_mouse_move(port_window_x(nx), port_window_y(ny));
        } else if (port_finger[s].button != B_NONE || port_overlay_active()) {
            int old = port_finger[s].button;
            b = port_hit(x, y);
            /* only the driving controls follow a sliding finger */
            if (b != old && (old == B_NONE || old <= B_GAS) && (b == B_NONE || b <= B_GAS)) {
                if (old != B_NONE) port_button(old, 0);
                if (b != B_NONE) port_button(b, 1);
                port_finger[s].button = b;
            }
        }
        return;
    }

    /* UP / CANCEL */
    if (port_finger[s].mouse) {
        port_mouse(0, port_window_x(nx), port_window_y(ny));
        port_mouse_owner = -1;
        port_kbd_tap = 3;                               /* check edit focus soon */
    }
    if (port_finger[s].button != B_NONE) port_button(port_finger[s].button, 0);
    port_finger[s].id = -1;
    port_finger[s].button = B_NONE;
    port_finger[s].mouse = 0;
}

void port_touch_cancel_all(void)
{
    int i;
    for (i = 0; i < PORT_MAX_TOUCH; i++)
        if (port_fingers_init && port_finger[i].id >= 0)
            port_touch(port_finger[i].id, 0, 0, PORT_TOUCH_CANCEL);
    for (i = 0; i < B_COUNT; i++)
        if (port_btn_down[i]) { port_btn_down[i] = 1; port_button(i, 0); }
    if (port_mouse_owner >= 0) { port_mouse(0, raydium_mouse_x, raydium_mouse_y); port_mouse_owner = -1; }
}

/* per frame: soft keyboard for GUI text fields, release buttons outside the race */
void port_overlay_frame(void)
{
    int edit = 0;

    if (!port_overlay_active()) {
        int i;
        for (i = 0; i < PORT_MAX_TOUCH; i++)
            if (port_fingers_init && port_finger[i].id >= 0 && port_finger[i].button != B_NONE) {
                port_button(port_finger[i].button, 0);
                port_finger[i].button = B_NONE;
            }
    }

    if (raydium_gui_isvisible() && raydium_gui_window_isvalid(raydium_gui_window_focused)) {
        raydium_gui_Window *w = &raydium_gui_windows[raydium_gui_window_focused];
        int f = w->focused_widget;
        if (f >= 0 && raydium_gui_widget_isvalid(f, raydium_gui_window_focused) &&
            w->widgets[f].type == RAYDIUM_GUI_EDIT)
            edit = 1;
    }
    if (port_kbd_tap > 0) {
        port_kbd_tap--;
        if (edit && !port_kbd_shown) { port_show_keyboard(1); port_kbd_shown = 1; }
    }
    if (!edit && port_kbd_shown) { port_show_keyboard(0); port_kbd_shown = 0; }
}

/* ------------------------------------------------------------ drawing */
static void port_quad(const port_rect *r, float a)
{
    glColor4f(0.05f, 0.05f, 0.08f, a);
    glBegin(GL_QUADS);
    glVertex2f(r->x0, r->y0); glVertex2f(r->x1, r->y0);
    glVertex2f(r->x1, r->y1); glVertex2f(r->x0, r->y1);
    glEnd();
    glColor4f(1, 1, 1, a + 0.25f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(r->x0, r->y0); glVertex2f(r->x1, r->y0);
    glVertex2f(r->x1, r->y1); glVertex2f(r->x0, r->y1);
    glEnd();
}

/* arrow inside r: dir 0=left 1=right 2=up 3=down */
static void port_arrow(const port_rect *r, int dir, float a)
{
    float cx = (r->x0 + r->x1) / 2, cy = (r->y0 + r->y1) / 2;
    float hw = (r->x1 - r->x0) * 0.25f, hh = (r->y1 - r->y0) * 0.25f;
    glColor4f(1, 1, 1, a);
    glBegin(GL_TRIANGLES);
    switch (dir) {
    case 0: glVertex2f(cx - hw, cy); glVertex2f(cx + hw, cy + hh); glVertex2f(cx + hw, cy - hh); break;
    case 1: glVertex2f(cx + hw, cy); glVertex2f(cx - hw, cy - hh); glVertex2f(cx - hw, cy + hh); break;
    case 2: glVertex2f(cx, cy + hh); glVertex2f(cx - hw, cy - hh); glVertex2f(cx + hw, cy - hh); break;
    case 3: glVertex2f(cx, cy - hh); glVertex2f(cx + hw, cy + hh); glVertex2f(cx - hw, cy + hh); break;
    }
    glEnd();
}

static void port_label(const port_rect *r, const char *txt, float size)
{
    /* raydium_osd_printf: glyph i is centred on x + i*2*s*spacer, s=size/12 */
    float s = size / RAYDIUM_OSD_FONT_SIZE_FACTOR;
    float span = (strlen(txt) - 1) * 2 * s * 0.5f;
    float cx = (r->x0 + r->x1) / 2, cy = (r->y0 + r->y1) / 2;
    raydium_osd_printf(cx - span / 2, cy, size, 0.5f, "font-impact.tga", "^f%s", (char *)txt);
}

void port_overlay_draw(void)
{
    int i;
    static const int arrow_dir[4] = { 0, 1, 3, 2 };
    if (!port_overlay_active() || port_overlay_hidden) return;
    port_layout();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    raydium_osd_start();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (i = 0; i < B_COUNT; i++) {
        float a = port_btn_down[i] ? 0.55f : 0.28f;
        port_quad(&port_btn[i], a);
        if (i <= B_GAS) port_arrow(&port_btn[i], arrow_dir[i], port_btn_down[i] ? 0.95f : 0.7f);
    }
    raydium_osd_stop();

    /* text labels (raydium_osd_printf manages its own state) */
    port_label(&port_btn[B_MENU], "MENU", 14);
    port_label(&port_btn[B_RESET], "RESET", 13);
    port_label(&port_btn[B_CAM], "CAM", 14);
    glPopAttrib();
    glEnable(GL_TEXTURE_2D);
}
