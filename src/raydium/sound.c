/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/sound.h"
#endif

// There's way too much "#ifdef" in this file, but eh ... OpenAL becomes
// complex when you comes to portability ...

#ifndef ALUT_API_MAJOR_VERSION
#warning You must use OpenAL 1.1 or greater ! See configure script.
#endif

#ifndef ALchar
#define ALchar char
#endif

// music OGG infos

void raydium_sound_music_info_init(void)
{
strcpy(raydium_sound_music_info.artist,"Unkown artist");
strcpy(raydium_sound_music_info.album, "Unkown album" );
strcpy(raydium_sound_music_info.title, "Unkown title" );
}

void raydium_sound_music_info_refresh(void)
{
char **ptr;
char part1[RAYDIUM_MAX_NAME_LEN];
char part2[RAYDIUM_MAX_NAME_LEN];


// reset ogg infos
raydium_sound_music_info_init();

ptr=ov_comment(&raydium_sound_vf,-1)->user_comments;
while(*ptr)
    {
    part1[0]=0;
    part2[0]=0;
    raydium_parser_cut(*ptr,part1,part2,'=');
    
    if(!strcasecmp("title",part1))
	strcpy(raydium_sound_music_info.title,part2);

    if(!strcasecmp("album",part1))
	strcpy(raydium_sound_music_info.album,part2);

    if(!strcasecmp("artist",part1))
	strcpy(raydium_sound_music_info.artist,part2);

      ++ptr;
    }
}

// Sound core functions

//VERIFIES THAT THE LAST OPERATION DID NOT MAKE ANY ERROR
//IF AN ERROR IS DETECTED IT PRINTS THE caller NAME
void raydium_sound_verify(char *caller)
{
 int error;
 if ((error = alGetError()) != AL_NO_ERROR)
 {
  raydium_log("sound: ERROR : %s :%d",caller,error);
//  raydium_log("sound: Deleting buffers");
//   alDeleteBuffers(RAYDIUM_SOUND_NUM_BUFFERS,raydium_sound_buffer);
//  raydium_log("sound: Releasing OpenAL");
//   alutExit();
//   exit(1);
 }
}

int raydium_sound_Array3IsValid(ALfloat *a)
{
if( !raydium_trigo_isfloat(a[0]) ||
    !raydium_trigo_isfloat(a[1]) ||
    !raydium_trigo_isfloat(a[2]) )
	{
	raydium_log("sound : ERROR: invalid 3xALfloat array provided");
	return 0;
	}
return 1;
}

//INITS A SOURCE WITH DEFAULT VALUES
//TAKES A SOURCE NUMBER
void raydium_sound_InitSource(int src)
{
 ALfloat srcPos[]={ 0.0, 0.0, 0.0};
 ALfloat srcVel[]={ 0.0, 0.0, 0.0};

 alSourcef(raydium_sound_source[src],AL_PITCH,1.0f);                //SETS SOURCE PITCH
   raydium_sound_verify("setting source pitch");
 alSourcef(raydium_sound_source[src],AL_GAIN,1.0f);                 //SETS SOURCE GAIN
   raydium_sound_verify("setting source gain");
 alSourcef(raydium_sound_source[src],AL_REFERENCE_DISTANCE,raydium_sound_DefaultReferenceDistance);
   raydium_sound_verify("setting source reference distance");
 alSourcefv(raydium_sound_source[src],AL_POSITION,srcPos);          //SETS SOURCE POSITION
   raydium_sound_verify("setting source position");
 alSourcefv(raydium_sound_source[src],AL_VELOCITY,srcVel);          //SETS SOURCE VELOCITY
   raydium_sound_verify("setting source velocity");
 alSourcei(raydium_sound_source[src],AL_BUFFER,raydium_sound_buffer[src]);               //ATTACHS A SOURCE TO A BUFFER
   raydium_sound_verify("attaching source to buffer");
 alSourcei(raydium_sound_source[src],AL_LOOPING,AL_TRUE);           //SETS SOURCE LOOPING TO TRUE
   raydium_sound_verify("setting source loop state");

 raydium_sound_source_fade_factor[src]=0;
 raydium_sound_source_fade_tofile[src][0]=0;
}


