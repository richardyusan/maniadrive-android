/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/network.h"
#endif

// Raydium low-level Network API.

// TODO:
// 1 - protection against duplicate data packets (how ?!)
// 01/07/2004: done ? (need more tests)
// 2 - per client delays for servers
// end of 2004: done too (more test needed)

// need proto
//char raydium_server_accept_new(struct sockaddr *from, char *name);
//void raydium_network_broadcast(char type,char *buff);
//void raydium_network_write(struct sockaddr *to, int from, char type,char *buff);
//char raydium_network_netcall_add(void *ptr, int type, char tcp);


int raydium_network_propag_find(int type)
{
int i;

for(i=0;i<RAYDIUM_NETWORK_MAX_PROPAGS;i++)
  if(raydium_network_propag[i].state && raydium_network_propag[i].type==type)
	return i;
return -1;
}


void raydium_network_propag_recv(int type, char *buff)
{
int dec;
unsigned int version;
int i;

i=raydium_network_propag_find(type);
if(i<0)
    {
    raydium_log("network: ERROR: received an invalid propag' type ! (%i)",type);
    return;
    }

dec=RAYDIUM_NETWORK_PACKET_OFFSET;
memcpy(&version,buff+dec,sizeof(int));
if(version>raydium_network_propag[i].version) // this propag is newer than our
    {
    dec+=sizeof(int);    
    raydium_network_propag[i].version=version;    
    memcpy(raydium_network_propag[i].data,buff+dec,raydium_network_propag[i].size);
    }
}


void raydium_network_propag_refresh_id(int i)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];
int dec;

if(raydium_network_mode==RAYDIUM_NETWORK_MODE_NONE) return;

if(i<0 || i>=RAYDIUM_NETWORK_MAX_PROPAGS || !raydium_network_propag[i].state)
    {
    raydium_log("network: ERROR: cannot refresh this propag': invalid id");
    return;
    }

raydium_network_propag[i].version++;
dec=RAYDIUM_NETWORK_PACKET_OFFSET;
memcpy(buff+dec,&raydium_network_propag[i].version,sizeof(int));
dec+=sizeof(int);
memcpy(buff+dec,raydium_network_propag[i].data,raydium_network_propag[i].size);
raydium_network_write(NULL,raydium_network_uid,raydium_network_propag[i].type,buff);
}


void raydium_network_propag_refresh(int type)
{
int i;

i=raydium_network_propag_find(type);
if(i<0)
    {
    raydium_log("network: ERROR: cannot refresh this propag': invalid type");
    return;
    }
raydium_network_propag_refresh_id(i);
}


void raydium_network_propag_refresh_all(void)
{
int i;

for(i=0;i<RAYDIUM_NETWORK_MAX_PROPAGS;i++)
  if(raydium_network_propag[i].state)
    raydium_network_propag_refresh_id(i);
}

int raydium_network_propag_add(int type, void *data, int size)
{
int i;

if(size>(RAYDIUM_NETWORK_PACKET_SIZE-RAYDIUM_NETWORK_PACKET_OFFSET-RAYDIUM_NETWORK_PROPAG_HEAD))
    {
    raydium_log("network: ERROR: propag' packet is too big for current network size");
    return -1;
    }

if(raydium_network_propag_find(type)>=0)
    {
    raydium_log("network: ERROR: propag' type already added !");
    return -1;
    }
    
for(i=0;i<RAYDIUM_NETWORK_MAX_PROPAGS;i++)
    if(!raydium_network_propag[i].state)
	{
	raydium_network_propag[i].state=1;
	raydium_network_propag[i].version=0;
	raydium_network_propag[i].type=type;
	raydium_network_propag[i].size=size;
	raydium_network_propag[i].data=data;
	raydium_network_netcall_add(raydium_network_propag_recv,type,1);
	return i;	
	}
raydium_log("network: ERROR: no more propag' slots !");
return -1;
}


void raydium_network_queue_element_init(raydium_network_Tcp *e)
{
e->state=0;
}


unsigned short raydium_network_queue_tcpid_gen(void)
{
// not very important.. used for easy client/server tcpid identification
//#ifdef RAYDIUM_NETWORK_ONLY_____
static unsigned short gen=100;
//#else
//static unsigned short gen=1000;
//#endif

gen++;
if(!gen) gen++;

return gen;
}


void raydium_network_queue_tcpid_known_add(int tcpid, int player)
{
	raydium_network_tcpid_i[raydium_network_tcpid_index]=tcpid; // mark this TCP-ID as "known"
	raydium_network_tcpid_p[raydium_network_tcpid_index]=player; // ... from this player
#ifdef DEBUG_NETWORK
	raydium_log("ACK adding tcpid=%i (player %i) to known packets",tcpid,player);
#endif	
	raydium_network_tcpid_index++;

	if(raydium_network_tcpid_index==RAYDIUM_NETWORK_TX_QUEUE_SIZE)
	    raydium_network_tcpid_index=0;
}

signed char raydium_network_queue_tcpid_known(unsigned short tcpid, unsigned short player)
{
int i;

if(!tcpid) return 0;

for(i=0;i<RAYDIUM_NETWORK_TX_QUEUE_SIZE;i++)
  if(raydium_network_tcpid_i[i]==tcpid &&
     raydium_network_tcpid_p[i]==player )
    return i;

return 0;
}

signed char raydium_network_queue_is_tcpid(int type)
{
int i=0;

if(type >= 0)
  for(i=0;i<RAYDIUM_NETWORK_MAX_NETCALLS;i++)
    if(raydium_network_netcall_type[i]==type && raydium_network_netcall_tcp[i])
	return 1;
return 0;
}


