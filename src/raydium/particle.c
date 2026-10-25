///////////////////////////////////////////////////////
//
// !!! THIS FILE IS OLD !!!
// (used as a reference for new particle engine)
//
// See particle2.* files
//
///////////////////////////////////////////////////////

/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// proto
void raydium_camera_replace(void);

void raydium_particle_init(GLuint expl, GLuint part)
{
raydium_particle_ttl[expl][part]=0;
}


void raydium_explosion_reset(GLuint exp)
{
GLuint i;

raydium_explosion_ttl[exp]=0;
for(i=0;i< RAYDIUM_MAX_PARTICLES;i++)
 raydium_particle_init(exp,i);

}


void raydium_explosion_add(GLfloat x, GLfloat y, GLfloat z, 
			   GLfloat vx, GLfloat vy, GLfloat vz, 
                           GLfloat dispersion, GLuint ttl, GLfloat dens_perc,
			   GLfloat grav)
{
GLuint n=0; // gngngngngng ... debug !


raydium_explosion_reset(n);
raydium_explosion_ttl[n]=ttl;
raydium_explosion_x[n]=x;
raydium_explosion_y[n]=y;
raydium_explosion_z[n]=z;
raydium_explosion_vector_x[n]=vx;
raydium_explosion_vector_y[n]=vy;
raydium_explosion_vector_z[n]=vz;
raydium_explosion_dispersion[n]=dispersion;
raydium_explosion_gravity[n]=grav;
// we need to know how many particles to generate per frame,
// user gives us dens_perc, 0 < x < 100
raydium_explosion_density_pf[n]=((float)RAYDIUM_MAX_PARTICLES*dens_perc)/(float)100;
}


char raydium_particle_life(GLuint expl, GLuint part, GLuint *created)
{
char ret=0;


 if(raydium_particle_ttl[expl][part]<=0 &&
    raydium_explosion_ttl[expl]>0 && 
    (*created)<raydium_explosion_density_pf[expl]) /* explosion alive, we can regenerate particle */
 {
 (*created)++;
 //ret=1;
 raydium_particle_ttl[expl][part]=RAYDIUM_MAX_PARTICLES/raydium_explosion_density_pf[expl];
 // set particle 
 raydium_particle_x[expl][part]=raydium_explosion_x[expl];
 raydium_particle_y[expl][part]=raydium_explosion_y[expl];
 raydium_particle_z[expl][part]=raydium_explosion_z[expl];
 
 // need to take direction vector
 raydium_particle_ix[expl][part]=raydium_random_neg_pos_1()*raydium_explosion_dispersion[expl];
 raydium_particle_iy[expl][part]=raydium_random_neg_pos_1()*raydium_explosion_dispersion[expl];
 raydium_particle_iz[expl][part]=raydium_random_neg_pos_1()*raydium_explosion_dispersion[expl]; 
 }


 if(raydium_particle_ttl[expl][part]>0) // alive
 {
 ret=1;
 // move particle
 raydium_particle_x[expl][part]+=raydium_particle_ix[expl][part];
 raydium_particle_y[expl][part]+=raydium_particle_iy[expl][part];
 raydium_particle_z[expl][part]+=raydium_particle_iz[expl][part];

 raydium_particle_iz[expl][part]-=raydium_explosion_gravity[expl];
 raydium_particle_ttl[expl][part]--;
 
 }

return ret;
}


void raydium_explosion_life(GLuint expl)
{
GLuint i;
char any_alive=0;
GLuint created=0;

for(i=0;i< RAYDIUM_MAX_PARTICLES;i++)
 if(raydium_particle_life(expl,i,&created)) any_alive=1;

if(raydium_explosion_ttl[expl]>0) raydium_explosion_ttl[expl]--;
if(raydium_explosion_ttl[expl]==0 && any_alive) raydium_explosion_ttl[expl]=-1;
if(raydium_explosion_ttl[expl]<0 && !any_alive) raydium_explosion_ttl[expl]=0;

}


void raydium_explosion_callback(void)
{
GLuint i;

for(i=0;i< RAYDIUM_MAX_EXPLOSIONS;i++)
 if(raydium_explosion_ttl[i]!=0)
 raydium_explosion_life(i);
}


void raydium_explosion_draw_particles(GLuint expl)
{
GLuint i;
#define TSIZE ((float)0.5)

GLfloat modmat[16];
GLfloat ux;
GLfloat uy;
GLfloat uz;
GLfloat rx;
GLfloat ry;
GLfloat rz;

glGetFloatv(GL_MODELVIEW_MATRIX,modmat);
ux=modmat[0];
uy=modmat[4];
uz=modmat[8];
ux*=TSIZE/2;
uy*=TSIZE/2;
uz*=TSIZE/2;

rx=modmat[1];
ry=modmat[5];
rz=modmat[9];
rx*=TSIZE/2;
ry*=TSIZE/2;
rz*=TSIZE/2;


for(i=0;i< RAYDIUM_MAX_PARTICLES;i++)
 if(raydium_particle_ttl[expl][i])
 {
  glBegin(GL_QUADS); // berk... but i'll switch to TRIANGLES one day ;)
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(raydium_particle_x[expl][i] + (-rx - ux),
	     raydium_particle_y[expl][i] + (-ry - uy),
	     raydium_particle_z[expl][i] + (-rz - uz));
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(raydium_particle_x[expl][i] + (rx - ux),
	     raydium_particle_y[expl][i] + (ry - uy),
	     raydium_particle_z[expl][i] + (rz - uz));
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(raydium_particle_x[expl][i] + (rx + ux),
	     raydium_particle_y[expl][i] + (ry + uy),
	     raydium_particle_z[expl][i] + (rz + uz));
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(raydium_particle_x[expl][i] + (ux - rx),
	     raydium_particle_y[expl][i] + (uy - ry),
	     raydium_particle_z[expl][i] + (uz - rz));
  glEnd();
 }
}


void raydium_explosion_draw_all(void)
{
GLuint i;
GLuint texsave;
char light;

texsave=raydium_texture_current;
light=raydium_light_enabled_tag;
raydium_light_disable();
if (raydium_camera_pushed) raydium_camera_replace(); // is it really our job to do it here ?
//raydium_rendering_internal_restore_render_state();
raydium_rendering_internal_prepare_texture_render(raydium_texture_current_set_name("flare.tga"));
glDepthMask(GL_FALSE);


for(i=0;i< RAYDIUM_MAX_EXPLOSIONS;i++)
 if(raydium_explosion_ttl[i]!=0)
 raydium_explosion_draw_particles(i);

glDepthMask(GL_TRUE);
if(light) raydium_light_enable();

raydium_texture_current_set(texsave);
//raydium_rendering_internal_prepare_texture_render(texsave);
}
