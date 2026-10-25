/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef RAY_ODE_H
#define RAY_ODE_H

#ifdef __cplusplus
// ugly workaround to avoid odemath.h ...
#define _ODE_ODEMATH_H_
#endif
#include "../ode/include/ode/ode.h"

#define RAYDIUM_ODE_MAX_OBJECTS			64
#define RAYDIUM_ODE_MAX_ELEMENTS		256
#define RAYDIUM_ODE_MAX_JOINTS			256
#define RAYDIUM_ODE_MAX_MOTORS			64
#define RAYDIUM_ODE_MAX_EXPLOSIONS		32
#define	RAYDIUM_ODE_MOTOR_MAX_JOINTS		10
#define	RAYDIUM_ODE_MOTOR_MAX_GEARS		10
#define	RAYDIUM_ODE_ELEMENT_MAX_FIXING		10


#define RAYDIUM_ODE_PHYSICS_FREQ		400
#define	RAYDIUM_ODE_TIMESTEP			(0.006f)

#define RAYDIUM_ODE_AUTODETECT			-1
#define RAYDIUM_ODE_STANDARD			1
#define RAYDIUM_ODE_STATIC			2
#define RAYDIUM_ODE_FIXING			3
#define RAYDIUM_ODE_MOTOR_ENGINE		1
#define RAYDIUM_ODE_MOTOR_ANGULAR		2
#define RAYDIUM_ODE_MOTOR_ROCKET		3

#define RAYDIUM_ODE_JOINT_AXE_X			1,0,0
#define RAYDIUM_ODE_JOINT_AXE_Y			0,1,0
#define RAYDIUM_ODE_JOINT_AXE_Z			0,0,1
#define RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES	RAYDIUM_ODE_JOINT_AXE_Z,RAYDIUM_ODE_JOINT_AXE_Y
#define RAYDIUM_ODE_JOINT_FIXED			-10

#define RAYDIUM_ODE_MATERIAL_HARD		0.9,0.1
#define RAYDIUM_ODE_MATERIAL_MEDIUM		0.5,0.5
#define RAYDIUM_ODE_MATERIAL_SOFT		0.1,0.9
#define RAYDIUM_ODE_MATERIAL_SOFT2		0.0,0.9
#define RAYDIUM_ODE_MATERIAL_DEFAULT		RAYDIUM_ODE_MATERIAL_HARD

#define RAYDIUM_ODE_SLIP_ICE			5.f
#define RAYDIUM_ODE_SLIP_PLAYER			10.f
#define RAYDIUM_ODE_SLIP_NORMAL			0.4f		
#define RAYDIUM_ODE_SLIP_DEFAULT		RAYDIUM_ODE_SLIP_NORMAL

#define RAYDIUM_ODE_TAG_EXPLOSION		-1
#define RAYDIUM_ODE_TAG_GROUND			-2

#define RAYDIUM_ODE_NETWORK_OPTIMAL		-1
#define RAYDIUM_ODE_NETWORK_MAXFREQ		20

#define RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL	1
#define RAYDIUM_ODE_NETWORK_EXPLOSION_BLOW	2

// do not change values, some other file are using numeric values ! (shadows)
#define RAYDIUM_ODE_DRAW_NORMAL			0
#define RAYDIUM_ODE_DRAW_DEBUG			1
#define RAYDIUM_ODE_DRAW_AABB			2
#define RAYDIUM_ODE_DRAW_RAY			3
#define RAYDIUM_ODE_DRAW_SHADOWERS		4
#define RAYDIUM_ODE_DRAW_NORMAL_NO_POST		5

