/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// window borders & moves
// multiple windows tests
// modal windows

#ifndef RAY_GUI_H
#define RAY_GUI_H

#define RAYDIUM_GUI_MAX_OBJECTS		128
#define RAYDIUM_GUI_MAX_WINDOWS		16

#define RAYDIUM_GUI_BUTTON	1
#define RAYDIUM_GUI_LABEL	2
#define RAYDIUM_GUI_TRACK	3
#define RAYDIUM_GUI_EDIT	4
#define RAYDIUM_GUI_CHECK	5
#define RAYDIUM_GUI_COMBO	6
#define RAYDIUM_GUI_ZONE	7

#define RAYDIUM_GUI_NORMAL	1
#define RAYDIUM_GUI_FOCUS	2
#define RAYDIUM_GUI_HOVER	3

#define RAYDIUM_GUI_DATASIZE	4096

#define RAYDIUM_GUI_ITEM_SEPARATOR	'\n'
#define RAYDIUM_GUI_COMBO_LIST_HEIGHT	5

typedef struct raydium_gui_Button
{
    void *   OnClick;
    char     caption[RAYDIUM_MAX_NAME_LEN];
    GLfloat  uv_normal[4];
    GLfloat  uv_focus[4];
    GLfloat  uv_hover[4];
    GLfloat  font_color[3];
} raydium_gui_Button;

typedef struct raydium_gui_Zone
{
    void *   OnClick;
    GLfloat  col_normal[4];
    GLfloat  col_focus[4];
    GLfloat  col_hover[4];
    int	     tag;
} raydium_gui_Zone;

typedef struct raydium_gui_Label
{
    char     caption[RAYDIUM_MAX_NAME_LEN];
    GLfloat  font_color[3];
} raydium_gui_Label;

typedef struct raydium_gui_Track
{
    GLfloat  uv_rule[4];
    GLfloat  uv_cursor_normal[4];
    GLfloat  uv_cursor_focus[4];
    int	     min;
    int      max;
    int      current;
} raydium_gui_Track;

typedef struct raydium_gui_Edit
{
    char     text[RAYDIUM_GUI_DATASIZE];
    GLfloat  uv_normal[4];
    GLfloat  uv_focus[4];
    GLfloat  font_color[3];
    int      cursor;
    int      offset;
} raydium_gui_Edit;

typedef struct raydium_gui_Check
{
    char     caption[RAYDIUM_MAX_NAME_LEN];
    signed char     checked;
    GLfloat  uv_normal[4];
    GLfloat  uv_checked[4];
    GLfloat  font_color_normal[3];
    GLfloat  font_color_focus[3];
} raydium_gui_Check;

typedef struct raydium_gui_Combo
{
    char     items[RAYDIUM_GUI_DATASIZE];
    int      current;
    char     current_str[RAYDIUM_GUI_DATASIZE]; // provided as a "bonus"
    signed char     expanded;
    int      offset;
    GLfloat  uv_body_normal[4];
    GLfloat  uv_body_focus[4];
    int      body_border_right;
    GLfloat  uv_list_top[4];
    GLfloat  uv_list_middle[4];
    GLfloat  uv_list_bottom[4];
    GLfloat  uv_list_current[4];
    GLfloat  uv_arrow[4];
    GLfloat  font_color[3];
} raydium_gui_Combo;

// Parent of all widgets
typedef struct raydium_gui_Object
{
    int      id;
    char     name[RAYDIUM_MAX_NAME_LEN];
    signed char     state;
    signed char     type;
    int      window;
    GLfloat  pos[2];
    GLfloat  size[2];
    GLfloat  font_size;
    void *   widget;
} raydium_gui_Object;


typedef struct raydium_gui_Window
{
    int      id;
    char     name[RAYDIUM_MAX_NAME_LEN];
    signed char     state;
    GLfloat  pos[2];
    GLfloat  size[2];
    raydium_gui_Object widgets[RAYDIUM_GUI_MAX_OBJECTS];
    int      focused_widget;
    int	     old_focused;
    void *   OnDelete;
} raydium_gui_Window;


typedef struct raydium_gui_Theme
{
    signed char     loaded;
    char     filename[RAYDIUM_MAX_NAME_LEN];
    int      texture;
    int      texture_size[2];
    GLfloat  background_uv[4];
    char     font[RAYDIUM_MAX_NAME_LEN];
} raydium_gui_Theme;

__global raydium_gui_Theme  raydium_gui_theme_current;
__global raydium_gui_Window raydium_gui_windows[RAYDIUM_GUI_MAX_WINDOWS];
__global signed char        raydium_gui_visible;
__global int                raydium_gui_window_focused;
__global signed char	    raydium_gui_oldstate;
__global GLfloat	    raydium_gui_widget_sizes_default[3];
__global int		    raydium_gui_button_clicked_id;
__global void *		    raydium_gui_AfterGuiDrawCallback;

#endif
