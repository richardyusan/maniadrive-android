/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/ode.h"
#endif 

// TODO:
// load object from file
// delete callbaks (objects, motors, ... ?) (done for joints, elements)
// parametric frictions
// other joints
// allow full RayODE reset (almost done now)
// multiple grounds
// bugfix for rotation friction

#include "ode.h"

void raydium_ode_name_auto(char *prefix, char *dest)
{
static int counter;
sprintf(dest,"%s_ode_%i",prefix,counter);
counter++;
}


void raydium_ode_init_object(int i)
{
raydium_ode_object[i].id=i;
raydium_ode_object[i].name[0]=0;
raydium_ode_object[i].state=0;
raydium_ode_object[i].colliding=0;
raydium_ode_object[i].group=NULL;
}


void raydium_ode_init_element(int i)
{
int j;
raydium_ode_element[i].id=i;
raydium_ode_element[i].name[0]=0;
raydium_ode_element[i].state=0;
raydium_ode_element[i].object=-1;
raydium_ode_element[i].mesh=-1;
raydium_ode_element[i]._touched=0;
raydium_ode_element[i]._movesfrom=-1;
raydium_ode_element[i].isplayer=0;
raydium_ode_element[i].playerangle=0;
raydium_ode_element[i].slip=0;
raydium_ode_element[i].rotfriction=0;
raydium_ode_element[i].geom=NULL;
raydium_ode_element[i].body=NULL;
raydium_ode_element[i].erp=0;
raydium_ode_element[i].cfm=0;
raydium_ode_element[i].user_data=NULL;
raydium_ode_element[i].user_tag=0;
// I consider every fixed element is already free()
for(j=0;j<RAYDIUM_ODE_ELEMENT_MAX_FIXING;j++)
    raydium_ode_element[i].fixed_elements[j]=NULL;
raydium_ode_element[i].nid=-1;
raydium_ode_element[i].distant=0;
raydium_ode_element[i].distant_owner=-1;
raydium_ode_element[i].OnBlow=NULL;
raydium_ode_element[i].OnDelete=NULL;
raydium_ode_element[i].ttl=-1; // infinite
raydium_ode_element[i].particle=-1;
memset(raydium_ode_element[i].particle_offset,0,sizeof(dReal)*3);
raydium_ode_element[i].ground_texture=0;
memset(raydium_ode_element[i].net_last_pos1,0,sizeof(dReal)*3);
memset(raydium_ode_element[i].net_last_pos2,0,sizeof(dReal)*3);
//raydium_ode_element[i].net_last_time=raydium_timecall_clock();
raydium_ode_element[i].net_last_time=0; // no previous update
raydium_ode_element[i].net_last_interval=0;
memset(raydium_ode_element[i].netvel,0,sizeof(dReal)*3);
raydium_ode_element[i].marked_as_deleted=0;
raydium_ode_element[i].ray.state=0;
}

void raydium_ode_init_joint(int i)
{
raydium_ode_joint[i].id=i;
raydium_ode_joint[i].name[0]=0;
raydium_ode_joint[i].state=0;
raydium_ode_joint[i].mesh=-1;	    
raydium_ode_joint[i].joint=NULL;
//raydium_ode_joint[i].hinge2correct=0;
raydium_ode_joint[i].breakableforce=0;
raydium_ode_joint[i].breaking=0;
raydium_ode_joint[i].OnDelete=NULL;
}

void raydium_ode_init_motor(int i)
{
int j;

raydium_ode_motor[i].id=i;
raydium_ode_motor[i].name[0]=0;
raydium_ode_motor[i].state=0;
raydium_ode_motor[i].object=-1;
raydium_ode_motor[i].speed=0;
raydium_ode_motor[i].force=0;
raydium_ode_motor[i].angle=0;
raydium_ode_motor[i].rocket_element=-1;
raydium_ode_motor[i].rocket_playermovement=0;
for(j=0;j<3;j++)
    {
    raydium_ode_motor[i].rocket_direction[j]=0;
    raydium_ode_motor[i].rocket_position[j]=0;
    raydium_ode_motor[i].rocket_orientation[j]=0;
    }

for(j=0;j<RAYDIUM_ODE_MOTOR_MAX_JOINTS;j++)
    {
    raydium_ode_motor[i].joints[j]=-1;
    raydium_ode_motor[i].joints_axe[j]=0;
    }
raydium_ode_motor[i].gears[0]=1.f;
raydium_ode_motor[i].gear=0;
raydium_ode_motor[i].gear_max=0;
}

void raydium_ode_init_explosion(int e)
{
int j;
raydium_ode_explosion[e].id=e;
raydium_ode_explosion[e].name[0]=0;
raydium_ode_explosion[e].state=0;
raydium_ode_explosion[e].config_radius=0;
raydium_ode_explosion[e].config_propag=0;
raydium_ode_explosion[e].radius=0;
raydium_ode_explosion[e].element=-1;
for(j=0;j<3;j++)
    raydium_ode_explosion[e].position[j]=0;
}

void raydium_ode_init(void)
{
int i;

if(sizeof(dReal) != sizeof(float))
    {
    raydium_log("physics: FAILED");
    raydium_log("physics: You need SINGLE precision for ODE");
    exit(34);
    }

raydium_ode_ExplosionCallback=NULL;
raydium_ode_CollideCallback=NULL;
raydium_ode_RayCallback=NULL;
raydium_ode_StepCallback=NULL;
raydium_ode_BeforeElementDrawCallback=NULL;
raydium_ode_AfterElementDrawCallback=NULL;
raydium_ode_ObjectNearCollide=NULL;
raydium_ode_world=dWorldCreate();
raydium_ode_space=dHashSpaceCreate(0);
raydium_ode_contactgroup=dJointGroupCreate(0);
dWorldSetGravity(raydium_ode_world,0,0,-1.0);
raydium_ode_ground_mesh=-1;
raydium_ode_network_distant_create=0;
raydium_ode_network_next_local_only=0;
raydium_ode_network_explosion_create=0;
raydium_ode_element_delete_LOCK=0;

for(i=0;i<RAYDIUM_ODE_MAX_OBJECTS;i++)
    raydium_ode_init_object(i);

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    raydium_ode_init_element(i);

raydium_ode_init_element(RAYDIUM_ODE_MAX_ELEMENTS);
raydium_ode_element[RAYDIUM_ODE_MAX_ELEMENTS].state=1;
strcpy(raydium_ode_element[RAYDIUM_ODE_MAX_ELEMENTS].name,"! dummy !");

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    raydium_ode_init_joint(i);

for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    raydium_ode_init_motor(i);

for(i=0;i<RAYDIUM_ODE_MAX_EXPLOSIONS;i++)
    raydium_ode_init_explosion(i);

#ifndef ODE_MANUAL_CALLBACK
raydium_ode_timecall=raydium_timecall_add(&raydium_ode_callback,RAYDIUM_ODE_PHYSICS_FREQ);
#endif

#ifdef ODE_QUICKSTEP
dWorldSetQuickStepNumIterations (raydium_ode_world,10);
//dWorldSetAutoDisableFlag (raydium_ode_world,1);
//dWorldSetContactMaxCorrectingVel (raydium_ode_world,0.1);
//dWorldSetContactSurfaceLayer (raydium_ode_world,0.001);
#endif
// must adjust this value /!!!/
dWorldSetContactMaxCorrectingVel(raydium_ode_world,10);

// wow ! This is my first init dependance ;)
raydium_ode_object_create("GLOBAL");
raydium_ode_network_init();

raydium_log("physics: ODE Net: %i element(s)/packet",raydium_ode_network_MaxElementsPerPacket());
raydium_log("physics: OK");
}


signed char raydium_ode_object_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_ODE_MAX_OBJECTS &&
   raydium_ode_object[i].state)
    return 1;
return 0;
}

signed char raydium_ode_element_isvalid(int i)
{
if(i>=0 && i<=RAYDIUM_ODE_MAX_ELEMENTS && /* "<=" is important, see ode.h ! */
   raydium_ode_element[i].state)
    return 1;
return 0;
}

signed char raydium_ode_joint_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_ODE_MAX_JOINTS &&
   raydium_ode_joint[i].state)
    return 1;
return 0;
}

signed char raydium_ode_motor_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_ODE_MAX_MOTORS &&
   raydium_ode_motor[i].state)
    return 1;
return 0;
}

signed char raydium_ode_explosion_isvalid(int i)
{
if(i>=0 && i<RAYDIUM_ODE_MAX_EXPLOSIONS &&
   raydium_ode_explosion[i].state)
    return 1;
return 0;
}


void raydium_ode_ground_dTriArrayCallback (
	dGeomID TriMesh, dGeomID RefObject,
        const int* TriIndices, int TriCount)
{
int offset;
int max_index;
GLfloat max_val,tmp;
dReal *pos;
int i;
raydium_ode_Element *e;

e=dGeomGetData(RefObject);
pos=(dReal *)dGeomGetPosition(RefObject);

if(!e) return;
// add ground object offset:
offset=raydium_object_start[raydium_ode_ground_mesh];
//one=0;
//one+=(TriIndices[0]*3);
//printf("%i %s\n",TriCount,e->name);
//printf("%i\n",one);
//e->ground_texture=raydium_vertex_texture[one];

max_val=
    ((pos[0]-raydium_vertex_x[TriIndices[0]*3+offset]) +
     (pos[1]-raydium_vertex_y[TriIndices[0]*3+offset]) +
     (pos[2]-raydium_vertex_z[TriIndices[0]*3+offset]) );
max_index=0;

for(i=1;i<TriCount;i++)
    {
    tmp=
    ((pos[0]-raydium_vertex_x[TriIndices[i]*3+offset]) +
     (pos[1]-raydium_vertex_y[TriIndices[i]*3+offset]) +
     (pos[2]-raydium_vertex_z[TriIndices[i]*3+offset]) );
    if(tmp<=max_val)
	{
	max_index=i;
	max_val=tmp;
	}    
    }

//printf("%i %s %i %i\n",TriCount,e->name,TriIndices[0],TriIndices[1]);
e->ground_texture=raydium_vertex_texture[TriIndices[max_index]*3+offset];
raydium_vertex_tag[TriIndices[max_index]*3+offset]=1;
}


int raydium_ode_ground_dTriCallback(
		dGeomID TriMesh,
		dGeomID RefObject,
		int TriangleIndex)
{
//printf("%i\n",TriangleIndex);
raydium_vertex_tag[TriangleIndex*3]=1;
return 1;
}

void raydium_ode_ground_set_name(char *name)
{
int i,j,k;
int obj,size;

static dTriMeshDataID Data;
static int *Indices;
static dReal *Vertices;
static dGeomID geom;

int global;
char replace=0;


if(raydium_ode_ground_mesh>=0)
    {
    //raydium_log("ODE: Error: Cannot change ground at runtime");
    //return;
    // OR
    replace=1;
    dGeomDestroy(geom);
    dGeomTriMeshDataDestroy(Data);
    /*
    Data=dGeomTriMeshDataCreate();
     dGeomTriMeshDataBuildSingle(Data,&Vertices[0],sizeof(dReal)*3,size,&Indices[0],size,3*sizeof(int));
    geom=dCreateTriMesh(raydium_ode_space, Data, 0, 0, 0);
    */
    free(Indices);
    free(Vertices);
    }

obj=raydium_object_find_load(name);
if(obj<0)
    {
    raydium_log("ODE: Error: cannot load ground (%s)",name);
    return;
    }

// let's transform tri file format to ODE format ...
size=raydium_object_end[obj]-raydium_object_start[obj];
Indices=malloc(size*sizeof(int));
Vertices=malloc(size*sizeof(dReal)*3);
raydium_ode_ground_mesh=obj;

for(i=raydium_object_start[obj],j=k=0;i<raydium_object_end[obj];i+=3,j+=3)
{
Indices[j]=j;
Vertices[k++]=raydium_vertex_x[i];
Vertices[k++]=raydium_vertex_y[i];
Vertices[k++]=raydium_vertex_z[i];

Indices[j+1]=j+1;
Vertices[k++]=raydium_vertex_x[i+1];
Vertices[k++]=raydium_vertex_y[i+1];
Vertices[k++]=raydium_vertex_z[i+1];

Indices[j+2]=j+2;
Vertices[k++]=raydium_vertex_x[i+2];
Vertices[k++]=raydium_vertex_y[i+2];
Vertices[k++]=raydium_vertex_z[i+2];
}

// There is NO way to delete the ground, yet ...
Data=dGeomTriMeshDataCreate();

// with newer ODE versions, use the following:
dGeomTriMeshDataBuildSingle(Data,&Vertices[0],sizeof(dReal)*3,size,&Indices[0],size,3*sizeof(int));
// and the following with ODE .039:
//dGeomTriMeshDataBuild(Data,&Vertices[0],sizeof(dReal)*3,size,&Indices[0], size, 3*sizeof(int));

geom=dCreateTriMesh(raydium_ode_space, Data, 0, 0, 0);

//if(!replace)
//    global=raydium_ode_object_create("GLOBAL");
//else
global=raydium_ode_object_find("GLOBAL");

raydium_ode_object_colliding(global,1);

raydium_ode_init_element(0);
raydium_ode_element[0].object=global;
strcpy(raydium_ode_element[0].name,"ground");
raydium_ode_element[0].state=RAYDIUM_ODE_STATIC;
raydium_ode_element[0].mesh=obj;
raydium_ode_element[0].geom=geom;
raydium_ode_element[0].user_tag=RAYDIUM_ODE_TAG_GROUND;
raydium_ode_element_material(0,RAYDIUM_ODE_MATERIAL_DEFAULT);
raydium_ode_element_slip(0,RAYDIUM_ODE_SLIP_DEFAULT);
dGeomSetData(geom,&raydium_ode_element[0]);

// sets Tri[Array]Callback: (no usable yet !)
//dGeomTriMeshSetArrayCallback(geom,raydium_ode_ground_dTriArrayCallback);
//dGeomTriMeshSetCallback(geom,raydium_ode_ground_dTriCallback);

raydium_shadow_ground_change(obj); // send new ground to shadow system
}


int raydium_ode_object_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_OBJECTS;i++)
    if(!strcmp(name,raydium_ode_object[i].name) && raydium_ode_object_isvalid(i))
     return i;
return -1;
}


int raydium_ode_element_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(!strcmp(name,raydium_ode_element[i].name) && raydium_ode_element_isvalid(i))
     return i;
return -1;
}

int raydium_ode_joint_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(!strcmp(name,raydium_ode_joint[i].name) && raydium_ode_joint_isvalid(i))
     return i;
return -1;
}

int raydium_ode_motor_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(!strcmp(name,raydium_ode_motor[i].name) && raydium_ode_motor_isvalid(i))
     return i;
return -1;
}

int raydium_ode_explosion_find(char *name)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_EXPLOSIONS;i++)
    if(!strcmp(name,raydium_ode_explosion[i].name))
     return i;
return -1;
}

int raydium_ode_object_create(char *name)
{
int i;

if(raydium_ode_object_find(name)>=0)
    {
    raydium_log("ODE: Error: Cannot add object \"%s\": name already exists",name);
    return -1;
    }

for(i=0;i<RAYDIUM_ODE_MAX_OBJECTS;i++)
    if(!raydium_ode_object[i].state)
     {
     strcpy(raydium_ode_object[i].name,name);
     raydium_ode_object[i].state=1;
     // raydium_ode_object[i].group=dCreateGeomGroup(raydium_ode_space);
     raydium_ode_object[i].group=dSimpleSpaceCreate(raydium_ode_space);
     dSpaceSetCleanup(raydium_ode_object[i].group,0);
     dGeomSetData((dGeomID)raydium_ode_object[i].group,&raydium_ode_object[i]);
     return i;
     }
raydium_log("ODE: Error: No more object slots ! aborting \"%s\" creation",name);
return -1;
}


signed char raydium_ode_object_rename(int o, char *newname)
{
int elem;

if(!raydium_ode_object_isvalid(o))
    {
    raydium_log("ODE: Error: cannot rename object: invalid name or index");
    return 0;
    }

elem=raydium_ode_object_find(newname);
if(elem>=0)
    {
    raydium_log("ODE: Error: cannot rename '%s' to '%s': name already exists");
    return 0;
    }

strcpy(raydium_ode_object[o].name,newname);
return 1;
}

signed char raydium_ode_object_rename_name(char *o, char *newname)
{
return raydium_ode_object_rename(raydium_ode_object_find(o),newname);
}


signed char raydium_ode_object_colliding(int o, signed char colliding)
{
if(!raydium_ode_object_isvalid(o))
    {
    raydium_log("ODE: Error: cannot set object internal colliding flag: invalid name or index");
    return 0;
    }
raydium_ode_object[o].colliding=colliding;
return 1;
}


signed char raydium_ode_object_colliding_name(char *o, signed char colliding)
{
return raydium_ode_object_colliding(raydium_ode_object_find(o),colliding);
}



void raydium_ode_object_linearvelocity_set(int o, dReal *vect)
{
int i;
if(!raydium_ode_object_isvalid(o))
    {
    raydium_log("ODE: Error: cannot set object linear velocity: invalid name or index");
    return;
    }
for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if( (raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD ||
         raydium_ode_element[i].state==RAYDIUM_ODE_STATIC) &&
         raydium_ode_element[i].object==o)
	    dBodySetLinearVel(raydium_ode_element[i].body,vect[0],vect[1],vect[2]);
}