__global dWorldID 	raydium_ode_world;
__global dSpaceID 	raydium_ode_space;
__global dJointGroupID 	raydium_ode_contactgroup;
__global int		raydium_ode_ground_mesh;
__global GLint		raydium_ode_timecall; // read only (timecall index for ode callback)
__global void *		raydium_ode_CollideCallback; // signed char f(int,int,dContact*)
__global void *		raydium_ode_StepCallback; // void f(void)
__global void *		raydium_ode_ObjectNearCollide; // signed char f(int,int)
__global void *		raydium_ode_RayCallback; // signed char f(int,int,dContact*)
__global signed char	raydium_ode_network_distant_create;
__global signed char	raydium_ode_network_next_local_only;
__global signed char	raydium_ode_network_explosion_create;
__global int		raydium_ode_network_maxfreq;
__global void *		raydium_ode_ExplosionCallback; // void f(signed char,dReal,dReal,dReal *)
__global void *		raydium_ode_BeforeElementDrawCallback;
__global void *		raydium_ode_AfterElementDrawCallback;
__global signed char	raydium_ode_element_delete_LOCK;

typedef struct raydium_ode_Explosion // used for "blowing" explosions
{
    int   id;
    char  name[RAYDIUM_MAX_NAME_LEN];
    signed char  state;
    dReal config_radius; // desired radius
    dReal config_propag; // propagation speed (increment to config_radius)
    dReal radius; // current radius
    int   element;
    dReal position[3];
} raydium_ode_Explosion;

typedef struct raydium_ode_Joint
{
    int      id;
    char     name[RAYDIUM_MAX_NAME_LEN];
    signed char     state;
    int      mesh; // not used for joints, yet
    signed char     hinge2correct; // this hinge2 needs to be re-aligned each timestep
    signed char     motored; // powered by a motor ?
    dReal    motorforce;
    dReal    breakableforce; // is breakable ? (max force without braking)
    signed char     breaking; // is currently breaking (used as a tag for OnDelete)
    dJointID joint;
    void *   OnDelete; // pointer to OnDelete user callback
} raydium_ode_Joint;

typedef struct raydium_ode_Motor
{
    int   id;
    char  name[RAYDIUM_MAX_NAME_LEN];
    signed char  state;
    int   object; // owner
    int   joints[RAYDIUM_ODE_MOTOR_MAX_JOINTS];      // attached to ... (joints)
    int	  joints_axe[RAYDIUM_ODE_MOTOR_MAX_JOINTS];  // wich axe ? (joint relative)
    int   rocket_element; // rocket only: attached to this element
    dReal rocket_direction[3]; // rocket only (relative to element)
    dReal rocket_orientation[3]; // rocket only (relative to element)
    dReal rocket_position[3]; // rocket only (relative to element)
    signed char  rocket_playermovement; // this rocket is used for a FPS player
    dReal speed; // "engine style" motor: desired speed
    dReal angle; // "angular style" motor: desired angle
    dReal force; // all styles (engine, angular and rocket)
    dReal gears[RAYDIUM_ODE_MOTOR_MAX_GEARS]; // gear box factors
    int   gear; // current gear
    int   gear_max; // max gear
} raydium_ode_Motor;

typedef struct raydium_ode_ElementInternalSave
{
    char  name[RAYDIUM_MAX_NAME_LEN];
    int	  type; // ODE geom type
    dReal sphere_radius;
    dReal box_sizes[3];
    dReal mass;
    int   object;
    int   mesh;
    dReal slip;
    dReal cfm;
    dReal erp;
    dReal rel_pos[3];
    dReal rel_rot[4];
    void *user_data;
    int   user_tag;
    void *OnBlow;
    void *OnDelete;
} raydium_ode_ElementInternalSave;


// Warning: there's a sample of this in doc. Sync when you change something here.
typedef struct raydium_ode_Ray
{
    signed char state; // is this ray active ?
    dGeomID geom;
    //signed char visible; // use "drawing debug" to display the ray
    dReal   rel_dir[3];
    // farest contact
    dReal   max_dist;
    int     max_elem;
    dReal   max_pos[3];
    // nearest contact
    dReal   min_dist;
    int     min_elem;
    dReal   min_pos[3];    
} raydium_ode_Ray;

