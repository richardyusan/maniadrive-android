#ifndef _ODE_H
#define _ODE_H
#include "../ode.h"

/*=
Integrated Physics (ODE)
3900
**/

// Introduction
/**
Raydium allows you to build applications with full physics, using ODE (Open
Dynamics Engine). ODE is "an open source, high performance library for 
simulating rigid body dynamics", and is fully integrated into Raydium, with
the usual abstraction. You can build cars, ragdolls, rockets, ... with
only few lines of code. Physics are linked to sound API, particles engine,
network layer, ... so you've almost nothing else to do but setting up objects.

Raydium's website provides tutorials for building physics ready applications.
**/

// Vocabulary
/**
Raydium physics use a simple vocabulary, with a few entities :
- **Objects:**
Objects are containers, with no direct visual appearance. An object contains
elements and joints (see below). By default, all elements in an object
doesn't collide each others. "Car", "Player", "Crane" are good object examples.

- **Elements:**
Elements are the main thing you will play with. An element is rendered using
an associated 3D mesh, is configured with a geometry, density, a size, 
collides with others elements, ...
An element **must** be owned by an object.
For now, there is 3 element types (standard, satic, fixing). Static elements
are unmovable, they just collide with other elements, usefull for very
big elements, or externally controlled elements (motion capture, network, 
haptic interface, ...), for example.
Raydium supports boxes and spheres.

- **Joints:**
Joints are dedicated to elements linking. A joint **must** be linked with two
elements or unwanted behaviors may happen.
For now, Raydium supports 4 joint types (hinge, hinge2, universal, fixed), and 
you will find more informations with suitable functions documentation, below.
On a joint, you could setup limits (min and max for all axes) and a maximum
force before joint breaks, if needed.
It's now possible to attach a joint to static environnement using the
constant ##RAYDIUM_ODE_JOINT_FIXED## (do not use this value with "_name" joint
functions, since they want a string, not a integer constant).

- **Motors:**
A motor is linked to joints, and may powering an unlimited amount of joints.
For now, 3 motor types are available: engine, angular and rocket.

**Engine** type works the same way as a car's engine: it will try to 
make "something" turning, at the desired **speed**. You can link a 
gearbox to this type (and only this one).

**Angular** type will try to rotate "something" to the desired **angle**,
usefull for car's front wheels, for example.

**Rocket type** is very simple: give a force and an orientation. Usefull for
creating copters, rockets, and for elements "pushing", for example. 
Special rocket is avaiblable for FPS style player controls.
Warning, a rocket is linked to an element ! (not a joint)

- **Explosions:**
Explosions are not directly related to rigid body physics, but consider it
as a high level primitive. 
With Raydium, you have two different ways to create an explosion.

First, you can create a "blowing explosion", generating a spherical blow. Any
element in this growing sphere will be ejected. 
Use this for very consequent explosions only !

Next, you can create an instantaneous explosion, with a degressive blowing
effect. A force is applied to every body found inside the blowing radius,
proportional to distance from the explosion's center. Usefull for smaller
explosions.

- **Launchers:**
Launchers are not real entities, but "only" tools. Obviously, they are
allowing you to launch an element (you must provice force and orientation)
from another element (relatively). More informations about launchers below.

**/

// Callbacks
/**
For **advanced** uses, you may want to enter into some "internal" parts of 
RayODE. Many callbacks are available for such needs.
To cancel any callback, set its value to ##NULL## (default value).
Here is a quick list:

- ##raydium_ode_StepCallback##
This callback is fired before every ODE callback. Since physcis callback
frequency may change (see ##raydium_ode_time_change##) during slow motion
scenes, for example, this callback is quiet useful.
Callback prototype: ##void f(void);##


- ##raydium_ode_ObjectNearCollide##
When two objects are too near, before lauching "expensive" collision tests,
Raydium is firing this event.

Callback prototype: ##signed char f(int obj1, int obj2);##
##obj1## and ##obj2## are the two objets, and you must return true (1) if
you want to "validate" collision, or false (0) if you don't want that two
objects to collide.


- ##raydium_ode_CollideCallback## 
When two objects collides, Raydium will search all collisions between
every elements. For each contact, this callback is fired. For complex
objects, with a lot of elements, this callback may be fired a **very** large
number of times during **one** ODE step ! Do only simple things here.

Callback prototype: ##signed char f(int e1, int e2, dContact *c);##
##e1## and ##e2## are the two colliding elements, and you must return true (1) 
if you want to "validate" this contact, or false (0) to cancel this contact 
(and only this one !)

See ODE documentation, chapter 7.3.7, for more informations about ##dContact##
structure.


- ##raydium_ode_ExplosionCallback## 
At every explosion, of any type, this event is fired. This is the best
place to play suitable sound, create particles and such things.

Callback prototype: ##void f(signed char type, dReal radius, dReal force_or_propag, dReal *pos);##
You can find in callback params:
explosion ##type## (see above), ##radius##, force or propag (depending on
explosion type) and ##pos##, an array of 3 dReal values for explosion position. 
The value you will find in ##force_or_propag## is something 
like ##RAYDIUM_ODE_NETWORK_EXPLOSION_*## (EXPL or BLOW).


- ##raydium_ode_BeforeElementDrawCallback##
When ##raydium_ode_draw_all(RAYDIUM_ODE_DRAW_NORMAL)## is called, for every 
element to draw, this callback is **before** element drawing.

Callback prototype: ##signed char f(int elem);##
##elem## is the element'id. Return true (1) if you want to draw this element,
or false (0) otherwise. This is also the best place to drawn team colors on 
cars, for example (see ##raydium_rendering_rgb_force## for this use).


- ##raydium_ode_AfterElementDrawCallback##
Same as the previous callback, but **after** element drawing.

Callback prototype: ##void f(int elem);##
With the previous example (team colors), this is the place to restore
default rendering state (see ##raydium_rendering_rgb_normal##).

- ##raydium_ode_RayCallback##
See ray related functions, below. This callback is used to filter
elements during a ray launch.
**/


