#ifndef _NETWORK_H
#define _NETWORK_H
/*=
Network
2800
**/

// Bases of Raydium's networking API
/**
Raydium supports networking via UDP/IP, providing high level functions 
for multiplayer game development.
Raydium servers are limited to 256 clients for now.

You will find in network.c a set of "low level" functions and vars dedicated to 
networked games: players names, event callbacks, UDP sockets, 
broadcasts, ... 
See a few chapters below for higher level functions.

All this is ready to use. As it's not done in the introduction of this 
guide, We will explain here some variables defined in common.h.

%%(c)
#define RAYDIUM_NETWORK_PORT          29104
#define RAYDIUM_NETWORK_PACKET_SIZE   230
#define RAYDIUM_NETWORK_TIMEOUT       5
#define RAYDIUM_NETWORK_PACKET_OFFSET 4
#define RAYDIUM_NETWORK_MAX_CLIENTS   8
#define RAYDIUM_NETWORK_MODE_NONE     0
#define RAYDIUM_NETWORK_MODE_CLIENT   1
#define RAYDIUM_NETWORK_MODE_SERVER   2
%%

Here, we can find network port declaration (Raydium will use only one 
port, allowing easy port forwarding management, if needed), default timeout 
(unit: second), and the three mode possible for a Raydium application.

But there is also two other very important defines: packet size 
(unit: byte) and max number of clients.. This is important because 
Raydium uses UDP sockets, and UDP sockets required fixed 
length packets, and as you need to set packet size as small as possible 
(for obvious speed reasons), you must calculate you maximum 
information packet size (players position, for example), multiply 
it by ##RAYDIUM_NETWORK_MAX_CLIENTS##,and add ##RAYDIUM_NETWORK_PACKET_OFFSET##
wich represent the required header of the packet.

It's more easy than it seems, look:
//
My game will support 8 players.
I will send players state with 3 floats (x,y,z).
My packet size must be: 8*3*sizeof(float)+RAYDIUM_NETWORK_PACKET_OFFSET = 100 bytes.
//
Please, do not change packet offset size, since Raydium will use it
for packet header.

%%(c)
#define RAYDIUM_NETWORK_DATA_OK     1
#define RAYDIUM_NETWORK_DATA_NONE   0
#define RAYDIUM_NETWORK_DATA_ERROR -1
%%

This three defines are used as network functions result:

%%(c)
if(raydium_network_read_flushed(&id,&type,buff)==RAYDIUM_NETWORK_DATA_OK)
{
...
%%

%%(c) #define RAYDIUM_NETWORK_PACKET_BASE 20 %%

In most network functions, you will find a "type" argument, used to 
determine packet goal. This type is 8 bits long (256 possible values), 
but Raydium is already using some of them. So you can use 
##RAYDIUM_NETWORK_PACKET_BASE## as a base for your own types:

%%(c)
#define NORMAL_DATA RAYDIUM_NETWORK_PACKET_BASE
#define BALL_TAKEN (NORMAL_DATA+1)
#define SCORE_INFO (NORMAL_DATA+2)
#define HORN (NORMAL_DATA+3)
...
%%

===Variables:===

Your own player id (0<= id < RAYDIUM_NETWORK_MAX_CLIENTS),
read only: ##int raydium_network_uid;##
Special value "-1" means that you're not connected (see below).

Current network mode (none, client, server),
read only: ##signed char raydium_network_mode;##

Boolean used to determine client state (connected or not), read only:
##signed char raydium_network_client[RAYDIUM_NETWORK_MAX_CLIENTS];##

example: 
%%(c)
if(raydium_network_client[4])
    draw_player(4);
%%

Can be used by a server to send data to his clients. Read only:
##struct sockaddr raydium_network_client_addr[RAYDIUM_NETWORK_MAX_CLIENTS];##

Players names, read only:
##char raydium_network_name[RAYDIUM_NETWORK_MAX_CLIENTS][RAYDIUM_MAX_NAME_LEN];##

##OnConnect## and ##OnDisconnect## events (server only):
##void * raydium_network_on_connect;
void * raydium_network_on_disconnect;##

You can place your owns callbacks (##void(int)##) on these events, as in 
this example:

%%(c)
void new_client(int client)
{
raydium_log("New player: %s", raydium_network_nameclient);
}

...

int main(int argc, char **argv)
{
...
raydium_network_on_connect=new_client;
...
%%
**/

