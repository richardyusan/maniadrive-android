/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/particle2.h"
#endif 

// Raydium Particle Engine, version 2

// TODO:
// relative texture filename (as for include statement) ?
// multiple file loading (composite generators)
// rotation

#include "particle2.h"

#ifdef ODE_SUPPORT
// proto
void raydium_ode_internal_particle_genetator_deleted_callback(int gen);
#endif

void raydium_particle_name_auto(char *prefix, char *dest)
{
static int counter;
sprintf(dest,"%s_particle_%i",prefix,counter);
counter++;
}

void raydium_particle_init(void)
{
int i;

raydium_particle_time_factor=1;
raydium_particle_scale_factor=1;

for(i=0;i<RAYDIUM_MAX_GENERATORS;i++)
    {
    raydium_particle_generators[i].state=0;
    raydium_particle_generators[i].id=i;
    }

for(i=0;i<RAYDIUM_MAX_PARTICLES;i++)
    raydium_particle_particles[i]=NULL;
    
raydium_log("particle: OK");
}

signed char raydium_particle_generator_isvalid(int g)
{
if(g>=0 && g<RAYDIUM_MAX_GENERATORS && raydium_particle_generators[g].state)
    return 1;
return 0;
}

int raydium_particle_generator_find(char *name)
{
int i;
                                                                                
for(i=0;i<RAYDIUM_MAX_GENERATORS;i++)
    if(!strcmp(name,raydium_particle_generators[i].name) && raydium_particle_generator_isvalid(i))
     return i;
return -1;
}

int raydium_particle_find_free(void)
{
int i;

for(i=0;i<RAYDIUM_MAX_PARTICLES;i++)
    if(raydium_particle_particles[i]==NULL)
     return i;
return -1;
}


void raydium_particle_generator_delete(int gen)
{

if(!raydium_particle_generator_isvalid(gen))
    {
    raydium_log("particle: cannot delete generator: invalid name or index");
    return;
    }
#ifdef ODE_SUPPORT
raydium_ode_internal_particle_genetator_deleted_callback(gen);
#endif
raydium_particle_generators[gen].state=0;
}

void raydium_particle_generator_delete_name(char *gen)
{
raydium_particle_generator_delete(raydium_particle_generator_find(gen));
}

void raydium_particle_generator_enable(int gen, signed char enabled)
{
if(!raydium_particle_generator_isvalid(gen))
    {
    raydium_log("particle: cannot enable/disable generator: invalid name or index");
    return;
    }
raydium_particle_generators[gen].enabled=enabled;
}

void raydium_particle_generator_enable_name(char *gen, signed char enable)
{
raydium_particle_generator_enable(raydium_particle_generator_find(gen),enable);
}

void raydium_particle_preload(char *filename)
{
FILE *fp;
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;


fp=raydium_file_fopen(filename,"rt"); // idem ("rb" ?)
if(!fp)
 {
 raydium_log("particle: ERROR: Cannot open %s particle file for preloading",filename);
 return;
 }

// parse (and cache) file
while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    if(!strcasecmp(var,"include"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("particle: parser: include: wrong type");
	    continue;
	    }
	raydium_particle_preload(val_s);
	}

    if(!strcasecmp(var,"texture"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("particle: parser: texture: wrong type");
	    continue; // in case of multiple textures ? (degenarated file)
	    }
	// cache texture
	raydium_texture_current_set_name(val_s);
	}
    }
fclose(fp);
}