// Miscallenous
/**
By default, ODE is called 400 times per second, allowing **very** accurate
physics. You may change this in ##ode.h## with ##RAYDIUM_ODE_PHYSICS_FREQ## and
##RAYDIUM_ODE_TIMESTEP##, but most ERP and CFM values must be changed in your
applications. ODE use a lot of cache mechanisms, so 400 Hz is a reasonable value.

Please note RayODE interface is using ##dReal## ODE type for variables. 
For now, ##dReal## is an alias to ##float## type. But please use ##sizeof()##.

Raydium provides some other functions for advanced uses, and you can
access directly to ODE API for very experienced users.

See also the ODE documentation: http://opende.sourceforge.net/ode-latest-userguide.html
**/

__rayapi void raydium_ode_name_auto (char *prefix, char *dest);
/**
This function will generate a single name, using ##prefix##. The generated
name is stored at ##dest## address. No memory allocation is done.
Example : ##raydium_ode_name_auto("prefix",str)## may generate something
like ##prefix_ode_0##.
**/

__rayapi void raydium_ode_init_object (int i);
/**
Will initialize (or erase) object ##i##. Mostly for internal uses.
**/

__rayapi void raydium_ode_init_element (int i);
/**
Will initialize (or erase) element ##i##. Mostly for internal uses.
**/

__rayapi void raydium_ode_init_joint (int i);
/**
Will initialize (or erase) joint ##i##. Mostly for internal uses.
**/

__rayapi void raydium_ode_init_motor (int i);
/**
Will initialize (or erase) motor ##i##. Mostly for internal uses.
**/

__rayapi void raydium_ode_init_explosion (int e);
/**
Will initialize (or erase) spherical explosiion ##i##. Mostly for internal uses.
**/

__rayapi void raydium_ode_init (void);
/**
Will initialize all RayODE interface. Never call this function by yourself.
**/

__rayapi signed char raydium_ode_object_isvalid (int i);
/**
Will return 0 (false) if object ##i## is not valid (free slot or out of bounds)
or 1 (true) otherwise.
**/

__rayapi signed char raydium_ode_element_isvalid (int i);
/**
Will return 0 (false) if element ##i## is not valid (free slot or out of bounds)
or 1 (true) otherwise.
**/

__rayapi signed char raydium_ode_joint_isvalid (int i);
/**
Will return 0 (false) if joint ##i## is not valid (free slot or out of bounds)
or 1 (true) otherwise.
**/

__rayapi signed char raydium_ode_motor_isvalid (int i);
/**
Will return 0 (false) if motor ##i## is not valid (free slot or out of bounds)
or 1 (true) otherwise.
**/

__rayapi signed char raydium_ode_explosion_isvalid (int i);
/**
Will return 0 (false) if explosion ##i## is not valid (free slot or out of bounds)
or 1 (true) otherwise.
**/

__rayapi void raydium_ode_ground_dTriArrayCallback (dGeomID TriMesh, dGeomID RefObject, const int *TriIndices, int TriCount);
/**
Internal. Unsupported.
**/

__rayapi int raydium_ode_ground_dTriCallback (dGeomID TriMesh, dGeomID RefObject, int TriangleIndex);
/**
Internal. Unsupported.
**/

__rayapi void raydium_ode_ground_set_name (char *name);
/**
##ground## is a primitive for RayODE interface. You only have to set ground
mesh ##name## (.tri file). You may call this function many times, if needed,
switching from one ground to another on the fly.
**Warning**: triangle normals are very important for ground models !
**/

__rayapi int raydium_ode_object_find (char *name);
/**
Resolves object id from its ##name##.
**/

__rayapi int raydium_ode_element_find (char *name);
/**
Resolves element id from its ##name##.
**/

__rayapi int raydium_ode_joint_find (char *name);
/**
Resolves joint id from its ##name##.
**/

__rayapi int raydium_ode_motor_find (char *name);
/**
Resolves motor id from its ##name##.
**/

__rayapi int raydium_ode_explosion_find (char *name);
/**
Resolves explosion id from its ##name##.
**/

__rayapi int raydium_ode_object_create (char *name);
/**
Will build a new object with ##name##. Returns new object id, or -1 when
it fails.
**/

__rayapi signed char raydium_ode_object_rename (int o, char *newname);
/**
Will rename object ##o## with a ##newname##.
**/

__rayapi signed char raydium_ode_object_rename_name (char *o, char *newname);
/**
Same as above, but from object's name (##o##).
**/

__rayapi signed char raydium_ode_object_colliding (int o, signed char colliding);
/**
By default, all elements from an object are not colliding each others.
The only exception is for ##GLOBAL## object.
If you want to change this behaviour for ##o## object, sets ##colliding##
to 1 (true). 0 (false) sets back to default behaviour (no internal collisions).
**/

__rayapi signed char raydium_ode_object_colliding_name (char *o, signed char colliding);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_object_linearvelocity_set (int o, dReal * vect);
/**
Sets linear velocity for all elements of object ##o##. Velocity is sets thru
##vect##, a 3 x dReal array.
Use with caution, setting an arbitrary linear velocity may cause unwanted
behaviours.
**/

__rayapi void raydium_ode_object_linearvelocity_set_name (char *o, dReal * vect);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_object_linearvelocity_set_name_3f (char *o, dReal vx, dReal vy, dReal vz);
/**
Same as above, but using 3 dReal values.
**/

__rayapi void raydium_ode_object_addforce (int o, dReal * vect);
/**
Add force ##vect## to all elements of object ##o##.
Force is sets thru ##vect##, a 3 x dReal array.
Prefer this method to ##..._linearvelocity_set...## functions.
**/

__rayapi void raydium_ode_object_addforce_name (char *o, dReal * vect);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_object_addforce_name_3f (char *o, dReal vx, dReal vy, dReal vz);
/**
Same as above, but using 3 dReal values.
**/

__rayapi void raydium_ode_element_addforce (int e, dReal * vect);
/**
Adds force ##vect## to element ##e##.
Force is sets thru ##vect##, a 3 x dReal array.
**/

__rayapi void raydium_ode_element_addforce_name (char *e, dReal * vect);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_addforce_name_3f (char *e, dReal vx, dReal vy, dReal vz);
/**
Same as above, but using 3 dReal values.
**/

__rayapi void raydium_ode_element_addtorque (int e, dReal * vect);
/**
Adds torque ##vect## to element ##e##.
Torque is sets thru ##vect##, a 3 x dReal array.
**/