//LOADS A WAV FILE
//RETURNS THE SOURCE NUMBER OR -1 IF IT FAILS
int raydium_sound_LoadWav(const char *fname)
{
 int snum;
 ALsizei size;
 ALenum format;
 ALvoid *data;
 FILE *fp;

#ifndef WIN32
#ifdef ALUT_API_MAJOR_VERSION
#define ALUT_API_MAJOR_VERSION_BUT_WIN32
#endif
#endif
    
#ifndef ALUT_API_MAJOR_VERSION_BUT_WIN32
 ALboolean boolean;
 ALsizei freq;
#else
 ALfloat freq;
#endif


 if(raydium_sound_top_buffer==RAYDIUM_SOUND_NUM_BUFFERS)
 {
  raydium_log("sound: ERROR loading %s no more buffers available",fname);
//  exit(1);
  return -1;
 }
 else
 {
  snum=raydium_sound_top_buffer;
 }

 fp=raydium_file_fopen((char *)fname,"r");
 if(fp==NULL)
 {
  raydium_log("sound: ERROR opening file %s",fname);
  return -1;
  //exit(1);
 }
 fclose(fp);

#ifdef ALUT_API_MAJOR_VERSION_BUT_WIN32
 data=alutLoadMemoryFromFile(fname,&format,&size,&freq);
     raydium_sound_verify("alutLoadMemoryFromFile");

 if(data)
 alBufferData(raydium_sound_buffer[snum],format,data,size,freq);
     raydium_sound_verify("alBufferData");

 if(data)
  free(data);

#else
  alutLoadWAVFile((ALbyte *)fname,&format,&data,&size,&freq,&boolean);
     raydium_sound_verify("alutLoadWAVFile");

 alBufferData(raydium_sound_buffer[snum],format,data,size,freq);
     raydium_sound_verify("alBufferData");

 alutUnloadWAV(format,data,size,freq);
     raydium_sound_verify("alutUnloadWAV");
#endif

 raydium_sound_top_buffer++;
 raydium_sound_InitSource(snum);
  return(snum);
}


//VERIFIES THAT THE SOURCE IS IN THE SOURCE ARRAY
//RETURNS 0 IF OK, -1 IF THERE'S AN ERROR
int raydium_sound_SourceVerify(int src)
{
 if(src<0 || src>=raydium_sound_top_buffer)
 {
  raydium_log("sound: source %d doesn't exist !",src);
  return(-1);
 }
 return(0);
}

//SETS THE LOOP ATTRIBUTE OF A SOURCE
int raydium_sound_SetSourceLoop(int src, signed char loop)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alSourcei(raydium_sound_source[src],AL_LOOPING,(loop?AL_TRUE:AL_FALSE));
  raydium_sound_verify("setting source loop");
 }
 return(result);
}

//GETS SOURCE PITCH
//TAKES A SOURCE NUMBER
//GIVES AN ALfloat
int raydium_sound_GetSourcePitch(int src, ALfloat *p)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcef(raydium_sound_source[src],AL_PITCH,p);
  raydium_sound_verify("getting source pitch");
 }
 return(result);
}


//SETS SOURCE PITCH
//TAKES A SOURCE NUMBER AND AN ALfloat
int raydium_sound_SetSourcePitch(int src, ALfloat p)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  if(p>2.0)
  {
   static signed char first=1;
   p=2.0;//CLIPPING
   if(first)
	{
	raydium_log("sound: Pitch Overflow, clipped to 2. Message will not be repeated !");
	first=0;
	}
  }
  if(p<=0.0)
  {
   p=0.1; // Seems ok :)
   raydium_log("sound: Tried to set negative or 0 Pitch , clipped to 0.1");
  }
  alSourcef(raydium_sound_source[src],AL_PITCH,p);
  raydium_sound_verify("setting source pitch");
 }
 return(result);
}


