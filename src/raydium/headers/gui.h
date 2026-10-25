#ifndef _GUI_H
#define _GUI_H

#include "../gui.h"

/*=
Graphic User Interfaces
3200
**/

// Introduction
/**
Raydium provides a support for simple GUI definitions thru a set of
functions (RayPHP interface is available).
Raydium's GUI are themable, using ".gui" theme text files. A default "full"
theme is provided as "theme-raydium2.gui" (and suitable ".tga" file) on the 
data repository.
Complete informations about theme building are readable in this file.
**/

// Vocabulary
/**
This API will allow declaration of:
- "widgets" (label, button, edit box, track bar, check box, combo box, zone)
- "windows" (containers for widgets)

"Focus" is supported for windows and widgets. The final user will not have
any control on windows focus. "Tab" key is used for widget focus cycling.

Widgets and windows are identified by a name or by a unique numeric id.
**/


// Building
/**
The idea is simple: build a window (position and size), and create
widgets over this window.
All widgets are created using the current sizes (x,y and font). See 
suitable function).
Buttons provides a simple callback, and all other widgets (but label)
provides an unified "read" function. Window deletion is also possible.

You must set current theme before any of this operations (see below).
A void(void) callback is available if you want to draw something **over**
the GUI, named ##raydium_gui_AfterGuiDrawCallback##.
**/

__rayapi void raydium_gui_window_init(int window);
/**
Internal use. Will reset ##window##.
**/

__rayapi void raydium_gui_init(void);
/**
Internal use. Will init all GUI API. Called once by Raydium.
**/

__rayapi void raydium_gui_theme_init(void);
/**
Internal use. Will init theme.
**/

__rayapi int raydium_gui_theme_load(char *filename);
/**
This function will load and set current theme (".gui" files). You must load
a theme by yourself, since Raydium will never do it for you.
This function must be called before GUI building.
**/

__rayapi signed char raydium_gui_window_isvalid(int i);
/**
Mostly internal. Will check if ##i## window is valid.
**/

__rayapi int raydium_gui_window_find(char *name);
/**
Will search ##name## window's numeric id.
**/

__rayapi void raydium_gui_window_OnDelete(int window, void *OnDelete);
/**
This function sets OnDelete callback for ##window## deletion.
This callback must follow void f(void) prototype. The call is done **before**
window deletion.
**/

__rayapi void raydium_gui_window_OnDelete_name(char *window, void *OnDelete);
/**
Same as above, but using ##window## name.
**/


__rayapi signed char raydium_gui_widget_isvalid(int i, int window);
/**
Mostly internal. Will check if ##i## widget of ##window## is valid.
**/

__rayapi int raydium_gui_widget_find(char *name, int window);
/**
Will search ##name## widget numeric id (for ##window##).
**/

__rayapi void raydium_gui_widget_next(void);
/**
Mostly internal. Cycle focus.
**/

__rayapi void raydium_gui_widget_draw_internal(GLfloat *uv, GLfloat *xy);
/**
Internal use. Generic drawing function.
**/

__rayapi void raydium_gui_button_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_track_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_label_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_edit_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_check_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_combo_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_zone_draw(int w, int window);
/**
Internal use.
**/

__rayapi void raydium_gui_window_draw(int window);
/**
Internal use.
**/

__rayapi void raydium_gui_draw(void);
/**
Internal use. GUI drawing callback.
**/

__rayapi int raydium_gui_button_read(int window, int widget, char *str);
/**
Internal use. Button read accessor (dummy).
**/

__rayapi int raydium_gui_button_write(int window, int widget, char *str);
/**
Internal use. Button write accessor.
**/

__rayapi int raydium_gui_label_read(int window, int widget, char *str);
/**
Internal use. Label read accessor (dummy).
**/

__rayapi int raydium_gui_label_write(int window, int widget, char *str);
/**
Internal use. Label write accessor.
**/

__rayapi int raydium_gui_track_read(int window, int widget, char *str);
/**
Internal use. Track read accessor.
**/