void raydium_ode_object_linearvelocity_set_name(char *o, dReal *vect)
{
raydium_ode_object_linearvelocity_set(raydium_ode_object_find(o),vect);
}

void raydium_ode_object_linearvelocity_set_name_3f(char *o, dReal vx, dReal vy, dReal vz)
{
dReal vect[3];
vect[0]=vx;
vect[1]=vy;
vect[2]=vz;
raydium_ode_object_linearvelocity_set_name(o,vect);
}

void raydium_ode_object_addforce(int o, dReal *vect)
{
int i;
if(!raydium_ode_object_isvalid(o))
    {
    raydium_log("ODE: Error: cannot add force to object: invalid name or index");
    return;
    }
for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if( raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD &&
        raydium_ode_element[i].object==o)
	    dBodyAddForce(raydium_ode_element[i].body,vect[0],vect[1],vect[2]);
}

void raydium_ode_object_addforce_name(char *o, dReal *vect)
{
raydium_ode_object_addforce(raydium_ode_object_find(o),vect);
}

void raydium_ode_object_addforce_name_3f(char *o, dReal vx, dReal vy, dReal vz)
{
dReal vect[3];
vect[0]=vx;
vect[1]=vy;
vect[2]=vz;
raydium_ode_object_addforce_name(o,vect);
}

void raydium_ode_element_addforce(int e, dReal *vect)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot add force to element: invalid name or index");
    return;
    }
if(raydium_ode_element[e].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: cannot add force to a static element");
    return;
    }
		
dBodyAddForce(raydium_ode_element[e].body,vect[0],vect[1],vect[2]);
}

void raydium_ode_element_addforce_name(char *e, dReal *vect)
{
raydium_ode_element_addforce(raydium_ode_element_find(e),vect);
}

void raydium_ode_element_addforce_name_3f(char *e, dReal vx, dReal vy, dReal vz)
{
dReal vect[3];
vect[0]=vx;
vect[1]=vy;
vect[2]=vz;
raydium_ode_element_addforce_name(e,vect);
}

void raydium_ode_element_addtorque(int e, dReal *vect)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot add torque to element: invalid name or index");
    return;
    }
if(raydium_ode_element[e].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: cannot add torque to a static element");
    return;
    }
		
dBodyAddTorque(raydium_ode_element[e].body,vect[0],vect[1],vect[2]);
}

void raydium_ode_element_addtorque_name(char *e, dReal *vect)
{
raydium_ode_element_addtorque(raydium_ode_element_find(e),vect);
}

void raydium_ode_element_addtorque_name_3f(char *e, dReal vx, dReal vy, dReal vz)
{
dReal vect[3];
vect[0]=vx;
vect[1]=vy;
vect[2]=vz;
raydium_ode_element_addtorque_name(e,vect);
}


signed char raydium_ode_element_material(int e, dReal erp, dReal cfm)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set element material: invalid name or index");
    return 0;
    }
raydium_ode_element[e].erp=erp;
raydium_ode_element[e].cfm=cfm;
return 1;
}

signed char raydium_ode_element_material_name(char *name, dReal erp, dReal cfm)
{
return raydium_ode_element_material(raydium_ode_element_find(name),erp,cfm);
}


signed char raydium_ode_element_slip(int e, dReal slip)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set element slip coef: invalid name or index");
    return 0;
    }
raydium_ode_element[e].slip=slip;
return 1;
}

signed char raydium_ode_element_slip_name(char *e, dReal slip)
{
return raydium_ode_element_slip(raydium_ode_element_find(e),slip);
}

signed char raydium_ode_element_rotfriction(int e, dReal rotfriction)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set element rotation friction: invalid name or index");
    return 0;
    }
raydium_ode_element[e].rotfriction=rotfriction;
return 1;
}

signed char raydium_ode_element_rotfriction_name(char *e, dReal rotfriction)
{
return raydium_ode_element_rotfriction(raydium_ode_element_find(e),rotfriction);
}

dReal *raydium_ode_element_linearvelocity_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get element linear velocity: invalid name or index");
    return NULL;
    }
if(raydium_ode_element[e].state!=RAYDIUM_ODE_STANDARD)
    {
    raydium_log("ODE: Error: cannot get element linear velocity: not a standard object");
    return NULL;
    }
return (dReal *)dBodyGetLinearVel(raydium_ode_element[e].body);
}

dReal *raydium_ode_element_linearvelocity_get_name(char *e)
{
return raydium_ode_element_linearvelocity_get(raydium_ode_element_find(e));
}

void raydium_ode_element_OnBlow(int e, void *OnBlow)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set OnBlow callback: invalid name or index");
    return;
    }
raydium_ode_element[e].OnBlow=OnBlow;
}

void raydium_ode_element_OnBlow_name(char *e, void *OnBlow)
{
raydium_ode_element_OnBlow(raydium_ode_element_find(e),OnBlow);
}

void raydium_ode_element_OnDelete(int e, void *OnDelete)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set OnDelete callback: invalid name or index");
    return;
    }
raydium_ode_element[e].OnDelete=OnDelete;
}

void raydium_ode_element_OnDelete_name(char *e, void *OnDelete)
{
raydium_ode_element_OnDelete(raydium_ode_element_find(e),OnDelete);
}

void raydium_ode_element_gravity(int e, signed char enable)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set gravity mode: invalid name or index");
    return;
    }

if(raydium_ode_element[e].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: cannot set gravity mode on a static element");
    return;
    }

dBodySetGravityMode(raydium_ode_element[e].body,enable);
}

void raydium_ode_element_gravity_name(char *e, signed char enable)
{
raydium_ode_element_gravity(raydium_ode_element_find(e),enable);
}

void raydium_ode_element_ttl_set(int e, int ttl)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set ttl: invalid name or index");
    return;
    }
raydium_ode_element[e].ttl=ttl;
}

void raydium_ode_element_ttl_set_name(char *e, int ttl)
{
raydium_ode_element_ttl_set(raydium_ode_element_find(e),ttl);
}


// aabb is dReal[6]
signed char raydium_ode_element_aabb_get(int element, dReal *aabb)
{
if(!raydium_ode_element_isvalid(element))
    {
    raydium_log("ODE: Error: cannot get AABB: invalid name or index");
    return 0;
    }
dGeomGetAABB(raydium_ode_element[element].geom,aabb);
return 1;
}

signed char raydium_ode_element_aabb_get_name(char *element, dReal *aabb)
{
return raydium_ode_element_aabb_get(raydium_ode_element_find(element),aabb);
}

int raydium_ode_element_touched_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get 'touched' flag: invalid name or index");
    return 0;
    }
return raydium_ode_element[e]._touched;
}

int raydium_ode_element_touched_get_name(char *e)
{
return raydium_ode_element_touched_get(raydium_ode_element_find(e));
}

signed char raydium_ode_element_player_set(int e, signed char isplayer)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set player flag: invalid name or index");
    return 0;
    }
raydium_ode_element[e].isplayer=isplayer;
if(isplayer)
    raydium_ode_element_slip(e,RAYDIUM_ODE_SLIP_PLAYER);
else
    raydium_ode_element_slip(e,RAYDIUM_ODE_SLIP_DEFAULT);

return 1;
}

signed char raydium_ode_element_player_set_name(char *name, signed char isplayer)
{
return raydium_ode_element_player_set(raydium_ode_element_find(name),isplayer);
}

signed char raydium_ode_element_player_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get player flag: invalid name or index");
    return 0;
    }
return raydium_ode_element[e].isplayer;
}

signed char raydium_ode_element_player_get_name(char *name)
{
return raydium_ode_element_player_get(raydium_ode_element_find(name));
}

signed char raydium_ode_element_player_angle(int e, dReal angle)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set player angle: invalid name or index");
    return 0;
    }
raydium_ode_element[e].playerangle=angle;
return 1;
}

signed char raydium_ode_element_player_angle_name(char *e, dReal angle)
{
return raydium_ode_element_player_angle(raydium_ode_element_find(e),angle);
}


int raydium_ode_element_ground_texture_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get ground texture: invalid name or index");
    return 0;
    }
return raydium_ode_element[e].ground_texture;
}

int raydium_ode_element_ground_texture_get_name(char *e)
{
return raydium_ode_element_ground_texture_get(raydium_ode_element_find(e));
}

int raydium_ode_element_object_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get element's owner: invalid name or index");
    return 0;
    }
return raydium_ode_element[e].object;
}

int raydium_ode_element_object_get_name(char *e)
{
return raydium_ode_element_object_get(raydium_ode_element_find(e));
}

int raydium_ode_object_sphere_add(char *name, int group, dReal mass, dReal radius, signed char type, int tag, char *mesh)
{
int i;
dMass m;

if(raydium_ode_element_find(name)>=0)
    {
    raydium_log("ODE: Error: Cannot add element \"%s\": name already exists",name);
    return -1;
    }

if(!raydium_ode_object_isvalid(group))
    {
    raydium_log("ODE: Error: object not found while adding \"%s\"",name);
    return -1;
    }

if(tag<0)
    {
    raydium_log("ODE: Error: Element creation failed: negative tags are forbidden");
    return -1;
    }

// First element is reserved
for(i=1;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(!raydium_ode_element[i].state)
     {
     strcpy(raydium_ode_element[i].name,name);
     raydium_ode_element[i].object=group;
     raydium_ode_element[i].user_tag=tag;
     if(strlen(mesh))
     {
      raydium_ode_element[i].mesh=raydium_object_find_load(mesh);
      if(radius<0) // AUTODETECT
         radius=raydium_object_find_dist_max(raydium_ode_element[i].mesh)*-radius;
     }

     if(type==RAYDIUM_ODE_STANDARD)
     {
        raydium_ode_element[i].body=dBodyCreate(raydium_ode_world);
        dMassSetSphere(&m,1,radius);
	dMassAdjust(&m,mass);
        dBodySetMass(raydium_ode_element[i].body,&m);
        dBodySetData(raydium_ode_element[i].body,&raydium_ode_element[i]);
//	dBodySetAutoDisableSF1(raydium_ode_element[i].body,1);
     }  
     else raydium_ode_element[i].body=0;

     raydium_ode_element[i].geom=dCreateSphere(0,radius);
     raydium_ode_element[i].state=type;
     dGeomSetBody(raydium_ode_element[i].geom,raydium_ode_element[i].body);
     dGeomSetData(raydium_ode_element[i].geom,&raydium_ode_element[i]);
     dSpaceAdd(raydium_ode_object[group].group,raydium_ode_element[i].geom);
     raydium_ode_element_material(i,RAYDIUM_ODE_MATERIAL_DEFAULT);
     raydium_ode_element_slip(i,RAYDIUM_ODE_SLIP_DEFAULT);
     raydium_ode_element[i].distant=raydium_ode_network_distant_create;
     raydium_ode_network_distant_create=0;
     if(!raydium_ode_network_next_local_only)
        raydium_ode_network_element_new(i);
     raydium_ode_network_next_local_only=0;
     return i;
     }
raydium_log("ODE: No more element slots ! aborting \"%s\" creation",name);
return -1;

}

int raydium_ode_object_box_add(char *name, int group, dReal mass, dReal tx, dReal ty, dReal tz, signed char type, int tag, char *mesh)
{
int i;
dMass m;

if(raydium_ode_element_find(name)>=0)
    {
    raydium_log("ODE: Cannot add element \"%s\": name already exists",name);
    return -1;
    }

if(!raydium_ode_object_isvalid(group))
    {
    raydium_log("ODE: Error: object not found while adding \"%s\"",name);
    return -1;
    }

if(tag<0)
    {
    raydium_log("ODE: Error: Element creation failed: negative tags are forbidden");
    return -1;
    }

// First element is reserved
for(i=1;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(!raydium_ode_element[i].state)
     {
     strcpy(raydium_ode_element[i].name,name);
     raydium_ode_element[i].object=group;
     raydium_ode_element[i].user_tag=tag;
     if(strlen(mesh))
     {
      raydium_ode_element[i].mesh=raydium_object_find_load(mesh);
      if(tx<0) // AUTODETECT
         {
	 int ratio=tx;
         raydium_object_find_axes_max(raydium_ode_element[i].mesh,&tx,&ty,&tz);
	 tx*=(-ratio);
	 ty*=(-ratio);
	 tz*=(-ratio);
	 }
     }

     if(type==RAYDIUM_ODE_STANDARD)
     {
        raydium_ode_element[i].body=dBodyCreate(raydium_ode_world);
        dMassSetBox(&m,1,tx,ty,tz);
	dMassAdjust(&m,mass);
        dBodySetMass(raydium_ode_element[i].body,&m);
        dBodySetData(raydium_ode_element[i].body,&raydium_ode_element[i]);
//	dBodySetAutoDisableSF1(raydium_ode_element[i].body,1);
     } 
     else raydium_ode_element[i].body=0;

     raydium_ode_element[i].geom=dCreateBox(0,tx,ty,tz);
     raydium_ode_element[i].state=type;
     dGeomSetBody(raydium_ode_element[i].geom,raydium_ode_element[i].body);
     dGeomSetData(raydium_ode_element[i].geom,&raydium_ode_element[i]);
     dSpaceAdd(raydium_ode_object[group].group,raydium_ode_element[i].geom);
     raydium_ode_element_material(i,RAYDIUM_ODE_MATERIAL_DEFAULT);
     raydium_ode_element_slip(i,RAYDIUM_ODE_SLIP_DEFAULT);
     raydium_ode_element[i].distant=raydium_ode_network_distant_create;
     raydium_ode_network_distant_create=0;
     if(!raydium_ode_network_next_local_only)
        raydium_ode_network_element_new(i);
     raydium_ode_network_next_local_only=0;
     return i;
     }
raydium_log("ODE: No more element slots ! aborting \"%s\" creation",name);
return -1;

}

signed char raydium_ode_element_ray_attach(int element, dReal length, dReal dirx, dReal diry, dReal dirz)
{
raydium_ode_Element *e;

if(!raydium_ode_element_isvalid(element))
    {
    raydium_log("ODE: Error: Cannot attach ray to element: element is not valid");
    return 0;
    }

if(raydium_ode_element[element].state!=RAYDIUM_ODE_STANDARD)
    {
    raydium_log("ODE: Error: Cannot attach ray to non standard elements");
    return 0;
    }

e=&raydium_ode_element[element];

// There's no problem with any previous ray, we only update it
if(!e->ray.state)
    {
    e->ray.geom=dCreateRay(raydium_ode_object[raydium_ode_object_find("GLOBAL")].group,length);
    dGeomSetData(e->ray.geom,e);
    e->ray.state=1;
    }
dGeomRaySetLength(e->ray.geom,length);
e->ray.rel_dir[0]=dirx;
e->ray.rel_dir[1]=diry;
e->ray.rel_dir[2]=dirz;
return 1;
}

signed char raydium_ode_element_ray_attach_name(char *element, dReal length, dReal dirx, dReal diry, dReal dirz)
{
return raydium_ode_element_ray_attach(raydium_ode_element_find(element),length,dirx,diry,dirz);
}

signed char raydium_ode_element_ray_delete(int element)
{
raydium_ode_Element *e;

if(!raydium_ode_element_isvalid(element))
    {
    raydium_log("ODE: Error: Cannot delete ray from element: element is not valid");
    return 0;
    }

e=&raydium_ode_element[element];

if(!e->ray.state)
    {
    raydium_log("ODE: Error: Cannot delete ray from element: there's no ray");
    return 0;
    }

dGeomDestroy(e->ray.geom);
e->ray.state=0;
return 1;
}

signed char raydium_ode_element_ray_delete_name(char *element)
{
return raydium_ode_element_ray_delete(raydium_ode_element_find(element));
}

signed char raydium_ode_element_ray_get(int element, raydium_ode_Ray *result)
{
raydium_ode_Element *e;

if(!raydium_ode_element_isvalid(element))
    {
    raydium_log("ODE: Error: Cannot get ray informations: element is not valid");
    return 0;
    }

e=&raydium_ode_element[element];

if(!e->ray.state)
    {
    raydium_log("ODE: Error: Cannot get ray informations: there's no ray");
    return 0;
    }
memcpy(result,&e->ray,sizeof(raydium_ode_Ray));
return 1;
}

signed char raydium_ode_element_ray_get_name(char *element, raydium_ode_Ray *result)
{
return raydium_ode_element_ray_get(raydium_ode_element_find(element),result);
}

int raydium_ode_element_fix(char *name, int *elem, int nelems, signed char keepgeoms)
{
dReal aabb[6];
dReal aabbR[6];
dReal bounding[3];
dReal position[3];
int group;
int elemN;
int i,j;
int done;
dReal *data;
dMass m;
dReal mass;


if(keepgeoms)
    raydium_log("ODE: element_fix: keepgeoms not implemented yet !");

if(raydium_ode_element_find(name)>=0)
    {
    raydium_log("ODE: Error: Cannot fix elements as \"%s\": name already exists",name);
    return -1;
    }

if(nelems<1)
    {
    raydium_log("ODE: Error: Must fix at least one element");
    return -1;    
    }

for(i=0;i<nelems;i++)
 if(!raydium_ode_element_isvalid(elem[i]))
    {
    raydium_log("ODE: Error: Cannot fix elements: one element is not valid");
    return -1;
    }

for(i=0;i<nelems;i++)
 if(raydium_ode_element[elem[i]].state!=RAYDIUM_ODE_STANDARD)
    {
    raydium_log("ODE: Error: Cannot fix non standard elements");
    return -1;
    }


group=raydium_ode_element[elem[0]].object;
for(i=1;i<nelems;i++)
 if(raydium_ode_element[elem[i]].object!=group)
    {
    raydium_log("ODE: Error: Cannot fix elements: not owned by the same object");
    return -1;
    }

// 2 - create new element (using AABB from elements)
dGeomGetAABB(raydium_ode_element[elem[0]].geom,aabbR);
    
for(i=1;i<nelems;i++)
    {
    dGeomGetAABB(raydium_ode_element[elem[i]].geom,aabb);
    for(j=0;j<6;j+=2)
	{
	aabbR[j  ]=raydium_trigo_min(aabb[j  ],aabbR[j  ]);
	aabbR[j+1]=raydium_trigo_max(aabb[j+1],aabbR[j+1]);
	}
    }

bounding[0]=aabbR[1]-aabbR[0];
bounding[1]=aabbR[3]-aabbR[2];
bounding[2]=aabbR[5]-aabbR[4];

position[0]=aabbR[0]+(bounding[0]/2.f);
position[1]=aabbR[2]+(bounding[1]/2.f);
position[2]=aabbR[4]+(bounding[2]/2.f);

// Now, we can create an new box element with some dummy values:
elemN=raydium_ode_object_box_add(name,group,0.f,bounding[0],bounding[1],bounding[2],RAYDIUM_ODE_STANDARD,0,"");
raydium_ode_element[elemN].state=RAYDIUM_ODE_FIXING;
raydium_ode_element_move(elemN,position);

// 3 - save elements to new element
done=0;
mass=0;
for(i=0;i<nelems;i++)
 for(j=0;j<RAYDIUM_ODE_ELEMENT_MAX_FIXING;j++)
    if(raydium_ode_element[elemN].fixed_elements[j]==NULL)
	{
	raydium_ode_element[elemN].fixed_elements[j]=malloc(sizeof(raydium_ode_ElementInternalSave));
	strcpy(raydium_ode_element[elemN].fixed_elements[j]->name,raydium_ode_element[elem[i]].name);
	raydium_ode_element[elemN].fixed_elements[j]->type=dGeomGetClass(raydium_ode_element[elem[i]].geom);
	dGeomBoxGetLengths(raydium_ode_element[elem[i]].geom,raydium_ode_element[elemN].fixed_elements[j]->box_sizes);
	raydium_ode_element[elemN].fixed_elements[j]->sphere_radius=dGeomSphereGetRadius(raydium_ode_element[elem[i]].geom);
	dBodyGetMass(raydium_ode_element[elem[i]].body,&m);
	raydium_ode_element[elemN].fixed_elements[j]->mass=m.mass;
	raydium_ode_element[elemN].fixed_elements[j]->object=raydium_ode_element[elem[i]].object;
	raydium_ode_element[elemN].fixed_elements[j]->mesh=raydium_ode_element[elem[i]].mesh;
	raydium_ode_element[elemN].fixed_elements[j]->slip=raydium_ode_element[elem[i]].slip;
	raydium_ode_element[elemN].fixed_elements[j]->cfm=raydium_ode_element[elem[i]].cfm;
	raydium_ode_element[elemN].fixed_elements[j]->erp=raydium_ode_element[elem[i]].erp;

	data=(dReal *)dGeomGetPosition(raydium_ode_element[elem[i]].geom);
	memcpy(raydium_ode_element[elemN].fixed_elements[j]->rel_pos,data,sizeof(dReal)*3);
	raydium_ode_element[elemN].fixed_elements[j]->rel_pos[0]-=position[0];
	raydium_ode_element[elemN].fixed_elements[j]->rel_pos[1]-=position[1];
	raydium_ode_element[elemN].fixed_elements[j]->rel_pos[2]-=position[2];
	dGeomGetQuaternion(raydium_ode_element[elem[i]].geom,raydium_ode_element[elemN].fixed_elements[j]->rel_rot);
	raydium_ode_element[elemN].fixed_elements[j]->user_data=raydium_ode_element[elem[i]].user_data;
	raydium_ode_element[elemN].fixed_elements[j]->user_tag=raydium_ode_element[elem[i]].user_tag;
	dBodyGetMass(raydium_ode_element[elem[i]].body,&m);
	raydium_ode_element[elemN].fixed_elements[j]->OnBlow=raydium_ode_element[elem[i]].OnBlow;
	raydium_ode_element[elemN].fixed_elements[j]->OnDelete=raydium_ode_element[elem[i]].OnDelete;
	mass+=m.mass;
	done++;
	j=RAYDIUM_ODE_ELEMENT_MAX_FIXING; // jump to next element to save
	}

if(done!=nelems)
    raydium_log("ODE: Error: only %i/%i elems were fixed to %s ! Continue anyway...",done,nelems,name);

// 4 - delete elements
for(i=0;i<nelems;i++)
    raydium_ode_element_delete(elem[i],1); // delete joints, too /!\ (must fix this)

// 5 - compute new mass
dMassSetBox(&m,1,bounding[0],bounding[1],bounding[2]);
dMassAdjust(&m,mass);
dBodySetMass(raydium_ode_element[elemN].body,&m);

return elemN;
}

void raydium_ode_element_unfix(int e)
{
int i;

raydium_log("ODE: unfix unimplemented !");
return;

if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot unfix element: element is not valid");
    return;
    }

if(raydium_ode_element[e].state!=RAYDIUM_ODE_FIXING)
    {
    raydium_log("ODE: Error: Cannot unfix a non-fixing element");
    return;
    }


for(i=0;i<RAYDIUM_ODE_ELEMENT_MAX_FIXING;i++)
    if(raydium_ode_element[e].fixed_elements[i])
	{
	// TODO:
	// test type
	// create element (using name)
	// move/rotate
	// restor params (erp, cfm, ...)
	}

raydium_ode_element_delete(e,1);
}


void raydium_ode_element_mass(int elem, dReal mass)
{
dMass m;

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot change mass of element: invalid index or name");
    return;
    }

if(raydium_ode_element[elem].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: Cannot change mass of a static element");
    return;
    }

if(dGeomGetClass(raydium_ode_element[elem].geom)==dBoxClass)
    {    
    dMassSetSphere(&m,1,dGeomSphereGetRadius(raydium_ode_element[elem].geom));
    }
else
    {
    dVector3 size;
    dGeomBoxGetLengths(raydium_ode_element[elem].geom,size);
    dMassSetBox(&m,1,size[0],size[1],size[2]);
    }

dMassAdjust(&m,mass);
dBodySetMass(raydium_ode_element[elem].body,&m);
}


