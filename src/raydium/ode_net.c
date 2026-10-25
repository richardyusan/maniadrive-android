/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/


// This .c file is included from ode.c, so there's no includes here.

void raydium_ode_network_element_new(int e);


int raydium_ode_network_MaxElementsPerPacket(void)
{
return ((RAYDIUM_NETWORK_PACKET_SIZE-RAYDIUM_NETWORK_PACKET_OFFSET)/sizeof(raydium_ode_network_Event)-1);
}


int raydium_network_nid_element_find(int nid)
{
int i;

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].state && raydium_ode_element[i].nid==nid)
	return i;
return -1;
}

void raydium_ode_network_newdel_event(int type,char *buff)
{
int id;
int nid,dec,elem;
int tag;
dReal get[4];
dReal default_pos[3]={0,0,9999};

id=buff[1];
if(id==raydium_network_uid) return; // do not update our elements
//raydium_log("ode_net: new or del event received... (%i)",type);

dec=RAYDIUM_NETWORK_PACKET_OFFSET;
memcpy(&nid,buff+dec,sizeof(int));
elem=raydium_network_nid_element_find(nid);
dec+=sizeof(int);

if(type==RAYDIUM_NETWORK_PACKET_ODE_NEWELEM)
    {
    int type;
    char mesh[RAYDIUM_MAX_NAME_LEN];
    char name[RAYDIUM_MAX_NAME_LEN];
    int group;

//    raydium_log("ode_net: NEWELEM event, from %i",buff[1]);
    
    // if we already have this element, refresh it
    if(elem>=0) 
	raydium_ode_element_delete(elem,1);
    
    memcpy(&type,buff+dec,sizeof(int));
    dec+=sizeof(int);

    memcpy(get,buff+dec,sizeof(dReal)*3);
    dec+=(sizeof(dReal)*3);

    memcpy(&tag,buff+dec,sizeof(int));
    dec+=sizeof(int);

    strcpy(mesh,buff+dec);
    sprintf(name,"net_%i",nid); // may use the other side element name ?

    group=raydium_ode_object_find("DISTANT");
    switch(type)
	{
	case dSphereClass:
	    raydium_ode_network_distant_create=1;
	    elem=raydium_ode_object_sphere_add(name,group,1,get[0],RAYDIUM_ODE_STATIC,tag,mesh);
	    break;
	case dBoxClass:
	    raydium_ode_network_distant_create=1;
	    elem=raydium_ode_object_box_add(name,group,1,get[0],get[1],get[2],RAYDIUM_ODE_STATIC,tag,mesh);
	    break;
	}
    raydium_ode_element[elem].distant_owner=buff[1];
    raydium_ode_element[elem].nid=nid;
    raydium_ode_element[elem].lastnetupdate=time(NULL);
    raydium_ode_element_move(elem,default_pos);
//    raydium_log("ode_net: ... new: %s",raydium_ode_element[elem].name);
    }

if(type==RAYDIUM_NETWORK_PACKET_ODE_REMELEM)
    {
    if(elem<0) return;
    if(!raydium_ode_element[elem].distant) return;
    raydium_ode_element_delete(elem,1);
//    raydium_log("ode_net: ... delete: %s",raydium_ode_element[elem].name);
    }

}

void raydium_ode_network_nidwho_event(int type,char *buff)
{
int nid,elem;
memcpy(&nid,buff+RAYDIUM_NETWORK_PACKET_OFFSET,sizeof(int));
elem=raydium_network_nid_element_find(nid);

if(elem<0) return;

// test if we are owner of elem
if(!raydium_ode_element[elem].distant)
    {
    //raydium_log("ode_net: sending answer to nidwho request for %s",raydium_ode_element[elem].name);
    raydium_ode_network_element_new(elem);
    }
}