typedef struct raydium_ode_Element
{
    int     id;
    char    name[RAYDIUM_MAX_NAME_LEN];
    signed char    state;
    int     object;
    int     mesh;
    signed char    _touched;	// touched during very last timestep
    signed char    _movesfrom;	// is leaving this object to "object" member
    signed char    _avoidedcol;	// is any collision was avoided because of this move ?
    signed char    isplayer;	// is this element an FPS player ? ("standing elem")
    dReal   playerangle; // FPS player angle
    dReal   slip;	// slipping factor
    dReal   rotfriction; // rotation friction factor (avoid infinite rolling spheres)
    dGeomID geom;
    dBodyID body;
    dReal   erp;
    dReal   cfm;    
    void *  user_data; // point to user data
    int	    user_tag; // tag reseverd to user (this tag is networked)
    raydium_ode_ElementInternalSave *fixed_elements[RAYDIUM_ODE_ELEMENT_MAX_FIXING];
    int	    nid; //  network ID
    signed char    distant; // owned by a distant machine
    int     distant_owner;
    time_t  lastnetupdate;
    void *  OnBlow; // when touched by a blowing explosion void (*f)(int,dReal,dReal)
    void *  OnDelete; // int (*f)(int)
    int	    ttl; // time to live, -1 for infinite (default)
    int	    particle;
    dReal   particle_offset[3];
    unsigned long net_last_time;
    dReal         net_last_pos1[3];
    dReal         net_last_pos2[3];
    dReal         netvel[3]; // test
    unsigned long net_last_interval;
    int		  ground_texture;
    signed char	  marked_as_deleted;
    raydium_ode_Ray ray;
} raydium_ode_Element;


typedef struct raydium_ode_Object
{
    int      id;
    char     name[RAYDIUM_MAX_NAME_LEN];
    signed char     state;
    signed char     colliding; // elements of this objet can collide each other
    dSpaceID group;
//    dGeomID group;
} raydium_ode_Object;


typedef struct raydium_ode_network_Event
{
    int nid;
    dReal pos[3];
    dReal rot[4];
    dReal vel[3]; // test
} raydium_ode_network_Event;

typedef struct raydium_ode_network_Explosion
{
    signed char type; // RAYDIUM_ODE_NETWORK_EXPLOSION_* (EXPL or BLOW)
    dReal pos[3];
    dReal radius;
    dReal force; // BLOW only
    dReal propag; // EXPL only
} raydium_ode_network_Explosion;


__global raydium_ode_Object    raydium_ode_object[RAYDIUM_ODE_MAX_OBJECTS];
__global raydium_ode_Element   raydium_ode_element[RAYDIUM_ODE_MAX_ELEMENTS+1]; // the last element is used for "static joint"
__global raydium_ode_Joint     raydium_ode_joint[RAYDIUM_ODE_MAX_JOINTS];
__global raydium_ode_Motor     raydium_ode_motor[RAYDIUM_ODE_MAX_MOTORS];
__global raydium_ode_Explosion raydium_ode_explosion[RAYDIUM_ODE_MAX_EXPLOSIONS];

/*
void raydium_ode_callback(void);
int  raydium_ode_object_create(char *);
void raydium_ode_motor_rocket_orientation(int m, dReal rx, dReal ry, dReal rz);
int  raydium_ode_object_find(char *name);
char raydium_ode_element_material(int e, dReal erp, dReal cfm);
char raydium_ode_element_slip(int e, dReal slip);
char raydium_ode_object_colliding(int o, char colliding);
char raydium_ode_element_delete(int e, char deletejoints);
void raydium_ode_element_move(int elem, dReal *pos);
dReal *raydium_ode_element_pos_get(int j);
*/

void raydium_ode_network_init(void);
void raydium_ode_network_element_delete(int e);
void raydium_ode_network_element_new(int e);
void raydium_ode_network_read(void);
int  raydium_network_nid_element_find(int nid);
int  raydium_ode_network_MaxElementsPerPacket(void);
void raydium_ode_network_explosion_send(raydium_ode_network_Explosion *exp);
void raydium_ode_network_element_trajectory_correct(int elem);

// (#ifdef RAY_ODE_H)
#endif