void raydium_network_queue_element_add(char *packet, struct sockaddr *to)
{
unsigned short tcpid;
raydium_network_Tcp *e;
memcpy(&tcpid,packet+2,sizeof(tcpid));


e=&raydium_network_queue[raydium_network_queue_index];

if(e->state)
    {
    raydium_network_queue_element_init(e);
    raydium_network_stat_lost++; // we're erasing an old waiting packet !
    }

e->state=1;
e->tcpid=tcpid;
memcpy(e->packet,packet,RAYDIUM_NETWORK_PACKET_SIZE);
e->time=raydium_timecall_clock();
e->retries_left=RAYDIUM_NETWORK_MAX_TRIES;
if(to) memcpy(&e->to,to,sizeof(struct sockaddr));
e->to_player=-1;

// if server mode, search for destination player
if(raydium_network_mode==RAYDIUM_NETWORK_MODE_SERVER)
    {
    int i;
    
    for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
	if(raydium_network_client[i] && &raydium_network_client_addr[i]==to)
	    break;
    
    if(i==RAYDIUM_NETWORK_MAX_CLIENTS) // not found
	{
	raydium_log("ERROR: server: TCP style: cannot find client");
	return;
	}
    e->to_player=i;
    }

raydium_network_queue_index++;
if(raydium_network_queue_index==RAYDIUM_NETWORK_TX_QUEUE_SIZE)
    raydium_network_queue_index=0;
}


unsigned long *raydium_network_internal_find_delay_addr(int player)
{
//if client
if(raydium_network_mode==RAYDIUM_NETWORK_MODE_CLIENT)
    return &raydium_netwok_queue_ack_delay_client;
else // (server)
    {
    if(player<0 || player>=RAYDIUM_NETWORK_MAX_CLIENTS)
	{
	raydium_log("ERROR: server: bad client id in resend queue ! SHOULD NEVER APPEND !");
	return NULL; // eeeerk !
	}
    return &raydium_netwok_queue_ack_delay_server[player];
    }
}


void raydium_network_queue_check_time(void)
{
unsigned long now;
unsigned long *delay;
raydium_network_Tcp *e;
int i;


// find current delay

for(i=0;i<RAYDIUM_NETWORK_TX_QUEUE_SIZE;i++)
  if(raydium_network_queue[i].state)
    {
    now=raydium_timecall_clock();
    e=&raydium_network_queue[i];

    delay=raydium_network_internal_find_delay_addr(e->to_player);
    
    // TCP style timeout: timeout=estimatedRTT*2
    if( now>(e->time+(*delay)*2) || e->time>now )
	{
	// resend packet (and do not add this packet to queue again !)
#ifdef DEBUG_NETWORK
	raydium_log("ACK re-asking: tcpid=%i",e->tcpid);
#endif
	raydium_network_write_notcp=1;
	raydium_network_write(&e->to,-1,e->packet[0],e->packet);
	raydium_network_stat_reemitted++;
	(*delay)*=2; // Karn/Partridge TCP algo
        if((*delay)/(double)raydium_timecall_clocks_per_sec>RAYDIUM_NETWORK_ACK_DELAY_MAX)
	    {
#ifdef DEBUG_NETWORK
	    raydium_log("ACK: slow network ! max ack delay reached");
#endif
	    (*delay)=raydium_timecall_clocks_per_sec*RAYDIUM_NETWORK_ACK_DELAY_MAX;
	    }
	e->retries_left--;
	e->time=now;
	if(e->retries_left==0) 
	    {
#ifdef DEBUG_NETWORK
	    raydium_log("ACK: packet lost, too many retries: tcpid=%i",e->tcpid);
#endif
	    raydium_network_queue_element_init(e);
	    raydium_network_stat_lost++;
	    }
	}
    }
}

void raydium_network_queue_ack_send(unsigned short tcpid, struct sockaddr *to)
{
char buff[RAYDIUM_NETWORK_PACKET_SIZE];
//unsigned short port;

memcpy(buff+RAYDIUM_NETWORK_PACKET_OFFSET,&tcpid,sizeof(tcpid));
raydium_network_write(to,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ACK,buff);

//memcpy(&port,&to->sa_data[0],sizeof(port));
//raydium_log("ACK ---> %i.%i.%i.%i:%i",to->sa_data[2],to->sa_data[3],to->sa_data[4],to->sa_data[5],ntohs(port));
}


void raydium_network_queue_ack_recv(int type,char *buff)
{
unsigned short tcpid;
int i;
raydium_network_Tcp *e;
unsigned long now;
// TCP style weight average params
const float a=0.85;
const float b=0.15;

memcpy(&tcpid,buff+RAYDIUM_NETWORK_PACKET_OFFSET,sizeof(tcpid));

for(i=0;i<RAYDIUM_NETWORK_TX_QUEUE_SIZE;i++)
    {
    e=&raydium_network_queue[i];
    if(e->state && e->tcpid==tcpid)
	{
	// ACK is correct, deleting packet from queue
#ifdef DEBUG_NETWORK
	raydium_log("ACK recv ok: tcpid=%i",tcpid);
#endif
	now=raydium_timecall_clock();
	if(e->time<now)
	    {
	    unsigned long *delay;
	    
	    delay=raydium_network_internal_find_delay_addr(e->to_player);
	    // Based on original TCP adaptative retransmission algorithm :
	    *(delay)=a * (*delay) + b*(now - e->time);
#ifdef DEBUG_NETWORK
	    raydium_log("ACK delay re-eval: %.2f msec (inst=%.2f msec) (client %i)",(*delay)/(double)raydium_timecall_clocks_per_sec*1000,(now - e->time)/(double)raydium_timecall_clocks_per_sec*1000,e->to_player);
#endif	
	    }
	raydium_network_queue_element_init(e);	    
	return;
	}    
    }
raydium_network_stat_bogus_ack++;
#ifdef DEBUG_NETWORK
raydium_log("ACK bogus (double ack, probably): tcpid=%i",tcpid);
#endif
}


// -----------------------------------------------------------------------


void raydium_network_player_name(char *str)
{
#ifndef WIN32
{
struct passwd *pn;
pn=getpwuid(geteuid()); // ("old" kernels ok ?)
strncpy(str,pn->pw_name,RAYDIUM_MAX_NAME_LEN-1);
}
#else
{
DWORD s;
s=RAYDIUM_MAX_NAME_LEN-1;
GetUserName(str,&s);
}
#endif

if(!strlen(str))
    gethostname(str,RAYDIUM_MAX_NAME_LEN-1);
}

signed char raydium_network_set_socket_block_internal(int socket, int block)
{
int ret=-1;
//int flags = fcntl(sock, F_GETFL); // bof... :)
#ifndef WIN32
if(!block) block=O_NONBLOCK; else block=0;
ret=fcntl(socket, F_SETFL, block);
#else
if(!block) block=1; else block=0;
ret = ioctlsocket(socket, FIONBIO, (unsigned long *)&block);
if(ret) ret=-1;
#endif

if(ret==-1)
    {
    raydium_log("ERROR ! network: cannot block/unblock socket");
    perror("System");
    return(0);
    }
return(1);
}