__rayapi int raydium_gui_track_write(int window, int widget, int value);
/**
Internal use. Track write accessor.
**/

__rayapi int raydium_gui_edit_read(int window, int widget, char *str);
/**
Internal use. Edit read accessor.
**/

__rayapi int raydium_gui_edit_write(int window, int widget, char *str);
/**
Internal use. Edit write accessor.
**/

__rayapi int raydium_gui_check_read(int window, int widget, char *str);
/**
Internal use. Check read accessor.
**/

__rayapi int raydium_gui_check_write(int window, int widget, int value);
/**
Internal use. Check write accessor.
**/

__rayapi int raydium_gui_combo_read(int window, int widget, char *str);
/**
Internal use. Combo read accessor.
**/

__rayapi int raydium_gui_combo_write(int window, int widget, int value);
/**
Internal use. Combo write accessor.
**/

__rayapi int raydium_gui_zone_read(int window, int widget, char *str);
/**
Internal use. Zone read accessor.
**/

__rayapi void raydium_gui_show(void);
/**
Will show current built GUI.
**/

__rayapi void raydium_gui_hide(void);
/**
Will hide current built GUI. This is the default state.
**/

__rayapi signed char raydium_gui_isvisible(void);
/**
Will return current visibility of GUI.
**/

__rayapi void raydium_gui_window_delete(int window);
/**
Will delete ##window##. No further access to widgets is possible.
**/

__rayapi void raydium_gui_window_delete_name(char *window);
/**
Same as above, but using ##window##'s name.
**/

__rayapi void raydium_gui_widget_sizes(GLfloat sizex, GLfloat sizey, GLfloat font_size);
/**
Each widget is created using 3 size: X size, Y size and font size. This
function will allow you to set all sizes for a widget or a group of widget.
Unit: percents (screen)
**/

__rayapi int raydium_gui_window_create(char *name, GLfloat px, GLfloat py, GLfloat sizex, GLfloat sizey);
/**
Obviously, this function will create a new window. This window will take focus
and overlap any previous window.
##px## and ##py## for X and Y position on the screen, and ##sizex## and ##sizey##
for sizes, obviously.
Unit: percents (screen)
**/

__rayapi int raydium_gui_internal_object_create(char *name, int window, signed char type, GLfloat px, GLfloat py, GLfloat sizex, GLfloat sizey, GLfloat font_size);
/**
Internal use.
Small (and ugly) tip: you can build many widgets with the same name, prefixing
the name with '*'.
**/

__rayapi int raydium_gui_button_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, void *OnClick);
/**
This function will create a new button, with ##name## and with ##window## for
parent. 
You need to provide a ##caption## ("title") and a OnClick callback function.
This callback must follow this prototype:
%%(c)void btnButtonClick(raydium_gui_Object *w)%%
You can find ##raydium_gui_Object## structure declaration in ##raydium/gui.h##,
if needed.

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_button_create_simple(char *name, int window,  GLfloat px, GLfloat py, char *caption);
/**
Same as above, but no OnClick callback function is asked. This type of button 
is "readable" thru ##raydium_gui_button_clicked()##.
**/