void raydium_ode_element_mass_name(char *elem, dReal mass)
{
raydium_ode_element_mass(raydium_ode_element_find(elem),mass);
}


void raydium_ode_element_move(int elem, dReal *pos)
{
if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot move element: invalid index or name");
    return;
    }
dGeomSetPosition(raydium_ode_element[elem].geom,pos[0],pos[1],pos[2]);
}

void raydium_ode_element_move_3f(int elem, dReal x,dReal y, dReal z)
{
dReal pos[3];
pos[0]=x;
pos[1]=y;
pos[2]=z;
raydium_ode_element_move(elem,pos);
}

void raydium_ode_element_move_name(char *name, dReal *pos)
{
raydium_ode_element_move(raydium_ode_element_find(name),pos);
}

void raydium_ode_element_move_name_3f(char *name, dReal x,dReal y, dReal z)
{
raydium_ode_element_move_3f(raydium_ode_element_find(name),x,y,z);
}

void raydium_ode_element_rotate(int elem, dReal *rot)
{
dMatrix3 R;

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot rotate element: invalid index or name");
    return;
    }
dRFromEulerAngles(R,rot[0],rot[1],rot[2]);
dGeomSetRotation(raydium_ode_element[elem].geom,R);
}

void raydium_ode_element_rotate_3f(int elem, dReal rx, dReal ry, dReal rz)
{
dReal rot[3];
rot[0]=rx;
rot[1]=ry;
rot[2]=rz;
raydium_ode_element_rotate(elem,rot);
}

void raydium_ode_element_rotate_name(char *name, dReal *rot)
{
raydium_ode_element_rotate(raydium_ode_element_find(name),rot);
}


void raydium_ode_element_rotateq(int elem, dReal *rot)
{

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot rotate element: invalid index or name");
    return;
    }
dGeomSetQuaternion(raydium_ode_element[elem].geom,rot);
}

void raydium_ode_element_rotateq_name(char *name, dReal * rot)
{
raydium_ode_element_rotateq(raydium_ode_element_find(name),rot);
}

void raydium_ode_element_rotate_name_3f(char *name, dReal rx, dReal ry, dReal rz)
{
raydium_ode_element_rotate_3f(raydium_ode_element_find(name),rx,ry,rz);
}

void raydium_ode_element_rotate_direction(int elem, signed char Force0OrVel1)
{
dReal *vel;
dMatrix3 R;

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot align element: invalid index or name");
    return;
    }

if(raydium_ode_element[elem].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: Cannot align a static element");
    return;
    }


if(!Force0OrVel1)
 vel=(dReal *)dBodyGetForce(raydium_ode_element[elem].body);
else
 vel=(dReal *)dBodyGetLinearVel(raydium_ode_element[elem].body);

dRFrom2Axes(R,vel[0],vel[1],vel[2], vel[0],vel[1],vel[2]*0); // 1 0 0
dBodySetRotation(raydium_ode_element[elem].body,R);
}

void raydium_ode_element_rotate_direction_name(char *e, signed char Force0OrVel1)
{
raydium_ode_element_rotate_direction(raydium_ode_element_find(e),Force0OrVel1);
}

void raydium_ode_element_data_set(int e, void *data)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot set element data: invalid index or name");
    return;
    }
raydium_ode_element[e].user_data=data;
}

void raydium_ode_element_data_set_name(char *e, void *data)
{
raydium_ode_element_data_set(raydium_ode_element_find(e),data);
}

void *raydium_ode_element_data_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot get element data: invalid index or name");
    return NULL;
    }
return raydium_ode_element[e].user_data;
}

void *raydium_ode_element_data_get_name(char *e)
{
return raydium_ode_element_data_get(raydium_ode_element_find(e));
}

int raydium_ode_element_tag_get(int e)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot get element tag: invalid index or name");
    return 0;
    }
return raydium_ode_element[e].user_tag;
}

int raydium_ode_element_tag_get_name(char *e)
{
return raydium_ode_element_tag_get(raydium_ode_element_find(e));
}

// move object elements to pos (pos is used for the LAST element of your object)
void raydium_ode_object_move(int obj, dReal *pos)
{

int i,n;
raydium_ode_Element *e;
dReal *refp;
dReal *act;
dReal diff[3];
dReal ref[3];

if(!raydium_ode_object_isvalid(obj))
    {
    raydium_log("ODE: Error: Cannot move object: invalid index or name");
    return;
    }

//n=dGeomGroupGetNumGeoms(raydium_ode_object[obj].group);
n=dSpaceGetNumGeoms(raydium_ode_object[obj].group);
if(!n) return;

//refp=(dReal *)dGeomGetPosition(dGeomGroupGetGeom(raydium_ode_object[obj].group,0));
refp=(dReal *)dGeomGetPosition(dSpaceGetGeom(raydium_ode_object[obj].group,0));
ref[0]=refp[0];
ref[1]=refp[1];
ref[2]=refp[2];
e=dGeomGetData(dSpaceGetGeom(raydium_ode_object[obj].group,0));
raydium_ode_element_move(e->id,pos);

for(i=1;i<n;i++)
    {
    act=(dReal *)dGeomGetPosition(dSpaceGetGeom(raydium_ode_object[obj].group,i));
    diff[0]=pos[0]+(act[0]-ref[0]);
    diff[1]=pos[1]+(act[1]-ref[1]);
    diff[2]=pos[2]+(act[2]-ref[2]);
    //raydium_log("%f %f %f",act[0],act[1],act[2]);
    e=dGeomGetData(dSpaceGetGeom(raydium_ode_object[obj].group,i));
    raydium_ode_element_move(e->id,diff);
    }


}

void raydium_ode_object_move_name(char *name, dReal *pos)
{
raydium_ode_object_move(raydium_ode_object_find(name),pos);
}

void raydium_ode_object_move_name_3f(char *name, dReal x,dReal y, dReal z)
{
dReal pos[3];
pos[0]=x;
pos[1]=y;
pos[2]=z;
raydium_ode_object_move_name(name,pos);
}

// rotate object (rotation is done around LAST element of obj)
void raydium_ode_object_rotateq(int obj, dReal *rot)
{
int i,n;
raydium_ode_Element *e;
raydium_ode_Element *ref;
dBodyID body;
dReal *pos;
dReal *epos; // allocated by raydium_ode_element_pos_get(), see below
//dReal vect[3];
dVector3 res;

//raydium_log("obj rotate unimplemented"); return;

if(!raydium_ode_object_isvalid(obj))
    {
    raydium_log("ODE: Error: Cannot rotateq object: invalid index or name");
    return;
    }

n=dSpaceGetNumGeoms(raydium_ode_object[obj].group);
if(!n) return;

// 1 - rotate elements (THIS IS NOT THE CORRECT WAY TO DO IT !)
for(i=1;i<n;i++)
    {
    e=dGeomGetData(dSpaceGetGeom(raydium_ode_object[obj].group,i));
    raydium_ode_element_rotateq(e->id,rot);
    }

// 2 - create a temporary body (based on reference element)
body=dBodyCreate(raydium_ode_world);
ref=dGeomGetData(dSpaceGetGeom(raydium_ode_object[obj].group,0));
pos=raydium_ode_element_pos_get(ref->id);
dBodySetPosition(body,pos[0],pos[1],pos[2]);
dBodySetQuaternion(body,rot);

// 3 - replace rotated elements
for(i=1;i<n;i++)
    {
    // 3 - 1 - Get relative point from real body
    e=dGeomGetData(dSpaceGetGeom(raydium_ode_object[obj].group,i));
    epos=raydium_ode_element_pos_get(e->id);
    dBodyGetPosRelPoint(ref->body,epos[0],epos[1],epos[2],res);
    //printf("---> %s %f %f %f\n",e->name,res[0],res[1],res[2]);
    // 3 - 2 - Apply relative point to world with "fake" body
    dBodyGetRelPointPos(body,res[0],res[1],res[2],epos);
    // 3 - 3 - Change position
    raydium_ode_element_move(e->id,epos);
    }

dBodyDestroy(body);
raydium_ode_element_rotateq(ref->id,rot);
}

void raydium_ode_object_rotateq_name(char *obj, dReal *rot)
{
raydium_ode_object_rotateq(raydium_ode_object_find(obj),rot);
}

void raydium_ode_object_rotate(int obj, dReal *rot)
{
dMatrix3 R;
dQuaternion Q;
dRFromEulerAngles(R,rot[0],rot[1],rot[2]);
dRtoQ(R,Q);
raydium_ode_object_rotateq(obj,Q);
}

void raydium_ode_object_rotate_name(char *obj, dReal *rot)
{
raydium_ode_object_rotate(raydium_ode_object_find(obj),rot);
}

void raydium_ode_object_rotate_name_3f(char *obj, dReal rx, dReal ry, dReal rz)
{
dReal r[3];
r[0]=rx;
r[1]=ry;
r[2]=rz;
raydium_ode_object_rotate_name(obj,r);
}


void raydium_ode_joint_suspension(int j, dReal erp, dReal cfm)
{
void (*f)(dJointID,int,dReal);
int type;

if(!raydium_ode_joint_isvalid(j))
    {
    raydium_log("ODE: Cannot set ERP and CFM for this joint: invalid name or index");
    return;
    }

type=dJointGetType(raydium_ode_joint[j].joint);
switch(type)
    {
    case dJointTypeHinge2:
	f=dJointSetHinge2Param;
	break;
    case dJointTypeHinge:
	f=dJointSetHingeParam;
	break;
    default: raydium_log("ODE: ERROR: suspension: joint type not supported!");
    }
f(raydium_ode_joint[j].joint,dParamSuspensionERP,erp);
f(raydium_ode_joint[j].joint,dParamSuspensionCFM,cfm);
}

void raydium_ode_joint_suspension_name(char *j, dReal erp, dReal cfm)
{
raydium_ode_joint_suspension(raydium_ode_joint_find(j),erp,cfm);
}