__rayapi void raydium_ode_element_addtorque_name (char *e, dReal * vect);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_addtorque_name_3f (char *e, dReal vx, dReal vy, dReal vz);
/**
Same as above, but using 3 dReal values.
**/

__rayapi signed char raydium_ode_element_material (int e, dReal erp, dReal cfm);
/**
When two elements collides, there's two important parameters used for
contact point generation : ERP and CFM.
ERP means "Error Reduction Parameter", and its value is between 0 and 1 and 
CFM means "Constraint Force Mixing".
Changing ERP and CFM values will change the way this element collides with
other elements, providing a "material" notion.
Raydium provides a few default values, see ##RAYDIUM_ODE_MATERIAL_*## defines 
in ##raydium/ode.h## file (hard, medium, soft, soft2, default, ...).

For more informations, see ODE documentation, chapters 3.7 and 3.8.
**/

__rayapi signed char raydium_ode_element_material_name (char *name, dReal erp, dReal cfm);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_slip (int e, dReal slip);
/**
Slip parameter is a complement of material values (ERP and CFM, see above).
Raydium provides a few default values, see ##RAYDIUM_ODE_SLIP_*## defines 
in ##raydium/ode.h## file (ice, player, normal, default).
**/

__rayapi signed char raydium_ode_element_slip_name (char *e, dReal slip);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_rotfriction (int e, dReal rotfriction);
/**
rotfriction stands for "Rotation Friction", "Rolling Friction", 
"Damping Effect", ...
With RayODE, by default, when a sphere element is rolling over a flat ground,
it will roll forever. Applying a rotfriction factor will solve this.
A value of 0 will disable rotation friction.
Example: %%(c)
#define ROTFRICTION     0.0005
raydium_ode_element_rotfriction(elem,ROTFRICTION);%%
**/

__rayapi signed char raydium_ode_element_rotfriction_name (char *e, dReal rotfriction);
/**
Same as above, but using element's name.
**/

__rayapi dReal *raydium_ode_element_linearvelocity_get (int e);
/**
Returns a pointer to element's linear velocity. Linear velocity is an
array of 3 x dReal.
Example: %%(c)
dReal *p;
p=raydium_ode_element_linearvelocity_get(elem);
raydium_log("%f %f %f",p[0],p[1],p[2]);
%%
Returned data is available only for the current frame.
**/

__rayapi dReal *raydium_ode_element_linearvelocity_get_name (char *e);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_OnBlow (int e, void *OnBlow);
/**
During an instantaneous explosion, all elements inside the blow's radius may
fire an OnBlow callback (event), if set.
##OnBlow## callback must follow this prototype :
##void blow_touched(int elem, dReal force, dReal max_force)##

##elem## is the element id.
##force## is the amount of force received from explosion.
##max_force## is the amount of force at the core of the explosion.

Sets ##OnBlow## to ##NULL## if you want to disable this callback.
**/

__rayapi void raydium_ode_element_OnBlow_name (char *e, void *OnBlow);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_OnDelete (int e, void *OnDelete);
/**
OnDelete callback is fired when someone or something tries to delete an element.
This callback can cancel deletion, if needed.

##OnBlow## callback must follow this prototype :
##int element_delete(int elem)##
##elem## is the element id. Return 1 (true) to confirm deletion, of 0 to cancel.

Sets ##OnDelete## to ##NULL## if you want to disable this callback.
**/

__rayapi void raydium_ode_element_OnDelete_name (char *e, void *OnDelete);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_gravity (int e, signed char enable);
/**
By default, gravity applies to every element of the scene. If you want
to disable gravity for element ##e##, set ##enable## to 0 (false).
You can restore gravity with ##enable## sets to 1 (true).
**/

__rayapi void raydium_ode_element_gravity_name (char *e, signed char enable);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_ttl_set (int e, int ttl);
/**
TTL means Time To Live. Setting a TTL on an element will automatically
delete this element when TTL expires.

- TTL unit: ##ttl## is given in ODE steps (see example, below).
- TTL deletion may be canceled by OnDelete callback (see above).
- TTL may be changed on the fly, at anytime.
- a ##ttl## value of -1 will disable TTL.

example: %%(c)
raydium_ode_element_ttl_set(elem,RAYDIUM_ODE_PHYSICS_FREQ*5); // 5 seconds
%%
**/

__rayapi void raydium_ode_element_ttl_set_name (char *e, int ttl);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_aabb_get (int element, dReal * aabb);
/**
AABB means Axis-Aligned Bounding Box. This function will return element's
bounding box on X, Y and Z axis.

##aabb## is a pointer to an array of 6 x dReal.
No memory allocation is done.
Will return 0 (false) in case of failure.
**/

__rayapi signed char raydium_ode_element_aabb_get_name (char *element, dReal * aabb);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_element_touched_get (int e);
/**
Every element provide a "touched" flag. If element ##e## is touching anything,
this function will return 1 (true).
**/

__rayapi int raydium_ode_element_touched_get_name (char *e);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_player_set (int e, signed char isplayer);
/**
RayODE provides a special behaviour for FPS style players, also
named "standing geoms". The idea is simple : a player element is always
upright, and you can set an arbitrary rotation angle around Z axis anytime.
Sets ##isplayer## to 1 (true) to transform element ##e## into a "player element".
**/

__rayapi signed char raydium_ode_element_player_set_name (char *name, signed char isplayer);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_player_get (int e);
/**
Returns if element ##e## is a "player element" (1, true) or not (0, false).
See above for more informations about player elements.
**/

__rayapi signed char raydium_ode_element_player_get_name (char *name);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_player_angle (int e, dReal angle);
/**
Sets "standing geom" Z rotation ##angle## (radian) for element ##e##.
See above for more informations about player elements.
**/

__rayapi signed char raydium_ode_element_player_angle_name (char *e, dReal angle);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_element_ground_texture_get (int e);
/**
Unsupported. Do not use for now.
**/

__rayapi int raydium_ode_element_ground_texture_get_name (char *e);
/**
Unsupported. Do not use for now.
**/

__rayapi int raydium_ode_element_object_get (int e);
/**
Since every element is owned by an object, this function will return
the owner's object id.
**/

