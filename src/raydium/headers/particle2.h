#ifndef _PARTICLE__H
#define _PARTICLE__H
#include "../particle2.h"

/*=
Particle engine
1400
**/

// Introduction
/**
This is the second version of Raydium's particle engine. This engine is build
on top of a dedicated file format (.prt and .sprt files), describing most 
(up to all, in facts) properties of generators.
It probably better to start by an example (fountain.prt) :
%%(c)
// Simple blue fountain (change 'vector' if needed)
ttl_generator=5;
ttl_particles=1.5;
ttl_particles_random=0;

particles_per_second=200;

texture="flare_nb.tga";

size=0.1;
size_inc_per_sec=0.1;

gravity={0,0,-5};
vector={0,0,4};
vector_random={0.2,0.2,0.2};

// RGBA
color_start={0.6,0.6,1,0.5};
color_start_random={0,0,0.2,0};
color_end={1,1,1,0.1};

// end of file.
%%

.prt files are readed using parsing functions (see appropriate chapter, if
needed), and the list of all available properties can be found in particle2.c
source file. A full toturial is also available on Raydium's Wiki.

Once the particle file is written, you only need to load the file using the
suitable function (see below). Some anchor are available to link generators to
physic entities, if needed, as callbacks for a few events (one, for now).

.sprt files are used to create a "snapshot" of particles, used for example by
3D captures, and are not meant to be edited by hand.
**/

__rayapi void raydium_particle_name_auto (char *prefix, char *dest);
/**
Will generate a unique string using ##prefix##. The string is created using
space provided by ##dest##.
You can use this function when building a new generator.
**/

__rayapi void raydium_particle_init (void);
/**
Internal use.
**/

__rayapi signed char raydium_particle_generator_isvalid (int g);
/**
Internal use, but you can call this function if you want to verify if a
generator's id is valid (in bounds, and loaded).
**/

__rayapi int raydium_particle_generator_find (char *name);
/**
Lookups a generator using is name. Returns -1 if ##name## is not found.
**/

__rayapi int raydium_particle_find_free (void);
/**
Finds a free **particle** slot.
**/

__rayapi void raydium_particle_generator_delete (int gen);
/**
Deletes a generator.
**/

__rayapi void raydium_particle_generator_delete_name (char *gen);
/**
Same as above, but using generator's name.
**/

__rayapi void raydium_particle_generator_enable (int gen, signed char enabled);
/**
Activate a disabled generator (see below).
**/

__rayapi void raydium_particle_generator_enable_name (char *gen, signed char enable);
/**
Disable a generator (TTL is still decremented).
**/

__rayapi void raydium_particle_preload (char *filename);
/**
Loads .prt file and associated textures into suitable caches.
Call this function if you want to avoid (small) jerks caused by "live"
loading a generator.
**/

__rayapi void raydium_particle_generator_load_internal (int generator, FILE * fp, char *filename);
/**
Internal use.
**/

__rayapi int raydium_particle_generator_load (char *filename, char *name);
/**
Loads generator from ##filename## as ##name##. This ##name## will be used for
future references to this generator, as the returned interger id.
**/

__rayapi void raydium_particle_generator_update (int g, GLfloat step);
/**
Internal use.
**/

__rayapi void raydium_particle_update (int part, GLfloat step);
/**
Internal use.
**/

__rayapi void raydium_particle_callback (void);
/**
Internal use.
**/

__rayapi int raydium_particle_state_dump(char *filename);
/**
Dumped current particles to ##filename## (.sprt [static particles]).
**/

__rayapi int raydium_particle_state_restore(char *filename);
/**
Append .sprt ##filename## to current scene.
**/

__rayapi void raydium_particle_draw (raydium_particle_Particle * p, GLfloat ux, GLfloat uy, GLfloat uz, GLfloat rx, GLfloat ry, GLfloat rz);
/**
Internal use.
**/

__rayapi void raydium_particle_draw_all (void);
/**
Internal use.
**/

__rayapi void raydium_particle_generator_move (int gen, GLfloat * pos);
/**
Moves ##gen## generator to ##pos## position (3 * GLfloat array).
**/

__rayapi void raydium_particle_generator_move_name (char *gen, GLfloat * pos);
/**
Same as above, but using generator's name.
**/

__rayapi void raydium_particle_generator_move_name_3f (char *gen, GLfloat x, GLfloat y, GLfloat z);
/**
Same as above, using 3 different GLfloat values.
**/

__rayapi void raydium_particle_generator_particles_OnDelete (int gen, void *OnDelete);
/**
Sets a callback for ##gen##, fired when any particle of this generator is
deleted, providing a easy way to create "cascading" generators.
The callback must respect the following prototype:
%%(c) void cb(raydium_particle_Particle *) %%
Do not free the provided particle.
**/

__rayapi void raydium_particle_generator_particles_OnDelete_name (char *gen, void *OnDelete);
/**
Same as above, but using generator's name.
**/

__rayapi void raydium_particle_scale_all(GLfloat scale);
/**
Will scale all particles with ##scale## factor. Use with caution.
Default is obviously 1.
**/


#endif