int raydium_ode_joint_attach_hinge2(char *name, int elem1, int elem2, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z)
{
int i;
const dReal *a;

if(raydium_ode_joint_find(name)>=0)
    {
    raydium_log("ODE: Cannot add (hinge2) joint \"%s\": name already exists",name);
    return -1;
    }

if(elem1==RAYDIUM_ODE_JOINT_FIXED)
    elem1=RAYDIUM_ODE_MAX_ELEMENTS;
if(elem2==RAYDIUM_ODE_JOINT_FIXED)
    elem2=RAYDIUM_ODE_MAX_ELEMENTS;

if(!raydium_ode_element_isvalid(elem1) || !raydium_ode_element_isvalid(elem2))
    {
    raydium_log("ODE: Error: Cannot attach hinge2: one element is not valid");
    return -1;
    }

if( raydium_ode_element[elem1].state==RAYDIUM_ODE_STATIC ||
    raydium_ode_element[elem2].state==RAYDIUM_ODE_STATIC )
    {
    raydium_log("ODE: Error: Cannot attach a static element");
    return -1;    
    }

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(!raydium_ode_joint[i].state)
     {
     dJointFeedback *jf;
     strcpy(raydium_ode_joint[i].name,name);
     raydium_ode_joint[i].joint=dJointCreateHinge2(raydium_ode_world,0);
     dJointAttach(raydium_ode_joint[i].joint,raydium_ode_element[elem1].body,raydium_ode_element[elem2].body);
     a=dBodyGetPosition(raydium_ode_element[elem2].body);
     dJointSetHinge2Anchor(raydium_ode_joint[i].joint,a[0],a[1],a[2]);
     dJointSetHinge2Axis1(raydium_ode_joint[i].joint,axe1x,axe1y,axe1z);
     dJointSetHinge2Axis2(raydium_ode_joint[i].joint,axe2x,axe2y,axe2z);
     dJointSetData(raydium_ode_joint[i].joint,&raydium_ode_joint[i]);
     jf=(dJointFeedback *)malloc(sizeof(dJointFeedback));
     dJointSetFeedback(raydium_ode_joint[i].joint,jf);
     raydium_ode_joint[i].state=1;
     dJointSetHinge2Param (raydium_ode_joint[i].joint,dParamSuspensionERP,0.1);
     dJointSetHinge2Param (raydium_ode_joint[i].joint,dParamSuspensionCFM,0.9);
     return i;
     }
raydium_log("ODE: No more joint slots ! aborting \"%s\" (hinge2) creation",name);
return -1;
}

int raydium_ode_joint_attach_hinge2_name(char *name, char *elem1, char *elem2, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z)
{
int e1,e2;
e1=raydium_ode_element_find(elem1);
e2=raydium_ode_element_find(elem2);
return raydium_ode_joint_attach_hinge2(name,e1,e2,axe1x,axe1y,axe1z,axe2x,axe2y,axe2z);
}


int raydium_ode_joint_attach_universal(char *name, int elem1, int elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z)
{
int i;

if(raydium_ode_joint_find(name)>=0)
    {
    raydium_log("ODE: Cannot add (universal) joint \"%s\": name already exists",name);
    return -1;
    }

if(elem1==RAYDIUM_ODE_JOINT_FIXED)
    elem1=RAYDIUM_ODE_MAX_ELEMENTS;
if(elem2==RAYDIUM_ODE_JOINT_FIXED)
    elem2=RAYDIUM_ODE_MAX_ELEMENTS;

if(!raydium_ode_element_isvalid(elem1) || !raydium_ode_element_isvalid(elem2))
    {
    raydium_log("ODE: Error: Cannot attach universal: one element is not valid");
    return -1;
    }

if( raydium_ode_element[elem1].state==RAYDIUM_ODE_STATIC ||
    raydium_ode_element[elem2].state==RAYDIUM_ODE_STATIC )
    {
    raydium_log("ODE: Error: Cannot attach a static element");
    return -1;    
    }

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(!raydium_ode_joint[i].state)
     {
     dJointFeedback *jf;
     strcpy(raydium_ode_joint[i].name,name);
     raydium_ode_joint[i].joint=dJointCreateUniversal(raydium_ode_world,0);
     dJointAttach(raydium_ode_joint[i].joint,raydium_ode_element[elem1].body,raydium_ode_element[elem2].body);
     dJointSetUniversalAnchor(raydium_ode_joint[i].joint,posx,posy,posz);
     dJointSetUniversalAxis1(raydium_ode_joint[i].joint,axe1x,axe1y,axe1z);
     dJointSetUniversalAxis2(raydium_ode_joint[i].joint,axe2x,axe2y,axe2z);
     dJointSetData(raydium_ode_joint[i].joint,&raydium_ode_joint[i]);
     jf=(dJointFeedback *)malloc(sizeof(dJointFeedback));
     dJointSetFeedback(raydium_ode_joint[i].joint,jf);
     raydium_ode_joint[i].state=1;
     return i;
     }
raydium_log("ODE: No more joint slots ! aborting \"%s\" (universal) creation",name);
return -1;
}

int raydium_ode_joint_attach_universal_name(char *name, char *elem1, char *elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z)
{
int e1,e2;
e1=raydium_ode_element_find(elem1);
e2=raydium_ode_element_find(elem2);
return raydium_ode_joint_attach_universal(name,e1,e2,posx,posy,posz,axe1x,axe1y,axe1z,axe2x,axe2y,axe2z);
}


int raydium_ode_joint_attach_hinge(char *name, int elem1, int elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z)
{
int i;

if(raydium_ode_joint_find(name)>=0)
    {
    raydium_log("ODE: Cannot add (hinge) joint \"%s\": name already exists",name);
    return -1;
    }

if(elem1==RAYDIUM_ODE_JOINT_FIXED)
    elem1=RAYDIUM_ODE_MAX_ELEMENTS;
if(elem2==RAYDIUM_ODE_JOINT_FIXED)
    elem2=RAYDIUM_ODE_MAX_ELEMENTS;

if(!raydium_ode_element_isvalid(elem1) || !raydium_ode_element_isvalid(elem2))
    {
    raydium_log("ODE: Error: Cannot attach hinge: one element is not valid");
    return -1;
    }

if( raydium_ode_element[elem1].state==RAYDIUM_ODE_STATIC ||
    raydium_ode_element[elem2].state==RAYDIUM_ODE_STATIC )
    {
    raydium_log("ODE: Error: Cannot attach a static element");
    return -1;    
    }

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(!raydium_ode_joint[i].state)
     {
     dJointFeedback *jf;
     strcpy(raydium_ode_joint[i].name,name);
     raydium_ode_joint[i].joint=dJointCreateHinge(raydium_ode_world,0);
     dJointAttach(raydium_ode_joint[i].joint,raydium_ode_element[elem1].body,raydium_ode_element[elem2].body);
     dJointSetHingeAnchor(raydium_ode_joint[i].joint,posx,posy,posz);
     dJointSetHingeAxis(raydium_ode_joint[i].joint,axe1x,axe1y,axe1z);
     dJointSetData(raydium_ode_joint[i].joint,&raydium_ode_joint[i]);
     jf=(dJointFeedback *)malloc(sizeof(dJointFeedback));
     dJointSetFeedback(raydium_ode_joint[i].joint,jf);
     raydium_ode_joint[i].state=1;
     return i;
     }
raydium_log("ODE: No more joint slots ! aborting \"%s\" (hinge) creation",name);
return -1;
}

int raydium_ode_joint_attach_hinge_name(char *name, char *elem1, char *elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z)
{
int e1,e2;
e1=raydium_ode_element_find(elem1);
e2=raydium_ode_element_find(elem2);
return raydium_ode_joint_attach_hinge(name,e1,e2,posx,posy,posz,axe1x,axe1y,axe1z);
}

int raydium_ode_joint_attach_fixed(char *name, int elem1, int elem2)
{
int i;

if(raydium_ode_joint_find(name)>=0)
    {
    raydium_log("ODE: Cannot add (fixed) joint \"%s\": name already exists",name);
    return -1;
    }

if(elem1==RAYDIUM_ODE_JOINT_FIXED)
    elem1=RAYDIUM_ODE_MAX_ELEMENTS;
if(elem2==RAYDIUM_ODE_JOINT_FIXED)
    elem2=RAYDIUM_ODE_MAX_ELEMENTS;

if(!raydium_ode_element_isvalid(elem1) || !raydium_ode_element_isvalid(elem2))
    {
    raydium_log("ODE: Error: Cannot attach fixed: one element is not valid");
    return -1;
    }

if( raydium_ode_element[elem1].state==RAYDIUM_ODE_STATIC ||
    raydium_ode_element[elem2].state==RAYDIUM_ODE_STATIC )
    {
    raydium_log("ODE: Error: Cannot attach a static element");
    return -1;    
    }

//raydium_log("ODE: Warning: fixed joint slows down physics !");

for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(!raydium_ode_joint[i].state)
     {
     dJointFeedback *jf;
     strcpy(raydium_ode_joint[i].name,name);
     raydium_ode_joint[i].joint=dJointCreateFixed(raydium_ode_world,0);
     dJointAttach(raydium_ode_joint[i].joint,raydium_ode_element[elem1].body,raydium_ode_element[elem2].body);
     dJointSetFixed(raydium_ode_joint[i].joint);
     dJointSetData(raydium_ode_joint[i].joint,&raydium_ode_joint[i]);
     jf=(dJointFeedback *)malloc(sizeof(dJointFeedback));
     dJointSetFeedback(raydium_ode_joint[i].joint,jf);
     raydium_ode_joint[i].state=1;
     return i;
     }
raydium_log("ODE: No more joint slots ! aborting \"%s\" (fixed) creation",name);
return -1;
}

int raydium_ode_joint_attach_fixed_name(char *name, char *elem1, char *elem2)
{
return raydium_ode_joint_attach_fixed(name,raydium_ode_element_find(elem1),raydium_ode_element_find(elem2));
}

void raydium_ode_joint_hinge_limits(int j, dReal lo, dReal hi)
{
if(raydium_ode_joint_isvalid(j))
    {
    dJointSetHingeParam(raydium_ode_joint[j].joint,dParamLoStop,lo);
    dJointSetHingeParam(raydium_ode_joint[j].joint,dParamHiStop,hi);
    dJointSetHingeParam(raydium_ode_joint[j].joint,dParamLoStop,lo); // ODE "feature" :)
    return;
    }
raydium_log("ODE: Error: cannot set joint limits: invalid index or name");
}

void raydium_ode_joint_hinge_limits_name(char *j, dReal lo, dReal hi)
{
return raydium_ode_joint_hinge_limits(raydium_ode_joint_find(j),lo,hi);
}

void raydium_ode_joint_universal_limits(int j, dReal lo1, dReal hi1, dReal lo2, dReal hi2)
{
if(raydium_ode_joint_isvalid(j))
    {
    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamLoStop ,lo1);
    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamHiStop ,hi1);
    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamLoStop ,lo1);

    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamLoStop2,lo2);
    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamHiStop2,hi2);
    dJointSetUniversalParam(raydium_ode_joint[j].joint,dParamLoStop2,lo2);
    return;
    }
raydium_log("ODE: Error: cannot set joint limits: invalid index or name");
}

void raydium_ode_joint_universal_limits_name(char *j, dReal lo1, dReal hi1, dReal lo2, dReal hi2)
{
return raydium_ode_joint_universal_limits(raydium_ode_joint_find(j),lo1,hi1,lo2,hi2);
}


void raydium_ode_joint_hinge2_block(int j, signed char block)
{
// TODO: test if joint is hinge2 type
if(raydium_ode_joint_isvalid(j))
    {
//    raydium_ode_joint[j].hinge2correct=block;
    if(!block)
	{
        dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamLoStop,-dInfinity);
	dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamHiStop,dInfinity);
        dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamLoStop,-dInfinity);
	}
    else
	{
        dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamLoStop,0);
	dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamHiStop,0);
        dJointSetHinge2Param(raydium_ode_joint[j].joint,dParamLoStop,0);
	}
    return;
    }
raydium_log("ODE: Error: cannot block: invalid index or name");
}



void raydium_ode_joint_hinge2_block_name(char *name, signed char block)
{
raydium_ode_joint_hinge2_block(raydium_ode_joint_find(name),block);
}

void raydium_ode_joint_delete_callback(int j, void (*f)(int))
{
if(!raydium_ode_joint_isvalid(j))
    {
    raydium_log("ODE: Error: Cannot set joint \"OnDelete\" callback: invalid index/name");
    return;
    }
raydium_ode_joint[j].OnDelete=f;
}

void raydium_ode_joint_delete_callback_name(char *name, void (*f)(int))
{
raydium_ode_joint_delete_callback(raydium_ode_joint_find(name),f);
}

void raydium_ode_joint_break_force(int j, dReal maxforce)
{
if(!raydium_ode_joint_isvalid(j))
    {
    raydium_log("ODE: Error: Cannot set break limit force: invalid index/name");
    return;
    }
raydium_ode_joint[j].breakableforce=maxforce;
}

void raydium_ode_joint_break_force_name(char *name, dReal maxforce)
{
raydium_ode_joint_break_force(raydium_ode_joint_find(name),maxforce);
}


void raydium_ode_joint_elements_get(int j, int *e1, int *e2)
{
raydium_ode_Element *elem1;
raydium_ode_Element *elem2;

if(!raydium_ode_joint_isvalid(j))
    {
    raydium_log("ODE: Error: Cannot get joint's attached elements: invalid index/name");
    return;
    }

// may crash if joint is disconnected at one of his ends !
elem1=dBodyGetData(dJointGetBody(raydium_ode_joint[j].joint,0));
elem2=dBodyGetData(dJointGetBody(raydium_ode_joint[j].joint,1));
    
*e1=elem1->id;
*e2=elem2->id;
}

void raydium_ode_joint_elements_get_name(char *j, int *e1, int *e2)
{
raydium_ode_joint_elements_get(raydium_ode_joint_find(j),e1,e2);
}

void raydium_ode_motor_update_joints_data_internal(int j)
{
int i;

if(raydium_ode_motor_isvalid(j))
    {
    if(raydium_ode_motor[j].state==RAYDIUM_ODE_MOTOR_ROCKET)
    {
    dReal speed;
    // test if element is attached:
    if(raydium_ode_motor[j].rocket_element<0)
	return;
	
    speed=raydium_ode_motor[j].speed;
    if(raydium_ode_motor[j].rocket_playermovement &&
       !raydium_ode_element[raydium_ode_motor[j].rocket_element]._touched )
       speed=0;

    if(speed!=0.f)
    dBodyAddRelForceAtRelPos(raydium_ode_element[raydium_ode_motor[j].rocket_element].body,
			     raydium_ode_motor[j].rocket_direction[0],
			     raydium_ode_motor[j].rocket_direction[1],
			     raydium_ode_motor[j].rocket_direction[2],
			     raydium_ode_motor[j].rocket_position[0],
			     raydium_ode_motor[j].rocket_position[1],
			     raydium_ode_motor[j].rocket_position[2]);
    return;
    }        
    // if motor is anything else than a rocket:
    for(i=0;i<RAYDIUM_ODE_MOTOR_MAX_JOINTS;i++)
	if(raydium_ode_motor[j].joints[i]>=0)
	{
	void (*SetParam)(dJointID,int,dReal);
	dReal (*GetAngle)(dJointID);
	int vel;
	int fmax;
	int type;
	char cancel=0;
	
	switch(raydium_ode_motor[j].joints_axe[i])
	    {
	    case 0:
		vel=dParamVel;
		fmax=dParamFMax;
		break;
	    case 1:
		vel=dParamVel2;
		fmax=dParamFMax2;
		break;
	    case 2:
		vel=dParamVel3;
		fmax=dParamFMax3;
		break;
	    default:
		cancel=1;
		raydium_log("ODE: Motor: Invalid axe for this joint (%s, motor is %s)",raydium_ode_joint[raydium_ode_motor[j].joints[i]].name,raydium_ode_motor[j].name);
	    }
	
	type=dJointGetType(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint);
    	    
	switch(type)
	    {
	    case dJointTypeHinge2:
		SetParam=dJointSetHinge2Param;
		GetAngle=dJointGetHinge2Angle1;
		if(raydium_ode_motor[j].joints_axe[i]!=0 && raydium_ode_motor[j].state==RAYDIUM_ODE_MOTOR_ANGULAR)
		    {
		    cancel=1;
		    raydium_log("ODE: Only axe Hinge2 axe 0 is supported with angular motors (%s, motor is %s)",raydium_ode_joint[raydium_ode_motor[j].joints[i]].name,raydium_ode_motor[j].name);
		    }
		break;
	    case dJointTypeHinge:
		SetParam=dJointSetHingeParam;
		GetAngle=dJointGetHingeAngle;
		break;
	    default:
		cancel=1;
		raydium_log("ODE: Motor: Invalid joint type (%s, motor is %s)",raydium_ode_joint[raydium_ode_motor[j].joints[i]].name,raydium_ode_motor[j].name);
	    }
	    
	
	if(cancel) continue; // previous tests failed
	
	if(raydium_ode_motor[j].state==RAYDIUM_ODE_MOTOR_ENGINE)
		{
		dReal speed;
		dReal force;
		speed=raydium_ode_motor[j].speed*raydium_ode_motor[j].gears[raydium_ode_motor[j].gear];
		
		if(raydium_ode_motor[j].gears[raydium_ode_motor[j].gear]==0.0)
		    force=0;
		else
		    force=raydium_ode_motor[j].force*(1/raydium_ode_motor[j].gears[raydium_ode_motor[j].gear]);
		
//		force=raydium_ode_motor[j].force;
//		raydium_log("speed=%f force=%f",speed,force);
		SetParam(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint,vel,speed);
		SetParam(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint,fmax,force);
		}
		
	if(raydium_ode_motor[j].state==RAYDIUM_ODE_MOTOR_ANGULAR)
	    {
		dReal v;
		SetParam(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint,fmax,raydium_ode_motor[j].force);
		v = raydium_ode_motor[j].angle - 
		    GetAngle(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint);
		SetParam(raydium_ode_joint[raydium_ode_motor[j].joints[i]].joint,vel,v*10);
	    }	    	
	}
    return;
    }
raydium_log("ODE: Error (internal): motor update failed: not found");
}