signed char raydium_network_set_socket_block(int block)
{
return raydium_network_set_socket_block_internal(raydium_network_socket,block);
}

//int raydium_network_get_socket_block(void)
//{
//return !( fcntl(raydium_network_socket, F_GETFL) & O_NONBLOCK );
//}

int raydium_network_socket_close(int fd)
{
#ifndef WIN32
return close(fd);
#else
return closesocket(fd);
#endif
}

// I've no idea why some version of Dev-CPP are losing this value ...
#ifndef FD_SETSIZE
#define FD_SETSIZE	64
#endif

signed char raydium_network_socket_is_readable(int fd)
{
fd_set readSet;
struct timeval timeVal;
int selectCount;

timeVal.tv_sec=0;
timeVal.tv_usec=0;


FD_ZERO(&readSet);
FD_SET(fd,&readSet);

selectCount=select(fd+1,&readSet,NULL,NULL,&timeVal);

if (selectCount<0)
    return 0; // failed
if (selectCount==0)
    return 0; // not readable
if (FD_ISSET(fd,&readSet))
    return 1; // readable

return 0; // ?!!
}

signed char raydium_network_netcall_add(void *ptr, int type, signed char tcp)
{
int i;

for(i=0;i<RAYDIUM_NETWORK_MAX_NETCALLS;i++)
 if(raydium_network_netcall_type[i]<0)
 {
  raydium_network_netcall_func[i]=ptr;
  raydium_network_netcall_type[i]=type;
  raydium_network_netcall_tcp[i]=tcp;
  break;
 }

if(i==RAYDIUM_NETWORK_MAX_NETCALLS)
 {
  raydium_log("network: ERROR: no more netcalls !");
  return 0;
 }

return 1;
}


void raydium_network_netcall_exec(int type,char *buff)
{
char tmpbuff[RAYDIUM_NETWORK_PACKET_SIZE];
int i;
void (*f)(int, char*);

if(type >= 0)
  for(i=0;i<RAYDIUM_NETWORK_MAX_NETCALLS;i++)
   if(raydium_network_netcall_type[i]==type)
   {
      memcpy(tmpbuff,buff,RAYDIUM_NETWORK_PACKET_SIZE);
      f=raydium_network_netcall_func[i];
      f(type,tmpbuff);
   }
}