__rayapi int raydium_ode_element_object_get_name (char *e);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_object_sphere_add (char *name, int group, dReal mass, dReal radius, signed char type, int tag, char *mesh);
/**
This function will add an new "sphere" element to an object (##group##).
You must provide:
- ##name##: **single** name for this new element.
- ##group##: owner object id.
- ##mass##: density of this new element. Mass will depend on radius.
- ##radius##: radius of the element sphere geometry. Raydium is able to
detect this value with ##RAYDIUM_ODE_AUTODETECT##. Things like 
##RAYDIUM_ODE_AUTODETECT*2## are ok, meaning "twice the detected radius".
- ##type##: ##RAYDIUM_ODE_STANDARD## or ##RAYDIUM_ODE_STATIC## (collide only,
no physics).
- ##tag##: use this integer value as you want. The important thing is that
this value is sent to network, and will be available on every connected peer.
This tag must be greater or equal to 0. Suitable functions are available
to read back this value later on an element.
- ##mesh##: 3D model used for rendering this element. Use an empty string to
disable rendering (and not ##NULL## !), and avoid ##RAYDIUM_ODE_AUTODETECT##
int this case.
**/

__rayapi int raydium_ode_object_box_add (char *name, int group, dReal mass, dReal tx, dReal ty, dReal tz, signed char type, int tag, char *mesh);
/**
This function will add an new "box" element to an object (##group##).
Arguments are the same as ##raydium_ode_object_sphere_add## (see above) but
##tx##, ##ty## and ##tz##, used for box sizes. As for spheres, you can
use ##RAYDIUM_ODE_AUTODETECT##. Give this value only for ##tx##, this will
automatically apply to ##ty## and ##tz##.
Again, Things like  ##RAYDIUM_ODE_AUTODETECT*2## are ok, meaning 
"twice the detected size".
**/

__rayapi signed char raydium_ode_element_ray_attach(int element, dReal length, dReal dirx, dReal diry, dReal dirz);
/**
This function will attach a ray to ##element##. This may be used as a
sensor, "hitscan" line, intersection test, ...
Then you can get from this ray things like distance between the start
of the ray (element's center) and the first "touched" element. You will also 
find wich element was touched, and where. The same applies for the last touched
element.
Do not try to retrieve informations until next frame.

You must provide ray's length (the ray won't detect "things" over that point),
and direction vector (relative to element).

Since you can't set more than one ray per element, there's no problem with
calling this function twice or more, it will simply override previous settings
for length and direction.

Warning, ray are linked to GLOBAL object, so they will detect EVERY element,
even if owned by the same object ! (only ##element## is never reported).

For now, a ray will never generate contact point for touched object, you
must consider them as "phantom" elements, only looking at the current world
without modifying it. If you need this feature, ask for it ;)

If you want to filter wich elements are used to generate rays'informations,
you can use ##raydium_ode_RayCallback##. This callback is following the
same prototype as ##raydium_ode_CollideCallback## (see at the top of
this chapter). Return 0 if you don't want this "contact" for ray informations,
or 1 if you want normal behaviour.
**/

__rayapi signed char raydium_ode_element_ray_attach_name(char *element, dReal length, dReal dirx, dReal diry, dReal dirz);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_ray_delete(int element);
/**
Delete ray from ##element##. No more ray "reports" will be available after
this call.
**/

__rayapi signed char raydium_ode_element_ray_delete_name(char *element);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_ray_get(int element, raydium_ode_Ray *result);
/**
This function allows you to retrieve informations about ray.

Here you are a sample of ##raydium_ode_Ray## structure with
interesting fields:
%%(c)
typedef struct raydium_ode_Ray
{
    signed char state; // is this ray active ?
    dReal   rel_dir[3];
    // farest contact
    dReal   max_dist;
    int     max_elem;   // touched element, -1 if no element was touched
    dReal   max_pos[3];
    // nearest contact
    dReal   min_dist;
    int     min_elem;   // touched element, -1 if no element was touched
    dReal   min_pos[3];
} raydium_ode_Ray;						
%%

Obviously, this function won't allocate any memory, you must provided a
valid pointer.
**/

__rayapi signed char raydium_ode_element_ray_get_name(char *element, raydium_ode_Ray *result);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_element_fix (char *name, int *elem, int nelems, signed char keepgeoms);
/**
Experimental code.

The idea here is to create a bounding single element for a group of elements.
You must provide:
- ##name##: the new bounding element's name.
- ##elems##: an array of all elements to fix (id array).
- ##nelems##: the number of elements in ##elems## array.
- ##keepgeoms##: set to 0.

You can only fix standard elements (no statics) and all elements must be
owned by the same object.
**/

__rayapi void raydium_ode_element_unfix (int e);
/**
Experimental code. Unimplemented, yet.
Symmetric function, see ##raydium_ode_element_fix##.
**/

__rayapi void raydium_ode_element_mass(int elem, dReal mass);
/**
Change ##mass## the the ##elem## element. Obviously, you can't change the
mass of static element.
**/

__rayapi void raydium_ode_element_mass_name(char *elem, dReal mass);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_move (int elem, dReal * pos);
/**
This function will move element ##elem## to ##pos##.
##pos## is a dReal array of 3 values (x,y,z).
Warning: arbitrary moves may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_element_move_name (char *name, dReal * pos);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_move_3f(int elem, dReal x,dReal y, dReal z);
/**
Same as ##raydium_ode_element_move##, but using 3 dReal values.
**/


__rayapi void raydium_ode_element_move_name_3f (char *name, dReal x, dReal y, dReal z);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_rotate (int elem, dReal * rot);
/**
This function will rotate element ##elem## using ##rot##.
##rot## is a dReal array of 3 values (rx,ry,rz), in radians.
Warning: arbitrary rotations may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_element_rotate_3f (int elem, dReal rx, dReal ry, dReal rz);
/**
Same as ##raydium_ode_element_rotate##, but using 3 dReal values.
**/

__rayapi void raydium_ode_element_rotate_name (char *name, dReal * rot);
/**
Same as ##raydium_ode_element_rotate##, but using element's name.
**/

__rayapi void raydium_ode_element_rotateq (int elem, dReal * rot);
/**
This function will rotate element ##elem## using ##rot## quaternion.
##rot## is a dReal array of 4 values.
Warning: arbitrary rotations may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_element_rotateq_name (char *name, dReal * rot);
/**
Same as ##raydium_ode_element_rotateq##, but using element's name.
**/