void raydium_ode_network_explosion_event(int type,char *buff)
{
raydium_ode_network_Explosion *exp;

exp=(raydium_ode_network_Explosion *)(buff+RAYDIUM_NETWORK_PACKET_OFFSET);
raydium_ode_network_explosion_create=1; // really create explosion

if(exp->type==RAYDIUM_ODE_NETWORK_EXPLOSION_EXPL)
    {
    // needs to (automaticaly) generates a "name" for explosions
    char name[RAYDIUM_MAX_NAME_LEN];
    raydium_ode_name_auto("net_expl",name);
    raydium_ode_explosion_create(name,exp->radius,exp->propag,exp->pos);
    }

if(exp->type==RAYDIUM_ODE_NETWORK_EXPLOSION_BLOW)
    raydium_ode_explosion_blow(exp->radius,exp->force,exp->pos);

// Useless, but...
raydium_ode_network_explosion_create=0;
}

void raydium_ode_network_init(void)
{
char opt[128];
raydium_ode_network_maxfreq=RAYDIUM_ODE_NETWORK_MAXFREQ;
raydium_network_netcall_add(raydium_ode_network_newdel_event,RAYDIUM_NETWORK_PACKET_ODE_NEWELEM,1);
raydium_network_netcall_add(raydium_ode_network_newdel_event,RAYDIUM_NETWORK_PACKET_ODE_REMELEM,1);
raydium_network_netcall_add(raydium_ode_network_nidwho_event,RAYDIUM_NETWORK_PACKET_ODE_NIDWHO,1);
raydium_network_netcall_add(raydium_ode_network_explosion_event,RAYDIUM_NETWORK_PACKET_ODE_EXPLOSION,1);
if(raydium_init_cli_option("ode-rate",opt))
    raydium_ode_network_maxfreq=atoi(opt);

raydium_ode_object_create("DISTANT");
raydium_ode_object_colliding_name("DISTANT",0);
}


signed char raydium_ode_network_TimeToSend(void)
{
static float time;

time+=raydium_frame_time;

if(time > (1.0/raydium_ode_network_maxfreq))
    {
    time=0;
    return 1;
    }
return 0;
}

// Refresh elements pos/rot to network
void raydium_ode_network_element_send(short nelems, int *e)
{
char data[RAYDIUM_NETWORK_PACKET_SIZE];
raydium_ode_network_Event set;
int dec;
int i;
short real=0;
dReal q[4];
dReal *p;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_CLIENT)
    return;

dec=RAYDIUM_NETWORK_PACKET_OFFSET;

dec+=sizeof(nelems);

for(i=0;i<nelems;i++)
 {
 if(!raydium_ode_element_isvalid(e[i])) continue;
 if(raydium_ode_element[e[i]].nid<0) continue;
 if(raydium_ode_element[e[i]].distant) continue;

 set.nid=raydium_ode_element[e[i]].nid;

 p=raydium_ode_element_pos_get(e[i]);
 memcpy(set.pos,p,sizeof(dReal)*3);

 raydium_ode_element_rotq_get(e[i],q);
 memcpy(set.rot,q,sizeof(dReal)*4);

 p=raydium_ode_element_linearvelocity_get(e[i]);
 memcpy(set.vel,p,sizeof(dReal)*3);

 memcpy(data+dec,&set,sizeof(set));
 dec+=sizeof(set);
 real++;
 if(dec>=RAYDIUM_NETWORK_PACKET_SIZE)
    {
    raydium_log("ode_net: PACKET SIZE TOO SMALL !");
    return;
    }
 }
memcpy(data+RAYDIUM_NETWORK_PACKET_OFFSET,&real,sizeof(real));
raydium_network_write(NULL,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ODE_DATA,data);

}

void raydium_ode_network_element_send_all(void)
{
int i,n;
int e[RAYDIUM_ODE_MAX_ELEMENTS];

if(!raydium_ode_network_TimeToSend()) return;

n=0;
for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
 if(raydium_ode_element_isvalid(i) &&
    raydium_ode_element[i].nid>=0 )
	{
	e[n]=i;
	n++;
	}
raydium_ode_network_element_send(n,e);
}


