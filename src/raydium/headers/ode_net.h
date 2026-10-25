#ifndef _ODE_NET_H
#define _ODE_NET_H

/*=
RayODE network layer
4000
**/

// Introduction
/**
Physics engines are extremely powerful tools, but it turns to nightmares when
the application must be networked. RayODE API provides its own network layer,
using Raydium lower level network API. And the great thing is that you've
almost anything to do !
Just choose the best "send" function and let Raydium do the rest.

RayODE Net will use udp streams, netcall (RPC), smart timeouts, predictions, 
dead reckoning, and many others voodoo things. Just trust.

A few things about internals:
- NID: Network ID. Every networked element have a NID.
- Distant elements are localy created using static elements, owned by
an object called "##DISTANT##".
- ##raydium_ode_network_maxfreq## defines the paquet sending frequency. By
default, this value is ##RAYDIUM_ODE_NETWORK_MAXFREQ##, but you can use
##--ode-rate## command line switch.
- No rotation prediction is done.
- See ##config.h## if you want to disable prediction (##ODE_PREDICTION##) or
to debug RayODE Net (##DEBUG_ODENET##, **very** verbose !).
- Explosions are also automatically managed by RayODE Net.
- **Do NOT use** Raydium lower level network API when using RayODE Net. Use 
netcalls, propags and so on.

Nothing is said here about how to create a RayODE Net server. There's only
a few more things to do if you already have a standard server, but since it's 
unsupported for now, you must have a look to existing RayODE Net servers.
**/

__rayapi int raydium_ode_network_MaxElementsPerPacket (void);
/**
This function will return how many elements may be sent with
current packet size (see ##common.h##).
**/

__rayapi int raydium_network_nid_element_find (int nid);
/**
Internal. Find wich element have ##nid##.
**/

__rayapi void raydium_ode_network_newdel_event (int type, char *buff);
/**
Internal. NEWDEL netcall event.
NEWDEL is fired when a new element is created or deleted somewhere in the
network.
**/

__rayapi void raydium_ode_network_nidwho_event (int type, char *buff);
/**
Internal. NIDWHO netcall event.
NIDWHO is sent when someone received some "update" informations about a
nid, but didn't received previous NEWDEL informations for this nid.
The nid owner will send a reply.

Most reasons for this are:
- We are a new client and we dont known anything about the whole scene.
- The NEWDEL packet was lost ("TCP style" packets may be lost too ...)

NIDWHO answer will be used by every peer to refresh its own copy of the
element informations (geometry type, mesh, size and tag).
**/

__rayapi void raydium_ode_network_explosion_event (int type, char *buff);
/**
Internal explosion netcall event.(##RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL## and
##RAYDIUM_ODE_NETWORK_EXPLOSION_BLOW##).
**/

__rayapi void raydium_ode_network_init (void);
/**
Internal. Will initialize all RayODE Net layer and register netcalls.
**/

__rayapi signed char raydium_ode_network_TimeToSend (void);
/**
Almost internal. Will return 1 (true) if it's time to send a new packet, using
##raydium_ode_network_maxfreq## value.
**/

__rayapi void raydium_ode_network_element_send (short nelems, int *e);
/**
Will send all elements of ##e## array to network. You must provide array lenght 
using ##nelems##.
To "time to send ?" test is done, you'll probably have to do it yourself.
**/

__rayapi void raydium_ode_network_element_send_all (void);
/**
Will try to send all elements to network. Warning, packet size may be to
small to send all elements !..
**/

__rayapi void raydium_ode_network_element_send_random (int nelems);
/**
Will send randomly chosen elements to network. You must provide how many 
elements you want with ##nelems##, but RAYDIUM_ODE_NETWORK_OPTIMAL is
available.
**/

__rayapi void raydium_ode_network_element_send_iterative (int nelems);
/**
Will send elements to network, iteratively chose. You must provide how many 
elements you want with ##nelems##, but RAYDIUM_ODE_NETWORK_OPTIMAL is
available.
**/

__rayapi void raydium_ode_network_nidwho (int nid);
/**
Internal. Will ask for informations for ##nid## (see above).
NID sending frequency is now limited, since a lot of overhead was generated
when new clients were joining a "big" network.
**/

__rayapi void raydium_ode_network_apply (raydium_ode_network_Event * ev);
/**
Internal. This callback is fired when new data is received. A lot of things
are done here (timeouts, dead reckoning, ...)
**/

__rayapi void raydium_ode_network_read (void);
/**
Internal. Reads new packets, if any.
**/

__rayapi void raydium_ode_network_element_new (int e);
/**
Internal. Send a new element to network.
**/

__rayapi void raydium_ode_network_element_delete (int e);
/**
Internal. Send "delete event" to network, since we're deleting one of "our" elements.
**/

__rayapi void raydium_ode_network_explosion_send (raydium_ode_network_Explosion * exp);
/**
Internal. Send a new explosion event.
**/

__rayapi signed char raydium_ode_network_element_isdistant (int elem);
/**
Will return true (1) if element ##elem## is "distant", or false (0) if it's
one of "our" elements.
**/

__rayapi signed char raydium_ode_network_element_isdistant_name (char *elem);
/**
Same as above, but using element's name.
**/

__rayapi signed char raydium_ode_network_element_distantowner(int elem);
/**
Returns UID (peer "user" ID) for the distant element owner. See ##network.c##
documentation for more informations about UID.
**/

__rayapi signed char raydium_ode_network_element_distantowner_name(char *elem);
/**
Same as above, but using element's name.
**/

__rayapi void raydium_ode_network_element_trajectory_correct (int elem);
/**
Internal. Applies dead reckoning values to element.
**/

__rayapi void raydium_ode_network_elment_next_local(void);
/**
Call this function when you don't want that the next created element is sent
to network ("local only" element).
**/

#endif