void raydium_ode_motor_speed(int j, dReal force)
{
if(raydium_ode_motor_isvalid(j))
    {
    raydium_ode_motor[j].speed=force;
    // refresh direction using direction and speed:
    if(raydium_ode_motor[j].state==RAYDIUM_ODE_MOTOR_ROCKET)
	raydium_ode_motor_rocket_orientation(j,raydium_ode_motor[j].rocket_orientation[0],raydium_ode_motor[j].rocket_orientation[1],raydium_ode_motor[j].rocket_orientation[2]);
    return;
    }
raydium_log("ODE: Error: cannot set motor's speed: invalid index or name");
}

void raydium_ode_motor_speed_name(char *name, dReal force)
{
raydium_ode_motor_speed(raydium_ode_motor_find(name),force);
}

// power = 0 means motor has no effect on joints (free moves)
void raydium_ode_motor_power_max(int j, dReal power)
{
if(raydium_ode_motor_isvalid(j))
    {
    raydium_ode_motor[j].force=power;
    return;
    }
raydium_log("ODE: Error: cannot set max motor's power: invalid index or name");
}

void raydium_ode_motor_power_max_name(char *name, dReal power)
{
raydium_ode_motor_power_max(raydium_ode_motor_find(name),power);
}


void raydium_ode_motor_angle(int j, dReal angle)
{
if(raydium_ode_motor_isvalid(j))
    {
    raydium_ode_motor[j].angle=angle;
    return;
    }
raydium_log("ODE: Error: cannot set motor's angle: invalid index or name");
}

void raydium_ode_motor_angle_name(char *motor, dReal angle)
{
raydium_ode_motor_angle(raydium_ode_motor_find(motor),angle);
}


void raydium_ode_motor_gears_set(int m,dReal *gears, int n_gears)
{
if(raydium_ode_motor_isvalid(m))
    {
    if(raydium_ode_motor[m].state!=RAYDIUM_ODE_MOTOR_ENGINE)
	{
	raydium_log("ODE: Error: cannot use a gearbox with a non-engine motor");
	return;
	}
    memcpy(raydium_ode_motor[m].gears,gears,sizeof(dReal)*n_gears);
    raydium_ode_motor[m].gear_max=n_gears-1;
    return;
    }
raydium_log("ODE: Error: cannot configure motor's gears: invalid index or name");
}

void raydium_ode_motor_gears_set_name(char *m,dReal *gears,int n_gears)
{
raydium_ode_motor_gears_set(raydium_ode_motor_find(m),gears,n_gears);
}

void raydium_ode_motor_gear_change(int m, int gear)
{
if(raydium_ode_motor_isvalid(m))
    {
    if(gear<0 || gear>raydium_ode_motor[m].gear_max)
	{
	raydium_log("ODE: Error: invalid gear (%i)",gear);
	return;
	}
    raydium_ode_motor[m].gear=gear;
    return;
    }
raydium_log("ODE: Error: cannot set motor's gear: invalid index or name");
}

void raydium_ode_motor_gear_change_name(char *m, int gear)
{
raydium_ode_motor_gear_change(raydium_ode_motor_find(m),gear);
}

dReal *raydium_ode_element_pos_get(int j)
{
if(raydium_ode_element_isvalid(j))
    {
    return (dReal *)dGeomGetPosition(raydium_ode_element[j].geom);
    }
raydium_log("ODE: Error: cannot get element position: invalid index or name");
return NULL;
}

dReal *raydium_ode_element_pos_get_name(char *name)
{
return raydium_ode_element_pos_get(raydium_ode_element_find(name));
}


signed char raydium_ode_element_rotq_get(int j, dReal * res)
{
if(raydium_ode_element_isvalid(j))
    {
    dGeomGetQuaternion(raydium_ode_element[j].geom,res);
    return 1;
    }
raydium_log("ODE: Error: cannot get element rotation (quaternion): invalid index or name");
return 0;
}

signed char raydium_ode_element_rotq_get_name(char *name, dReal * res)
{
return raydium_ode_element_rotq_get(raydium_ode_element_find(name),res);
}

/*
signed char raydium_ode_element_rot_get(int e, dReal *rx, dReal *ry, dReal *rz)
{
// From Andrzej Szombierski <qq@kuku.eu.org> (ODE ML)
// patched by Daniel Monteiro Basso <dmbasso@inf.ufrgs.br> (ODE ML)
// but it's not what i want, again.. arrrgh ;)
const dReal epsilon=0.0000001;
dReal *matrix;
dReal c;

if(raydium_ode_element_isvalid(e))
    {
    matrix=(dReal *)dGeomGetRotation(raydium_ode_element[e].geom);
    
    if(matrix[2] < 1-epsilon && matrix[2] > -1+epsilon)
    {
	*ry=-asin(matrix[2]);
	c=cos(*ry);
	*rx= atan2(matrix[6]/c,matrix[10]/c);
	*rz= atan2(matrix[1]/c,matrix[0]/c);
    } 
    else 
    {       
	*rz=0;   
	*ry=-atan2(matrix[2],0);
	*rx= atan2(-matrix[9],matrix[5]);
    }
    // rad to deg
    //(*rx)*=180/PI;
    //(*ry)*=180/PI;
    //(*rz)*=180/PI;
    return 1;
    }
raydium_log("ODE: Error: cannot get element rotation (3f): invalid index or name");
return 0;
}
*/

signed char raydium_ode_element_rot_get(int e, dReal *rx, dReal *ry, dReal *rz)
{
// From Andrzej Szombierski <qq@kuku.eu.org> (ODE ML)
// Original code version: "absolute" angles
// Do not apply back returned angles ! (not ODE "formated")
const dReal epsilon=0.0000001;
dReal *matrix;
dReal c;

if(raydium_ode_element_isvalid(e))
    {
    matrix=(dReal *)dGeomGetRotation(raydium_ode_element[e].geom);
    
    if(matrix[8] < 1-epsilon && matrix[8] > -1+epsilon)
    {
	*ry=-asin(matrix[8]);
	c=cos(*ry);
	*rx= atan2(matrix[9]/c,matrix[10]/c);
	*rz= atan2(matrix[4]/c,matrix[0]/c);
    }       
    else    
    {       
	*rz=0;   
	*ry=-atan2(matrix[8],0);
	*rx= atan2(-matrix[6],matrix[5]);
    }       

    // rad to deg
    //(*rx)*=180/PI;
    //(*ry)*=180/PI;
    //(*rz)*=180/PI;
    return 1;
    }
raydium_log("ODE: Error: cannot get element rotation (3f): invalid index or name");
return 0;
}


signed char raydium_ode_element_rot_get_name(char *e, dReal *rx, dReal *ry, dReal *rz)
{
return raydium_ode_element_rot_get(raydium_ode_element_find(e),rx,ry,rz);
}

void raydium_ode_element_sound_update(int e, int source)
{
if(raydium_ode_element_isvalid(e))
    {
    dReal *pos;
    pos=raydium_ode_element_pos_get(e);
    raydium_sound_SetSourcePos(source,pos);
    return;
    }
raydium_log("ODE: Error: cannot update source: invalid index or name");
return;
}

void raydium_ode_element_sound_update_name(char *e, int source)
{
raydium_ode_element_sound_update(raydium_ode_element_find(e),source);
}

void raydium_ode_element_RelPointPos(int e, dReal px, dReal py, dReal pz, dReal *res)
{
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot get RelPointPos: invalid index or name");
    return;    
    }

if(raydium_ode_element[e].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: Cannot get RelPointPos on a static element");
    return;
    }

dBodyGetRelPointPos(raydium_ode_element[e].body,px,py,pz,res);
}


void raydium_ode_element_RelPointPos_name(char *e, dReal px, dReal py, dReal pz, dReal *res)
{
raydium_ode_element_RelPointPos(raydium_ode_element_find(e),px,py,pz,res);
}

int raydium_ode_motor_create(char *name, int obj, signed char type)
{
int i;

if(raydium_ode_motor_find(name)>=0)
    {
    raydium_log("ODE: Cannot add motor \"%s\": name already exists",name);
    return -1;
    }

if(!raydium_ode_object_isvalid(obj))
    {
    raydium_log("ODE: Cannot add motor \"%s\": parent object is invalid",name);
    return -1;    
    }

for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(!raydium_ode_motor[i].state)
     {
     strcpy(raydium_ode_motor[i].name,name);
     raydium_ode_motor[i].state=type;
     raydium_ode_motor[i].object=obj;     
     return i;
     }
raydium_log("ODE: No more motor slots ! aborting \"%s\" creation",name);
return -1;
}


void raydium_ode_motor_attach(int motor, int joint, int joint_axe)
{
int i;
int e1,e2;
raydium_ode_Element *elem1;
raydium_ode_Element *elem2;

if(raydium_ode_motor_isvalid(motor) && raydium_ode_joint_isvalid(joint))
    {
    // may crash if joint is disconnected at one of its ends !
    elem1=dBodyGetData(dJointGetBody(raydium_ode_joint[joint].joint,0));
    elem2=dBodyGetData(dJointGetBody(raydium_ode_joint[joint].joint,1));
    
    e1=elem1->id;
    e2=elem2->id;
    
    if(raydium_ode_motor[motor].object!=raydium_ode_element[e1].object &&
       raydium_ode_motor[motor].object!=raydium_ode_element[e2].object )
        {
	raydium_log("ODE: Cannot attach motor: joint must be attached to at least one element from motor's object");
	return;
	}	
    
    for(i=0;i<RAYDIUM_ODE_MOTOR_MAX_JOINTS;i++)
	if(raydium_ode_motor[motor].joints[i]<0)
	    {
	    raydium_ode_motor[motor].joints[i]=joint;
	    raydium_ode_motor[motor].joints_axe[i]=joint_axe;
	    return;
	    }
    raydium_log("ODE: Error: no more joint slots for motor \"%s\" (while adding %s)", raydium_ode_motor[motor].name,raydium_ode_joint[joint].name);
    return;
    }
raydium_log("ODE: Error: cannot attach joint to motor: invalid index or name");
}

void raydium_ode_motor_attach_name(char *motor, char *joint, int joint_axe)
{
raydium_ode_motor_attach(raydium_ode_motor_find(motor),raydium_ode_joint_find(joint),joint_axe);
}

dReal raydium_ode_motor_speed_get(int m, int gears)
{
dReal *vel;
dReal velf=0;
int i,n=0;
dBodyID body;

if(!raydium_ode_motor_isvalid(m))
    {
    raydium_log("ODE: Error: Cannot get motor speed: invalid name or index");
    return 0;
    }

if(raydium_ode_motor[m].state==RAYDIUM_ODE_MOTOR_ROCKET)
    return raydium_ode_motor[m].speed;

for(i=0;i<RAYDIUM_ODE_MOTOR_MAX_JOINTS;i++)
    if(raydium_ode_motor[m].joints[i]>=0)
	{
	body=dJointGetBody(raydium_ode_joint[raydium_ode_motor[m].joints[i]].joint,raydium_ode_motor[m].joints_axe[i]);
	vel=(dReal *)dBodyGetAngularVel(body);
	velf+=sqrt(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]);
	n++;
	}
if(n) 
    {
    velf/=n;
    if(gears) velf*=(1/raydium_trigo_abs(raydium_ode_motor[m].gears[raydium_ode_motor[m].gear]));
    }
else velf=0;
return velf;
}

dReal raydium_ode_motor_speed_get_name(char *name, int gears)
{
return raydium_ode_motor_speed_get(raydium_ode_motor_find(name),gears);
}


void raydium_ode_motor_rocket_set(int m, int element, dReal x, dReal y, dReal z)
{
if(!raydium_ode_motor_isvalid(m) || !raydium_ode_element_isvalid(element))
    {
    raydium_log("ODE: Error: Cannot set rocket element: invalid name or index");
    return;
    }

if(raydium_ode_motor[m].state!=RAYDIUM_ODE_MOTOR_ROCKET)
    {
    raydium_log("ODE: Error: Cannot set rocket element: motor is not a rocket");
    return;
    }
    
raydium_ode_motor[m].rocket_element=element;
raydium_ode_motor[m].rocket_position[0]=x;
raydium_ode_motor[m].rocket_position[1]=y;
raydium_ode_motor[m].rocket_position[2]=z;
}

void raydium_ode_motor_rocket_set_name(char *motor,char *element, dReal x, dReal y, dReal z)
{
raydium_ode_motor_rocket_set(raydium_ode_motor_find(motor),raydium_ode_element_find(element),x,y,z);
}

void raydium_ode_motor_rocket_orientation(int m, dReal rx, dReal ry, dReal rz)
{
GLfloat res[3];
GLfloat dir[3]={0,0,1};
if(!raydium_ode_motor_isvalid(m))
    {
    raydium_log("ODE: Error: Cannot set rocket orientation: invalid name or index");
    return;
    }
if(raydium_ode_motor[m].state!=RAYDIUM_ODE_MOTOR_ROCKET)
    {
    raydium_log("ODE: Error: Cannot set rocket orientation: motor is not a rocket");
    return;
    }

raydium_ode_motor[m].rocket_orientation[0]=rx;
raydium_ode_motor[m].rocket_orientation[1]=ry;
raydium_ode_motor[m].rocket_orientation[2]=rz;
raydium_trigo_rotate(dir,rx,ry,rz,res);
res[0]*=raydium_ode_motor[m].speed;
res[1]*=raydium_ode_motor[m].speed;
res[2]*=raydium_ode_motor[m].speed;
raydium_ode_motor[m].rocket_direction[0]=res[0];
raydium_ode_motor[m].rocket_direction[1]=res[1];
raydium_ode_motor[m].rocket_direction[2]=res[2];
}

void raydium_ode_motor_rocket_orientation_name(char *name, dReal rx, dReal ry, dReal rz)
{
raydium_ode_motor_rocket_orientation(raydium_ode_motor_find(name),rx,ry,rz);
}


void raydium_ode_motor_rocket_playermovement(int m, signed char isplayermovement)
{
if(!raydium_ode_motor_isvalid(m))
    {
    raydium_log("ODE: Error: Cannot set rocket type (player movement): invalid name or index");
    return;
    }
if(raydium_ode_motor[m].state!=RAYDIUM_ODE_MOTOR_ROCKET)
    {
    raydium_log("ODE: Error: Cannot set rocket type (player movement): motor is not a rocket");
    return;
    }
raydium_ode_motor[m].rocket_playermovement=isplayermovement;
}

void raydium_ode_motor_rocket_playermovement_name(char *m, signed char isplayermovement)
{
return raydium_ode_motor_rocket_playermovement(raydium_ode_motor_find(m),isplayermovement);
}

signed char raydium_ode_motor_delete(int e)
{
if(!raydium_ode_motor_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot delete motor: invalid name or index");
    return 0;
    }
raydium_ode_init_motor(e);
return 1;
}

signed char raydium_ode_motor_delete_name(char *name)
{
return raydium_ode_motor_delete(raydium_ode_motor_find(name));
}


signed char raydium_ode_joint_delete(int joint)
{
int i,j;
void (*f)(int);

if(!raydium_ode_joint_isvalid(joint))
    {
    raydium_log("ODE: Error: Cannot delete joint: invalid name or index");
    return 0;
    }
for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    for(j=0;j<RAYDIUM_ODE_MOTOR_MAX_JOINTS;j++)
	if(raydium_ode_motor[i].joints[j]==joint)
	{
	raydium_ode_motor[i].joints[j]=-1;
	raydium_ode_motor[i].joints_axe[j]=0;
	}

free(dJointGetFeedback(raydium_ode_joint[joint].joint));
dJointDestroy(raydium_ode_joint[joint].joint);

f=raydium_ode_joint[joint].OnDelete;
if(f) f(joint);
raydium_ode_init_joint(joint);
return 1;
}

signed char raydium_ode_joint_delete_name(char *name)
{
return raydium_ode_joint_delete(raydium_ode_joint_find(name));
}


signed char raydium_ode_element_delete(int e, signed char deletejoints)
{
int i;
int (*f)(int);

if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot delete element: invalid name or index");
    return 0;
    }




// Only for "first pass"
if(!raydium_ode_element[e].marked_as_deleted)
{
    f=raydium_ode_element[e].OnDelete;
    if(f && !f(e)) return 0; // user cancel
}