__rayapi void raydium_ode_element_rotate_name_3f (char *name, dReal rx, dReal ry, dReal rz);
/**
Same as ##raydium_ode_element_rotate_name##, but using 3 dReal values.
**/

__rayapi void raydium_ode_object_rotate(int obj, dReal *rot);
/**
This function will try to rotate object ##obj##.
For now, rotation is done around the last element of the object.
##rot## is a dReal array of 3 values (rx,ry,rz), in radians.
Warning: arbitrary rotations may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_object_rotate_name(char *obj, dReal *rot);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_object_rotate_name_3f(char *obj, dReal rx, dReal ry, dReal rz);
/**
Same as above, but using 3 dReal values.
**/

__rayapi void raydium_ode_object_move (int obj, dReal * pos);
/**
This function will move object ##obj## to ##pos##.
Obviously, every element of object will be moved.
##pos## is a dReal array of 3 values (x,y,z).
Warning: arbitrary moves may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_object_move_name (char *name, dReal * pos);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_object_move_name_3f (char *name, dReal x, dReal y, dReal z);
/**
Same as above, but using 3 dReal values.
**/

__rayapi void raydium_ode_object_rotateq (int obj, dReal * rot);
/**
This function will try to rotate object ##obj## using ##rot## quaternion.
For now, rotation is done around the last element of the object.
##rot## is a dReal array of 4 values.
Warning: arbitrary rotations may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_object_rotateq_name (char *obj, dReal * rot);
/**
Same as above, but using object's name.
**/

__rayapi void raydium_ode_element_rotate_direction (int elem, signed char Force0OrVel1);
/**
This function will rotate element ##elem## from its force or velocity.
Sets ##Force0OrVel1## to ##0## if you want to align element using its
force or ##1## using its linear velocity.
Warning: arbitrary rotations may lead to unwanted behaviours.
**/

__rayapi void raydium_ode_element_rotate_direction_name (char *e, signed char Force0OrVel1);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_data_set (int e, void *data);
/**
You may want to link some data to any element. If so, use this function
and provide a pointer to your ##data## for element ##e##.
**/

__rayapi void raydium_ode_element_data_set_name (char *e, void *data);
/**
Same as above, but using element's name.
**/

__rayapi void *raydium_ode_element_data_get (int e);
/**
This function will return a pointer to your linked data, if any (see above).
**/

__rayapi void *raydium_ode_element_data_get_name (char *e);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_element_tag_get (int e);
/**
When you create a new element, you must provide a "tag". This function
allows you to get back the tag's value, even on "distant" elements.
**/

__rayapi int raydium_ode_element_tag_get_name (char *e);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_joint_suspension (int j, dReal erp, dReal cfm);
/**
ERP means "Error Reduction Parameter", and its value is between 0 and 1 and 
CFM means "Constraint Force Mixing".
Changing ERP and CFM values will change joint energy absorption and restitution.

For more informations, see ODE documentation, chapters 3.7 and 3.8.
**/

__rayapi void raydium_ode_joint_suspension_name (char *j, dReal erp, dReal cfm);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_joint_attach_hinge2 (char *name, int elem1, int elem2, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z);
/**
Will create a new joint between two elements (##elem1## and ##elem2##).
Hinge2 is a very specialized joint, perfect for car wheel's for example.

[[http://ode.org/pix/hinge2.jpg hinge2]]

"Axis 1 is specified relative to body 1 (this would be the steering 
axis if body 1 is the chassis). Axis 2 is specified relative to body 2 
(this would be the wheel axis if body 2 is the wheel)."

You must also provide joint's ##name##.

Raydium provides ##RAYDIUM_ODE_JOINT_SUSP_DEFAULT_AXES## define, useful for 
most chassis-wheel joints, and ##RAYDIUM_ODE_JOINT_AXE_X##, Y and Z for
other cases.

You cannot attach a static element.
**/

__rayapi int raydium_ode_joint_attach_hinge2_name (char *name, char *elem1, char *elem2, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z);
/**
Same as above, but using elements's names.
**/

__rayapi int raydium_ode_joint_attach_universal (char *name, int elem1, int elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z);
/**
Will create a new joint between two elements (##elem1## and ##elem2##).

[[http://ode.org/pix/universal.jpg universal]]

"Given axis 1 on body 1, and axis 2 on body 2 that is perpendicular to 
axis 1, it keeps them perpendicular. In other words, rotation of the two 
bodies about the direction perpendicular to the two axes will be equal."

"Axis 1 and axis 2 should be perpendicular to each other."

You must also provide joint's ##name##, and joint position (##posx##, ##posy##,
##posz##) in world coordinates.

Raydium provides ##RAYDIUM_ODE_JOINT_AXE_X##, ##RAYDIUM_ODE_JOINT_AXE_Y##
and ##RAYDIUM_ODE_JOINT_AXE_Z## defines, that may help.

You cannot attach a static element.
**/

__rayapi int raydium_ode_joint_attach_universal_name (char *name, char *elem1, char *elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z, dReal axe2x, dReal axe2y, dReal axe2z);
/**
Same as above, but using elements's names.
**/

__rayapi int raydium_ode_joint_attach_hinge (char *name, int elem1, int elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z);
/**
Will create a new joint between two elements (##elem1## and ##elem2##).

[[http://ode.org/pix/hinge.jpg hinge]]

You must provide joint's ##name##, and joint position (##posx##, ##posy##,
##posz##) in world coordinates.

Raydium provides ##RAYDIUM_ODE_JOINT_AXE_X##, ##RAYDIUM_ODE_JOINT_AXE_Y##
and ##RAYDIUM_ODE_JOINT_AXE_Z## defines, that may help for setting axis.

You cannot attach a static element.
**/

__rayapi int raydium_ode_joint_attach_hinge_name (char *name, char *elem1, char *elem2, dReal posx, dReal posy, dReal posz, dReal axe1x, dReal axe1y, dReal axe1z);
/**
Same as above, but using elements's names.
**/

__rayapi int raydium_ode_joint_attach_fixed (char *name, int elem1, int elem2);
/**
Will create a new joint between two elements (##elem1## and ##elem2##).

Fixed joint is more a hack than a real joint. Use only when it's absolutely 
necessary, and have a look to ##raydium_ode_element_fix##.

You must provide joint's ##name##
You cannot attach a static element.
**/