//GETS SOURCE GAIN
//TAKES A SOURCE NUMBER
//GIVES AN ALfloat
int raydium_sound_GetSourceGain(int src, ALfloat *g)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcef(raydium_sound_source[src],AL_GAIN,g);
  raydium_sound_verify("getting source gain");
 }
 return(result);
}


//SETS SOURCE GAIN
//TAKES A SOURCE NUMBER AND AN ALfloat
int raydium_sound_SetSourceGain(int src, ALfloat g)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  if(g<0.0)
  {
   g=0.0;
   raydium_log("sound: Tried to set negative Gain , clipped to 0");
  }
  alSourcef(raydium_sound_source[src],AL_GAIN,g);
  raydium_sound_verify("setting source gain");
 }
 return(result);
}

//SETS SOURCE POSITION
//TAKES A SOURCE NUMBER AND AN ALfloat ARRAY {x,y,z}
int raydium_sound_SetSourcePos(int src, ALfloat Pos[])
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0 && raydium_sound_Array3IsValid(Pos))
 {
  alSourcefv(raydium_sound_source[src],AL_POSITION,Pos);
  raydium_sound_verify("setting source position");
 }
 return(result);
}


int raydium_sound_SetSourcePosCamera(int src)
{
ALfloat p[3];
p[0]=raydium_camera_x;
p[1]=raydium_camera_y;
p[2]=raydium_camera_z;
return raydium_sound_SetSourcePos(src,p);
}

//GETS SOURCE POSITION
//TAKES A SOURCE NUMBER
//GIVES AN ALfloat ARRAY {x,y,z}
int raydium_sound_GetSourcePos(int src, ALfloat *Pos[] )
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcefv(raydium_sound_source[src],AL_POSITION,(ALfloat *)&(*Pos));
  raydium_sound_verify("getting source position");
 }
 return(result);
}


//SETS SOURCE DIRECTION
//TAKES A SOURCE NUMBER AND AN ALfloat ARRAY {x,y,z}
int raydium_sound_SetSourceDir(int src, ALfloat Dir[])
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0 && raydium_sound_Array3IsValid(Dir))
 {
  alSourcefv(raydium_sound_source[src],AL_DIRECTION,Dir);
  raydium_sound_verify("setting source direction");
 }
 return(result);
}

//GETS SOURCE DIRECTION
//TAKES A SOURCE NUMBER
//GIVES AN ALfloat ARRAY {x,y,z}
int raydium_sound_GetSourceDir(int src, ALfloat *Dir[] )
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcefv(raydium_sound_source[src],AL_DIRECTION,(ALfloat *)&(*Dir));
  raydium_sound_verify("getting source direction");
 }
 return(result);
}


//SETS SOURCE VELOCITY
//TAKES A SOURCE NUMBER AND AN ALfloat ARRAY {x,y,z}
int raydium_sound_SetSourceVel(int src, ALfloat Vel[])
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0 && raydium_sound_Array3IsValid(Vel))
 {
  alSourcefv(raydium_sound_source[src],AL_VELOCITY,Vel);
  raydium_sound_verify("setting source velocity");
 }
 return(result);
}

//GETS SOURCE VELOCITY
//TAKES A SOURCE NUMBER
//GIVES AN ALfloat ARRAY {x,y,z}
int raydium_sound_GetSourceVel(int src, ALfloat *Vel[] )
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcefv(raydium_sound_source[src],AL_VELOCITY,(ALfloat *)&(*Vel));
  raydium_sound_verify("getting source velocity");
 }
 return(result);
}



//SETS LISTENER POSITION
//TAKES AN ALfloat ARRAY {x,y,z}
void raydium_sound_SetListenerPos(ALfloat Pos[])
{
 if(!raydium_sound_Array3IsValid(Pos)) return;
 
 alListenerfv(AL_POSITION,Pos);
   raydium_sound_verify("setting listener position");
}

