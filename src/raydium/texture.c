/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/texture.h"
#endif

#include "live.h"

// proto
int raydium_init_cli_option(char *option, char *value);
FILE *raydium_file_fopen(char *path, char *mode);


signed char raydium_texture_size_is_correct(GLuint size)
{
int i;

if(raydium_window_mode==RAYDIUM_RENDERING_NONE)
    return 1;

// must be a power of 2 and <= to raydium_texture_size_max
if(size==1) return 1;
for(i=2;i<=raydium_texture_size_max;i*=2)
    if(i==size) return 1;
return 0;
}

GLuint raydium_texture_load_internal(char *filename, char *as, signed char faked, int faked_tx, int faked_ty, int faked_bpp, int or_live_id_fake)
{
FILE *file;
unsigned char temp[RAYDIUM_MAX_NAME_LEN];
unsigned char *data;
GLuint tx,ty,bpp,id;
GLuint i,j,k,GLbpp=0,GLbppi;
GLuint texsize=0;
char blended=0,filter=0,cutout=0,simulate=0;
char rgb;
GLfloat r,g,b;
signed char reflect=0;

// "as" is duplicated ?
for(i=0;i<raydium_texture_index;i++)
    if(!strcmp(raydium_texture_name[i],as)) 
    {
    raydium_log("texture: (internal) %s is duplicated",as);
    return i;
    }


if(raydium_window_mode==RAYDIUM_RENDERING_NONE)
    simulate=1;

/* is RGB color ? (or texture) */
strcpy((char *)temp,filename);
temp[4]=0;
if(!strcmp("rgb(",(char *)temp)) rgb=1; else rgb=0;

strcpy((char *)temp,filename);
temp[3]=0;
if(!strcmp("ENV",(char *)temp)) reflect=1;


if(!rgb && !faked)
{
 file=raydium_file_fopen(filename,"rb");
 if(!file)
  {
    raydium_log("Cannot open %s texture",filename);
    return 0;
  }

 fread(temp,1,12,file);
 if(temp[2]!=2 && temp[2]!=3) 
 { 
 fclose(file); 
 raydium_log("%s is not an uncompressed TGA RGB or grayscale file (type %i)",filename,temp[2]);
 return 0; //not a: uncompressed TGA RGB file
 } 

 fread(temp,1,6,file);

 tx = temp[1] * 256 + temp[0]; // highbyte*256+lowbyte
 ty = temp[3] * 256 + temp[2]; // highbyte*256+lowbyte

 if( !raydium_texture_size_is_correct(tx) || !raydium_texture_size_is_correct(ty) )
 {
 raydium_log("texture: ERROR: cannot load %s: invalid size %ix%i (must be a power of two and inferior to your max hardware size: %i)",filename,tx,ty,raydium_texture_size_max);
 raydium_log("texture: I will fake this texture.");
 tx=ty=1;
 }
 bpp= temp[4] / 8;
 texsize = tx * ty * bpp;

 data=malloc(texsize);
 if(!data) { 
 fclose(file); 
 raydium_log("texture: ERROR ! malloc for %s failed ! (%i bytes needed)",filename,tx*ty*bpp);
 return 0; }

 for(j=0; j<ty; j++)
 for(i=0; i<tx; i++)
 {
 if(fread(temp,1,bpp,file)!=bpp) 
 { free(data); fclose(file); 
 raydium_log("Invalid data in %s",filename);
 return 0; }
 k=(( (ty-j-1) *tx)+i)*bpp;
 if(bpp == 1) data[k]=temp[0];
 else // no greyscale
  {
   if(reflect)
    {
    temp[0]*=RAYDIUM_RENDER_REFLECTION_FACT;
    temp[1]*=RAYDIUM_RENDER_REFLECTION_FACT;
    temp[2]*=RAYDIUM_RENDER_REFLECTION_FACT;
    }

   data[k]=temp[2];
   data[k+1]=temp[1];
   data[k+2]=temp[0];
   if(bpp == 4) 
    { 
    data[k+3]=temp[3]; 
    if(temp[3]>0 && temp[3]<255)
	blended=1;
    if(temp[3]==0)
	cutout=1;
    }
  }
 }

 fclose(file);
 glPixelStorei(GL_UNPACK_ALIGNMENT,1);
} //end !rgb && !faked



if(raydium_texture_to_replace)
 { id=raydium_texture_to_replace; raydium_texture_to_replace=0; }
 else
 id=raydium_texture_index++;
 
if(raydium_texture_index>RAYDIUM_MAX_TEXTURES) 
    { 
    raydium_log("texture: No more texture slots left ! (%i max)",
		RAYDIUM_MAX_TEXTURES); 
    return 0; 
    }

strcpy(raydium_texture_name[id],as);

if(faked)
    {
    raydium_live_Texture *tex;

    if(or_live_id_fake>=0)
	{
	tex=&raydium_live_texture[or_live_id_fake];
	tx=tex->hardware_tx;
	ty=tex->hardware_ty;
	bpp=tex->bpp/8;
	}
    else
	{
	tx=faked_tx;
	ty=faked_ty;
	bpp=faked_bpp;
	}

    texsize = tx * ty * bpp;
    data=malloc(texsize);
    memset(data,0,texsize);
    if(!data) 
	{ 
	fclose(file); 
    	raydium_log("texture: ERROR ! malloc for %s failed ! (%i bytes needed)",filename,tx*ty*bpp);
	return 0; 
	}
    }

if(!rgb)
{
 if(bpp==1) 
    { 
    GLbpp=GL_ALPHA;
    GLbppi=GL_ALPHA8;
    blended=1; 
    }

 if(bpp==3) 
    {
    GLbpp=GL_RGB;
    GLbppi=GL_RGBA8; // force RGBA (since ATI cards needs it)
    texsize += tx * ty;
    }
    
 if(bpp==4) 
    {
    GLbpp=GL_RGBA;
    GLbppi=GL_RGBA8;
    }

 raydium_texture_blended[id]=0;
 if(blended)
    raydium_texture_blended[id]=1;

 if(cutout && !blended)
    {
    raydium_texture_blended[id]=2;
    blended=2;
    }


 raydium_texture_used_memory+=texsize;
 if(raydium_texture_filter==RAYDIUM_TEXTURE_FILTER_TRILINEAR && !faked)
    raydium_texture_used_memory+=(texsize/3); // mipmaps

if(!simulate)
 glBindTexture(GL_TEXTURE_2D,id);


 if(strstr(filename,".tri."))
    raydium_texture_islightmap[id]=1;

 memcpy(temp,filename,3);						// TEMP !!
 temp[3]=0;								// TEMP !!
 if(!simulate)
 {
  if(!strcmp("BOX",(char *)temp) || faked)				// TEMP !!
  {									// TEMP !!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// TEMP !!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	// TEMP !!
  }									// TEMP !!
  else
  {
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }
 }

 filter=raydium_texture_filter;

 if(!strcmp("HDR",(char *)temp))				// TEMP !!
    {
    raydium_texture_hdr[id]=1;
    raydium_texture_nolight[id]=1;
    }

 if(reflect)
    raydium_texture_env[id]=1;

 if(filter==RAYDIUM_TEXTURE_FILTER_TRILINEAR && blended)
    filter=RAYDIUM_TEXTURE_FILTER_BILINEAR;

 if(faked)
    filter=RAYDIUM_TEXTURE_FILTER_BILINEAR;
 
  if(filter==RAYDIUM_TEXTURE_FILTER_NONE && !simulate)
  {
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
  glTexImage2D(GL_TEXTURE_2D,0,GLbppi,tx,ty,0,GLbpp,GL_UNSIGNED_BYTE,data);
  }

  if(filter==RAYDIUM_TEXTURE_FILTER_BILINEAR && !simulate)
  {
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,GLbppi,tx,ty,0,GLbpp,GL_UNSIGNED_BYTE,data);
  }

  if(filter==RAYDIUM_TEXTURE_FILTER_TRILINEAR && !simulate)
  {
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Trilinear filtering
  gluBuild2DMipmaps(GL_TEXTURE_2D, GLbppi, tx, ty, GLbpp, GL_UNSIGNED_BYTE, data);
  }

 raydium_log("Texture num %i (%s) %s: %ix%i, %i Bpp (b%i lm%i hdr%i)",
	     id,raydium_texture_name[id],
	     (faked?"FAKED":"loaded"),
	     tx,ty,bpp,
	     blended,raydium_texture_islightmap[id],
	     raydium_texture_hdr[id]);
 free(data);
} else /* is rgb color */
{
 sscanf(filename,"rgb(%f,%f,%f)",&r,&g,&b);
 raydium_texture_rgb[id][0]=r;
 raydium_texture_rgb[id][1]=g;
 raydium_texture_rgb[id][2]=b;
 raydium_texture_rgb[id][3]=1.f;
 if(r<0 && g<0 && b<0)
    {
    raydium_texture_blended[id]=RAYDIUM_TEXTURE_PHANTOM;
    raydium_log("Texture num %i is Phantom (depth buffer only)",id);    
    }
 else
 {
    raydium_texture_blended[id]=0;
    raydium_log("Texture num %i, rgb(%f,%f,%f) is RGB Color",id,r,g,b);
 }
}
 