__rayapi int raydium_ode_joint_attach_fixed_name (char *name, char *elem1, char *elem2);
/**
Same as above, but using elements's names.
**/

__rayapi void raydium_ode_joint_hinge_limits (int j, dReal lo, dReal hi);
/**
Sets low (##lo##) and high (##hi##) limits for hinge joint ##j##.
**/

__rayapi void raydium_ode_joint_hinge_limits_name (char *j, dReal lo, dReal hi);
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_joint_universal_limits (int j, dReal lo1, dReal hi1, dReal lo2, dReal hi2);
/**
Sets low and hight limits for axe 1 (##lo1##, ##hi1##) and axe 2 (##lo2##, 
##hi2##) for universal joint ##j##. See ##raydium_ode_joint_attach_universal##
for more informations about universal joint axes.
**/

__rayapi void raydium_ode_joint_universal_limits_name (char *j, dReal lo1, dReal hi1, dReal lo2, dReal hi2);
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_joint_hinge2_block (int j, signed char block);
/**
Sometime, you may need to block rotation for first axe of hinge2 joints, for
example with rear wheels of a car. If so, set ##block## to 1 (true).
Setting ##block## back to 0 (false) will restore standard rotation behaviour.
**/

__rayapi void raydium_ode_joint_hinge2_block_name (char *name, signed char block);
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_joint_delete_callback (int j, void (*f) (int));
/**
Since joints may break (see ##raydium_ode_joint_break_force##), it may be
useful to get a callback on joint deletion.
This callback must this prototype:
##void joint_delete(int jid)##
##jid## is the deleted joint id. You can't cancel joint deletion (yet).
**/

__rayapi void raydium_ode_joint_delete_callback_name (char *name, void (*f) (int));
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_joint_break_force (int j, dReal maxforce);
/**
Setting a non-zero ##maxforce## on a joint will transform this joint into
a "breakable joint". There's no unit for ##maxforce##, you'll probably have
to find the suitable value empirically.
**/

__rayapi void raydium_ode_joint_break_force_name (char *name, dReal maxforce);
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_joint_elements_get (int j, int *e1, int *e2);
/**
Will return elements (##e1## and ##e2##) linked to joint ##j##.
**/

__rayapi void raydium_ode_joint_elements_get_name (char *j, int *e1, int *e2);
/**
Same as above, but using joint's name.
**/

__rayapi void raydium_ode_motor_update_joints_data_internal (int j);
/**
Internal function.
**/

__rayapi void raydium_ode_motor_speed (int j, dReal force);
/**
Sets motor ##j## speed parameter. This is only suitable for "engine"
and "rocket" type motors. There's no special unit for ##force##.
**/

__rayapi void raydium_ode_motor_speed_name (char *name, dReal force);
/**
Same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_power_max (int j, dReal power);
/**
Sets motor ##j## max power parameter. This is only suitable for "engine"
and "angular" motors. There's no special unit for ##power##.
**/

__rayapi void raydium_ode_motor_power_max_name (char *name, dReal power);
/**
Same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_angle (int j, dReal angle);
/**
Sets motor ##j## angle parameter. This is only suitable for "angular" motors.
##angle## has the units of radians.
**/

__rayapi void raydium_ode_motor_angle_name (char *motor, dReal angle);
/**
Same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_gears_set (int m, dReal * gears, int n_gears);
/**
Sets a gearbox for motor ##m## (only suitable for "engine" motors).
##gears## is an array of dReal values, with all gears factors).
##n_gears## is the array length (total number of gears for this gearbox).
example:
%%(c)
// rear,1,2,3,4,5
dReal gears[]={-0.4,0.4,0.6,0.8,0.9,1.0};
...
raydium_ode_motor_gears_set(main_engine,gears,6);
%%
If you want to cancel a gearbox, set a gearbox with only one gear with 1.0
factor value.

Raydium gearboxes implementation is very naive, with 100% output.
For example, a 0.5 gear factor will divide maximum speed by two, but will 
provide twice the normal torque.
**/

__rayapi void raydium_ode_motor_gears_set_name (char *m, dReal * gears, int n_gears);
/**
Same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_gear_change (int m, int gear);
/**
Switch motor ##m## to ##gear##.
**/

__rayapi void raydium_ode_motor_gear_change_name (char *m, int gear);
/**
Same as above, but using motor's name.
**/

__rayapi dReal *raydium_ode_element_pos_get (int j);
/**
This function will return element ##j##'s current position, as an array of
3 dReal values.
example:
%%(c)
dReal *pos;
dReal pos_copy;
...
pos=raydium_ode_element_pos_get(my_element);
raydium_log("%f %f %f",pos[0],pos[1],pos[2]);
memcpy(pos_copy,pos,sizeof(dReal)*3);
...
%%
Returned data is available only for the current frame.
**/

__rayapi dReal *raydium_ode_element_pos_get_name (char *name);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_rotq_get (int j, dReal * res);
/**
This function will return element ##j##'s current rotation, as an array of
4 dReal values (quaternion), thru ##res##.
No memory allocation will be done.
**/

__rayapi signed char raydium_ode_element_rotq_get_name (char *name, dReal * res);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_element_rot_get (int e, dReal * rx, dReal * ry, dReal * rz);
/**
This code is experimental. It should returns element ##e##'s current rotation
using 3 dReal angles, in radians. Do not apply back values to the
element since there're not "ODE formated".
**/

__rayapi signed char raydium_ode_element_rot_get_name (char *e, dReal * rx, dReal * ry, dReal * rz);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_sound_update (int e, int source);
/**
This function is a small bridge between RayODE and sound API, updating sound
##source## using element ##e##'s position.
**/

__rayapi void raydium_ode_element_sound_update_name (char *e, int source);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_element_RelPointPos (int e, dReal px, dReal py, dReal pz, dReal * res);
/**
Give a point (##px##, ##py## and ##pz##) on element ##e## to this function,
and il will return this point in global coordinates (##res##).
Returned data is available only for the current frame.
**/

__rayapi void raydium_ode_element_RelPointPos_name (char *e, dReal px, dReal py, dReal pz, dReal * res);
/**
Same as above, but using element's name.
**/