//GETS LISTENER POSITION
//GIVES AN ALfloat ARRAY {x,y,z}
void raydium_sound_GetListenerPos(ALfloat *Pos[] )
{
 alGetListenerfv(AL_POSITION,(ALfloat *)&(*Pos));
   raydium_sound_verify("getting listener position");
}

//SETS LISTENER ORIENTATION
//TAKES AN ALfloat ARRAY {x,y,z,x2,y2,z2}
//                        ATvector UPvector
void raydium_sound_SetListenerOr(ALfloat Or[])
{
 if(!raydium_sound_Array3IsValid(Or)) return;
 if(!raydium_sound_Array3IsValid(Or+3)) return;
 
 alListenerfv(AL_ORIENTATION,Or);
   raydium_sound_verify("setting listener orientation");
}

//GETS LISTENER ORIENTATION
//GIVES AN ALfloat ARRAY {x,y,z,x2,y2,z2}
//                        ATvector UPvector
void raydium_sound_GetListenerOr(ALfloat *Or[] )
{
 
 alGetListenerfv(AL_ORIENTATION,(ALfloat *)&(*Or));
   raydium_sound_verify("getting listener orientation");
}

//SETS LISTENER VELOCITY
//TAKES AN ALfloat ARRAY {x,y,z}
void raydium_sound_SetListenerVel(ALfloat Vel[])
{
if(!raydium_sound_Array3IsValid(Vel)) return;

 alListenerfv(AL_VELOCITY,Vel);
   raydium_sound_verify("setting listener velocity");
}

//GETS LISTENER VELOCITY
//GIVES AN ALfloat ARRAY {x,y,z}
void raydium_sound_GetListenerVel(ALfloat *Vel[] )
{
 alGetListenerfv(AL_VELOCITY,(ALfloat *)&(*Vel));
   raydium_sound_verify("getting listener velocity");
}



//INITIALISATION
void raydium_sound_init(void)
{
 int i;
 ALfloat listenerPos[]={-10.0,0.0,0.0};
 ALfloat listenerVel[]={0.0,0.0,0.0};
// ALfloat back[] ={ 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
 ALfloat front[]={ 1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f };
#ifdef ALUT_API_MAJOR_VERSION
 const ALchar *tempString;
 ALCdevice* pDevice;
 ALCcontext* pContext;
#endif
 
#ifdef NO_SOUND_DEBUG
return;
#endif

#ifndef WIN32
// I've noticed problems with "manual contexts" under Linux, so let's use ALUT
#ifdef ALUT_API_MAJOR_VERSION
if(!alutInit(&raydium_init_argc, raydium_init_argv))
    {
    alGetError();
    raydium_log("ERROR: Cannot open Sound System");
    raydium_sound=0;
    return;
    }
#else
 alutInit(&raydium_init_argc, raydium_init_argv);
#endif

#else
// With win32, alut may do "bad things" while chosing device ... let's force :
 pDevice = alcOpenDevice("Generic Software"); // or any asked device name ?
 pContext=alcCreateContext(pDevice,NULL);
 alcMakeContextCurrent(pContext);
 alutInitWithoutContext(&raydium_init_argc, raydium_init_argv);
#endif
 
//alutInit(0, NULL) ;
 alGetError();

 alListenerfv(AL_POSITION,listenerPos);
 alListenerfv(AL_VELOCITY,listenerVel);
 alListenerfv(AL_ORIENTATION,front);
 alDistanceModel(AL_INVERSE_DISTANCE);
//#ifdef __linux
 alGetError(); /* clear error */
 alGenBuffers(RAYDIUM_SOUND_NUM_BUFFERS, raydium_sound_buffer);
 raydium_sound_verify("alGenBuffers");
//#else
//alGenBuffers(RAYDIUM_SOUND_NUM_BUFFERS,raydium_sound_buffer);
// if (!raydium_sound_buffer)
// {
//  raydium_log("sound: Error creating buffers !!");
//  exit(1);
// }
//#endif

 raydium_log("sound: Buffer creation successfull");

//#ifdef __linux
 alGetError(); /* clear error */
 alGenSources(RAYDIUM_SOUND_NUM_SOURCES, raydium_sound_source);
 raydium_sound_verify("alGenSources");
//#else
// raydium_sound_verify("alGenSources");
//#endif
 raydium_sound_top_buffer=2; //first available buffer (2 first are used by music)
 raydium_sound=1;
 raydium_sound_music_file=NULL;
 raydium_sound_DefaultReferenceDistance=50.f; // default distance where the source sound is half volume
 raydium_sound_music_eof_callback=NULL;
 raydium_sound_music_changed_callback=NULL;

 for(i=0;i<RAYDIUM_SOUND_NUM_BUFFERS;i++)
    raydium_sound_source_fade_factor[i]=0;

#ifdef ALUT_API_MAJOR_VERSION
 pContext = alcGetCurrentContext();
 pDevice = alcGetContextsDevice(pContext);
 tempString = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);
 raydium_log("sound: OK, using '%s'",(strlen(tempString)?tempString:"default device"));