void raydium_ode_network_element_send_random(int nelems)
{
int i=0;
int n;
int done[RAYDIUM_ODE_MAX_ELEMENTS];
int e[RAYDIUM_ODE_MAX_ELEMENTS];
int total=0;

if(!raydium_ode_network_TimeToSend()) return;

if(nelems==RAYDIUM_ODE_NETWORK_OPTIMAL)
    nelems=raydium_ode_network_MaxElementsPerPacket();

memset(done,0,RAYDIUM_ODE_MAX_ELEMENTS*sizeof(int));

while(i<nelems)
    {
    n=raydium_random_i(0,RAYDIUM_ODE_MAX_ELEMENTS);
    if(raydium_ode_element[n].state && raydium_ode_element[n].nid>=0 && !done[n])
	{
	done[n]=1;
	e[i]=n;
	i++;
	}
    total++;
    if(total>RAYDIUM_ODE_MAX_ELEMENTS) break;
    }
raydium_ode_network_element_send(i,e);
}


void raydium_ode_network_element_send_iterative(int nelems)
{
static int curr=-1;
int i=0;
int e[RAYDIUM_ODE_MAX_ELEMENTS];
int total=0;

if(!raydium_ode_network_TimeToSend()) return;

if(nelems==RAYDIUM_ODE_NETWORK_OPTIMAL)
    nelems=raydium_ode_network_MaxElementsPerPacket();

while(i<nelems)
    {
    curr++;
    if(curr>=RAYDIUM_ODE_MAX_ELEMENTS) curr=0;
    if(raydium_ode_element[curr].state && raydium_ode_element[curr].nid>=0)
	{
	e[i]=curr;
	i++;
	}
    total++;
    if(total>RAYDIUM_ODE_MAX_ELEMENTS) break;
    }
raydium_ode_network_element_send(i,e);
}


void raydium_ode_network_nidwho(int nid)
{
char data[RAYDIUM_NETWORK_PACKET_SIZE];

// limit nidwho req freq
if(!raydium_ode_network_TimeToSend()) return;

memcpy(data+RAYDIUM_NETWORK_PACKET_OFFSET,&nid,sizeof(int));
raydium_network_write(NULL,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ODE_NIDWHO,data);
}

void raydium_ode_network_apply(raydium_ode_network_Event *ev)
{
int elem,i;
unsigned long now;
unsigned long before;
dReal factor;
dReal *pos;
dReal Pcross[3];

elem=raydium_network_nid_element_find(ev->nid);
if(elem<0) 
    {
    // ask for this nid:
    raydium_ode_network_nidwho(ev->nid);
    return; // nid not found.. unknown element !
    }

raydium_ode_element[elem].lastnetupdate=time(NULL);

// must test time modulo here !
now=raydium_timecall_clock();
before=raydium_ode_element[elem].net_last_time;
raydium_ode_element[elem].net_last_interval=now-raydium_ode_element[elem].net_last_time;
raydium_ode_element[elem].net_last_time=now;

raydium_ode_element_rotateq(elem,ev->rot);

if(!before || raydium_timecall_interval[raydium_ode_timecall]==0)
{
    raydium_ode_element_move(elem,ev->pos);
    memcpy(raydium_ode_element[elem].netvel,ev->vel,sizeof(dReal)*3);
#ifdef DEBUG_ODENET    
    raydium_log("%i elem's first update (or 0 hz ODE callback)",elem);
#endif
}
else
{

// we must modify "netvel" to force reconciliation
pos=raydium_ode_element_pos_get(elem);
factor=((raydium_ode_element[elem].net_last_interval/(float)raydium_timecall_clocks_per_sec)*(float)RAYDIUM_ODE_PHYSICS_FREQ)*RAYDIUM_ODE_TIMESTEP;
#ifdef DEBUG_ODENET
raydium_log("ODE 1 sec factor : %f",factor);
#endif

if(factor<0.01) // probably another packet from the same read loop
    {
    for(i=0;i<3;i++)
	raydium_ode_element[elem].netvel[i]=0;
    return;
    }

// 1 - determine probable next point (real and predi vectors cross point)
for(i=0;i<3;i++)
    Pcross[i]=ev->pos[i]+
		(ev->vel[i] * factor);

#ifdef DEBUG_ODENET
raydium_log("pcross = %f %f %f | pos = %f %f %f",Pcross[0],Pcross[1],Pcross[2],pos[0],pos[1],pos[2]);
#endif

// 2 - determine vector to this point from estimated last prediction (pos)
for(i=0;i<3;i++)
    raydium_ode_element[elem].netvel[i]=(Pcross[i]-pos[i])/factor;


#ifdef DEBUG_ODENET
raydium_log("netvel = %f %f %f | org = %f %f %f",
				raydium_ode_element[elem].netvel[0],
				raydium_ode_element[elem].netvel[1],
				raydium_ode_element[elem].netvel[2],
				ev->vel[0],
				ev->vel[1],
				ev->vel[2]);
raydium_log("---");
#endif

// start previ (client side)... (may be unused)
//memcpy(raydium_ode_element[elem].net_last_pos2,raydium_ode_element[elem].net_last_pos1,sizeof(dReal)*3);
//memcpy(raydium_ode_element[elem].net_last_pos1,ev->pos,sizeof(dReal)*3);
// ... end previ
}

#ifndef ODE_PREDICTION
raydium_ode_element_move(elem,ev->pos);
raydium_ode_element_rotateq(elem,ev->rot);
#endif

#ifdef ODE_NETWORK_GHOSTS
 {
 // horrible... but usefull for debugging.
 dMatrix3 R;
 dQtoR(ev->rot,R);
 ev->pos[0]+=0.01;
 raydium_camera_replace_go(ev->pos,R);
 raydium_object_draw(raydium_ode_element[elem].mesh);
 //printf("ode_net: %s updated (packet usage: %i byte(s))\n", raydium_ode_element[elem].name,sizeof(*ev)+RAYDIUM_NETWORK_PACKET_OFFSET);
 }
#endif
}