if(raydium_ode_element_delete_LOCK)
    {
    //raydium_log("ODE: Warning: deleting of elements is locked !");
    // element will be delete at the end of the current collide test loop (but
    // is ignored for all future collisions in this test)
    raydium_ode_element[e].marked_as_deleted=1;
    return 1;
    }


// verify element state after user callback (element may be already deleted ! :)
// note : this test is bad.. an new element may have taken the free slot ...
// in facts, a user must not delete an element during his own OnDelete callback
if(!raydium_ode_element_isvalid(e)) 
    return 1; // delete is successfull thru user callback


raydium_ode_network_element_delete(e);

if(deletejoints && raydium_ode_element[e].state!=RAYDIUM_ODE_STATIC)
    {
    raydium_ode_Joint *j;
    int *to_delete;
    int n_joints;

    n_joints=dBodyGetNumJoints(raydium_ode_element[e].body);
    if(n_joints)
     {
	// create a list of joint to delete ...
	to_delete=malloc(n_joints*sizeof(int));
	for(i=0;i<n_joints;i++)
	    {
	    j=dJointGetData(dBodyGetJoint(raydium_ode_element[e].body,i));
	    if(j)
		to_delete[i]=j->id; 
	    else 
		to_delete[i]=-1;
	    }    
	// ... and delete joints
	for(i=0;i<n_joints;i++)
	  if(to_delete[i]>=0)
	    raydium_ode_joint_delete(to_delete[i]);

	free(to_delete);
     }
    }

if(raydium_ode_element[e].ray.state)
    raydium_ode_element_ray_delete(e);
dGeomSetData(raydium_ode_element[e].geom,NULL); // no more linked to this structure
dGeomDestroy(raydium_ode_element[e].geom);
if(raydium_ode_element[e].body)
 dBodyDestroy(raydium_ode_element[e].body);

for(i=0;i<RAYDIUM_ODE_ELEMENT_MAX_FIXING;i++)
    if(raydium_ode_element[e].fixed_elements[i])
	{
	free(raydium_ode_element[e].fixed_elements[i]);
	raydium_ode_element[e].fixed_elements[i]=NULL;
	}

// linked rockets
for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(raydium_ode_motor[i].state==RAYDIUM_ODE_MOTOR_ROCKET &&
       raydium_ode_motor[i].rocket_element==e)
        raydium_ode_motor_delete(i);


if(raydium_ode_element[e].particle>=0)
    raydium_particle_generator_delete(raydium_ode_element[e].particle);

raydium_ode_init_element(e);
return 1;
}

signed char raydium_ode_element_delete_name(char *name, signed char deletejoints)
{
return raydium_ode_element_delete(raydium_ode_element_find(name),deletejoints);
}


signed char raydium_ode_object_delete(int obj)
{
int i;
//raydium_ode_Element *e;

if(!raydium_ode_object_isvalid(obj))
    {
    raydium_log("ODE: Error: Cannot delete object: invalid name or index");
    return 0;
    }

if(obj == raydium_ode_object_find("GLOBAL"))
    {
    raydium_log("ODE: Error: Cannot delete special \"GLOBAL\" object");
    return 0;
    }

for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(raydium_ode_motor[i].state &&
       raydium_ode_motor[i].object==obj)
	raydium_ode_motor_delete(i);	
    
// Wow... group indices are unstable while deleting group's bodies !
//for(i=0;i<dGeomGroupGetNumGeoms(raydium_ode_object[obj].group);i++)
//    {
//    e=dGeomGetData(dGeomGroupGetGeom(raydium_ode_object[obj].group,i));
//    raydium_ode_element_delete(e->id,1);
//    }
// So i'll search for bodies in my own structures
for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].object==obj)
	raydium_ode_element_delete(i,1);

dSpaceDestroy(raydium_ode_object[obj].group);
raydium_ode_init_object(obj);
return 1;
}

signed char raydium_ode_object_delete_name(char *name)
{
return raydium_ode_object_delete(raydium_ode_object_find(name));
}

signed char raydium_ode_explosion_delete(int e)
{
if(!raydium_ode_explosion_isvalid(e))
    {
    raydium_log("ODE: Error: Cannot delete explosion: invalid name or index");
    return 0;
    }
if(raydium_ode_explosion[e].element>=0)
    raydium_ode_element_delete(raydium_ode_explosion[e].element,0);
raydium_ode_init_explosion(e);
return 1;
}


signed char raydium_ode_element_moveto(int element, int object, signed char deletejoints)
{
int i;
raydium_ode_Joint *j;
dBodyID e1,e2;

if(!raydium_ode_element_isvalid(element) ||
   !raydium_ode_object_isvalid(object) )
   {
   raydium_log("ODE: Error: Cannot move element to object: invalid index/name");
   return 0;
   }

for(i=0;i<dBodyGetNumJoints(raydium_ode_element[element].body);i++)
    {
    j=dJointGetData(dBodyGetJoint(raydium_ode_element[element].body,i));
    if(deletejoints)
	raydium_ode_joint_delete(j->id);
    else
	{
	e1=dJointGetBody(j->joint,0);
	e2=dJointGetBody(j->joint,1);
	if(e1==raydium_ode_element[element].body) e1=0;
	if(e2==raydium_ode_element[element].body) e2=0;
	dJointAttach(j->joint,e1,e2);	
	}
    }
// Well well well.. donno if dGeomGroupAdd removes body from previous
// GeomGroup, so i do it myself.
raydium_ode_element[element]._movesfrom=raydium_ode_element[element].object;
dSpaceRemove(raydium_ode_object[raydium_ode_element[element].object].group,
	     raydium_ode_element[element].geom);
raydium_ode_element[element].object=object;
dSpaceAdd(raydium_ode_object[raydium_ode_element[element].object].group,
	     raydium_ode_element[element].geom);

return 1;
}


signed char raydium_ode_element_moveto_name(char *element, char *object, char signed deletejoints)
{
return raydium_ode_element_moveto(raydium_ode_element_find(element),raydium_ode_object_find(object),deletejoints);
}


void raydium_ode_joint_break(int j)
{
dJointFeedback *jf;
dReal force=0;
if(!raydium_ode_joint[j].breakableforce) return;

if(!raydium_ode_joint_isvalid(j))
    {
    raydium_log("ODE: Error: cannot test joint breaking: invalid index/name");
    return;
    }
jf=dJointGetFeedback(raydium_ode_joint[j].joint);
if(!jf) return;
force+=raydium_trigo_abs(jf->f1[0]);
force+=raydium_trigo_abs(jf->f1[1]);
force+=raydium_trigo_abs(jf->f1[2]);
force+=raydium_trigo_abs(jf->f2[0]);
force+=raydium_trigo_abs(jf->f2[1]);
force+=raydium_trigo_abs(jf->f2[2]);
//raydium_log("%f %s",force,raydium_ode_joint[j].name);
if(force>raydium_ode_joint[j].breakableforce)
    {
    raydium_ode_joint[j].breaking=1;
    raydium_ode_joint_delete(j);
    }
}

signed char raydium_ode_launcher(int element, int from_element, dReal *rot, dReal force)
{
dReal res[3];
dReal *initial;
dVector3 final;
dReal dir[3]={0,0,1};

if(!raydium_ode_element_isvalid(element) ||
   !raydium_ode_element_isvalid(from_element) )
    {
    raydium_log("ODE: Cannot launch element: invalid name or index");
    return 0;
    }

if( raydium_ode_element[from_element].state==RAYDIUM_ODE_STATIC ||
    raydium_ode_element[element].state==RAYDIUM_ODE_STATIC )
    {
    raydium_log("ODE: Cannot launch element: you must use non-static elements");
    return 0;
    }

raydium_trigo_rotate(dir,rot[0],rot[1],rot[2],res);
res[0]*=force;
res[1]*=force;
res[2]*=force;
dBodyVectorToWorld(raydium_ode_element[from_element].body,res[0],res[1],res[2],final);
initial=(dReal *)dBodyGetLinearVel(raydium_ode_element[from_element].body);
final[0]+=initial[0];
final[1]+=initial[1];
final[2]+=initial[2];
dBodyAddForce(raydium_ode_element[element].body,final[0],final[1],final[2]);
return 1;
}

signed char raydium_ode_launcher_name(char *element, char *from_element, dReal *rot, dReal force)
{
return raydium_ode_launcher(raydium_ode_element_find(element),raydium_ode_element_find(from_element),rot,force);
}

signed char raydium_ode_launcher_name_3f(char *element, char *from_element, dReal rx, dReal ry, dReal rz, dReal force)
{
dReal tmp[3];
tmp[0]=rx;
tmp[1]=ry;
tmp[2]=rz;
return raydium_ode_launcher_name(element,from_element,tmp,force);
}


signed char raydium_ode_launcher_simple(int element, int from_element, dReal *lrot, dReal force)
{
dReal *pos;
dQuaternion rot;

if(!raydium_ode_element_isvalid(element) ||
   !raydium_ode_element_isvalid(from_element) )
    {
    raydium_log("ODE: Cannot (simple) launch element: invalid name or index");
    return 0;
    }

// test if element and from_element are from the same objet
if(raydium_ode_element[element].object!=raydium_ode_element[from_element].object)
    {
    raydium_log("ODE: Cannot (simple) launch element: element and from_element are not from the same object");
    return 0;
    }

pos=raydium_ode_element_pos_get(from_element);
raydium_ode_element_rotq_get(from_element,rot);
raydium_ode_element_move(element,pos);
raydium_ode_element_rotateq(element,rot);
raydium_ode_launcher(element,from_element,lrot,force);
raydium_ode_element_moveto(element,raydium_ode_object_find("GLOBAL"),0);
return 1;
}

signed char raydium_ode_launcher_simple_name(char *element, char *from_element, dReal *rot, dReal force)
{
return raydium_ode_launcher_simple(raydium_ode_element_find(element),raydium_ode_element_find(from_element),rot,force);
}

signed char raydium_ode_launcher_simple_name_3f(char *element, char *from_element, dReal rx, dReal ry, dReal rz, dReal force)
{
dReal tmp[3];
tmp[0]=rx;
tmp[1]=ry;
tmp[2]=rz;
return raydium_ode_launcher_simple_name(element,from_element,tmp,force);
}


void raydium_ode_explosion_blow_rand(dReal radius, dReal max_force, dReal rand_factor, dReal *pos)
{
int i;
void (*f)(signed char, dReal, dReal, dReal *);
void (*g)(int, dReal, dReal);
float rmx[3];
float minrandtorq, maxrandtorq;

if(raydium_network_mode==RAYDIUM_NETWORK_MODE_CLIENT && !raydium_ode_network_explosion_create)
{
// propag over network
raydium_ode_network_Explosion exp;
exp.type=RAYDIUM_ODE_NETWORK_EXPLOSION_BLOW;
exp.radius=radius;
memcpy(exp.pos,pos,sizeof(dReal)*3);
exp.force=max_force;
raydium_ode_network_explosion_send(&exp);
return;
}
raydium_ode_network_explosion_create=0;

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD)
	{
	dReal vect[3];
	dReal *elp;
	dReal len;
	dReal force;
	elp=(dReal *)dGeomGetPosition(raydium_ode_element[i].geom);
	
	//raydium_log("Blow: %s",raydium_ode_element[i].name);
	
	// explosion to element vector
	vect[0]=elp[0]-pos[0];
	vect[1]=elp[1]-pos[1];
	vect[2]=elp[2]-pos[2];
	// distance from explosion
	len=sqrt(vect[0]*vect[0] + vect[1]*vect[1] + vect[2]*vect[2]);
	
	if(len==0.f) continue;
//	raydium_log("dist from core: %f (radius=%f)",len,radius);
	// normalize vector
	vect[0]/=len;
	vect[1]/=len;
	vect[2]/=len;
	
	if(len>radius) continue;
	force = ((radius*radius) - (len*len)) / (radius * radius) * max_force;
//	raydium_log("resulting force: %f (max force at core=%f)",force,max_force);
	vect[0]*=force;
	vect[1]*=force;
	vect[2]*=force;
//	raydium_log("resulting impulse vector: [%f %f %f]",vect[0],vect[1],vect[2]);
	dBodyAddForce(raydium_ode_element[i].body,vect[0],vect[1],vect[2]);

	if(rand_factor)
	{
	    //ugly way of get a random float array
    	    minrandtorq=-rand_factor;
	    maxrandtorq= rand_factor;
	    rmx[0]=raydium_random_f(minrandtorq,maxrandtorq)*force;
	    rmx[1]=raydium_random_f(minrandtorq,maxrandtorq)*force;
	    rmx[2]=raydium_random_f(minrandtorq,maxrandtorq)*force;
    	    //added random torque to the pieces
	    dBodyAddTorque(raydium_ode_element[i].body,rmx[0],rmx[1],rmx[2]);
	}

	g=raydium_ode_element[i].OnBlow;
	if(g) g(i,force,max_force);
	}
f=raydium_ode_ExplosionCallback;
if(f)
    f(RAYDIUM_ODE_NETWORK_EXPLOSION_BLOW,radius,max_force,pos);

}

void raydium_ode_explosion_blow(dReal radius, dReal max_force, dReal *pos)
{
raydium_ode_explosion_blow_rand(radius,max_force,0,pos);
}

void raydium_ode_explosion_blow_3f(dReal radius, dReal max_force, dReal px, dReal py, dReal pz)
{
dReal pos[3];
pos[0]=px;
pos[1]=py;
pos[2]=pz;
raydium_ode_explosion_blow(radius,max_force,pos);
}

void raydium_ode_explosion_blow_rand_3f(dReal radius, dReal max_force, dReal rand_factor, dReal px, dReal py, dReal pz)
{
dReal pos[3];
pos[0]=px;
pos[1]=py;
pos[2]=pz;
raydium_ode_explosion_blow_rand(radius,max_force,rand_factor,pos);
}

int raydium_ode_explosion_create(char *name, dReal final_radius, dReal propag, dReal *pos)
{
int i;
void (*f)(signed char, dReal, dReal, dReal *);

if(raydium_network_mode==RAYDIUM_NETWORK_MODE_CLIENT && !raydium_ode_network_explosion_create)
{
// propag over network
raydium_ode_network_Explosion exp;
exp.type=RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL;
exp.radius=final_radius;
memcpy(exp.pos,pos,sizeof(dReal)*3);
exp.propag=propag;
raydium_ode_network_explosion_send(&exp);
return -1; // WARNING: During networked games, this function always returns -1
}
raydium_ode_network_explosion_create=0;


if(raydium_ode_explosion_find(name)>=0)
    {
    raydium_log("ODE: Cannot add explosion \"%s\": name already exists",name);
    return -1;
    }

for(i=0;i<RAYDIUM_ODE_MAX_EXPLOSIONS;i++)
    if(!raydium_ode_explosion[i].state)
     {
     strcpy(raydium_ode_explosion[i].name,name);
     raydium_ode_explosion[i].state=1;
     raydium_ode_explosion[i].config_radius=final_radius;
     raydium_ode_explosion[i].config_propag=propag;
     raydium_ode_explosion[i].radius=0;
     memcpy(raydium_ode_explosion[i].position,pos,sizeof(dReal)*3);
     
     f=raydium_ode_ExplosionCallback;
     if(f)
	f(RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL,final_radius,propag,pos);

     return i;
     }
raydium_log("ODE: No more explosion slots ! aborting \"%s\" creation",name);
return -1;
}


// load object from disk
// raydium_ode_object_load(char *filename, char *suffix)

////////


void raydium_ode_element_camera_inboard(int e, dReal px, dReal py, dReal pz, dReal lookx, dReal looky, dReal lookz)
{
dBodyID body;
dVector3 face,up,cam;
if(!raydium_ode_element_isvalid(e))
    {
    raydium_log("ODE: Error: cannot set camera on element: invalid name or index");
    return;
    }

if(raydium_ode_element[e].state==RAYDIUM_ODE_STATIC)
    {
    raydium_log("ODE: Error: cannot put camera on a static element");
    return;
    }
//glLoadIdentity();
raydium_camera_internal_prepare();
body=raydium_ode_element[e].body;
dBodyGetRelPointPos(body,px,py,pz,cam);
dBodyGetRelPointPos(body,lookx,looky,lookz,face);
dBodyVectorToWorld (body,0,0,1,up);
gluLookAt(cam[0],cam[1],cam[2],face[0],face[1],face[2],up[0],up[1],up[2]);
raydium_camera_internal(cam[0], cam[1], cam[2]);
}


void raydium_ode_element_camera_inboard_name(char *name, dReal px, dReal py, dReal pz, dReal lookx, dReal looky, dReal lookz)
{
raydium_ode_element_camera_inboard(raydium_ode_element_find(name),px,py,pz,lookx,looky,lookz);
}