return id;
}

GLuint raydium_texture_load(char *filename)
{
GLuint res;
res=raydium_texture_load_internal(filename,filename,0,0,0,0,0);
if(res<=0)
    {
    raydium_log("texture: faking '%s' with pink color",filename);
    res=raydium_texture_load_internal("rgb(1,0,1)",filename,0,0,0,0,0);
    }
return res;
}


GLuint raydium_texture_load_erase(char *filename, GLuint to_replace)
{
if(to_replace<=0) return 0;
raydium_texture_to_replace=to_replace;
return raydium_texture_load(filename);
}


signed char raydium_texture_current_set(GLuint current)
{
if(current<RAYDIUM_MAX_TEXTURES)
{ raydium_texture_current_main=current; return current; }
return 0;
}

GLuint raydium_texture_find_by_name(char *name)
{
GLuint i;
char flag=0;
GLuint ret=0;

for(i=0;i<raydium_texture_index;i++)
if(!strcmp(raydium_texture_name[i],name)) { flag++; ret=i; }

if(!flag) ret=raydium_texture_load(name);
return ret;
}

GLuint raydium_texture_exists(char *name)
{
int i;

for(i=0;i<raydium_texture_index;i++)
if(!strcmp(raydium_texture_name[i],name))
    return i;

return -1;
}


signed char raydium_texture_current_set_name(char *name)
{
/*
GLuint i;
char flag=0;
char ret=0;

for(i=0;i<raydium_texture_index;i++)
if(!strcmp(raydium_texture_name[i],name)) { flag++; raydium_texture_current_set(i); ret=i; }

if(!flag) ret=raydium_texture_current_set(raydium_texture_load(name));
*/
return raydium_texture_current_set(raydium_texture_find_by_name(name));
}


void raydium_texture_filter_change(GLuint filter)
{
//GLuint i;
char force[RAYDIUM_MAX_NAME_LEN];

if(raydium_init_cli_option("filter",force))
    {
    if(!strcmp(force,"none")) filter=RAYDIUM_TEXTURE_FILTER_NONE;
    if(!strcmp(force,"bilinear")) filter=RAYDIUM_TEXTURE_FILTER_BILINEAR;
    if(!strcmp(force,"trilinear")) filter=RAYDIUM_TEXTURE_FILTER_TRILINEAR;
    }

raydium_texture_filter=filter;

// too ... dangerous
/*for(i=0;i<raydium_texture_index;i++)
    raydium_texture_load_erase(raydium_texture_name[i],i);*/

}