signed char raydium_network_timeout_check(void)
{
char str[RAYDIUM_NETWORK_PACKET_SIZE];
int i,n;
time_t now;
void (*f)(int);

time(&now);

 if(raydium_network_mode==RAYDIUM_NETWORK_MODE_SERVER)
 {
  for(i=n=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    if(raydium_network_client[i] && now>raydium_network_keepalive[i]+RAYDIUM_NETWORK_TIMEOUT)
    {
    raydium_log("network: TIMEOUT for client %i (%i sec): %s deleted !",i,RAYDIUM_NETWORK_TIMEOUT,raydium_network_name[i]);
    raydium_network_client[i]=0;
    if(raydium_network_on_disconnect)
       {
       f=raydium_network_on_disconnect;
       f(i);
       }
    raydium_network_name[i][0]=0;
    str[RAYDIUM_NETWORK_PACKET_OFFSET]=i;
    str[RAYDIUM_NETWORK_PACKET_OFFSET+1]=0;
    raydium_network_broadcast(RAYDIUM_NETWORK_PACKET_INFO_NAME,str);
    n++;
    }
 return n;
 }
return 0;
}

    
void raydium_network_init_sub(void)
{
int i;

if(raydium_network_mode)
    raydium_network_socket_close(raydium_network_socket);
raydium_network_uid=-1;
raydium_network_mode=RAYDIUM_NETWORK_MODE_NONE;
raydium_network_socket=-1;

raydium_network_beacon[RAYDIUM_NETWORK_PACKET_OFFSET]=0;
raydium_network_beacon_search.active=0;

for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    {
    raydium_network_client[i]=0;
    raydium_network_name[i][0]=0;
    }

for(i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
    raydium_network_server_list[i].when=0;

}

signed char raydium_network_init(void)
{
int i;

#ifdef WIN32
int ret;
WSADATA WSAData;
ret = WSAStartup(MAKEWORD(1,1), &WSAData);
if (ret) { raydium_log("network: FAILED ! (Winsock 2 Error: %i while asking for version 1.1)",ret); return(0); }
#endif

raydium_network_init_sub();

for(i=0;i<RAYDIUM_NETWORK_MAX_NETCALLS;i++)
    {
    raydium_network_netcall_type[i]=-1;
    raydium_network_netcall_func[i]=0;
    raydium_network_netcall_tcp[i]=0;
    }

for(i=0;i<RAYDIUM_NETWORK_TX_QUEUE_SIZE;i++)
    raydium_network_queue_element_init(&raydium_network_queue[i]);

for(i=0;i<RAYDIUM_NETWORK_TX_QUEUE_SIZE;i++)
    {
    raydium_network_tcpid_i[i]=0;
    raydium_network_tcpid_p[i]=0;
    }

for(i=0;i<RAYDIUM_NETWORK_MAX_PROPAGS;i++)
    raydium_network_propag[i].state=0;

raydium_network_queue_index=0;
raydium_network_tcpid_index=0;

raydium_network_on_connect=raydium_network_on_disconnect=NULL;
raydium_network_stat_rx=0;
raydium_network_stat_tx=0;
raydium_network_stat_lost=0;
raydium_network_stat_double=0;
raydium_network_stat_reemitted=0;
raydium_network_stat_bogus_ack=0;
raydium_network_netcall_add(raydium_network_queue_ack_recv,RAYDIUM_NETWORK_PACKET_ACK,0);
raydium_netwok_queue_ack_delay_client=raydium_timecall_clocks_per_sec; // 1sec default delay

for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    raydium_netwok_queue_ack_delay_server[i]=raydium_timecall_clocks_per_sec; // 1sec default delay
    
raydium_network_write_notcp=0;

raydium_network_name_local[0]=0;
raydium_network_connected_server[0]=0;
if(raydium_init_cli_option("name",raydium_network_name_local))
{
    if(strlen(raydium_network_name_local)==0)
	raydium_log("Warning: network: --name option needs an argument");
}

if(strlen(raydium_network_name_local)==0)
    raydium_network_player_name(raydium_network_name_local);


raydium_log("network: OK");
return(1);
}

// if from < 0, "from" field is not modified (used for broadcasts, mainly)
void raydium_network_write(struct sockaddr *to, int from, signed char type,char *buff)
{
int ret=-1;
unsigned short tcpid=0;

buff[0]=type;

if(from>=0)
    buff[1]=(char)from; // 256 clients MAX for now.

if(raydium_network_write_notcp==0 && raydium_network_queue_is_tcpid(type))
    tcpid=raydium_network_queue_tcpid_gen();
    
if(raydium_network_write_notcp==0) // do not erase tcpid packet's element if it's a re-send
    memcpy(buff+2,&tcpid,sizeof(unsigned short));

raydium_network_write_notcp=0;

raydium_network_stat_tx+=RAYDIUM_NETWORK_PACKET_SIZE;
if(raydium_network_mode==RAYDIUM_NETWORK_MODE_CLIENT)
    ret=send(raydium_network_socket, buff, RAYDIUM_NETWORK_PACKET_SIZE, 0);

else if(raydium_network_mode==RAYDIUM_NETWORK_MODE_SERVER)
    ret=sendto(raydium_network_socket, buff, RAYDIUM_NETWORK_PACKET_SIZE, 0, to, sizeof(struct sockaddr));

raydium_network_timeout_check();
//raydium_log("send ret: %i",ret);
if(ret<0)
    {
    raydium_log("network: ERROR sending ! (%i)",ret);
    return;
    }

if(tcpid)
    {
    raydium_network_queue_element_add(buff,to);
#ifdef DEBUG_NETWORK
    raydium_log("ACK asking to peer: tcpid=%i type=%i",tcpid,type);
#endif
    }    
}

void raydium_network_broadcast(signed char type,char *buff)
{
int i;

for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
 if(raydium_network_client[i])
  raydium_network_write(&raydium_network_client_addr[i],-1,type,buff);
}

// buff could be modified, even if return is RAYDIUM_NETWORK_DATA_NONE
signed char raydium_network_read(int *id, signed char *type, char *buff)
{
int i;
struct sockaddr from;
socklen_t len;
int ret;
char dbl=0;
time_t now;

time(&now);


raydium_network_timeout_check();
raydium_network_queue_check_time();
raydium_network_server_broadcast_check();

// search for outdated server slots (should isolate this in a function ?)
for(i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
    if(raydium_network_server_list[i].when!=0)
	if(raydium_network_server_list[i].when+RAYDIUM_NETWORK_BEACON_DEFAULT_TTL<now)
	    raydium_network_server_list[i].when=0;

len=sizeof(struct sockaddr);
ret=recvfrom(raydium_network_socket,buff,RAYDIUM_NETWORK_PACKET_SIZE,0,&from,&len);

if(ret==RAYDIUM_NETWORK_PACKET_SIZE) 
 {
 unsigned short tcpid;
 
 *type=buff[0];
 *id=buff[1];
 raydium_network_stat_rx+=RAYDIUM_NETWORK_PACKET_SIZE;

 memcpy(&tcpid,buff+2,sizeof(unsigned short));

 if(tcpid)
    {
#ifdef DEBUG_NETWORK
    raydium_log("ACK read (and will send): tcpid=%i type=%i",tcpid,*type);
#endif
    if(raydium_network_queue_tcpid_known(tcpid,*id))
	{
	raydium_network_stat_double++;
	dbl=1;
#ifdef DEBUG_NETWORK
	raydium_log("ACK double: tcpid=%i type=%i",tcpid,*type);
#endif
	}
    /*else*/ raydium_network_queue_ack_send(tcpid,&from);
     raydium_network_queue_tcpid_known_add(tcpid,buff[1]);
    }
 
 if(dbl) // discard double packet
    return(RAYDIUM_NETWORK_DATA_NONE);

 // user must no see this type with netcalls !
 if(*type==RAYDIUM_NETWORK_PACKET_SERVER_BEACON)
    {
    if(raydium_network_mode==RAYDIUM_NETWORK_MODE_DISCOVER && 
       raydium_network_beacon_search.active)
	{	
	int id;
	int dec;
	int version;
	char *app_or_mod;
	char *name;
	char *info;
	int player_count;
	int player_max;
	int slot=-1;

	dec=RAYDIUM_NETWORK_PACKET_OFFSET;
	dec++; // 1st byte is useless for us (server side flag)

	memcpy(&id,buff+dec,sizeof(id));
	dec+=sizeof(id);

	// search id -> id already found -> update time -> return
	for(i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
	    if(raydium_network_server_list[i].when!=0)
		if(raydium_network_server_list[i].id==id)
		    {
		    //raydium_network_server_list[i].when=now;
		    //return(RAYDIUM_NETWORK_DATA_NONE);
		    slot=i;
		    break;
		    }


	memcpy(&version,buff+dec,sizeof(version));
	dec+=sizeof(version);

	app_or_mod=buff+dec;
	dec+=(strlen(app_or_mod)+1);
	
	// else -> id not found -> test game+version
	if(version != raydium_network_beacon_search.version ||
	   strcmp(app_or_mod,raydium_network_beacon_search.app_or_mod))
		return(RAYDIUM_NETWORK_DATA_NONE); // not for us ...

	name=buff+dec;	
	dec+=(strlen(name)+1);
	
	info=buff+dec;
	dec+=RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN;

	memcpy(&player_count,buff+dec,sizeof(player_count));
	dec+=sizeof(player_count);

	memcpy(&player_max,buff+dec,sizeof(player_max));
	dec+=sizeof(player_max);


	// true -> search free -> add server
	if(slot==-1)
	    for(i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
		if(raydium_network_server_list[i].when==0)
		    slot=i;

	if(slot<0)
	    {
	    raydium_log("network: discover: too much server in this LAN ! (max=%i)",RAYDIUM_NETWORK_MAX_SERVERS);
	    return(RAYDIUM_NETWORK_DATA_NONE);
	    }

	raydium_network_server_list[slot].id=id;
	raydium_network_server_list[slot].when=now;
	strcpy(raydium_network_server_list[slot].name,name);
	strcpy(raydium_network_server_list[slot].ip,inet_ntoa(((struct sockaddr_in *)(&from))->sin_addr));
	strcpy(raydium_network_server_list[slot].info,info);
	raydium_network_server_list[slot].player_count=player_count;
	raydium_network_server_list[slot].player_max=player_max;
	}
    return(RAYDIUM_NETWORK_DATA_NONE);
    }
 
 raydium_network_netcall_exec(*type,buff);
 
 if( raydium_network_mode==RAYDIUM_NETWORK_MODE_SERVER && (*id>=0) && (*id<RAYDIUM_NETWORK_MAX_CLIENTS) )
    time(&raydium_network_keepalive[(*id)]); // update keepalive
 
 if(*type==RAYDIUM_NETWORK_PACKET_REQUEST_UID && raydium_network_mode==RAYDIUM_NETWORK_MODE_SERVER)
    {
    raydium_server_accept_new(&from,buff+RAYDIUM_NETWORK_PACKET_OFFSET);
    return(RAYDIUM_NETWORK_DATA_NONE);
    }

 if(*type==RAYDIUM_NETWORK_PACKET_ACK)
    return(RAYDIUM_NETWORK_DATA_NONE);

 if(*type==RAYDIUM_NETWORK_PACKET_INFO_NAME && raydium_network_mode==RAYDIUM_NETWORK_MODE_CLIENT)
    {
    i=buff[RAYDIUM_NETWORK_PACKET_OFFSET];
    strcpy(raydium_network_name[i],buff+RAYDIUM_NETWORK_PACKET_OFFSET+1);
    raydium_log("network: client %i is %s",i,raydium_network_name[i]);
    if(strlen(raydium_network_name[i]))
	raydium_network_propag_refresh_all(); // spread propags to this new client
    return(RAYDIUM_NETWORK_DATA_NONE);
    }


 return(RAYDIUM_NETWORK_DATA_OK);
 }
else if(errno==EAGAIN) return(RAYDIUM_NETWORK_DATA_NONE); // POSIX
else { 
#ifdef WIN32
	ret=WSAGetLastError();
	if(ret==WSAEWOULDBLOCK) return(RAYDIUM_NETWORK_DATA_NONE); // NON POSIX (GRRrrr)
#else
//	perror("System");	
#endif
//	raydium_log("ERROR ! network: error receiving ! (%i)",ret);
	return(RAYDIUM_NETWORK_DATA_ERROR);
     }
}


signed char raydium_network_read_flushed(int *id, signed char *type, char *buff)
{
char ret,data=0;
char save_buff[RAYDIUM_NETWORK_PACKET_SIZE];
int save_id;
char save_type;

do
 {
 // erreur !: meme si le retour est NONE, buff peut avoir ete modifie ! 
 // (et donc, on le lit pas tt a fait le dernier paquet de donnes)
 // solution: sauver buff avant et le restauter au final ? (lent ?)
 // 13/04/2004: solution en question mise en place: en test, premier resultats
 // probants
 ret=raydium_network_read(id,type,buff);
 if(ret==RAYDIUM_NETWORK_DATA_OK) 
    {
    data++;
    memcpy(save_buff,buff,RAYDIUM_NETWORK_PACKET_SIZE);
    save_id=*id;
    save_type=*type;
    }
 }while(ret==RAYDIUM_NETWORK_DATA_OK);
 
if(data) 
    {
    memcpy(buff,save_buff,RAYDIUM_NETWORK_PACKET_SIZE);
    *id=save_id;
    *type=save_type;
    return RAYDIUM_NETWORK_DATA_OK;
    }
else return ret;
}

void raydium_network_read_faked(void)
{
int id;
signed char type;
char buff[RAYDIUM_NETWORK_PACKET_SIZE];

raydium_network_read_flushed(&id,&type,buff);
}

signed char raydium_network_server_broadcast(char *name, char *app_or_mod, int version)
{
int dec=0;
int id;
int player_count;
int player_max;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_SERVER)
    {
    raydium_log("network: ERROR: cannot set server broadcast attributes: not a server");
    return 0;
    }

dec=RAYDIUM_NETWORK_PACKET_OFFSET;

if( strlen(name)+strlen(app_or_mod)+dec+
    sizeof(version)+sizeof(id) +
    sizeof(player_count) + sizeof(player_max) +
    RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN >= 
    RAYDIUM_NETWORK_PACKET_SIZE-10)
    {
    raydium_log("network: ERROR: cannot set server attributes: packet's too small");
    return 0;   
    }    


player_count=0;
player_max=0;

id=rand();
dec=RAYDIUM_NETWORK_PACKET_OFFSET;
raydium_network_beacon[dec]=1;
dec++;
memcpy(raydium_network_beacon+dec,&id,sizeof(id)); // server id
dec+=sizeof(id);
memcpy(raydium_network_beacon+dec,&version,sizeof(version));
dec+=sizeof(version);
strcpy(raydium_network_beacon+dec,app_or_mod);
dec+=(strlen(app_or_mod)+1);
strcpy(raydium_network_beacon+dec,name);
dec+=(strlen(name)+1);
raydium_network_beacon_info_offset=dec;
raydium_network_beacon[dec]=0; // no info yet
dec+=RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN;
memcpy(raydium_network_beacon+dec,&player_count,sizeof(player_count));
dec+=sizeof(player_count);
memcpy(raydium_network_beacon+dec,&player_max,sizeof(player_max));
dec+=sizeof(player_max);

raydium_log("network: now broadcasting : '%s' (%s) version %i",name,app_or_mod,version);
return 1;
}


void raydium_network_server_broadcast_info(char *info)
{
if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_SERVER)
    {
    raydium_log("network: ERROR: cannot set server broadcast infos: not a server");
    return;
    }

if(strlen(info)<RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN-1)
    strcpy(raydium_network_beacon+raydium_network_beacon_info_offset,info);
else
    raydium_log("network: ERROR: cannot set server broadcast info: string's too long");    
}


void raydium_network_server_broadcast_check(void)
{
static time_t last=0;
time_t now;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_SERVER)
    return;

// no beacon is ready ? (see _server_broadcast())
if(!raydium_network_beacon[RAYDIUM_NETWORK_PACKET_OFFSET])
    return;

time(&now);
if(now>last+RAYDIUM_NETWORK_BEACON_DELAY)
    {
    int player_count;
    int player_max;
    int dec;
    int i;
#ifndef linux
    struct sockaddr_in sock;
#endif

    player_max=RAYDIUM_NETWORK_MAX_CLIENTS;
    player_count=0;
    for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
	if(raydium_network_client[i])
	    player_count++;

    dec=raydium_network_beacon_info_offset+RAYDIUM_NETWORK_BEACON_INFO_MAX_LEN;
    memcpy(raydium_network_beacon+dec,&player_count,sizeof(player_count));
    dec+=sizeof(player_count);
    memcpy(raydium_network_beacon+dec,&player_max,sizeof(player_max));
    dec+=sizeof(player_max);

#ifdef linux
    for(i=0;i<raydium_network_broadcast_interface_index;i++)
	    raydium_network_write((struct sockaddr *)&raydium_network_broadcast_interfaces[i],
	    255,RAYDIUM_NETWORK_PACKET_SERVER_BEACON,raydium_network_beacon);
#else
    sock.sin_family=AF_INET;
    sock.sin_addr.s_addr=htonl(INADDR_BROADCAST);
    sock.sin_port=htons(RAYDIUM_NETWORK_BEACON_PORT);
    raydium_network_write((struct sockaddr *)&sock,255,RAYDIUM_NETWORK_PACKET_SERVER_BEACON,raydium_network_beacon);
#endif
    last=now;
    }
}


signed char raydium_network_server_create(void)
{
struct sockaddr_in sock;
int on=1;
int ret;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_NONE)
    {
    raydium_log("network: ERROR: cannot create server : already connected");
    return 0;
    }

raydium_network_start=time(NULL);
raydium_network_socket=socket(AF_INET,RAYDIUM_NETWORK_UDP,0);
if(raydium_network_socket==-1)
    {
    raydium_log("ERROR ! network: cannot create server socket");
    perror("System");
    return(0);
    }
raydium_log("network: server socket created");

sock.sin_family=AF_INET;
sock.sin_addr.s_addr=htonl(INADDR_ANY);
sock.sin_port=htons(RAYDIUM_NETWORK_PORT);
ret=bind(raydium_network_socket,(struct sockaddr *)&sock,sizeof(sock));
if(ret)
    {
    raydium_log("ERROR ! network: cannot open server socket (already used ?)");
    perror("System");
    return(0);
    }

#ifdef linux
raydium_network_linux_find_broadcast_interfaces();
#endif

raydium_log("network: server OK: waiting for clients (%i max) at udp port %i",RAYDIUM_NETWORK_MAX_CLIENTS,RAYDIUM_NETWORK_PORT);
raydium_network_mode=RAYDIUM_NETWORK_MODE_SERVER;
setsockopt(raydium_network_socket,SOL_SOCKET,SO_BROADCAST,(char *)&on,sizeof(on));
raydium_network_set_socket_block(0);
return(1);
}


// will resolv "server" name
signed char raydium_network_client_connect_to(char *server)
{
struct sockaddr_in sock;
int ret,empty;
char str[RAYDIUM_NETWORK_PACKET_SIZE];
signed char type;
struct hostent *server_addr;
int on=1;

// should automaticaly stop discover mode here ...
if(raydium_network_mode==RAYDIUM_NETWORK_MODE_DISCOVER)
    {
    raydium_network_socket_close(raydium_network_socket);
    raydium_network_mode=RAYDIUM_NETWORK_MODE_NONE;
    }

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_NONE)
    {
    raydium_log("network: ERROR: cannot create client : already connected");
    return 0;
    }

raydium_network_start=time(NULL);
raydium_network_socket=socket(AF_INET,RAYDIUM_NETWORK_UDP,0);
if(raydium_network_socket==-1)
    {
    raydium_log("ERROR ! network: cannot create client socket");
    perror("System");
    return(0);
    }
raydium_log("network: client socket created");


server_addr = gethostbyname(server);
if(!server_addr) 
    {
    raydium_log("ERROR ! DNS/resolv error with \"%s\"",server);
    perror("System");
    return(0);
    }

//inet_pton(AF_INET,server,&sock.sin_addr);
memcpy((char*)(&(sock.sin_addr.s_addr)), server_addr->h_addr, server_addr->h_length);
sock.sin_family=AF_INET;
sock.sin_port=htons(RAYDIUM_NETWORK_PORT);

ret=connect(raydium_network_socket,(struct sockaddr *)&sock,sizeof(sock));
if(ret) 
    {
    raydium_log("ERROR ! local UDP socket error (contacting %s)",server);
    perror("System");
    return(0);
    }
raydium_log("network: connecting to %s and waiting UID...",server);
raydium_network_set_socket_block(1);
setsockopt(raydium_network_socket,SOL_SOCKET,SO_BROADCAST,(char *)&on,sizeof(on));
// needed now, because we use network_write
raydium_network_mode=RAYDIUM_NETWORK_MODE_CLIENT;
// we need to send request for uid (and send our name)
strcpy(str+RAYDIUM_NETWORK_PACKET_OFFSET,raydium_network_name_local);
raydium_network_write(NULL,-1,RAYDIUM_NETWORK_PACKET_REQUEST_UID,str);


// probably needs timeouts, here ...
if (raydium_network_read(&empty,&type,str)!=RAYDIUM_NETWORK_DATA_OK)
    {
    raydium_network_mode=RAYDIUM_NETWORK_MODE_NONE;
    raydium_log("ERROR ! network: cannot connect to server %s",server);
    perror("System");
    raydium_network_socket_close(raydium_network_socket);
    return(0);
    }

if(type==RAYDIUM_NETWORK_PACKET_ATTRIB_UID)
    {
    raydium_network_uid=str[RAYDIUM_NETWORK_PACKET_OFFSET];
    raydium_log("network: accepted as client %i",raydium_network_uid);
    raydium_network_set_socket_block(0);
    strcpy(raydium_network_connected_server,server);
    return(1);
    }

if(type==RAYDIUM_NETWORK_PACKET_ERROR_NO_MORE_PLACE)
    {
    raydium_network_mode=RAYDIUM_NETWORK_MODE_NONE;
    raydium_log("ERROR ! network: no more room (server said: %s)",str+RAYDIUM_NETWORK_PACKET_OFFSET);
    raydium_network_socket_close(raydium_network_socket);
    return(0);
    }

raydium_network_mode=RAYDIUM_NETWORK_MODE_NONE;
raydium_log("ERROR ! network: unknow server message type (%i). abort.",type);
raydium_network_socket_close(raydium_network_socket);
return(0);
}