// Reliablility versus Speed
/**
As explained above, Raydium is using UDP network packets, and as 
you may know, UDP is not a reliable protocol, aiming speed before all.
This system is interesting for sending non-sensible data, as player positions,
for example.
But Raydium can handle more important data, using some of methods of TCP
protocol, as Timeouts, ACK, resending, ...
This TCP style packets are available thru "Netcalls".
**/

// High level API: "Netcalls" and "Propags"
/**
Netcalls provides you a good way to handle network exchanges using
callbacks functions, like a simple RPC system.
The idea is simple, built over the notion of "type". See suitable functions for
more information about this system.

Another available mechanism is called Propags, and allows you to "share"
variables over the network (scores, game state, ...) in a very few steps.
You only need to "create" a type, and link a variable to it (any C type or
structure is allowed). After each modification of this (local copy of the) 
variable, just call ##raydium_network_propag_refresh*## and that's it. If 
any other client (or the server) is applying a modification to this "type", 
your local copy is automatically updated.
**/

__rayapi int raydium_network_propag_find (int type);
/**
Lookups a "propag" by his ##type##. Returns -1 is no propag is found.
**/

__rayapi void raydium_network_propag_recv (int type, char *buff);
/**
Internal callback for "propag" receiving.
**/

__rayapi void raydium_network_propag_refresh_id (int i);
/**
Will refresh a propag by his ##id##.
**/

__rayapi void raydium_network_propag_refresh (int type);
/**
Will refresh a propag by his ##type##.
**/

__rayapi void raydium_network_propag_refresh_all (void);
/**
Will refresh all propags
**/

__rayapi int raydium_network_propag_add (int type, void *data, int size);
/**
This function will "register" a new propag. You need to provide the address
of your variable/structure (##data##), ans its ##size##. A dedicated ##type##
is also required (see at the top of this chapter).
**/

__rayapi void raydium_network_queue_element_init (raydium_network_Tcp * e);
/**
Internal use. (TCP style packets)
**/

__rayapi unsigned short raydium_network_queue_tcpid_gen (void);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_queue_tcpid_known_add (int tcpid, int player);
/**
Internal use. (TCP style packets)
**/

__rayapi signed char raydium_network_queue_tcpid_known (unsigned short tcpid, unsigned short player);
/**
Internal use. (TCP style packets)
**/

__rayapi signed char raydium_network_queue_is_tcpid (int type);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_queue_element_add (char *packet, struct sockaddr *to);
/**
Internal use. (TCP style packets)
**/

__rayapi unsigned long *raydium_network_internal_find_delay_addr (int player);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_queue_check_time (void);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_queue_ack_send (unsigned short tcpid, struct sockaddr *to);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_queue_ack_recv (int type, char *buff);
/**
Internal use. (TCP style packets)
**/

__rayapi void raydium_network_player_name (char *str);
/**
This function will returns the current player name.
Raydium will ask the OS for "current logged user", but player name may
be provided thru ##--name## command line argument.
**/

// internal hack, no doc provided
__rayapi signed char raydium_network_set_socket_block_internal(int socket, int block);

__rayapi signed char raydium_network_set_socket_block (int block);
/**
This function will sets ##block## (true or false) status to the network stack.
A blocking socket will wait indefinitely an incoming packet. A non blocking one
will return "no data" instead.
You've almost no reason to call this function by yourself.
**/

__rayapi int raydium_network_socket_close(int fd);
/**
Portable socket closing function. See "man 2 close" or closesocket (win32)
docs.
**/

