#ifndef _SOUND_H
#define _SOUND_H
/*=
Sound and music
2600
**/

// Introduction
/**
The Raydium sound API is pretty easy to use and there's only need to use a 
few functions to make your program ouput sounds or music.

On top of this, there are a bunch of functions to modify the sound behavior.

Raydium uses OpenAL and OggVorbis for its sounds and musics, for a basic 
use of our sound API you only need to know one thing: OpenAL uses buffers 
for its sounds and you need to be able to address the sounds separately.
For this we use ALuint in our code. Each buffer is associated to a source,
we have an array of all available sources and then, you only need to have 
a simple int that acts as an index in this array. See below for more
informations.

Music is readed thru libogg, streamed from disk. If you want to play an
OGG audio track, the only thing you've to do is to call the suitable function.
You can use ##raydium_sound_music_eof_callback## if needed. This event is
fired when sound track ends, allowing you to switch to another file.
Prototype for this callback is ##int callback(char *new_track)##, allowing
you to do something like ##strcpy(new_track,"foobar.ogg"); return 1;##.
Return 0 if you do not want to switch to another audio file (this will stops
music playback).
Another callback is available, ##raydium_sound_music_changed_callback##, fired
just after a music track switch, allowing you to get new informations from the
new stream, such as artist, album and title. See ##raydium_sound_load_music()##
for more informations about this.

This document is not an alternative to OpenAL papers, and only provides
informations about Raydium's interface to OpenAL.
See specifications here: http://www.openal.org/documentation.html
**/

__rayapi void raydium_sound_verify (char *caller);
/**
This functions checks if any error occured during last OpenAL operation.
You don't have to call this function by yourself, since every function of
this API will do it.
**/

__rayapi int raydium_sound_Array3IsValid(ALfloat *a);
/**
Since OpenAL is very sensitive to malformed values, this function is used
internally to check consistency of provided ALfloat arrays.
**/

__rayapi void raydium_sound_InitSource (int src);
/**
Internal use.
**/

__rayapi int raydium_sound_LoadWav (const char *fname);
/**
This function tries to load the ##fname## wav file into a buffer, if 
successful, it returns the source id, else 0.
**/

__rayapi int raydium_sound_SourceVerify (int src);
/**
Internal id checks.
**/

__rayapi int raydium_sound_SetSourceLoop (int src, signed char loop);
/**
Modifies the ##loop## property of the ##src## source (loops if loop is non-zero, 
default value for a source is "true").
Returns 0 if ok, -1 if error.
**/

__rayapi int raydium_sound_GetSourcePitch (int src, ALfloat * p);
/**
Returns current pitch for ##src## source.
**/

__rayapi int raydium_sound_SetSourcePitch (int src, ALfloat p);
/**
Sets pitch for ##src## source.
Current OpenAL spec is not clear about pitch's limits. Raydium will 
clamp values to to ]0,2] interval.
**/

__rayapi int raydium_sound_GetSourceGain (int src, ALfloat * g);
/**
Returns current gain ("volume") for ##src## source.
**/

__rayapi int raydium_sound_SetSourceGain (int src, ALfloat g);
/**
Sets gain ("volume") for ##src## source.
Current OpenAL spec is not clear about pitch's limits. Raydium do not allows
negative values, but no upper limit is set.
Warning: some OpenAL implementations will provide strange gain curves. More
work is needed on this issue.
**/

__rayapi int raydium_sound_SetSourcePos (int src, ALfloat Pos[]);
/**
Sets 3D position of ##src## source.
##Pos## is a 3 * ALfloat array.
**/

__rayapi int raydium_sound_SetSourcePosCamera(int src);
/**
Sets 3D position of ##src## source on the current camera position.
**/

__rayapi int raydium_sound_GetSourcePos (int src, ALfloat * Pos[]);
/**
Returns current 3D position of ##src## source.
##Pos## is a 3 * ALfloat array.
**/

__rayapi int raydium_sound_SetSourceDir (int src, ALfloat Dir[]);
/**
Sets 3D direction of ##src## source.
##Dir## is a 3 * ALfloat array.
**/

__rayapi int raydium_sound_GetSourceDir (int src, ALfloat * Dir[]);
/**
Returns current 3D direction of ##src## source.
##Dir## is a 3 * ALfloat array.
**/