void raydium_particle_generator_load_internal(int generator,FILE *fp, char *filename)
{
int ret;
char var[RAYDIUM_MAX_NAME_LEN];
char val_s[RAYDIUM_MAX_NAME_LEN];
GLfloat val_f[4];
int size;
char done;

while( (ret=raydium_parser_read(var,val_s,val_f,&size,fp))!=RAYDIUM_PARSER_TYPE_EOF)
    {
    done=0;

    if(!strcasecmp(var,"include"))
	{
	FILE *sub;
	char dir[RAYDIUM_MAX_NAME_LEN];
	
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("particle: parser: include: wrong type");
	    continue;
	    }
	raydium_file_dirname(dir,filename);
	strcat(dir,val_s);
	strcpy(val_s,dir);
	sub=raydium_file_fopen(val_s,"rt"); // idem
	if(!sub)
	 {
	 raydium_log("particle: ERROR: %s cannot open %s particle subfile",filename,val_s);
	 continue;
	 }
        raydium_particle_generator_load_internal(generator,sub,val_s);
	fclose(sub);
	done=1;
	}

    if(!strcasecmp(var,"position"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: position: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].position,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"position_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: position_random: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].position_random,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"ttl_generator"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: ttl_generator: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].ttl_generator=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"ttl_particles"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: ttl_particles: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].ttl_particles=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"ttl_particles_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: ttl_particles_random: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].ttl_particles_random=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"particles_per_second"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: particles_per_second: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].particles_per_second=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"texture"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_STRING)
	    {
	    raydium_log("particle: parser: texture: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].texture=
	    raydium_texture_find_by_name(val_s);
	done=1;
	}


    if(!strcasecmp(var,"size"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: size: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].size=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"size_inc_per_sec"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: size_inc_per_sec: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].size_inc_per_sec=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"size_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: size_random: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].size_random=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"size_limit"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: size_limit: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].size_limit=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"gravity"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: gravity: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].gravity,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"vector"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: vector: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].vector,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"vector_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: vector_random: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].vector_random,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"vector_sphere_angles"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: vector_sphere_angles: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].vector_sphere_angles,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"vector_sphere_angles_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=3)
	    {
	    raydium_log("particle: parser: vector_sphere_angles_random: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].vector_sphere_angles_random,val_f,sizeof(GLfloat)*3);
	done=1;
	}

    if(!strcasecmp(var,"vector_sphere_force"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: vector_sphere_force: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].vector_sphere_force=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"vector_sphere_force_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: vector_sphere_force_random: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].vector_sphere_force_random=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"rotation_speed"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: rotation_speed: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].rotation_speed=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"rotation_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: rotation_random: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].rotation_random=val_f[0];
	done=1;
	}

    if(!strcasecmp(var,"color_start"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
	    {
	    raydium_log("particle: parser: color_start: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].color_start,val_f,sizeof(GLfloat)*4);
	done=1;
	}

    if(!strcasecmp(var,"color_start_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
	    {
	    raydium_log("particle: parser: color_start_random: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].color_start_random,val_f,sizeof(GLfloat)*4);
	done=1;
	}

    if(!strcasecmp(var,"color_end"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
	    {
	    raydium_log("particle: parser: color_end: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].color_end,val_f,sizeof(GLfloat)*4);
	done=1;
	}

    if(!strcasecmp(var,"color_end_random"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=4)
	    {
	    raydium_log("particle: parser: color_end_random: wrong type");
	    continue;
	    }
	memcpy(raydium_particle_generators[generator].color_end_random,val_f,sizeof(GLfloat)*4);
	done=1;
	}

    if(!strcasecmp(var,"visibility"))
	{
	if(ret!=RAYDIUM_PARSER_TYPE_FLOAT || size!=1)
	    {
	    raydium_log("particle: parser: visibility: wrong type");
	    continue;
	    }
	raydium_particle_generators[generator].visibility=val_f[0];
	done=1;
	}

    if(!done)
	raydium_log("particle: parser: invalid or unsupported option '%s' (%s)",var,filename);
    }
}

int raydium_particle_generator_load(char *filename,char *name)
{
FILE *fp;
int generator;
int i;


if(raydium_particle_generator_find(name)>=0)
    {
    raydium_log("particle: Cannot load \"%s\": '%s' already exists",filename,name);
    return -1;    
    }

fp=raydium_file_fopen(filename,"rt"); // rb ? must test under win32
if(!fp)
 {
 raydium_log("particle: ERROR: Cannot open %s particle file",filename);
 return -1;
 }

for(i=0;i<RAYDIUM_MAX_GENERATORS;i++)
    if(raydium_particle_generators[i].state==0)
	break;
	
if(i==RAYDIUM_MAX_GENERATORS)
    {
    raydium_log("particle: no more available slots !");
    fclose(fp);
    return -1;
    }
generator=i;

raydium_particle_generators[generator].state=1;
raydium_particle_generators[generator].enabled=1;
strcpy(raydium_particle_generators[generator].name,name);
raydium_particle_generators[generator].position[0]=0;
raydium_particle_generators[generator].position[1]=0;
raydium_particle_generators[generator].position[2]=0;
raydium_particle_generators[generator].position_random[0]=0;
raydium_particle_generators[generator].position_random[1]=0;
raydium_particle_generators[generator].position_random[2]=0;
raydium_particle_generators[generator].position_user[0]=0;
raydium_particle_generators[generator].position_user[1]=0;
raydium_particle_generators[generator].position_user[2]=0;
raydium_particle_generators[generator].ttl_generator=1;
raydium_particle_generators[generator].ttl_particles=1;
raydium_particle_generators[generator].particles_per_second=100;
raydium_particle_generators[generator].texture=0;
raydium_particle_generators[generator].size=1;
raydium_particle_generators[generator].size_inc_per_sec=0;
raydium_particle_generators[generator].size_random=0;
raydium_particle_generators[generator].size_limit=-1;
raydium_particle_generators[generator].gravity[0]=0;
raydium_particle_generators[generator].gravity[1]=0;
raydium_particle_generators[generator].gravity[2]=0;
raydium_particle_generators[generator].vector[0]=0;
raydium_particle_generators[generator].vector[1]=0;
raydium_particle_generators[generator].vector[2]=0;
raydium_particle_generators[generator].vector_random[0]=0;
raydium_particle_generators[generator].vector_random[1]=0;
raydium_particle_generators[generator].vector_random[2]=0;
raydium_particle_generators[generator].vector_sphere_angles[0]=0;
raydium_particle_generators[generator].vector_sphere_angles[1]=0;
raydium_particle_generators[generator].vector_sphere_angles[2]=0;
raydium_particle_generators[generator].vector_sphere_angles_random[0]=0;
raydium_particle_generators[generator].vector_sphere_angles_random[1]=0;
raydium_particle_generators[generator].vector_sphere_angles_random[2]=0;
raydium_particle_generators[generator].vector_sphere_force=0;
raydium_particle_generators[generator].vector_sphere_force_random=0;
// velocity_limit
raydium_particle_generators[generator].rotation_speed=45;
raydium_particle_generators[generator].rotation_random=0;
raydium_particle_generators[generator].color_start[0]=1;
raydium_particle_generators[generator].color_start[1]=1;
raydium_particle_generators[generator].color_start[2]=1;
raydium_particle_generators[generator].color_start[3]=1;
raydium_particle_generators[generator].color_start_random[0]=0;
raydium_particle_generators[generator].color_start_random[1]=0;
raydium_particle_generators[generator].color_start_random[2]=0;
raydium_particle_generators[generator].color_start_random[3]=0;
raydium_particle_generators[generator].color_end[0]=1;
raydium_particle_generators[generator].color_end[1]=1;
raydium_particle_generators[generator].color_end[2]=1;
raydium_particle_generators[generator].color_end[3]=1;
raydium_particle_generators[generator].color_end_random[0]=0;
raydium_particle_generators[generator].color_end_random[1]=0;
raydium_particle_generators[generator].color_end_random[2]=0;
raydium_particle_generators[generator].color_end_random[3]=0;
raydium_particle_generators[generator].visibility=1;
raydium_particle_generators[generator].OnDeleteParticle=NULL;

// transform
raydium_particle_generator_load_internal(generator,fp,filename);

fclose(fp);
return generator;
}


void raydium_particle_generator_update(int g,GLfloat step)
{
int i,j,p,to_create;
raydium_particle_Generator *gen;
raydium_particle_Particle *part;

/*
if(!raydium_particle_generators[g].state) 
    {
    raydium_log("particle: Cannot update generator: invalid index");
    return;
    }
*/

gen=&raydium_particle_generators[g];

// This +1 is bad, must use a "float" counter on generator
to_create=(step*gen->particles_per_second)+1;
//raydium_log("will create %i particles for %s (%i)",to_create,gen->name,g);

if(!gen->enabled) to_create=0;

for(i=0;i<to_create;i++)
    {
    p=raydium_particle_find_free();
    if(p<0)
	{
	//raydium_log("particle: No more particle slots !");
	break;
	}

    raydium_particle_particles[p]=malloc(sizeof(raydium_particle_Particle));
    if(!raydium_particle_particles[p])
	{
	raydium_log("particle: ERROR: malloc failed !");
	return;
	}


    part=raydium_particle_particles[p];

    part->ttl_init=raydium_random_f(gen->ttl_particles-gen->ttl_particles_random,gen->ttl_particles+gen->ttl_particles_random);
    part->ttl=part->ttl_init;
    part->texture=gen->texture;

    memcpy(part->position,gen->position,sizeof(GLfloat)*3);
    for(j=0;j<3;j++)
	part->position[j]+=raydium_random_f(-gen->position_random[j],gen->position_random[j]);

    for(j=0;j<3;j++)
	part->position[j]+=gen->position_user[j];

    part->size=raydium_random_f(gen->size-gen->size_random,gen->size+gen->size_random);
    part->size_inc_per_sec=gen->size_inc_per_sec;
    part->size_limit=gen->size_limit;

    memcpy(part->gravity,gen->gravity,sizeof(GLfloat)*3);


    if(gen->vector_sphere_force==0 && gen->vector_sphere_force_random==0) 
    { 
     // ortho
     memcpy(part->vel,gen->vector,sizeof(GLfloat)*3);
     for(j=0;j<3;j++)
	part->vel[j]+=raydium_random_f(-gen->vector_random[j],gen->vector_random[j]);
    }
    else 
    {
    // spherical
    GLfloat def_angles[3]={0,0,1};
    GLfloat angles[3];
    GLfloat force;

    memcpy(angles,gen->vector_sphere_angles,sizeof(GLfloat)*3);
    for(j=0;j<3;j++)
	angles[j]+=raydium_random_f(-gen->vector_sphere_angles_random[j],gen->vector_sphere_angles_random[j]);

    
    force=gen->vector_sphere_force
         +raydium_random_f(-gen->vector_sphere_force_random,
			    gen->vector_sphere_force_random);
    for(j=0;j<3;j++)
	def_angles[j]*=force;
    
    raydium_trigo_rotate(def_angles,angles[0],angles[1],angles[2],part->vel);
    }

    memcpy(part->color_start,gen->color_start,sizeof(GLfloat)*4);
    for(j=0;j<4;j++)
	part->color_start[j]+=raydium_random_f(-gen->color_start_random[j],gen->color_start_random[j]);

    memcpy(part->color_end,gen->color_end,sizeof(GLfloat)*4);
    for(j=0;j<4;j++)
	part->color_end[j]+=raydium_random_f(-gen->color_end_random[j],gen->color_end_random[j]);

    part->rotation_speed=raydium_random_f(gen->rotation_speed-gen->rotation_random,
					  gen->rotation_speed+gen->rotation_random);

    part->visibility=gen->visibility;
    part->OnDelete=gen->OnDeleteParticle;
    }


if(gen->ttl_generator==0) 
    return; // infinite generator

gen->ttl_generator-=step;
if(gen->ttl_generator<=0) 
    {
    // we've a OnDelete callback for particles and not for
    // generators ... 'must code it.
    raydium_particle_generator_delete(gen->id);
    }
}

void raydium_particle_update(int part, GLfloat step)
{
int i;
GLfloat age,age_factor;
raydium_particle_Particle *p;

p=raydium_particle_particles[part];
// ttl
if(p->ttl!=0) // if not an infinite particle
{
 p->ttl-=step;
 if(p->ttl<=0) // "timeout" ...
    {
    void (*f)(raydium_particle_Particle *);
    
    f=p->OnDelete;
    if(f) f(p);
    
    free(p);
    raydium_particle_particles[part]=NULL;
    return;
    }
}

age=(p->ttl_init-p->ttl);
if(p->ttl_init)
    age_factor=age/p->ttl_init;
else
    age_factor=0;

// pos
for(i=0;i<3;i++)
    p->position[i]+=(p->vel[i]*step);

// vel
for(i=0;i<3;i++)
    p->vel[i]+=(p->gravity[i]*step);

// size
p->size+=(p->size_inc_per_sec*step);
if(p->size<0) p->size=0;
if(p->size>p->size_limit && p->size_limit>0) p->size=p->size_limit;

// color
for(i=0;i<4;i++)
    p->current_color[i]=(p->color_end[i]-p->color_start[i])*age_factor+p->color_start[i];

// rotation
p->current_rotation=age*p->rotation_speed;

}

void raydium_particle_callback(void)
{
int i;

//raydium_profile_start();

for(i=0;i<RAYDIUM_MAX_GENERATORS;i++)
    if(raydium_particle_generators[i].state)
	raydium_particle_generator_update(i,raydium_frame_time*raydium_particle_time_factor);

for(i=0;i<RAYDIUM_MAX_PARTICLES;i++)
    if(raydium_particle_particles[i])
	raydium_particle_update(i,raydium_frame_time*raydium_particle_time_factor);

//raydium_profile_end("particles updating");
}


int raydium_particle_state_dump(char *filename)
{
FILE *fp;
int i;
int cpt=0;
raydium_particle_Particle *p;

fp=raydium_file_fopen(filename,"wt");
if(!fp)
    {
    raydium_log("particle: ERROR: cannot create '%s' filename",filename);
    return 0;
    }

fprintf(fp,"0\n");

for(i=0;i<RAYDIUM_MAX_PARTICLES;i++)
    if(raydium_particle_particles[i])
	{
	cpt++;
	p=raydium_particle_particles[i];
	fprintf(fp,"%f %f %f %f %f %f %f %f %f %s\n",
		p->position[0],
		p->position[1],
		p->position[2],
		p->size,
		p->current_color[0],
		p->current_color[1],
		p->current_color[2],
		p->current_color[3],
		p->visibility,
		raydium_texture_name[p->texture]);	
	}
fclose(fp);
raydium_log("particle: %i particle(s) dumped",cpt);
return 1;
}

int raydium_particle_state_restore(char *filename)
{
FILE *fp;
int p,visu,cpt=0;
raydium_particle_Particle *part;
GLfloat pos[3],color[4],size,visibility;
char texture[RAYDIUM_MAX_NAME_LEN];

fp=raydium_file_fopen(filename,"rt");
if(!fp) 
    { 
    raydium_log("particle: ERROR: cannot read from file '%s'",filename);
    return 0; 
    }
fscanf(fp,"%i\n",&visu);

if(visu!=0)
    { 
    raydium_log("particle: ERROR: '%s' file must be 'version 0'",filename);
    return 0; 
    }


while( fscanf(fp,"%f %f %f %f %f %f %f %f %f %s\n",
    &pos[0],
    &pos[1],
    &pos[2],
    &size,
    &color[0],
    &color[1],
    &color[2],
    &color[3],
    &visibility,
    texture)!=EOF )
    {
    cpt++;
    p=raydium_particle_find_free();
    if(p<0)
	{
	raydium_log("particle: No more particle slots !");
	return -1;
	}

    raydium_particle_particles[p]=malloc(sizeof(raydium_particle_Particle));
    if(!raydium_particle_particles[p])
	{
	raydium_log("particle: ERROR: malloc failed !");
	return 0;
	}


    part=raydium_particle_particles[p];

    part->ttl_init=0;
    part->ttl=part->ttl_init;
    part->texture=raydium_texture_find_by_name(texture);

    memcpy(part->position,pos,sizeof(GLfloat)*3);

    part->size=size;
    part->size_inc_per_sec=0;
    part->size_limit=size+1;

    part->gravity[0]=part->gravity[1]=part->gravity[2]=0;

    part->vel[0]=part->vel[1]=part->vel[2]=0;
    
    memcpy(part->color_start,   color,sizeof(GLfloat)*4);
    memcpy(part->color_end,     color,sizeof(GLfloat)*4);
    memcpy(part->current_color, color,sizeof(GLfloat)*4);

    part->rotation_speed=0;
    part->visibility=visibility;
    part->OnDelete=NULL;
    }
fclose(fp);
raydium_log("particle: %i infinite particle(s) created",cpt);
return 1;
}


void raydium_particle_draw(raydium_particle_Particle *p,GLfloat ux, GLfloat uy, GLfloat uz, GLfloat rx, GLfloat ry, GLfloat rz)
{
GLfloat TSIZE;

if(!raydium_random_proba(p->visibility))
    return;

// need to order drawing by texture id
raydium_rendering_internal_prepare_texture_render(raydium_texture_current_set(p->texture));
glColor4fv(p->current_color);
TSIZE=p->size;

ux*=TSIZE/2;
uy*=TSIZE/2;
uz*=TSIZE/2;

rx*=TSIZE/2;
ry*=TSIZE/2;
rz*=TSIZE/2;

// p->rotation

  glBegin(GL_QUADS); // berk... but i'll switch to triangles one day ;)
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(p->position[0] + (-rx - ux),
	     p->position[1] + (-ry - uy),
	     p->position[2] + (-rz - uz));
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(p->position[0] + (rx - ux),
	     p->position[1] + (ry - uy),
	     p->position[2] + (rz - uz));
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(p->position[0] + (rx + ux),
	     p->position[1] + (ry + uy),
	     p->position[2] + (rz + uz));
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(p->position[0] + (ux - rx),
	     p->position[1] + (uy - ry),
	     p->position[2] + (uz - rz));
  glEnd();
}


void raydium_particle_scale_all(GLfloat scale)
{
raydium_particle_scale_factor=scale;
}

void raydium_particle_draw_all(void)
{
GLuint i;
GLuint texsave;
char light;
GLfloat modmat[16];
GLfloat ux;
GLfloat uy;
GLfloat uz;
GLfloat rx;
GLfloat ry;
GLfloat rz;

//raydium_profile_start();

texsave=raydium_texture_current_main;
light=raydium_light_enabled_tag;
raydium_light_disable();
if (raydium_camera_pushed) raydium_camera_replace(); // is it really our job to do it here ?
//raydium_rendering_internal_restore_render_state();

glGetFloatv(GL_MODELVIEW_MATRIX,modmat);
ux=modmat[0]*raydium_particle_scale_factor;
uy=modmat[4]*raydium_particle_scale_factor;
uz=modmat[8]*raydium_particle_scale_factor;
rx=modmat[1]*raydium_particle_scale_factor;
ry=modmat[5]*raydium_particle_scale_factor;
rz=modmat[9]*raydium_particle_scale_factor;

glDepthMask(GL_FALSE);

for(i=0;i<RAYDIUM_MAX_PARTICLES;i++)
    if(raydium_particle_particles[i])
	raydium_particle_draw(raydium_particle_particles[i],ux,uy,uz,rx,ry,rz);

glDepthMask(GL_TRUE);
if(light) raydium_light_enable();

raydium_texture_current_set(texsave);
//raydium_rendering_internal_prepare_texture_render(texsave);
//raydium_profile_end("particles drawing");
}


void raydium_particle_generator_move(int gen, GLfloat *pos)
{
if(!raydium_particle_generator_isvalid(gen))
    {
    raydium_log("particle: cannot move generator: invalid name or index");
    return;
    }
memcpy(raydium_particle_generators[gen].position_user,pos,sizeof(GLfloat)*3);
}

void raydium_particle_generator_move_name(char *gen, GLfloat *pos)
{
raydium_particle_generator_move(raydium_particle_generator_find(gen),pos);
}

void raydium_particle_generator_move_name_3f(char *gen, GLfloat x, GLfloat y, GLfloat z)
{
GLfloat tmp[3];
tmp[0]=x;
tmp[1]=y;
tmp[2]=z;
raydium_particle_generator_move_name(gen,tmp);
}

void raydium_particle_generator_particles_OnDelete(int gen, void *OnDelete)
{
if(!raydium_particle_generator_isvalid(gen))
    {
    raydium_log("particle: cannot set OnDelete: invalid name or index");
    return;
    }
raydium_particle_generators[gen].OnDeleteParticle=OnDelete;
}

void raydium_particle_generator_particles_OnDelete_name(char *gen, void *OnDelete)
{
raydium_particle_generator_particles_OnDelete(raydium_particle_generator_find(gen),OnDelete);
}
