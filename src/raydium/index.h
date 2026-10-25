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

// Include this file if you want to use Makefile and libraydium, for
// a quick compilation (use odyncomp.sh script, for example)

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#ifndef RAYDIUM_NETWORK_ONLY
#include "headers/myglut.h"
#include "headers/log.h"
#include "headers/atexit.h"
#include "headers/signal.h"
#include "headers/trigo.h"
#include "headers/random.h"
#include "headers/timecall.h"
#include "headers/profile.h"
#include "headers/parser.h"
#include "headers/fog.h"
#include "headers/window.h"
#include "headers/capture.h"
#include "headers/clear.h"
#include "headers/background.h"
#include "headers/light.h"
#include "headers/key.h"
#include "headers/mouse.h"
#include "headers/joy.h"
#include "headers/texture.h"
#include "headers/shadow.h"
#include "headers/render.h"
#include "headers/particle2.h"
#include "headers/sound.h"
#include "headers/callback.h"
#include "headers/normal.h"
#include "headers/vertex.h"
#include "headers/osd.h"
#include "headers/hdr.h"
#include "headers/shader.h"
#include "headers/register.h"
#ifdef PHP_SUPPORT
#include "headers/php.h"
#include "headers/rayphp.h"
#endif
#include "headers/console.h"
#include "headers/gui.h"
#include "headers/land.h"
#include "headers/sky.h"
#include "headers/internal.h"
#include "headers/file.h"
#include "headers/path.h"
#include "headers/file_tri.h"
#include "headers/camera.h"
#include "headers/object.h"
#include "headers/cli.h"
#include "headers/network.h"
#include "headers/init.h"
#ifdef ODE_SUPPORT
#include "headers/ode.h"
#endif
#include "headers/live.h"
#include "headers/video.h"
#include "headers/web.h"
#include "headers/reg_api.h"

#else

#warning "unless you know what you're doing, RAYDIUM_NETWORK_ONLY with \
dynamic linking is a bad idea. Use static linking instead (ex: comp.sh)"
#include "headers/atexit.h"
#include "network.h"
#include "headers/log.h"
#include "headers/trigo.h"
#include "headers/random.h"
#include "headers/timecall.h"
#include "headers/parser.h"
#include "headers/cli.h"
#include "headers/network.h"
#include "headers/file.h"
#include "headers/path.h"
#include "headers/register.h"
#ifdef PHP_SUPPORT
#include "headers/rayphp.h"
#include "headers/php.h"
#endif
#include "headers/console.h"
#include "headers/web.h"
#endif

#ifdef __cplusplus
} // close extern "C"
#endif
//EOF