__rayapi int raydium_ode_motor_create (char *name, int obj, signed char type);
/**
This function will create a new motor, using ##name## (single), for
object ##obj##, with ##type##. As said before, available types are
##RAYDIUM_ODE_MOTOR_ENGINE##, ##RAYDIUM_ODE_MOTOR_ANGULAR## and
##RAYDIUM_ODE_MOTOR_ROCKET##. See the first part of this chapter for more
informations about motor types.
**/

__rayapi void raydium_ode_motor_attach (int motor, int joint, int joint_axe);
/**
This function will link ##motor## to ##joint##, on axe ##joint_axe## (first axe
is axe ##0## and so on ...). This is only suitable for engine and angular motors.
**/

__rayapi void raydium_ode_motor_attach_name (char *motor, char *joint, int joint_axe);
/**
Same as above, but using motor's name and joint's name.
**/

__rayapi dReal raydium_ode_motor_speed_get (int m, int gears);
/**
Will return current motor speed.
For engine style motors, if ##gears## is sets to 1 (true), returned speed
will be relative to current motor's gear. Useless for other types.
**/

__rayapi dReal raydium_ode_motor_speed_get_name (char *name, int gears);
/**
same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_rocket_set (int m, int element, dReal x, dReal y, dReal z);
/**
This function will configure rocket motor ##m## on ##element## at position
(##x##,##y##,##z##). Rocket motors are unusable until this function is called.
**/

__rayapi void raydium_ode_motor_rocket_set_name (char *motor, char *element, dReal x, dReal y, dReal z);
/**
same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_rocket_orientation (int m, dReal rx, dReal ry, dReal rz);
/**
This function will rotate rocket ##m## using ##rx##,##ry## and ##rz## angles
in degrees. Base orientation is z up.
**/

__rayapi void raydium_ode_motor_rocket_orientation_name (char *name, dReal rx, dReal ry, dReal rz);
/**
same as above, but using motor's name.
**/

__rayapi void raydium_ode_motor_rocket_playermovement (int m, signed char isplayermovement);
/**
Will configure rocket ##m## for player movements. This type of rocket will be
automatically disabled when linked element is not touched by 
anything (ground in most cases).
**/

__rayapi void raydium_ode_motor_rocket_playermovement_name (char *m, signed char isplayermovement);
/**
same as above, but using motor's name.
**/

__rayapi signed char raydium_ode_motor_delete (int e);
/**
Will obviously delete motor ##e##.
**/

__rayapi signed char raydium_ode_motor_delete_name (char *name);
/**
same as above, but using motor's name.
**/

__rayapi signed char raydium_ode_joint_delete (int joint);
/**
Will obviously delete ##joint##.
**/

__rayapi signed char raydium_ode_joint_delete_name (char *name);
/**
same as above, but using joint's name.
**/

__rayapi signed char raydium_ode_element_delete (int e, signed char deletejoints);
/**
Will obviously delete element ##e##. Deletion may me queued for some reason,
for a very short time (current collide loop). For now, you **must** set
##deletejoints## to 1 (true), since joints without 2 linked elements
are invalid.
Linked rocket engines will be deleted, too.
**/

__rayapi signed char raydium_ode_element_delete_name (char *name, signed char deletejoints);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_object_delete (int obj);
/**
Will obviously delete object ##obj##. All elements, joints and motors will
be deleted with object.
**/

__rayapi signed char raydium_ode_object_delete_name (char *name);
/**
Same as above, but using object's name.
**/

__rayapi signed char raydium_ode_explosion_delete (int e);
/**
Will delete ##RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL## type explosion ##e##.
**/

__rayapi signed char raydium_ode_element_moveto (int element, int object, signed char deletejoints);
/**
This function will move ##element## from his owner object to another ##object##.
This "migration" will not be completed until ##element## is not touching
anymore his previous owner.
For now, you **must** set ##deletejoints## to 1 (true), deleting linked joints.
**/

__rayapi signed char raydium_ode_element_moveto_name (char *element, char *object, signed char deletejoints);
/**
Same as above, but using element's name and object's name.
**/

__rayapi void raydium_ode_joint_break (int j);
/**
Internal joint testing function.
**/

__rayapi signed char raydium_ode_launcher (int element, int from_element, dReal * rot, dReal force);
/**
This function will launch an ##element## from ##from_element##.
You must provide ##rot##, an array of 3 dReal angles in degreees, relative
to ##from_element## current orientation.
You must also provide a ##force##, with no particular unit.
**/

__rayapi signed char raydium_ode_launcher_name (char *element, char *from_element, dReal * rot, dReal force);
/**
Same as above, using ##element## and ##from_element## names.
**/

__rayapi signed char raydium_ode_launcher_name_3f (char *element, char *from_element, dReal rx, dReal ry, dReal rz, dReal force);
/**
Same as above, but using 3 dReal values for rotation.
**/

__rayapi signed char raydium_ode_launcher_simple (int element, int from_element, dReal * lrot, dReal force);
/**
This function will act the same as previous ones, adding a few things:
- ##element## will be aligned with ##from_element## (position and rotation).
- ##element## will be "migrated" to GLOBAL object during launch.
**/

__rayapi signed char raydium_ode_launcher_simple_name (char *element, char *from_element, dReal * rot, dReal force);
/**
Same as above, using ##element## and ##from_element## names.
**/

__rayapi signed char raydium_ode_launcher_simple_name_3f (char *element, char *from_element, dReal rx, dReal ry, dReal rz, dReal force);
/**
Same as above, but using 3 dReal values for rotation.
**/

__rayapi void raydium_ode_explosion_blow (dReal radius, dReal max_force, dReal * pos);
/**
This function will create an instantaneous explosion, generating a degressive
blowing effect.
You must provide a ##radius## (normal world units), a maximum force 
(##max_force##), and a position (##pos##, 3 x dReal array).
**/

__rayapi void raydium_ode_explosion_blow_3f (dReal radius, dReal max_force, dReal px, dReal py, dReal pz);
/**
Same as above, but using 3 dReal values for position.
**/

__rayapi void raydium_ode_explosion_blow_rand(dReal radius, dReal max_force, dReal rand_factor, dReal *pos);
/**
Same as ##raydium_ode_explosion_blow_rand()##, but introducing a random factor
on resulting torque for blowed objects, for a more realistic effect.
**/