// Read new packets (flushed read must be an option !)
void raydium_ode_network_read(void)
{
signed char type;
int id,i;
short n;
char data[RAYDIUM_NETWORK_PACKET_SIZE];
raydium_ode_network_Event *get;


if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_CLIENT)
    return;

for(i=0;i<RAYDIUM_ODE_MAX_ELEMENTS;i++)
    if(raydium_ode_element[i].nid>=0 && 
       raydium_ode_element[i].distant &&
       (time(NULL)>raydium_ode_element[i].lastnetupdate+RAYDIUM_NETWORK_TIMEOUT))
    	    {
	    raydium_ode_element_delete(i,1);
	    //raydium_log("element %i deleted: timeout",i);
	    }

// read (flushed ?), and if RAYDIUM_NETWORK_PACKET_DATA, search nid and update pos/rot
if(raydium_network_read(&id,&type,data)!=RAYDIUM_NETWORK_DATA_OK)
//if(raydium_network_read_flushed(&id,&type,data)!=RAYDIUM_NETWORK_DATA_OK)
    return;
    
#ifndef ODE_NETWORK_GHOSTS
if(id==raydium_network_uid) 
    return; // do not update our elements !
#endif

memcpy(&n,data+RAYDIUM_NETWORK_PACKET_OFFSET,sizeof(n));

if(type==RAYDIUM_NETWORK_PACKET_ODE_DATA)
 for(i=0;i<n;i++)
    {
    get=(raydium_ode_network_Event *)(data+RAYDIUM_NETWORK_PACKET_OFFSET+sizeof(n)+(i*sizeof(*get)));
    raydium_ode_network_apply(get);
    }

}

// Add new element to network
void raydium_ode_network_element_new(int e)
{
char data[RAYDIUM_NETWORK_PACKET_SIZE];
int dec,i;
dReal set[3];

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_CLIENT)
    return;
    
if(!raydium_ode_element_isvalid(e)) return;
if(raydium_ode_element[e].distant) return;

raydium_ode_element[e].nid=(raydium_network_uid+1)*1000+e;

dec=RAYDIUM_NETWORK_PACKET_OFFSET;
memcpy(data+dec,&raydium_ode_element[e].nid,sizeof(int));
dec+=sizeof(int);

i=dGeomGetClass(raydium_ode_element[e].geom);
memcpy(data+dec,&i,sizeof(int));
dec+=sizeof(int);

switch(i)
    {
    case dSphereClass:
	set[0]=dGeomSphereGetRadius(raydium_ode_element[e].geom);
	set[1]=set[2]=0;
	break;
    case dBoxClass:
	dGeomBoxGetLengths(raydium_ode_element[e].geom,set);
	break;
    }