#else
 raydium_log("sound: OK");
#endif
/* 
{
		const ALchar *tempString;

		ALCcontext* pContext;
		ALCdevice* pDevice;
		pContext = alcGetCurrentContext();
		pDevice = alcGetContextsDevice(pContext);
		tempString = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);
		printf("OpenAL Context Device Specifier is '%s'\n", tempString);
		tempString = alGetString(AL_RENDERER);
		printf("OpenAL Renderer is '%s'\n", tempString);
		tempString = alGetString(AL_VERSION);
		printf("OpenAL Version is '%s'\n", tempString);
		tempString = alGetString(AL_VENDOR);
		printf("OpenAL Vendor is '%s'\n", tempString);
		tempString = alGetString(AL_EXTENSIONS);
		printf("OpenAL Extensions supported are :\n%s\n", tempString);
}    
*/ 
raydium_sound_music_info_init();
}

//PLAYS A SOURCE
int raydium_sound_SourcePlay(int src)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alSourcePlay(raydium_sound_source[src]);
 }
 return(result);
}

//STOPS A SOURCE
int raydium_sound_SourceStop(int src)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alSourceStop(raydium_sound_source[src]);
 }
 return(result);
}

//PAUSE A SOURCE
int raydium_sound_SourcePause(int src)
{
 int result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alSourcePause(raydium_sound_source[src]);
 }
 return(result);
}


// RETURNS PLAYING STATE
signed char raydium_sound_IsPlaying(int src)
{
 int result;
 int sourcestate;

 result=raydium_sound_SourceVerify(src);

 if(result!=0)
    return 0;

 alGetSourcei(raydium_sound_source[src],AL_SOURCE_STATE,&sourcestate);
 return sourcestate==AL_PLAYING;
}


//UNPAUSE A SOURCE
int raydium_sound_SourceUnpause(int src)
{
 int result;
 int state;
 result=raydium_sound_SourceVerify(src);
 if(result==0)
 {
  alGetSourcei(raydium_sound_source[src], AL_SOURCE_STATE, &state);
  if(state==AL_PAUSED)
    alSourcePlay(raydium_sound_source[src]);
 }
 return(result);
}