__rayapi void raydium_ode_explosion_blow_rand_3f(dReal radius, dReal max_force, dReal rand_factor, dReal px, dReal py, dReal pz);
/**
Same as above, but using 3 dReal values for position.
**/

__rayapi int raydium_ode_explosion_create (char *name, dReal final_radius, dReal propag, dReal * pos);
/**
This function will create an spherical growing explosion. Any element in the
explosion will be ejected.
As said before: "Use this for very consequent explosions only !".
You must provide ##final_radius##, ##propag## (growing size) and a 
position (##pos##, 3 x dReal array).
When an explosion reach its final radius, it will be deleted.
**/

__rayapi void raydium_ode_element_camera_inboard (int e, dReal px, dReal py, dReal pz, dReal lookx, dReal looky, dReal lookz);
/**
RayODE to camera API bridge.
Sets the camera on element ##e## at relative position (##px##,##py##,##pz##),
and looking at (##lookx##,##looky##,##lookz##) relative point.
**/

__rayapi void raydium_ode_element_camera_inboard_name (char *name, dReal px, dReal py, dReal pz, dReal lookx, dReal looky, dReal lookz);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_draw_all (signed char names);
/**
This function will draw all RayODE scene. You must call this function
by yourself.
Sets ##names## to ##RAYDIUM_ODE_DRAW_NORMAL## for normal rendering.
Other ##names## values will:
- draw only elements, joints and motors names and elements bounding boxes 
with ##RAYDIUM_ODE_DRAW_DEBUG##
- draw only objets AABB (Axis-Aligned Bounding Box) with ##RAYDIUM_ODE_DRAW_AABB##
- draw only element rays (if any) with ##RAYDIUM_ODE_DRAW_RAY##

... so you may need multiple call to this function each frame.
**/

__rayapi void raydium_ode_near_callback (void *data, dGeomID o1, dGeomID o2);
/**
Internal callback.
**/

__rayapi void raydium_ode_callback (void);
/**
Internal frame callback.
**/

__rayapi void raydium_ode_time_change (GLfloat perc);
/**
This function will change RayODE timecall frequency, allowing slow motion
effects, for example. This function will automatically adjust particle
engine time base.
##perc## is the percentage of the normal time base.
Since this function obviously do not change physics accuracy, be careful 
with ##perc## > 100, wich will generate a big load for the CPU.
This function also change particles and mesh animations time.
**/

__rayapi void raydium_ode_element_particle (int elem, char *filename);
/**
This function will "fix" a particle generator on element ##elem##. You must
provide particle generator's ##filename##.
**/

__rayapi void raydium_ode_element_particle_name (char *elem, char *filename);
/**
Same as above, using element's name.
**/

__rayapi void raydium_ode_element_particle_offset (int elem, char *filename, dReal * offset);
/**
Same as ##raydium_ode_element_particle##, but with an ##offset##, relative
to element. ##offset## is an array of 3 dReal values.
**/

__rayapi void raydium_ode_element_particle_offset_name (char *elem, char *filename, dReal * offset);
/**
Same as above, using element's name.
**/

__rayapi void raydium_ode_element_particle_offset_name_3f (char *elem, char *filename, dReal ox, dReal oy, dReal oz);
/**
Same as above, but using 3 dReal values for offset.
**/

__rayapi void raydium_ode_element_particle_point (int elem, char *filename);
/**
Same as ##raydium_ode_element_particle##, but generator will not be linked
with element, only positioned at current element's position.
**/

__rayapi void raydium_ode_element_particle_point_name (char *elem, char *filename);
/**
Same as above, using element's name.
**/

void raydium_ode_internal_particle_genetator_deleted_callback(int gen);
/**
Internal callback.
**/

__rayapi void raydium_camera_smooth_path_to_element (char *path, int element, GLfloat path_step, GLfloat smooth_step);
/**
This function is a clone of ##raydium_camera_smooth_path_to_pos## dedicated to
RayODE, looking at ##element## from path.
You may look at suitable chapter for more informations about ##path##,
##path_step## and ##smooth_step##.
**/

__rayapi void raydium_camera_smooth_path_to_element_name (char *path, char *element, GLfloat path_step, GLfloat smooth_step);
/**
Same as above, using element's name.
**/

__rayapi void raydium_camera_smooth_element_to_path_name (char *element, char *path, GLfloat path_step, GLfloat smooth_step);
/**
This function is a clone of ##raydium_camera_smooth_pos_to_path## dedicated to
RayODE, looking at path, from ##element##.
Here, you must provide element's name.
You may look at suitable chapter for more informations about ##path##,
##path_step## and ##smooth_step##.
**/

__rayapi void raydium_camera_smooth_element_to_path_offset (int element, GLfloat offset_x, GLfloat offset_y, GLfloat offset_z, char *path, GLfloat path_step, GLfloat smooth_step);
/**
This function is a clone of ##raydium_camera_smooth_pos_to_path## dedicated to
RayODE and providing an offset (for ##element##), looking at path, from
##element##.
You may look at suitable chapter for more informations about ##path##,
##path_step## and ##smooth_step##.
**/

__rayapi void raydium_camera_smooth_element_to_path_offset_name (char *element, GLfloat offset_x, GLfloat offset_y, GLfloat offset_z, char *path, GLfloat path_step, GLfloat smooth_step);
/**
Same as above, using element's name.
**/


__rayapi int raydium_ode_capture_3d(char *filename);
/**
This function is provided "for fun" only. The main idea is to dump all scene
to a .tri file (##filename##). A .sprt file will also be created, wich is a
special file format with all particles found during the dump. You can reload
.sprt files with ##raydium_particle_state_restore##.
Note from source code:
%%(c)
// This function is provided "for fun" only. Not all effects are dumped:
// Missing : shadows, forced colors, before/after callbacks, 
// fixed elements, ...
// Some code is pasted from file.c (and this is BAD ! :)
%%
**/

__rayapi int raydium_ode_orphans_check(void);
/**
Search orphans in all objects. An orphan is a geometry that exists into ODE
but is not managed by RayODE.
This function will print object with orphans and return total orphan count.
**/

#include "ode_net.h"
#endif