__rayapi signed char raydium_network_socket_is_readable(int fd);
/**
Will return true (1) if there is some data ready on ##fd## socket,
false (0) otherwise.
**/

__rayapi signed char raydium_network_netcall_add (void *ptr, int type, signed char tcp);
/**
This function will register a new Network Callback ("netcall").
With Raydium, you can read the main data stream with 
##raydium_network_read_flushed()##, and configure netcalls on random 
events (using packet type).

Netcalls signature is: ##void(int type, char *buff)##

As you may configure the same callback function for multiples packet types,
this type is passed to your function, with the temporary ##buff## buffer.
You can extract from field from packet if needed.

If you sets the ##tcp## flag to true (1), your packet will use "TCP style"
network protocol (see a the top of this chapter).
**/

__rayapi void raydium_network_netcall_exec (int type, char *buff);
/**
Internal callback for "netcall" receiving.
**/

__rayapi signed char raydium_network_timeout_check (void);
/**
Internal use.
**/

__rayapi void raydium_network_init_sub(void);
/**
Internal use.
**/

__rayapi signed char raydium_network_init (void);
/**
Nothing interesting unless you're creating a console server (using the
##RAYDIUM_NETWORK_ONLY## directive), since in this case you must do all
inits by yourself...
example :
%%(c)
#define RAYDIUM_NETWORK_ONLY
#include "raydium/index.c"

...

int main(int argc, char **argv)
{
setbuf(stdout,NULL);
signal(SIGINT,quit);
raydium_php_init(); // only if you need PHP support
raydium_network_init();
raydium_network_server_create();
...
%%
**/

__rayapi void raydium_network_write (struct sockaddr *to, int from, signed char type, char *buff);
/**
Obviously, this function will send data.
If you're a client, you don't need to determine to field, as the only 
destination is the server, so you can use ##NULL##, for example. If you're 
a server, you can use ##raydium_network_client_addr[]## array.

As a client, ##from## argument is generally your own uid (##raydium_network_uid##),
but you can use any other player number if needed.
As a server, ##from## field is useless, since you are the only machine able 
to send data to clients.

As you may expect, ##type## field is used to determine packet's type.
You can use any (8 bits) value greater or equal to ##RAYDIUM_NETWORK_PACKET_BASE##.

Finally, ##buff## is a pointer to data's buffer. This buffer 
must be ##RAYDIUM_NETWORK_PACKET_SIZE## long, and can be cleared 
or re-used after this call.
**/

__rayapi void raydium_network_broadcast (signed char type, char *buff);
/**
Sends data over network.
Obviously, from network point of vue, only a server can broadcast 
(to his clients).

When a client needs to broadcast (from the game point of vue) some 
informations (his own position, for example), he must send this information
to server, and the server will broadcast it.

This function uses the same arguments as previous one, except ##to## and 
##from##, not needed here.
**/


__rayapi signed char raydium_network_read (int *id, signed char *type, char *buff);
/**
Reads next packet from network (FIFO) stack.
This function uses the same arguments as previous ones, and returns 
data availability: ##RAYDIUM_NETWORK_DATA_OK##, ##RAYDIUM_NETWORK_DATA_NONE##
or ##RAYDIUM_NETWORK_DATA_ERROR##.
**/

__rayapi signed char raydium_network_read_flushed (int *id, signed char *type, char *buff);
/**
Reads last packet from network stack.
All previous packets will be ignored, only the newest packet will
be read (if any).

As you may miss some important informations, you can use netcalls 
(see above) if you want to capture packets with a particular 
type, even with flushed reading.
**/

__rayapi void raydium_network_read_faked(void);
/**
Reads from network, but do not care of received data. This is useful for
listen to internal packets (server "beacon" broadcasts, for example).
Reading is done thru ##raydium_network_read_flushed##.
Mostly for internal use.
**/