void raydium_ode_draw_all(signed char names)
{
int i,j;
signed char (*bef)(int);
void (*aft)(int);

bef=raydium_ode_BeforeElementDrawCallback;
aft=raydium_ode_AfterElementDrawCallback;

if(names==RAYDIUM_ODE_DRAW_DEBUG)
 for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(raydium_ode_motor[i].state==RAYDIUM_ODE_MOTOR_ROCKET)
     {
     int j;
     j=raydium_ode_motor[i].rocket_element;
     raydium_camera_replace_go((dReal *)dGeomGetPosition(raydium_ode_element[j].geom), (dReal *)dGeomGetRotation(raydium_ode_element[j].geom));
     glDisable(GL_LIGHTING);
     raydium_texture_current_set_name("rgb(1,0,0)");
     raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
     glLineWidth(1.f);
     glBegin(GL_LINES);
     glVertex3f(raydium_ode_motor[i].rocket_position[0],raydium_ode_motor[i].rocket_position[1],raydium_ode_motor[i].rocket_position[2]);
     glVertex3f(raydium_ode_motor[i].rocket_position[0]+raydium_ode_motor[i].rocket_direction[0],
    		raydium_ode_motor[i].rocket_position[1]+raydium_ode_motor[i].rocket_direction[1],
		raydium_ode_motor[i].rocket_position[2]+raydium_ode_motor[i].rocket_direction[2]);
     glEnd();
     if(raydium_light_enabled_tag)
        glEnable(GL_LIGHTING);
     }

if(names==RAYDIUM_ODE_DRAW_DEBUG)
 for(i=0;i<RAYDIUM_ODE_MAX_EXPLOSIONS;i++)
    if(raydium_ode_explosion[i].state)
	{
	int j;
	j=raydium_ode_explosion[i].element;
    	raydium_camera_replace_go((dReal *)dGeomGetPosition(raydium_ode_element[j].geom), (dReal *)dGeomGetRotation(raydium_ode_element[j].geom));
	glutWireSphere(raydium_ode_explosion[i].radius,10,10);
	}


if(names==RAYDIUM_ODE_DRAW_DEBUG)
 for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(raydium_ode_joint[i].state)
    {
    void (*f1)(dJointID,dVector3);
    void (*f2)(dJointID,dVector3);
    dVector3 res1;
    dVector3 res2;
    dMatrix3 r;
    int type;
    char col;
    dRSetIdentity(r);
    type=dJointGetType(raydium_ode_joint[i].joint);
    switch(type)
	{
	case dJointTypeHinge2:
	    f1=dJointGetHinge2Anchor;
	    f2=dJointGetHinge2Anchor2;
	    col='3';
	    break;
	case dJointTypeUniversal:
	    f1=dJointGetUniversalAnchor;
	    f2=dJointGetUniversalAnchor2;
	    col='5';
	    break;
	case dJointTypeHinge:
	    f1=dJointGetHingeAnchor;
	    f2=dJointGetHingeAnchor2;
	    col='2';
	    break;
	case dJointTypeFixed:
	    f1=NULL;
	    f2=NULL;
	    col='0';
	    break;
	default: raydium_log("ODE: ERROR: draw_all: invalid joint type !");
	}

     if(f1 && f2)
     {
      f1(raydium_ode_joint[i].joint,res1);
      f2(raydium_ode_joint[i].joint,res2);
      raydium_texture_current_set_name("rgb(1,0,0)");
      raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
      raydium_camera_replace_go(res1,r);
      glutWireSphere(0.05,5,5);
      raydium_camera_replace_go(res2,r);
      glutWireSphere(0.05,5,5);
      raydium_osd_color_ega(col);
      raydium_osd_printf_3D(res1[0],res1[1],res1[2],12,0.6,"font2.tga","<%s>",raydium_ode_joint[i].name);
     }
    }
     

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state)
     {
	
     if(names==RAYDIUM_ODE_DRAW_NORMAL || 
        names==RAYDIUM_ODE_DRAW_SHADOWERS ||
	names==RAYDIUM_ODE_DRAW_NORMAL_NO_POST)
        {
	
	if(names==RAYDIUM_ODE_DRAW_SHADOWERS && raydium_ode_element[i].mesh==raydium_shadow_ground_mesh)
	    continue;	
	if(names!=RAYDIUM_ODE_DRAW_SHADOWERS && bef && !bef(i))
    	    continue;

    	raydium_camera_replace_go((dReal *)dGeomGetPosition(raydium_ode_element[i].geom), (dReal *)dGeomGetRotation(raydium_ode_element[i].geom));
	if(raydium_ode_element[i].mesh>=0)
	    {
	    if(raydium_shadow_rendering)
    		raydium_shadow_object_draw(raydium_ode_element[i].mesh);
	    else
    		raydium_object_draw(raydium_ode_element[i].mesh);
	    }
	if(names!=RAYDIUM_ODE_DRAW_SHADOWERS && aft)
	    aft(i);

	if(raydium_ode_element[i].particle>=0 && names!=RAYDIUM_ODE_DRAW_SHADOWERS)
	    {
	    dVector3 res;

	    // if element is static, fake a temporary body
	    if(raydium_ode_element[i].state==RAYDIUM_ODE_STATIC)
	    {
	     dBodyID body;
	     dReal *pos;
	     dQuaternion rot;
	     body=dBodyCreate(raydium_ode_world);
	     pos=raydium_ode_element_pos_get(i);
	     raydium_ode_element_rotq_get(i,rot);
	     dBodySetPosition(body,pos[0],pos[1],pos[2]);
	     dBodySetQuaternion(body,rot);
	     dBodyGetRelPointPos(body,
				raydium_ode_element[i].particle_offset[0],
				raydium_ode_element[i].particle_offset[1],
				raydium_ode_element[i].particle_offset[2],	    
				res);
	     dBodyDestroy(body);
	    }
	    else
	    if(raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD)
	    {
	     dBodyGetRelPointPos(raydium_ode_element[i].body,
				raydium_ode_element[i].particle_offset[0],
				raydium_ode_element[i].particle_offset[1],
				raydium_ode_element[i].particle_offset[2],	    
				res);	    
	    }
	    
	    raydium_particle_generator_move(
		raydium_ode_element[i].particle,res);
	    }
	   
	for(j=0;j<RAYDIUM_ODE_ELEMENT_MAX_FIXING;j++)
	  if(raydium_ode_element[i].fixed_elements[j])
	    {
	    glPushMatrix();
	    glTranslatef(raydium_ode_element[i].fixed_elements[j]->rel_pos[0],
			 raydium_ode_element[i].fixed_elements[j]->rel_pos[1],
			 raydium_ode_element[i].fixed_elements[j]->rel_pos[2]);
	    {
	    dMatrix3 R;
	    GLfloat matrix[16];
	    dQtoR(raydium_ode_element[i].fixed_elements[j]->rel_rot,R);
	    matrix[0]=R[0];
	    matrix[1]=R[4];
	    matrix[2]=R[8];
	    matrix[3]=0;
	    matrix[4]=R[1];
	    matrix[5]=R[5];
	    matrix[6]=R[9];
	    matrix[7]=0;
	    matrix[8]=R[2];
	    matrix[9]=R[6];
	    matrix[10]=R[10];
	    matrix[11]=0;
	    matrix[12]=0;
	    matrix[13]=0;
	    matrix[14]=0;
	    matrix[15]=1;
	    glMultMatrixf (matrix);
	    }
	    raydium_object_draw(raydium_ode_element[i].fixed_elements[j]->mesh);
	    glPopMatrix();
	    }
	}
     else
        {
	const dReal *p;

	
	if(names==RAYDIUM_ODE_DRAW_DEBUG) // draw shape
	    {
	    raydium_rendering_internal_prepare_texture_render(0); // !!!
    	    raydium_texture_current_set_name("rgb(1,0,0)");
	    raydium_camera_replace_go((dReal *)dGeomGetPosition(raydium_ode_element[i].geom), (dReal *)dGeomGetRotation(raydium_ode_element[i].geom));
	    raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);

	    if(dGeomGetClass(raydium_ode_element[i].geom)==dBoxClass)
		{
		dVector3 sides;
		float lx;
		float ly;
		float lz;
		dGeomBoxGetLengths(raydium_ode_element[i].geom,sides);
		lx = sides[0]*0.5f;
		ly = sides[1]*0.5f;
		lz = sides[2]*0.5f;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// sides
		glBegin (GL_TRIANGLE_STRIP);
		glNormal3f (-1,0,0);
		glVertex3f (-lx,-ly,-lz);
		glVertex3f (-lx,-ly,lz);
		glVertex3f (-lx,ly,-lz);
		glVertex3f (-lx,ly,lz);
		glNormal3f (0,1,0);
		glVertex3f (lx,ly,-lz);
		glVertex3f (lx,ly,lz);
		glNormal3f (1,0,0);
		glVertex3f (lx,-ly,-lz);
		glVertex3f (lx,-ly,lz);
		glNormal3f (0,-1,0);
		glVertex3f (-lx,-ly,-lz);
		glVertex3f (-lx,-ly,lz);
		glEnd();
	    
    	        // top face
	        glBegin (GL_TRIANGLE_FAN);
	        glNormal3f (0,0,1);
	        glVertex3f (-lx,-ly,lz);
	        glVertex3f (lx,-ly,lz);
	        glVertex3f (lx,ly,lz);
	        glVertex3f (-lx,ly,lz);
	        glEnd();

	        // bottom face
	        glBegin (GL_TRIANGLE_FAN);
	        glNormal3f (0,0,-1);
	        glVertex3f (-lx,-ly,-lz);
	        glVertex3f (-lx,ly,-lz);
	        glVertex3f (lx,ly,-lz);
	        glVertex3f (lx,-ly,-lz);
	        glEnd();
	        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	    
	        }
	    else
		//if(dGeomGetClass(raydium_ode_element[i].geom)==dSphereClass)
		{
	        glutWireSphere(dGeomSphereGetRadius(raydium_ode_element[i].geom),10,10);
	        }
		
	
	    p=dGeomGetPosition(raydium_ode_element[i].geom);
	    if(raydium_ode_element[i]._touched)
		raydium_osd_color_ega('c');
	    else
		raydium_osd_color_ega('f');
		
	    if(raydium_ode_element[i]._avoidedcol)
		raydium_osd_color_ega('d');
	
	    if(!dGeomIsEnabled(raydium_ode_element[i].geom))
		raydium_osd_color_ega('a');

	    if( raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD &&
		!dBodyIsEnabled(raydium_ode_element[i].body) )
		    raydium_osd_color_ega('0');
	
	    raydium_osd_printf_3D(p[0],p[1],p[2],12,0.6,"font2.tga","%i %s (%s) @ %i",i,raydium_ode_element[i].name,raydium_ode_object[raydium_ode_element[i].object].name,raydium_ode_element[i].distant_owner);
	    }

	    if(names==RAYDIUM_ODE_DRAW_AABB) // draw AABB
	    {
	    dReal aabb[6];

	    raydium_camera_replace();
	    raydium_rendering_internal_prepare_texture_render(0); // !!
	    raydium_texture_current_set_name("rgb(0,0,1)");
	    raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
	    dGeomGetAABB(raydium_ode_element[i].geom,aabb);
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	    // sides
	    glBegin (GL_TRIANGLE_STRIP);
	    glNormal3f (-1,0,0);
	    glVertex3f (aabb[0],aabb[2],aabb[4]);
	    glVertex3f (aabb[0],aabb[2],aabb[5]);
	    glVertex3f (aabb[0],aabb[3],aabb[4]);
	    glVertex3f (aabb[0],aabb[3],aabb[5]);

	    glNormal3f (0,1,0);
	    glVertex3f (aabb[1],aabb[3],aabb[4]);
	    glVertex3f (aabb[1],aabb[3],aabb[5]);

	    glNormal3f (1,0,0);
	    glVertex3f (aabb[1],aabb[2],aabb[4]);
	    glVertex3f (aabb[1],aabb[2],aabb[5]);

	    glNormal3f (0,-1,0);
	    glVertex3f (aabb[0],aabb[2],aabb[4]);
	    glVertex3f (aabb[0],aabb[2],aabb[5]);
	    glEnd();
	    
    	    // top face
	    glBegin (GL_TRIANGLE_FAN);
	    glNormal3f (0,0,1);
	    glVertex3f (aabb[0],aabb[2],aabb[5]);
	    glVertex3f (aabb[1],aabb[2],aabb[5]);
	    glVertex3f (aabb[1],aabb[3],aabb[5]);
	    glVertex3f (aabb[0],aabb[3],aabb[5]);
	    glEnd();

	    // bottom face
	    glBegin (GL_TRIANGLE_FAN);
	    glNormal3f (0,0,-1);
	    glVertex3f (aabb[0],aabb[2],aabb[4]);
	    glVertex3f (aabb[0],aabb[3],aabb[4]);
	    glVertex3f (aabb[1],aabb[3],aabb[4]);
	    glVertex3f (aabb[1],aabb[2],aabb[4]);
	    glEnd();
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	    }
	    
	    if(names==RAYDIUM_ODE_DRAW_RAY)
	    {
	        if(raydium_ode_element[i].ray.state)
		{
		dVector3 start,dir;
		dReal len;

		raydium_rendering_internal_prepare_texture_render(0); // !!!
    		raydium_texture_current_set_name("rgb(1,0,0)");
		raydium_rendering_internal_prepare_texture_render(raydium_texture_current_main);
		
		dGeomRayGet(raydium_ode_element[i].ray.geom,start,dir);
		len=dGeomRayGetLength(raydium_ode_element[i].ray.geom);
		raydium_camera_replace();
		glBegin(GL_LINES);
		//printf("%f %f %f | %f %f %f\n",start[0],start[1],start[2],dir[0],dir[1],dir[2]);
	        glVertex3f(start[0],start[1],start[2]);
	        glVertex3f(start[0]+(dir[0]*len),start[1]+(dir[1]*len),start[2]+(dir[2]*len));
		glEnd();
	        }
	    }
	}
     }

if(names==RAYDIUM_ODE_DRAW_NORMAL) // but not RAYDIUM_ODE_DRAW_NORMAL_NO_POST
    {
    raydium_hdr_map(); // create HDR map
    raydium_shadow_map_render();
    }
}


void raydium_ode_near_callback(void *data, dGeomID o1, dGeomID o2)
{
int i,n;
// dirty, but...
#define N 400
static  dContact contact[N];
dJointID c;
raydium_ode_Element *e1, *e2;
int ground_elem_id,distan_obj_id;
dReal erp=0;
dReal cfm=0;
dReal slip=0;
int count=0;
signed char (*f)(int,int, dContact *);
signed char (*r)(int,int, dContact *);

f=raydium_ode_CollideCallback;
r=raydium_ode_RayCallback;


if(dGeomIsSpace (o1) || dGeomIsSpace (o2)) 
    {
    raydium_ode_Object *oo1, *oo2;
    signed char (*g)(int,int);
    oo1=dGeomGetData(o1);
    oo2=dGeomGetData(o2);
    g=raydium_ode_ObjectNearCollide;
    if(g && !g(oo1->id,oo2->id)) return;
    dSpaceCollide2 (o1,o2,data,&raydium_ode_near_callback);
    return;
    }

ground_elem_id=raydium_ode_element_find("ground");
distan_obj_id=raydium_ode_object_find("DISTANT");

e1=dGeomGetData(o1);
e2=dGeomGetData(o2);

/*
  // avoid "ground - distant elements" contacts (may do it for all static-static contacts ?)
  if(e1->id == ground_elem_id && e2->object == distan_obj_id)
    return;
  if(e2->id == ground_elem_id && e1->object == distan_obj_id)
    return;
  // .
*/

  // As said above, we will avoid all static-statics contacts.
  // I think we must provide a "pre-filter" callback, since the user
  // may need this contact infos

  if(e1->state==RAYDIUM_ODE_STATIC && e2->state==RAYDIUM_ODE_STATIC)
    return;

  n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
  if(n>=N-1)
    raydium_log("ODE: WARNING ! Not enought contact points available ! (%i max)",N);
  if (n > 0) 
  {
  
    for (i=0; i<n; i++) 
	{	
	e1=dGeomGetData(contact[i].geom.g1);
	e2=dGeomGetData(contact[i].geom.g2);
	
	if(e1==NULL || e2==NULL)
	    continue; // Deleted, or not one of our elements

	if(e1==e2)
	    continue; // May happend with ray

	if(e1 && e1->marked_as_deleted)
	    return;
	if(e2 && e2->marked_as_deleted)
	    return;

	if( e1 && e2 && e1->_movesfrom>=0 &&
	    e1->_movesfrom==e2->object)
		{
		e1->_avoidedcol=1;
		continue; // avoid collision ! (where are moving from this object)
		}

	if( e1 && e2 && e2->_movesfrom>=0 &&
	    e2->_movesfrom==e1->object)
		{
		e2->_avoidedcol=1;
		continue; // avoid collision ! (where are moving from this object)
		}
	
	erp=cfm=slip=0;

	if(e1)
	    {
	    erp=e1->erp;
	    cfm=e1->cfm;
	    slip=e1->slip;
	    count=1;
	    }
	    
	if(e2)
	    {
	    erp+=e2->erp;
	    cfm+=e2->cfm;
	    slip+=e2->slip;
	    count++;
	    }
	    
	if(count)
	    {
	    erp/=count;
	    cfm/=count;
	    slip/=count;
	    }
	    
	contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
	dContactSoftERP | dContactSoftCFM | dContactApprox1;
	contact[i].surface.mu = dInfinity;

	contact[i].surface.slip1 = slip;
	contact[i].surface.slip2 = slip;
	
	contact[i].surface.soft_erp = erp;
	contact[i].surface.soft_cfm = cfm;



	if(dGeomGetClass(contact[i].geom.g1)==dRayClass)
	    {
	    // raydium_ode_RayCallback (1)
	    if(r)
		{
		int id1,id2;
		id1=id2=-1;
		if(e1) id1=e1->id;
		if(e2) id2=e2->id;
		if(!r(id1,id2,&contact[i])) continue;
		}

	    if(e1 && e2 && 
	    (e1->ray.min_dist > contact[i].geom.depth || e1->ray.min_dist==0) )
		{
		e1->ray.min_dist=contact[i].geom.depth;
		e1->ray.min_elem=e2->id;
		memcpy(e1->ray.min_pos,contact[i].geom.pos,sizeof(dReal)*3);
		}

	    if(e1 && e2 && e1->ray.max_dist < contact[i].geom.depth)
		{
		e1->ray.max_dist=contact[i].geom.depth;
		e1->ray.max_elem=e2->id;
		memcpy(e1->ray.max_pos,contact[i].geom.pos,sizeof(dReal)*3);
		}
	    continue;
	    }

	if(dGeomGetClass(contact[i].geom.g2)==dRayClass)
	    {
	    // raydium_ode_RayCallback (2)
	    if(r)
		{
		int id1,id2;
		id1=id2=-1;
		if(e1) id1=e1->id;
		if(e2) id2=e2->id;
		if(!r(id1,id2,&contact[i])) continue;
		}

	    if(e1 && e2 && 
	    (e2->ray.min_dist > contact[i].geom.depth || e2->ray.min_dist==0) )
		{
		e2->ray.min_dist=contact[i].geom.depth;
		e2->ray.min_elem=e1->id;
		memcpy(e2->ray.min_pos,contact[i].geom.pos,sizeof(dReal)*3);
		}

	    if(e1 && e2 && e2->ray.max_dist < contact[i].geom.depth)
		{
		e2->ray.max_dist=contact[i].geom.depth;
		e2->ray.max_elem=e1->id;
		memcpy(e2->ray.max_pos,contact[i].geom.pos,sizeof(dReal)*3);
		}
	    continue;
	    }
		
	// raydium_ode_CollideCallback
	if(f)
	    {
	    int id1,id2;
	    id1=id2=-1;
	    if(e1) id1=e1->id;
	    if(e2) id2=e2->id;
	    if(!f(id1,id2,&contact[i])) continue;
	    }


	if(e1) e1->_touched=1; // may use it as a counter ?
	if(e2) e2->_touched=1; // ...
	//printf("%s <-> %s\n",e1->name,e2->name); // let's flood :)
	c = dJointCreateContact (raydium_ode_world,raydium_ode_contactgroup,&contact[i]);
	dJointAttach (c,dGeomGetBody(contact[i].geom.g1),dGeomGetBody(contact[i].geom.g2));
    }
  }
}


