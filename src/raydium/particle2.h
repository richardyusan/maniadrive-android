/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// Raydium Particle Engine, version 2

#ifndef PARTICLE2_H
#define PARTICLE2_H

#define RAYDIUM_MAX_PARTICLES	8192
#define RAYDIUM_MAX_GENERATORS	64

typedef struct raydium_particle_Generator
{
 int      id;
 signed char	  state;
 signed char	  enabled;
 char     name[RAYDIUM_MAX_NAME_LEN];
 GLfloat  position[3];
 GLfloat  position_random[3];
 GLfloat  position_user[3];
 GLfloat  ttl_generator;
 GLfloat  ttl_particles;
 GLfloat  ttl_particles_random;
 GLfloat  particles_per_second;
 int	  texture;
 GLfloat  size;
 GLfloat  size_random;
 GLfloat  size_inc_per_sec;
 GLfloat  size_limit;
 GLfloat  gravity[3];
 GLfloat  vector[3];
 GLfloat  vector_random[3];
 GLfloat  vector_sphere_angles[3];
 GLfloat  vector_sphere_angles_random[3];
 GLfloat  vector_sphere_force;
 GLfloat  vector_sphere_force_random;
// GLfloat  velocity_limit[3];
 GLfloat  rotation_speed;
 GLfloat  rotation_random;
 GLfloat  color_start[4];
 GLfloat  color_start_random[4];
 GLfloat  color_end[4];
 GLfloat  color_end_random[4];
// char     transform[RAYDIUM_MAX_NAME_LEN];
 GLfloat  visibility;
 void *   OnDeleteParticle;
} raydium_particle_Generator;

typedef struct raydium_particle_Particle
{
 GLfloat ttl_init;
 GLfloat ttl; // is updated over time
 int	 texture;
 GLfloat  size;
 GLfloat  size_inc_per_sec;
 GLfloat  size_limit;
 GLfloat position[3];
 GLfloat vel[3]; // is updated over time
 GLfloat gravity[3];
// GLfloat velocity_limit[3];
 GLfloat color_start[4]; // will use ttl to determine current color
 GLfloat color_end[4];   // use ttl too
 GLfloat rotation_speed; // use ttl too
 GLfloat visibility;
 void *  OnDelete; 
 
// --- Current resulting data
 GLfloat current_color[4];
 GLfloat current_rotation;
} raydium_particle_Particle;

__global raydium_particle_Generator raydium_particle_generators[RAYDIUM_MAX_GENERATORS];
__global raydium_particle_Particle *raydium_particle_particles[RAYDIUM_MAX_PARTICLES];
__global GLfloat raydium_particle_time_factor;
__global GLfloat raydium_particle_scale_factor;

// proto
void raydium_camera_replace(void);
void raydium_file_dirname(char *dest,char *from);

#endif
