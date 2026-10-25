#ifndef _SHADOW__H
#define _SHADOW__H
#include "../shadow.h"

__rayapi void raydium_shadow_init(void);
__rayapi void raydium_shadow_enable(void);
__rayapi void raydium_shadow_disable(void);
__rayapi signed char raydium_shadow_isenabled(void);
__rayapi void raydium_shadow_light_main(GLuint l);
__rayapi void raydium_shadow_ground_change(int object);
__rayapi void raydium_shadow_map_generate(void);
__rayapi void raydium_shadow_map_render(void);
__rayapi void raydium_shadow_object_draw(GLuint o);


#endif