__rayapi int raydium_gui_label_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, GLfloat r, GLfloat g, GLfloat b);
/**
This function will create a new label, with ##name## and with ##window## for
parent. 
You need to provide a ##caption## ("title") and an RGB color (0..1 interval)

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_track_create(char *name, int window,  GLfloat px, GLfloat py, int min, int max, int current);
/**
This function will create a new trackbar, with ##name## and with ##window## for
parent. 
You need to provide a ##min## interger value, a ##max## and ##current## value.

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_edit_create(char *name, int window,  GLfloat px, GLfloat py, char *default_text);
/**
This function will create a new edit box, with ##name## and with ##window## 
for parent. 
You may provide a default text (or an empty string), if needed. Unless all
others Raydium's data, max string length is ##RAYDIUM_GUI_DATASIZE## and
not ##RAYDIUM_MAX_NAME_LEN##, since this component may handle bigger strings.
See ##raydium/gui.h## for more informations.

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_check_create(char *name, int window,  GLfloat px, GLfloat py, char *caption, signed char checked);
/**
This function will create a new check box, with ##name## and with ##window## 
for parent. 
You need to provide a ##caption## ("title") and a boolean state (checked or not).

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_combo_create(char *name, int window,  GLfloat px, GLfloat py, char *items, int current);
/**
This function will create a new edit box, with ##name## and with ##window## 
for parent. 
##items## is a string, using '\n' as a separator. It's allowed to create an
empty item.
##current## is the default selected item in ##items##. (first = 0)
Unless all others Raydium's data, max string length is ##RAYDIUM_GUI_DATASIZE## 
and not ##RAYDIUM_MAX_NAME_LEN##, since this component may handle bigger
strings. See ##raydium/gui.h## for more informations.

Unit for position (##px## and ##py##): percents (**window**)
**/

__rayapi int raydium_gui_zone_create(char *name, int window,  GLfloat px, GLfloat py, GLfloat sx, GLfloat sy, int tag, void *OnClick);
/**
This function will create a "zone" with ##name## and with ##window## for
parent. A zone will act like a button, but  will highlight a rectangular area 
of the window.

This widget will return its ##tag## when you'll read it, and will 
update ##raydium_gui_button_clicked()## value when clicked.

Unit for position/size (##px##, ##py##, ##sx## and ##sy##): percents (**window**)
**/

__rayapi int raydium_gui_read(int window, int widget, char *str);
/**
Use this function to get ##widget##'s state (for ##window##).
This function will always return this information thru two variable:
an integer (returned value) and a string (##str##).
This information is specific to ##widget##'s type (checked or not for a
checkbox, current choice for a combo, current string for an edit box, ...)
Please, note ##str## must be allocated before function call. This is also
the case for PHP scripts :
%%(php)
$str=str_pad("",256); // "pre-alloc"
$val=raydium_gui_read_name("main","track",$str);
echo "value=$val, string='$str'";
%%
**/

__rayapi int raydium_gui_read_name(char *window, char *widget, char *str);
/**
Same as above, but ##window## and ##widget## are resolved thru names, and
not numeric id.
**/

__rayapi int raydium_gui_read_widget(raydium_gui_Object *w, char *str);
/**
Same as ##raydium_gui_read()##, but using a ##raydium_gui_Object## pointer.
Useful for button callbacks, for example.
**/

__rayapi signed char raydium_gui_write(int window, int widget, char *str, int value);
/**
With this function, you can change the value of a ##widget## on a ##window##.
- With buttons, you must use ##str## to change caption.
- With labels, you must use ##str## to change caption.
- With tracks, you must use ##value## to change the current value.
- With edits, you must use ##str## to change text.
- With cheks, you must use ##value##: ##1## means "checked", ##0## "unchecked".
- With combos, you must use ##value## as an ID to change wich entry is selected.

Returns 1 when all is OK, 0 when it fails and -1 when nothing was changed.
**/

__rayapi int raydium_gui_write_name(char *window, char *widget, char *str, int value);
/**
Same as above, but ##window## and ##widget## are resolved thru names, and
not numeric id.
**/

__rayapi int raydium_gui_button_clicked(void);
/**
This function will return the id of the last clicked button,
or -1 if none were clicked.
The id is built like this : ##window * 1000 + widget_id##
Usefull for PHP scripts, since it's not possible to create callback for
buttons with RayPHP.
**/

__rayapi int raydium_gui_list_id(char *item, char *list);
/**
This function will return ##item##'s id in ##list##. Returns -1 if not found.
Useful for combo index, for example.
**/

__rayapi void raydium_gui_widget_focus(int widget, int window);
/**
Sets focus on ##widget## for ##window##.
**/

__rayapi void raydium_gui_widget_focus_name(char *widget, char *window);
/**
Same as above, but using widget and window names
**/

#endif