memcpy(data+dec,set,sizeof(dReal)*3);
dec+=(sizeof(dReal)*3);

memcpy(data+dec,&raydium_ode_element[e].user_tag,sizeof(int));
dec+=sizeof(int);

strncpy(data+dec,raydium_object_name[raydium_ode_element[e].mesh],RAYDIUM_NETWORK_PACKET_SIZE-dec-1);
data[RAYDIUM_NETWORK_PACKET_SIZE-1]='\0';
raydium_network_write(NULL,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ODE_NEWELEM,data);
}

void raydium_ode_network_element_delete(int e)
{
char data[RAYDIUM_NETWORK_PACKET_SIZE];
int dec;

if(raydium_network_mode!=RAYDIUM_NETWORK_MODE_CLIENT)
    return;

if(!raydium_ode_element_isvalid(e)) return;
if(raydium_ode_element[e].nid<0) return;
if(raydium_ode_element[e].distant) return;

dec=RAYDIUM_NETWORK_PACKET_OFFSET;
memcpy(data+dec,&raydium_ode_element[e].nid,sizeof(int));
dec+=sizeof(int);

raydium_network_write(NULL,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ODE_REMELEM,data);
}

void raydium_ode_network_explosion_send(raydium_ode_network_Explosion *exp)
{
char data[RAYDIUM_NETWORK_PACKET_SIZE];
memcpy(data+RAYDIUM_NETWORK_PACKET_OFFSET,exp,sizeof(raydium_ode_network_Explosion));
raydium_network_write(NULL,raydium_network_uid,RAYDIUM_NETWORK_PACKET_ODE_EXPLOSION,data);
}


signed char raydium_ode_network_element_isdistant(int elem)
{
if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: cannot get element 'is distant' flag: invalid name/index");
    return 0;
    }

return raydium_ode_element[elem].distant;
}

signed char raydium_ode_network_element_isdistant_name(char *elem)
{
return raydium_ode_network_element_isdistant(raydium_ode_element_find(elem));
}

signed char raydium_ode_network_element_distantowner(int elem)
{
if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: cannot get element 'distant owner': invalid name/index");
    return 0;
    }
return raydium_ode_element[elem].distant_owner;
}

signed char raydium_ode_network_element_distantowner_name(char *elem)
{
return raydium_ode_network_element_distantowner(raydium_ode_element_find(elem));
}


void raydium_ode_network_element_trajectory_correct(int elem)
{
//dReal vector[3];
dReal pos[3];
dReal *cur;
//unsigned long now;
int i;
raydium_ode_Element *e;

if(!raydium_ode_element_isvalid(elem))
    {
    raydium_log("ODE: Error: cannot correct trajectory: invalid name/index");
    return;
    }

e=&raydium_ode_element[elem];
//now=raydium_timecall_clock();

// determine vector (last1 - last2)
//for(i=0;i<3;i++)
//    vector[i]=e->net_last_pos1[i]-e->net_last_pos2[i];

// pos = last1 + ( vector * ( (now - last1) / interval ))
//for(i=0;i<3;i++)
//    pos[i]=e->net_last_pos1[i] + ( vector[i] * ( (now - e->net_last_time) / (dReal)(e->net_last_interval) ));


// Use "dead reckoning" method :
cur=raydium_ode_element_pos_get(elem);
memcpy(pos,cur,sizeof(dReal)*3);
for(i=0;i<3;i++)
	pos[i]+=(e->netvel[i]*RAYDIUM_ODE_TIMESTEP);
//	pos[i]+=(e->netvel[i]/RAYDIUM_ODE_PHYSICS_FREQ);

//printf("cur = %f %f %f | next = %f %f %f (%f %f %f)\n",cur[0],cur[1],cur[2],pos[0],pos[1],pos[2],e->netvel[0],e->netvel[1],e->netvel[2]);

#ifdef ODE_PREDICTION
// pray.
raydium_ode_element_move(elem,pos);
#endif
}

void raydium_ode_network_elment_next_local(void)
{
raydium_ode_network_next_local_only=1;
}