signed char raydium_network_client_discover(char *game,int version)
{
struct sockaddr_in sock;
int on=1;
int ret;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_NONE)
    {
    raydium_log("network: ERROR: cannot create discover : already connected");
    return 0;
    }

raydium_network_start=time(NULL);
raydium_network_socket=socket(AF_INET,RAYDIUM_NETWORK_UDP,0);
if(raydium_network_socket==-1)
    {
    raydium_log("ERROR ! network: cannot create discover socket");
    perror("System");
    return(0);
    }
raydium_log("network: discover socket created");

sock.sin_family=AF_INET;
sock.sin_addr.s_addr=htonl(INADDR_ANY);
sock.sin_port=htons(RAYDIUM_NETWORK_BEACON_PORT);
ret=bind(raydium_network_socket,(struct sockaddr *)&sock,sizeof(sock));
if(ret)
    {
    raydium_log("ERROR ! network: cannot open discover socket (already used ?)");
    perror("System");
    return(0);
    }

raydium_network_beacon_search.active=1;
strcpy(raydium_network_beacon_search.app_or_mod,game);
raydium_network_beacon_search.version=version;
raydium_network_mode=RAYDIUM_NETWORK_MODE_DISCOVER;
setsockopt(raydium_network_socket,SOL_SOCKET,SO_BROADCAST,(char *)&on,sizeof(on));
raydium_network_set_socket_block(0);
raydium_log("network: discover OK: waiting for server beacons with '%s' (version %i)",game,version);
return 1;
}