__rayapi int raydium_sound_SetSourceVel (int src, ALfloat Vel[]);
/**
Sets 3D velocity of ##src## source.
##Vel## is a 3 * ALfloat array.
**/

__rayapi int raydium_sound_GetSourceVel (int src, ALfloat * Vel[]);
/**
Returns current 3D velocity of ##src## source.
##Vel## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_SetListenerPos (ALfloat Pos[]);
/**
Sets 3D position of listener.
This is done automatically by Raydium, each frame, using camera informations
##Pos## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_GetListenerPos (ALfloat * Pos[]);
/**
Returns current 3D position of listener.
##Pos## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_SetListenerOr (ALfloat Or[]);
/**
Sets 3D orientation of listener.
This is done automatically by Raydium, each frame, using camera informations
##Or## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_GetListenerOr (ALfloat * Or[]);
/**
Returns current 3D orientation of listener.
##Or## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_SetListenerVel (ALfloat Vel[]);
/**
Sets 3D velocity of Listener.
##Vel## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_GetListenerVel (ALfloat * Vel[]);
/**
Returns current 3D velocity of Listener.
##Vel## is a 3 * ALfloat array.
**/

__rayapi void raydium_sound_init (void);
/**
Internal use.
**/

__rayapi int raydium_sound_SourcePlay (int src);
/**
Plays the ##src## source.
If ##src## was already in "play" state, the buffer is rewinded.
Returns 0 if ok, -1 if error.
**/

__rayapi int raydium_sound_SourceStop (int src);
/**
Stops the ##src## source.
Returns 0 if ok, -1 if error.
**/

__rayapi int raydium_sound_SourcePause (int src);
/**
Will pause the ##src## source.
Returns 0 if ok, -1 if error.
**/

__rayapi int raydium_sound_SourceUnpause (int src);
/**
##src## will restart playback after being paused.
Returns 0 if ok, -1 if error.
**/

__rayapi signed char raydium_sound_IsPlaying(int src);
/**
Returns true (1) if ##src## is playing, false (0) if stopped or invalid.
**/


__rayapi void raydium_sound_close (void);
/**
Internal use.
**/

__rayapi int BufferData (ALuint buffer, OggVorbis_File * file, vorbis_info * ogginfo);
__rayapi void raydium_sound_internal_cleanstreambuffs (void);
__rayapi int StartMusic (ALuint musicsource, ALuint * buffers, OggVorbis_File * file, vorbis_info * ogginfo);
__rayapi int raydium_sound_load_music (char *fname);
/**
Opens fname **OGG** music file and prepairs Raydium for playing it.
The music will be automatically played after a call to this function.
This function will use R3S (data repositories) if needed.
To switch to another audio track, simply call again this function.
Send ##NULL## or an empty string to cancel music playback.
Returns 0 if ok, -1 if error

See also ##raydium_sound_music_eof_callback## at the top of this chapter.

You can get OGG informations from ##raydium_sound_music_info##, using
its members:
%%(c)
char artist[RAYDIUM_MAX_NAME_LEN];
char title [RAYDIUM_MAX_NAME_LEN];
char album [RAYDIUM_MAX_NAME_LEN];
%%
**/

__rayapi void raydium_sound_music_info_init(void);
/**
Internal use. Will reset infos.
**/

__rayapi void raydium_sound_music_info_refresh(void);
/**
Internal use. Will flush infos from disk to ##raydium_sound_music_info##.
**/


__rayapi void raydium_sound_music_callback (void);
/**
Internal use.
**/

__rayapi void raydium_sound_callback (void);
/**
Internal use.
**/

__rayapi void raydium_sound_source_fade(int src, ALfloat len);
/**
This function will fade down source ##src## over ##len## seconds.
Since gain is not linear, you may have to play a bit with ##len## to
find the correct value for you.
Use source 0 for music source.
**/

// Sound API Example
/**
%%(c)
int sound;
sound=raydium_sound_LoadWav("explo.wav");
raydium_sound_SetSourceLoop(sound,0);
[...]
if(explosion) raydium_sound_SourcePlay(sound);
%%
**/

__rayapi void raydium_sound_source_fade_to(int src, ALfloat len, char *to);
/**
Same as above, but plays ##to## file at the end of the fade.
Warning: Works only for "music" source (##src## = 0).
**/

#endif