//THINGS TO DO WHEN EXITING
void raydium_sound_close(void)
{
int i;
#ifdef WIN32
ALCdevice* pDevice;
ALCcontext* pContext;
#endif
    
 if(raydium_sound==1) // WE DO THESE THINGS ONLY IF OPEN AL INIT WAS OK
 {
  for(i=0;i<raydium_sound_top_buffer;i++)
    raydium_sound_SourceStop(i);
  raydium_log("sound: Deleting sources");
   alDeleteSources(RAYDIUM_SOUND_NUM_SOURCES,raydium_sound_source);
  raydium_log("sound: Deleting buffers");
   alDeleteBuffers(RAYDIUM_SOUND_NUM_BUFFERS,raydium_sound_buffer);
  raydium_log("sound: Releasing OpenAL");
   

#ifdef WIN32
  	//Get active context
	pContext=alcGetCurrentContext();
	//Get device for active context
	pDevice=alcGetContextsDevice(pContext);
	//Release context(s)
    alcMakeContextCurrent(NULL);
    alcDestroyContext(pContext);
	//Close device
	alcCloseDevice(pDevice);
#endif
  alutExit();
 }
}


int BufferData(ALuint buffer,OggVorbis_File *file,vorbis_info *ogginfo) 
{
  unsigned int count=0;
  int stream;
  int amt;
  ALint format;

  while(count<SOUNDDATASIZE)
  {
    amt=ov_read(file,&(raydium_sound_music_buf[count]),
          SOUNDDATASIZE-count,
          0,2,1,&stream);
    if(amt<=0)
    {
//      if(amt<0)fprintf(stderr,"ov_read error: ");
      raydium_log("sound: ov_read error");
      break;
    }
    count+=amt;
  }

  if(!count)
    return 0;

  if(ogginfo->channels==1)
    format=AL_FORMAT_MONO16;
  else
    format=AL_FORMAT_STEREO16;

  alBufferData(buffer,format,raydium_sound_music_buf,count,ogginfo->rate);
  //printf("buffered to %i buffer\n",buffer);
  return 1;
}

void raydium_sound_internal_cleanstreambuffs(void)
{
    ALint lProcessed;
    ALuint TempBufferID;

    alGetSourcei(raydium_sound_source[0], AL_BUFFERS_PROCESSED, &lProcessed);
    //printf("%i -----\n",lProcessed);
			while (lProcessed)
			{
                //printf(".");
				alSourceUnqueueBuffers(raydium_sound_source[0], 1, &TempBufferID);
                lProcessed--;
			}
}


int StartMusic(ALuint musicsource,ALuint *buffers,OggVorbis_File *file,
      vorbis_info *ogginfo)
{
  int ok;
  alSourceStop(musicsource);
  raydium_sound_internal_cleanstreambuffs();
  memset(raydium_sound_music_buf,0,SOUNDDATASIZE);
  ok=BufferData(raydium_sound_buffer[0],file,ogginfo) 
     && BufferData(raydium_sound_buffer[1],file,ogginfo);
  alSourceQueueBuffers(musicsource,2,raydium_sound_buffer);
  alSourcePlay(musicsource);
  return ok;
} 



//try to open music
int raydium_sound_load_music(char *fname)
{
#ifdef NO_SOUND_DEBUG
return -1;
#endif
if(!raydium_sound)
    return -1;



 if(raydium_sound_music_file) fclose(raydium_sound_music_file);
 raydium_sound_music_file=NULL;
 
 if(fname==NULL || strlen(fname)==0)
    {
    // seems to fail ... :/
    //raydium_sound_internal_cleanstreambuffs();
    return 0;
    }
 
 raydium_sound_music_file = raydium_file_fopen( fname, "rb" );
 if(raydium_sound_music_file == NULL)
 {
  raydium_log("sound: Could not open %s", fname);
  perror("raydium_sound_load_music");
  return(-1);
 }

 alSourcei( raydium_sound_source[0], AL_SOURCE_RELATIVE, AL_TRUE );
 alSourcei( raydium_sound_source[0], AL_LOOPING, AL_FALSE );
 raydium_sound_SetSourceGain(0,1);


 if(ov_open(raydium_sound_music_file, &raydium_sound_vf, NULL, 0) < 0)
 {
  raydium_log("ERROR: Failed to open %s as vorbis",fname);
  return(-1);
 }
 raydium_sound_ogginfo=ov_info(&raydium_sound_vf,-1);
 raydium_sound_music_info_refresh();
 
 if(raydium_sound_music_changed_callback)
    raydium_sound_music_changed_callback();
 
// size = bits/8 * ov_info(&raydium_sound_vf, 0)->channels; 
StartMusic(raydium_sound_source[0],raydium_sound_buffer,
           &raydium_sound_vf,raydium_sound_ogginfo);
 
return(0);
}


