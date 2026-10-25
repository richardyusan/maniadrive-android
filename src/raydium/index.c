#ifndef FORCE_LIBRAYDIUM
/*
* Raydium - CQFD Corp.
* http://raydium.org/
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Include this file if you want to use "[o]comp*.sh" scripts, for
// a full compilation + run.
// Define "FORCE_LIBRAYDIUM" (-DFORCE_LIBRAYDIUM) if you want to
// force index.h (see dyncomp.sh).

#ifdef __cplusplus
extern "C" {
#endif

#define DONT_INCLUDE_HEADERS 

#include "main.c"

// let's start "compile time static linking" ;)
#ifndef RAYDIUM_NETWORK_ONLY
#include "myglut.c"
#include "log.c"
#include "atexit.c"
#include "signal.c"
#include "trigo.c"
#include "random.c"
#include "timecall.c"
#include "profile.c"
#include "parser.c"
#include "fog.c"
#include "window.c"
#include "capture.c"
#include "clear.c"
#include "background.c"
#include "light.c"
#include "key.c"
#include "mouse.c"
#include "joy.c"
#include "texture.c"
#include "shadow.c"
#include "render.c"
#include "particle2.c"
#include "sound.c"
#include "callback.c"
#include "normal.c"
#include "vertex.c"
#include "osd.c"
#include "hdr.c"
#include "shader.c"
#include "register.c"
#ifdef PHP_SUPPORT
#include "php.c"
#include "rayphp.c"
#endif
#include "console.c"
#include "gui.c"
#include "land.c"
#include "sky.c"
#include "internal.c"
#include "file.c"
#include "path.c"
#include "file_tri.c"
#include "camera.c"
#include "object.c"
#include "cli.c"
#include "network.c"
#include "init.c"
#ifdef ODE_SUPPORT
#include "ode.c"
#endif
#include "live.c"
#include "video.c"
#include "web.c"
#include "reg_api.c"

#else

#include "atexit.c"
#include "network.h"
#include "log.c"
#include "trigo.c"
#include "random.c"
#include "timecall.c"
#include "parser.c"
#include "cli.c"
#include "network.c"
#include "file.c"
#include "path.c"
#include "register.c"
#ifdef PHP_SUPPORT
#include "php.c"
#include "rayphp.c"
#endif
#include "console.c"
#include "web.c"
#endif

#ifdef __cplusplus
} // close extern "C"
#endif

#else // FORCE_LIBRAYDIUM is defined
#include "index.h"
#endif
// EOF