__rayapi signed char raydium_network_server_broadcast(char *name, char *app_or_mod, int version);
/**
This function will start to broadcast a server to the LAN.
You must provide a party ##name##, the application or mod name (##app_or_mod##)
and a "protocol" version of you choice.
The server is going to broadcast a "beacon" packet  to the LAN 
every ##RAYDIUM_NETWORK_BEACON_DELAY##.
Any client in "discovery mode" with the same ##app_or_mod## and ##version##
will see this beacon.
**/

__rayapi void raydium_network_server_broadcast_info(char *info);
/**
Update "information" field of this server (current track or map, for example).
Size cannot exceed ##RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN##.
**/


__rayapi void raydium_network_server_broadcast_check(void);
/**
Internal use.
**/

__rayapi signed char raydium_network_server_create (void);
/**
Will transform you application into a server, accepting new clients 
instantaneously.
See also the ##RAYDIUM_NETWORK_ONLY## directive if you want to create console
servers.
**/

__rayapi signed char raydium_network_client_connect_to (char *server);
/**
This function will try to connect your application to ##server## (hostname or 
ip address).
WARNING: For now, this call could be endless ! (server failure while connecting).
This function will succed returning 1 or 0 otherwise.
You are connected instantaneously, and you must start sending data 
before server timeout (defined by ##RAYDIUM_NETWORK_TIMEOUT##).
You player number can be found with ##raydium_network_uid## variable, 
as said before.
**/

__rayapi signed char raydium_network_client_discover(char *game,int version);
/**
This function will set client in ##RAYDIUM_NETWORK_MODE_DISCOVER## mode.
While using this mode, a client will search every LAN server with the
same ##game## (or mod name) and ##version## as itself.
Then, you can access to this server list using [undocumented yet].
**/

__rayapi int raydium_network_discover_numservers(void);
/**
While the client is in ##RAYDIUM_NETWORK_MODE_DISCOVER## mode, you
can fetch all "detected" servers in the LAN.
This function will return :
 - -1 : "not in discovery mode". See ##raydium_network_client_discover()##.
 - 0 : no server detected (yet ... try during next frame)
 - more : total number of compatible servers (same game/application 
 and protocol version)
**/

__rayapi signed char raydium_network_discover_getserver(int num, char *name, char *ip, char *info, int *player_count, int *player_max);
/**
Use this function with the help of ##raydium_network_discover_numservers()##, 
with something like :
%%(c)
int i;
char name[RAYDIUM_MAX_NAME_LEN];
char ip[RAYDIUM_MAX_NAME_LEN];
char info[RAYDIUM_MAX_NAME_LEN];
int player_count;
int player_max;
...
for(i=0;i<raydium_network_discover_numservers();i++)
    {
    raydium_network_discover_getserver(i,name,ip,info,&player_count,&player_max);
    raydium_log("server %02i: %s (%s)",i,name,ip);
    }

No memory allocation is done for ##name## and ##ip##. It's your job.

This function will return :
 - -1 : "not in discovery mode". See ##raydium_network_client_discover()##.
 - 0 : invalid ##num##.
 - 1 : OK.
%%
**/

__rayapi void raydium_network_client_disconnect(void);
/**
This function will disconnect client from server, if connected.
**/

__rayapi signed char raydium_server_accept_new (struct sockaddr *from, char *name);
/**
Internal server callback for new clients.
**/

__rayapi void raydium_network_close (void);
/**
Obvious. Raydium will do it for you, anyway.
**/

__rayapi void raydium_network_internal_server_delays_dump (void);
/**
Dumps "TCP Style" timeouts for all clients to console.
**/

__rayapi void raydium_network_internal_dump (void);
/**
Dumps various stats about network stack to console.
**/

__rayapi signed char raydium_network_internet_test(void);
/**
This function will test if direct internet connection is available,
using Raydium webiste. This function supports proxies.
**/

#ifdef linux
__rayapi signed char raydium_network_linux_find_broadcast_interfaces(void);
/**
Internal use. Linux only.
**/
#endif

#endif