void raydium_sound_music_callback(void)
{
ALint nprocessed;
ALuint buffer;
ALint sourcestate;
char newfile[RAYDIUM_MAX_NAME_LEN];

 if(raydium_sound_music_file==NULL) return;

 if(feof(raydium_sound_music_file))
    {
    fseek(raydium_sound_music_file,0,SEEK_SET);//rewind in the file if we hit the end
    raydium_log("sound: end of file reached");
    if(raydium_sound_music_eof_callback && raydium_sound_music_eof_callback(newfile)>0)
	{
	if(strlen(newfile)) 
	    raydium_sound_load_music(newfile);
	else
	    {
	    // may cause glitches on next music.. ?
	    //fclose(raydium_sound_music_file);
	    //raydium_sound_music_file=NULL;
	    raydium_sound_load_music(NULL);
	    }
	return ;
	}
    }

 alGetSourcei(raydium_sound_source[0],AL_BUFFERS_PROCESSED,&nprocessed);
 while(nprocessed)
 {
  static int last;
     alSourceUnqueueBuffers(raydium_sound_source[0],1,&buffer);
        //printf("%i, buff = %i\n",nprocessed,buffer);
  if(buffer==last)
    {
    alSourceStop(raydium_sound_source[0]);
    }
    else  
    {
    last=buffer;
    BufferData(buffer,&raydium_sound_vf,raydium_sound_ogginfo);
    alSourceQueueBuffers(raydium_sound_source[0],1,&buffer);
    nprocessed--;
    }
 alGetSourcei(raydium_sound_source[0],AL_BUFFERS_PROCESSED,&nprocessed);
 } 

 //restart playing if needed
 alGetSourcei(raydium_sound_source[0],AL_SOURCE_STATE,&sourcestate);
 if(sourcestate!=AL_PLAYING)
 {
    raydium_sound_internal_cleanstreambuffs();
    alSourcePlay(raydium_sound_source[0]);
    alGetSourcei(raydium_sound_source[0],AL_SOURCE_STATE,&sourcestate);
     if(sourcestate!=AL_PLAYING)
        StartMusic(raydium_sound_source[0],raydium_sound_buffer,
           &raydium_sound_vf,raydium_sound_ogginfo);         
 } 

}

void raydium_sound_callback(void)
{
int i;
ALfloat g;
raydium_sound_music_callback();

// source fading ?
for(i=0;i<raydium_sound_top_buffer;i++)
    if(raydium_sound_source_fade_factor[i]!=0)
	{
	raydium_sound_GetSourceGain(i,&g);
	g+=(raydium_sound_source_fade_factor[i]*raydium_frame_time);
	raydium_sound_SetSourceGain(i,g);
	if(g<=0)
	    {
	    raydium_sound_source_fade_factor[i]=0;
	    
	    if(i==0) // music source
		{
		if(raydium_sound_source_fade_tofile[i][0]==0)
		    {
		    raydium_sound_load_music(NULL);
		    }
		else
		    {
		    raydium_sound_load_music(raydium_sound_source_fade_tofile[i]);
		    raydium_sound_source_fade_tofile[i][0]=0;
		    }
		}
	    else
		    raydium_sound_SourceStop(i);
	    }
	}
}

void raydium_sound_source_fade(int src, ALfloat len)
{
ALfloat g;
// get current gain
raydium_sound_GetSourceGain(src,&g);
raydium_sound_source_fade_factor[src]=-(g/len);
}

void raydium_sound_source_fade_to(int src, ALfloat len, char *to)
{
raydium_sound_source_fade(src,len);
strcpy(raydium_sound_source_fade_tofile[src],to);
}