// return :
// -1 : not in discover mode
// 0 and more : number of broadcasting servers
int raydium_network_discover_numservers(void)
{
int i;
int cpt=0;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_DISCOVER ||
   !raydium_network_beacon_search.active)
    return -1;

for(i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
    if(raydium_network_server_list[i].when!=0)
	cpt++;

return cpt;
}

// "num" starts from 0 to raydium_network_discover_numservers()-1
// return :
// -1 : not in discover mode
// 0 : "num" is invalid
// 1 : ok
signed char raydium_network_discover_getserver(int num, char *name, char *ip, char *info, int *player_count, int *player_max)
{
int i,cpt;
int slot=-1;

if( raydium_network_mode!=RAYDIUM_NETWORK_MODE_DISCOVER ||
   !raydium_network_beacon_search.active)
    return -1;

for(cpt=0,i=0;i<RAYDIUM_NETWORK_MAX_SERVERS;i++)
    if(raydium_network_server_list[i].when!=0)
	{
	if(cpt==num)
	    {
	    slot=i;
	    break;
	    }
	cpt++;
	}

if(slot<0)
    return 0;

//	printf(".\n");

strcpy(name,raydium_network_server_list[slot].name);
strcpy(ip,raydium_network_server_list[slot].ip);
strcpy(info,raydium_network_server_list[slot].info);
*player_count=raydium_network_server_list[slot].player_count;
*player_max=raydium_network_server_list[slot].player_max;
return 1;
}