void raydium_ode_callback(void)
{
int i;
void (*f)(void);
f=raydium_ode_StepCallback;

raydium_ode_network_read();

if(f) f();

/*
for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(raydium_ode_joint[i].state)
	{
	if(raydium_ode_joint[i].hinge2correct)
	    {
	    //dJointCorrectHinge2(raydium_ode_joint[i].joint);
	    }
	}
*/

for(i=0;i<RAYDIUM_ODE_MAX_MOTORS;i++)
    if(raydium_ode_motor[i].state)
	raydium_ode_motor_update_joints_data_internal(i);

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state)
	{
	// init per_step variables
	raydium_ode_element[i]._touched=0;
	raydium_ode_element[i]._avoidedcol=0;
	raydium_ode_element[i].ground_texture=0;
	// test for damping effect:
	if( raydium_ode_element[i].state==RAYDIUM_ODE_STANDARD &&
	    raydium_ode_element[i].rotfriction!=0.f)
	    {
	    dReal *torque;
	    dReal torqueinv[3];
	    // may use element size, too ...
	    torque=(dReal *)dBodyGetAngularVel(raydium_ode_element[i].body);
	    torqueinv[0]=torque[0]*-raydium_ode_element[i].rotfriction;
	    torqueinv[1]=torque[1]*-raydium_ode_element[i].rotfriction;
	    torqueinv[2]=torque[2]*-raydium_ode_element[i].rotfriction;
	    dBodyAddTorque(raydium_ode_element[i].body,torqueinv[0],torqueinv[1],torqueinv[2]);
	    }
	
	if(raydium_ode_element[i].ray.state)
	    {
	    // update ray position and direction
	    dReal *pos;
	    dReal dir[3];
	    raydium_ode_Ray *r;

	    r=&raydium_ode_element[i].ray;
	    dBodyVectorToWorld(raydium_ode_element[i].body,
			r->rel_dir[0],
			r->rel_dir[1],
			r->rel_dir[2],
			dir);
	    //printf("set : %f %f %f\n",dir[0],dir[1],dir[2]);
	    pos=(dReal *)dBodyGetPosition(raydium_ode_element[i].body);
	    dGeomRaySet(r->geom,pos[0],pos[1],pos[2],dir[0],dir[1],dir[2]);
	    r->max_dist=0;
	    r->min_dist=0;
	    r->max_elem=r->min_elem=-1;
	    }
	}

for(i=0;i<RAYDIUM_ODE_MAX_EXPLOSIONS;i++)
    if(raydium_ode_explosion[i].state)
	{
	// 1 - check if radius > config_radius (and then delete explosion)
	if(raydium_ode_explosion[i].radius>raydium_ode_explosion[i].config_radius)
	    raydium_ode_explosion_delete(i);
	else
	 {
	 // 2 - increment radius
	 raydium_ode_explosion[i].radius+=raydium_ode_explosion[i].config_propag;
	 
	 // 3 - delete previous element if exists
	 if(raydium_ode_explosion[i].element>=0)
	    raydium_ode_element_delete(raydium_ode_explosion[i].element,0);

	 // 4 - create new element
	 // (not really a distant element, but we don't want progap' ...)
	 raydium_ode_network_distant_create=1;
	 raydium_ode_explosion[i].element=
	 raydium_ode_object_sphere_add(raydium_ode_explosion[i].name,
	                               raydium_ode_object_find("GLOBAL"),
				       0,
				       raydium_ode_explosion[i].radius,
				       RAYDIUM_ODE_STATIC,
				       0, /* see below */
				       "");
	 raydium_ode_element[raydium_ode_explosion[i].element].user_tag=RAYDIUM_ODE_TAG_EXPLOSION;
	 raydium_ode_element_material(raydium_ode_explosion[i].element,RAYDIUM_ODE_MATERIAL_SOFT2);
	 raydium_ode_element_move(raydium_ode_explosion[i].element,raydium_ode_explosion[i].position);
	 }
	}


raydium_ode_element_delete_LOCK=1;
//raydium_profile_start();
dSpaceCollide (raydium_ode_space,0,&raydium_ode_near_callback);
for(i=0;i<RAYDIUM_ODE_MAX_OBJECTS;i++)
    if(raydium_ode_object[i].state && raydium_ode_object[i].colliding)
	dSpaceCollide ((dSpaceID)raydium_ode_object[i].group,0,&raydium_ode_near_callback);
//raydium_profile_end("collision detection");
raydium_ode_element_delete_LOCK=0;



//raydium_profile_start();
#ifdef ODE_QUICKSTEP
dWorldQuickStep(raydium_ode_world,RAYDIUM_ODE_TIMESTEP);
#else
dWorldStep(raydium_ode_world,RAYDIUM_ODE_TIMESTEP);
#endif
//raydium_profile_end("world stepping");


dJointGroupEmpty (raydium_ode_contactgroup);

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state  && 
       raydium_ode_element[i].marked_as_deleted)
	    raydium_ode_element_delete(i,1);
    	    


for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state  	    && 
       raydium_ode_element[i]._movesfrom>=0 &&
       !raydium_ode_element[i]._avoidedcol  )
    	    raydium_ode_element[i]._movesfrom=-1;

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state   && 
       raydium_ode_element[i].isplayer )
        {
	dMatrix3 R;
	dRFromEulerAngles(R,0,0,raydium_ode_element[i].playerangle);
	dBodySetRotation(raydium_ode_element[i].body,R);
	dBodySetAngularVel(raydium_ode_element[i].body,0,0,0);
	}

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state  &&
       raydium_ode_element[i].ttl!=-1 )
	{
        if(raydium_ode_element[i].ttl==0)
	    raydium_ode_element_delete(i,1);
	else
    	    raydium_ode_element[i].ttl--;	
	}


for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state  &&
       raydium_ode_element[i].nid>=0  &&
       raydium_ode_element[i].distant)
	    raydium_ode_network_element_trajectory_correct(i);


for(i=0;i<RAYDIUM_ODE_MAX_JOINTS;i++)
    if(raydium_ode_joint[i].state)
	raydium_ode_joint_break(i);

}

void raydium_ode_time_change(GLfloat perc)
{
raydium_timecall_freq_change(raydium_ode_timecall,(RAYDIUM_ODE_PHYSICS_FREQ*perc)/100.f);
raydium_particle_time_factor=perc/100.f;
raydium_object_anim_time_factor=perc/100.f;
}

void raydium_ode_internal_particle_genetator_deleted_callback(int gen)
{
int i;

for(i=1;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state &&
	raydium_ode_element[i].particle==gen)
	    raydium_ode_element[i].particle=-1;
}

void raydium_ode_element_particle(int elem, char *filename)
{
if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot attach particle generator: invalid index or name");
    return;
    }
raydium_ode_element[elem].particle=raydium_particle_generator_load(filename,raydium_ode_element[elem].name);
memset(raydium_ode_element[elem].particle_offset,0,sizeof(dReal)*3);
}

void raydium_ode_element_particle_name(char *elem, char *filename)
{
raydium_ode_element_particle(raydium_ode_element_find(elem),filename);
}

void raydium_ode_element_particle_offset(int elem, char *filename, dReal *offset)
{
if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot attach particle generator (offset): invalid index or name");
    return;
    }
raydium_ode_element[elem].particle=raydium_particle_generator_load(filename,raydium_ode_element[elem].name);
memcpy(raydium_ode_element[elem].particle_offset,offset,sizeof(dReal)*3);
}

void raydium_ode_element_particle_offset_name(char *elem, char *filename, dReal *offset)
{
raydium_ode_element_particle_offset(raydium_ode_element_find(elem),filename,offset);
}

void raydium_ode_element_particle_offset_name_3f(char *elem, char *filename, dReal ox, dReal oy, dReal oz)
{
dReal offset[3];
offset[0]=ox;
offset[1]=oy;
offset[2]=oz;
raydium_ode_element_particle_offset(raydium_ode_element_find(elem),filename,offset);
}


// no offset provided yet for particle_point ... (easy to write, indeed)
void raydium_ode_element_particle_point(int elem, char *filename)
{
char n[RAYDIUM_MAX_NAME_LEN];
int p;
dReal *pos;

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: Cannot attach particle genrator: invalid index or name");
    return;
    }
raydium_particle_name_auto(raydium_ode_element[elem].name,n);
p=raydium_particle_generator_load(filename,n);
if(p<0) return;
pos=raydium_ode_element_pos_get(elem);
raydium_particle_generator_move(p,pos);
}

void raydium_ode_element_particle_point_name(char *elem, char *filename)
{
raydium_ode_element_particle_point(raydium_ode_element_find(elem),filename);
}

// smooth camera related functions

void raydium_camera_smooth_path_to_element(char *path, int element, GLfloat path_step, GLfloat smooth_step)
{
GLfloat x,y,z,zoom,roll;
GLfloat *pos;

pos=raydium_ode_element_pos_get(element);

if(raydium_camera_smooth_path(path,path_step,&x,&y,&z,&zoom,&roll)==-1)
    raydium_log("camera path error with '%s'",path);

raydium_camera_smooth(x,y,z,pos[1],-pos[2],pos[0],zoom,roll,smooth_step);
}

void raydium_camera_smooth_path_to_element_name(char *path,char *element, GLfloat path_step, GLfloat smooth_step)
{
raydium_camera_smooth_path_to_element(path,raydium_ode_element_find(element),path_step,smooth_step);
}


void raydium_camera_smooth_element_to_path_offset(int element, GLfloat offset_x, GLfloat offset_y, GLfloat offset_z, char *path, GLfloat path_step, GLfloat smooth_step)
{
GLfloat x,y,z,zoom,roll;
GLfloat *pos;
GLfloat offset[3];

pos=raydium_ode_element_pos_get(element);

if(raydium_camera_smooth_path(path,path_step,&x,&y,&z,&zoom,&roll)==-1)
    raydium_log("camera path error with '%s'",path);

// static elems are not supported
dBodyVectorToWorld(raydium_ode_element[element].body,offset_x,offset_y,offset_z,offset);

raydium_camera_smooth(pos[0]+offset[0],pos[1]+offset[1],pos[2]+offset[2],y,-z,x,zoom,roll,smooth_step);
}

void raydium_camera_smooth_element_to_path_offset_name(char *element, GLfloat offset_x, GLfloat offset_y, GLfloat offset_z, char *path, GLfloat path_step, GLfloat smooth_step)
{
raydium_camera_smooth_element_to_path_offset(raydium_ode_element_find(element),offset_x,offset_y,offset_z,path,path_step,smooth_step);
}

void raydium_camera_smooth_element_to_path_name(char *element, char *path, GLfloat path_step, GLfloat smooth_step)
{
raydium_camera_smooth_element_to_path_offset(raydium_ode_element_find(element),0,0,0,path,path_step,smooth_step);
}


// This function is provided "for fun" only. Not all effects are dumped:
// Missing : shadows, forced colors, before/after callbacks, 
// fixed elements, ...
// Some code is pasted from file.c (and this is BAD ! :)

int raydium_ode_capture_3d(char *filename)
{
int i,j,k;
FILE *fp;
dVector3 res,norm;
char text[256];
char sprt[256];


fp=raydium_file_fopen(filename,"wt");
if(!fp)
    {
    raydium_log("ERROR : failed to create 3D capture file '%s'",filename);
    return 0;
    }
fprintf(fp,"1\n");

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state && raydium_ode_element[i].mesh>=0)
     {
        int end;
	
        dBodyID body;
	dReal *pos;
	dQuaternion rot;
	body=dBodyCreate(raydium_ode_world);
	pos=raydium_ode_element_pos_get(i);
	raydium_ode_element_rotq_get(i,rot);
	dBodySetPosition(body,pos[0],pos[1],pos[2]);
	dBodySetQuaternion(body,rot);
	k=raydium_ode_element[i].mesh;
	
	// should prepare "ode" instance here, too ...
	if(raydium_object_anims[k]>0)
	    end=raydium_object_start[k]+raydium_object_anim_len[k];
	else
	    end=raydium_object_end[k];
	
	for(j=raydium_object_start[k];j<end;j++)
	{
	  if(raydium_vertex_texture_multi[j])
	    {
	    sprintf(text,"%s;%f|%f|%s",raydium_texture_name[raydium_vertex_texture[j]],
	                               raydium_vertex_texture_multi_u[j],
	                               raydium_vertex_texture_multi_v[j],
	                               raydium_texture_name[raydium_vertex_texture_multi[j]]);
	    }
	else
	    strcpy(text,raydium_texture_name[raydium_vertex_texture[j]]);
													       
	dBodyGetRelPointPos(body,
                        	raydium_vertex_x[j],
                        	raydium_vertex_y[j],
                        	raydium_vertex_z[j],
                        	res);
				
	dBodyVectorToWorld(body,
				raydium_vertex_normal_visu_x[j],
				raydium_vertex_normal_visu_y[j],
				raydium_vertex_normal_visu_z[j],
				norm);
	
	fprintf(fp,"%f %f %f %f %f %f %f %f %s\n",
					res[0],
					res[1],
					res[2],
					norm[0],
					norm[1],
					norm[2],
					raydium_vertex_texture_u[j],
					raydium_vertex_texture_v[j],
					text);
	}

	dBodyDestroy(body);
    	// raydium_ode_element[i].mesh
     }
fclose(fp);
raydium_log("3D capture saved to '%s'",filename);
sprintf(sprt,"%s.sprt",filename);
raydium_particle_state_dump(sprt);
return 1;
}


int raydium_ode_orphans_check(void)
{
int i,j,k,n;
dGeomID g;
int cpt=0;

for(i=0;i<RAYDIUM_ODE_MAX_OBJECTS;i++)
    if(raydium_ode_object[i].state)
    {
    n=dSpaceGetNumGeoms(raydium_ode_object[i].group);
    if(!n) continue;

    for(j=0;j<n;j++)
	{
	g=dSpaceGetGeom(raydium_ode_object[i].group,j);
	for(k=0;k<RAYDIUM_ODE_MAX_ELEMENTS;k++)
	    if(raydium_ode_element[k].state)
		if(raydium_ode_element[k].geom==g)
		    break;
	
	if(k==RAYDIUM_ODE_MAX_ELEMENTS)
	    {
	    cpt++;
	    raydium_log("new orphan in '%s'",raydium_ode_object[i].name);
	    }
	}
    }
return cpt;
}


#include "ode_net.c"
