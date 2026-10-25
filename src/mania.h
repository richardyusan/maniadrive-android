#define TYPE_CHECKPOINT 	100

#define TYPE_LOOP 		30

#define TYPE_START_E 		10
#define TYPE_START_W		11
#define TYPE_START_N 		12
#define TYPE_START_S 		13

#define TYPE_END 		20

#define TYPE_TURBO_E		2
#define TYPE_TURBO_W		3
#define TYPE_TURBO_N		4
#define TYPE_TURBO_S		5

#define TYPE_CAR		1
#define TYPE_CAR_BODY		7
#define TYPE_CAR_BODY_SP	8
#define TYPE_BALANCIER		6

#define TURBO_POWA		0.5

#define IS_CAR_BODY(a)		(((a)==TYPE_CAR_BODY) || ((a)==TYPE_CAR_BODY_SP))

typedef struct Grid
    {
    char state;
    float x;
    float y;
    float z;
    int rot;
    int flags;
    int obj;
    } Grid;

typedef struct Box
    {
    char state;
    float x;
    float y;
    float z;
    float tx;
    float ty;
    float tz;
    int type;
    int id;
    } Box;
    
#define MAX_ELEMS	4096
#define FACT 		6
#define MOVE_X 		-5
#define MOVE_Y 		-3
#define MOVE_Z 		-1

Grid grid[MAX_ELEMS];
Box box[MAX_ELEMS];
char tdata[4096];

void extract_data(char *from, char *to_name, char *to_author, char *to_gold, char *to_author_time, char *message_file)
{
char d[4096];
int i,start,cpt;

message_file[0]=0;

strcpy(d,from);
cpt=0;
start=0;
for(i=0;i<=strlen(from);i++)
    {
    if(d[i]==';' || d[i]==0 || d[i]=='\r')
	{
	d[i]=0;
	//printf("%i %s\n",cpt,d+start);
	if(cpt==0)
	    strcpy(to_name,d+start);
	if(cpt==1)
	    strcpy(to_author,d+start);
	if(cpt==2)
	    strcpy(to_gold,d+start);
	if(cpt==3)
	    strcpy(to_author_time,d+start);
	if(cpt==4)
	    strcpy(message_file,d+start);

	// finalize
	cpt++;
	start=i+1;
	}
    }
}

void dump_data_to(char *filename)
{
FILE *fp;
char d[5][512];

fp=fopen(filename,"wt");
if(!fp) { printf("cannot write to file \"%s\", fopen() failed\n",filename); return; }

fprintf(fp,"// generated track data (mania2)\n\n");

extract_data(tdata,d[0],d[1],d[2],d[3],d[4]);
fprintf(fp,"name=\"%s\";\n",d[0]);
fprintf(fp,"author=\"%s\";\n",d[1]);
fprintf(fp,"gold_time=%s;\n",d[2]);
fprintf(fp,"author_time=%s;\n",d[3]);
fprintf(fp,"message=\"%s\";\n",d[4]);

fprintf(fp,"\n// EOF\n");
fclose(fp);
printf("data file generated.\n");
}


// read mni file and generate .tri, .box and .dat
signed char mni_generate(char *filename)
{
FILE *in;
FILE *tri, *box;
char c;
char str[RAYDIUM_MAX_NAME_LEN];
float x,y,z;
float vx,vy,vz;
float rvx,rvy,rvz;
float nx,ny,nz;
float rnx,rny,rnz;
float tx,ty,tz;
int rot,flags,type;
int obj;
int i;

in=raydium_file_fopen(filename,"rt");
if(!in)
    {
    raydium_log("mni_gen: unable to open '%s'",filename);
    return 0;
    }

tri=fopen(raydium_file_home_path("mania.tri"),"wt");
if(!tri)
    {
    raydium_log("mni_gen: unable to create TRI file");
    fclose(in);
    return 0;
    }

box=fopen(raydium_file_home_path("mania.box"),"wt");
if(!box)
    {
    raydium_log("mni_gen: unable to create BOX file");
    fclose(in);
    fclose(tri);
    return 0;
    }

unlink(raydium_file_home_path("mania.dat"));

fprintf(tri,"1\n");
while(1)
{
c=fgetc(in);
fgetc(in);

if(c==EOF) break;

switch(c)
    {
    case 'b':
	fscanf(in,"%f %f %f %f %f %f %i\n", &x,
					    &y,
				    	    &z,
				    	    &tx,
				    	    &ty,
				    	    &tz,
					    &type);
	x=x*FACT+MOVE_X;
	y=y*FACT+MOVE_Y;
	z=z*FACT+MOVE_Z;
	tx=tx*FACT;
	ty=ty*FACT;
	tz=tz*FACT;
	fprintf(box,"%f %f %f %f %f %f %i\n",x,y,z,tx,ty,tz,type);
	break;
    case 'g':
	fscanf(in,"%f %f %f %i %i %s\n",&x,
				        &y,
				        &z,
				        &rot,
				        &flags,
				        str);
	obj=raydium_object_find_load(str);
	for(i=raydium_object_start[obj];i<raydium_object_end[obj];i++)
	    {
	    vx=raydium_vertex_x[i];
	    vy=raydium_vertex_y[i];
	    vz=raydium_vertex_z[i];
	    nx=raydium_vertex_normal_visu_x[i];
	    ny=raydium_vertex_normal_visu_y[i];
	    nz=raydium_vertex_normal_visu_z[i];
	    vx-=0.5;
	    vy-=0.5;

	    rvx=vx*raydium_trigo_cos(rot) - vy*raydium_trigo_sin(rot);
	    rvy=vx*raydium_trigo_sin(rot) + vy*raydium_trigo_cos(rot);
	    rvz=vz;
	    rnx=nx*raydium_trigo_cos(rot) - ny*raydium_trigo_sin(rot);
	    rny=nx*raydium_trigo_sin(rot) + ny*raydium_trigo_cos(rot);
	    rnz=nz;

	    rvx+=0.5;
	    rvy+=0.5;
	    rvx+=x;
	    rvy+=y;
	    rvz+=z;
	    rvx*=FACT;
	    rvy*=FACT;
	    rvz*=FACT;
	    rvx+=MOVE_X;
	    rvy+=MOVE_Y;
	    rvz+=MOVE_Z;
	    fprintf(tri,"%f %f %f %f %f %f %f %f %s\n",rvx,rvy,rvz,rnx,rny,rnz,raydium_vertex_texture_u[i],raydium_vertex_texture_v[i],raydium_texture_name[raydium_vertex_texture[i]]);
	    }
	break;    
    case 'd':
	fgets(tdata,4000,in);
	if(tdata[strlen(tdata)-1]=='\n')
	    tdata[strlen(tdata)-1]=0;
	dump_data_to(raydium_file_home_path("mania.dat"));
	break;
    default:
	raydium_log("mni_gen: invalid track file (block type '%c' unknown)",c);
	break;
    }
} // end while


fclose(in);
fclose(tri);
fclose(box);
return 1;
}