void raydium_network_client_disconnect(void)
{
raydium_network_init_sub();
raydium_log("network: disconnected");
}

signed char raydium_server_accept_new(struct sockaddr *from, char *name)
{
int socklen,i,n;
char str[RAYDIUM_NETWORK_PACKET_SIZE];
void (*f)(int);

socklen=sizeof(struct sockaddr);

 for(i=0,n=-1;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
  if(!raydium_network_client[i]) {n=i; break;}

 if(n<0) 
    { 
    // no more room in this server
    sprintf(str+RAYDIUM_NETWORK_PACKET_OFFSET,"Server limited to %i client(s)",RAYDIUM_NETWORK_MAX_CLIENTS);
    raydium_network_write(from,-1,RAYDIUM_NETWORK_PACKET_ERROR_NO_MORE_PLACE,str);
    return(0);
    }
 
 memcpy(&raydium_network_client_addr[n],from,sizeof(struct sockaddr));
 raydium_network_client[n]=1;
 time(&raydium_network_keepalive[n]); // first keepalive
 strcpy(raydium_network_name[n],name);
 raydium_netwok_queue_ack_delay_server[n]=raydium_timecall_clocks_per_sec; // 1sec default delay
 
 raydium_log("network: client %i connected as %s"/*,inet_ntoa(from->sin_addr)*/,n,name);
 
 /* send uid to client */
 str[RAYDIUM_NETWORK_PACKET_OFFSET]=n;
 raydium_network_write(from,-1,RAYDIUM_NETWORK_PACKET_ATTRIB_UID,str);

 // OnConnect:
 for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
    if(i!=n && raydium_network_client[i])
    {
    strcpy(str+RAYDIUM_NETWORK_PACKET_OFFSET+1,raydium_network_name[i]);
    str[RAYDIUM_NETWORK_PACKET_OFFSET]=i;
    raydium_network_write(from,i,RAYDIUM_NETWORK_PACKET_INFO_NAME,str);
    }
 strcpy(str+RAYDIUM_NETWORK_PACKET_OFFSET+1,raydium_network_name[n]); // send name to all others...
 str[RAYDIUM_NETWORK_PACKET_OFFSET]=n;
 raydium_network_broadcast(RAYDIUM_NETWORK_PACKET_INFO_NAME,str);    
 
 if(raydium_network_on_connect)
    {
    f=raydium_network_on_connect;
    f(n);
    }
    
 return(n);
}

void raydium_network_close(void)
{
raydium_network_socket_close(raydium_network_socket);
#ifdef WIN32
WSACleanup();
#endif
}


void raydium_network_internal_server_delays_dump(void)
{
int i;

raydium_log("Network server delays:");
for(i=0;i<RAYDIUM_NETWORK_MAX_CLIENTS;i++)
         if(raydium_network_client[i])
	    raydium_log("player %i : %.2f msec (%s)",
		i,
		raydium_netwok_queue_ack_delay_server[i]/(double)raydium_timecall_clocks_per_sec*1000,
		raydium_network_name[i]);
}

void raydium_network_internal_dump(void)
{
time_t diff;
diff=time(NULL)-raydium_network_start;
raydium_log("Network stats:");
raydium_log("Rx: %i byte(s) / Tx: %i bytes(s) / %.2f min",raydium_network_stat_rx,raydium_network_stat_tx,diff/60.f);
raydium_log("Transfert rates: Rx: %.2f KB/s / Tx: %.2f KB/s",raydium_network_stat_rx/(float)diff/1024.f,raydium_network_stat_tx/(float)diff/1024.f);
raydium_log("Packets (err): Tx: %i re-emitted, Rx: %i doubles",raydium_network_stat_reemitted,raydium_network_stat_double);
raydium_log("Packets (err): Tx: %i erased or lost, bogus ACK: %i",raydium_network_stat_lost,raydium_network_stat_bogus_ack);
}


/*
// Test internet connection using a root dns server ... shame ? :)
#define RAYDIUM_NETWORK_INTERNET_TEST_HOST	"198.41.0.4"
#define RAYDIUM_NETWORK_INTERNET_TEST_PORT	53
#define RAYDIUM_NETWORK_INTERNET_TEST_TIMEOUT	8
*/

signed char raydium_network_internet_test(void)
{
/*
int sockfd;
struct sockaddr_in serv_addr;
struct hostent *server;
//struct timeval sv;
//int svlen;
struct timeval timeout;
fd_set writable;

sockfd = socket(AF_INET, SOCK_STREAM, 0);
server = gethostbyname(RAYDIUM_NETWORK_INTERNET_TEST_HOST);
//server = gethostbyname("192.168.2.1");

memset(&serv_addr, 0, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;

memcpy(&(serv_addr.sin_addr.s_addr),server->h_addr,server->h_length);
serv_addr.sin_port = htons(RAYDIUM_NETWORK_INTERNET_TEST_PORT);

raydium_network_set_socket_block_internal(sockfd,0);

// i'm tired of win32's sockets bad behaviours ... let's trust connect() ...
connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

// dont ask me why ... (Dev-CPP's mingw needs it)
#ifdef WIN32
#ifndef FD_SETSIZE
#define FD_SETSIZE	64
#endif
#endif

FD_ZERO(&writable);
FD_SET(sockfd, &writable);
timeout.tv_sec=RAYDIUM_NETWORK_INTERNET_TEST_TIMEOUT;
timeout.tv_usec=0;

#ifndef ENETUNREACH
#define ENETUNREACH   WSAENETUNREACH
#endif

if (select(sockfd + 1, NULL, &writable, NULL, &timeout) <= 0 || errno==ENETUNREACH)
    {
    raydium_log("network: cannot contact remote server, no internet connection detected");
    raydium_network_socket_close(sockfd);
    return 0; // not writable
    }
    
//raydium_log("network: internet link is ok");
raydium_network_socket_close(sockfd);
return 1; // writable
*/

// may use a cache system ? (time-limited ? with an arg ?)
return raydium_rayphp_http_test();
}


#ifdef linux
signed char raydium_network_linux_find_broadcast_interfaces(void)
{
int fd,len, ifflags;
struct sockaddr_in addr;
struct ifconf ifconf;
struct ifreq *ifreqp, ifreq, ifbuf[RAYDIUM_NETWORK_BROADCAST_INTERFACE_MAX];
char name[RAYDIUM_MAX_NAME_LEN];
char msg[RAYDIUM_MAX_NAME_LEN];

msg[0]=0;
raydium_network_broadcast_interface_index=0;

if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
    raydium_log("network: linux broadcast find interfaces: ERROR");
    perror("socket");
    return 0;
    }

ifconf.ifc_len = sizeof(ifbuf);
ifconf.ifc_buf = (caddr_t)ifbuf;
memset((void *)ifbuf, 0, sizeof(ifbuf));

if (ioctl(fd, SIOCGIFCONF, (char *)&ifconf) == -1) 
    {
    raydium_log("network: linux broadcast find interfaces: ERROR");
    perror("ioctl SIOCGIFCONF");
    close(fd);
    return 0;
    }

for (len = 0; len + (int)sizeof(struct ifreq) <= ifconf.ifc_len;)
    {
    ifreqp = (struct ifreq *)&ifconf.ifc_buf[len];


    len += sizeof(struct ifreq);

    if (ifreqp->ifr_addr.sa_family != AF_INET)
	    continue;

    addr = *(struct sockaddr_in *)&ifreqp->ifr_addr;
    strcpy(name,ifreqp->ifr_name);
    //printf("\taddress %s\n", inet_ntoa(addr.sin_addr));

    ifreq = *ifreqp;
    if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifreq) == -1) 
	{
	raydium_log("network: linux broadcast find interfaces: ERROR");
	perror("ioctl SIOCGIFFLAGS");
	continue;
	}

    ifflags = ifreq.ifr_flags;

    // interface up ?
    if ((ifflags & IFF_UP) == 0) 
	continue;

    // running ?
    if ((ifflags & IFF_RUNNING) == 0)
	continue;

    // is loopback ?
    if ((ifflags & IFF_LOOPBACK) != 0) 
	continue;

    // can broadcast ?
    if ((ifflags & IFF_BROADCAST) == 0)
	continue;
    
    ifreq = *ifreqp;

    if (ioctl(fd, SIOCGIFBRDADDR, (char *)&ifreq) == -1) 
	{
	perror("ioctl SIOCGIFBRDADDR");
	continue;
	}

    addr = *(struct sockaddr_in *)&ifreq.ifr_addr;

    addr.sin_family = AF_INET;
    addr.sin_port=htons(RAYDIUM_NETWORK_BEACON_PORT);
    memcpy( &raydium_network_broadcast_interfaces[raydium_network_broadcast_interface_index],
	    &addr,sizeof(addr)); // broadcast address
    strcat(msg,name);
    strcat(msg," ");
    raydium_network_broadcast_interface_index++;
    }

close(fd);
raydium_log("network: linux broadcast interface(s): %s",msg);
return 1;
}
#endif
