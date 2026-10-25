/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/timecall.h"
#endif

#ifdef WIN32
#define __GETTIMEOFDAY_USEC 1000
#else
#define __GETTIMEOFDAY_USEC 1000000
#endif

// needed proto
int raydium_timecall_add(void *funct, GLint hz);


void raydium_timecall_raydium(GLfloat step)
{
raydium_frame_time=step;
}

#ifdef WIN32
float raydium_timecall_internal_w32_detect_modulo(int div)
{
 LARGE_INTEGER t;
 unsigned long mx;
 
 QueryPerformanceFrequency(&t);
 t.QuadPart >>= div;
 mx=(0xFFFFFFFF / t.LowPart); 
 return mx/60.f;
}


int raydium_timecall_internal_w32_divmodulo_find(void)
{
float modulo_time;
int div;

div=-1;
do{
div++;
modulo_time=raydium_timecall_internal_w32_detect_modulo(div);
}while(modulo_time<RAYDIUM_TIMECALL_W32_MODULO_MIN);

raydium_log("timecall: win32 modulo every %.2f minutes, modulodiv is 2^%i",modulo_time,div);
return div;
}
#endif


unsigned long raydium_timecall_devrtc_clock(void)
{
#ifndef WIN32
struct timeval tv={0, 0};
fd_set readfds;
int ret;
unsigned long data,missed;

FD_ZERO(&readfds);
FD_SET(raydium_timecall_devrtc_handle, &readfds);
if( (ret=select(raydium_timecall_devrtc_handle+1, &readfds, NULL, NULL, &tv)) == -1)
    {
    raydium_log("timecall: ERROR: selecting /dev/rtc failed at runtime");
    perror("system");
    }

// IRQ fired !
if(ret>0)
 {
        if( read(raydium_timecall_devrtc_handle, &data, sizeof(unsigned long)) == -1)
	{
	raydium_log("timecall: ERROR: reading /dev/rtc failed at runtime");
        perror("system");
        }
	else 
	{
	// read first 3 bytes only
	missed=(data & 0xffffff00UL)>>8;
	raydium_timecall_devrtc_clocks+=missed;
//	raydium_log("%i",raydium_timecall_devrtc_clocks);
	}
 }
 
return raydium_timecall_devrtc_clocks;
#else
return 0;
#endif
}

unsigned long raydium_timecall_clock(void)
{
struct timeval tv;
if(raydium_timecall_method==RAYDIUM_TIMECALL_METHOD_CLOCK)
 {
 #ifdef WIN32
  {
// return GetTickCount();
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    t.QuadPart>>=raydium_timecall_w32_divmodulo;
    return t.LowPart;
  }
 #else
 gettimeofday(&tv,NULL);
 return (tv.tv_sec*1000000 + tv.tv_usec);
 #endif
 //return clock();
 }
//else if(raydium_timecall_method==RAYDIUM_TIMECALL_METHOD_DEVRTC)
 return raydium_timecall_devrtc_clock();
}

signed char raydium_timecall_devrtc_rate_change(unsigned long new)
{
#ifndef WIN32
        if(ioctl(raydium_timecall_devrtc_handle, RTC_IRQP_SET, new)==-1)
	{
		raydium_log("timecall: ERROR: changing /dev/rtc rate to %lu Hz failed !",new);
		raydium_log("timecall: is /proc/sys/dev/rtc/max-user-freq correct for this value ?");
                perror("system");
//                exit(errno);
                return 0;
        }
	raydium_log("timecall: /dev/rtc rate changed to %lu Hz",new);
	return 1;
#else
return 0;
#endif
}

void raydium_timecall_devrtc_close(void)
{
#ifndef WIN32
        if(ioctl(raydium_timecall_devrtc_handle, RTC_PIE_OFF, 0) == -1)
	{
		raydium_log("timecall: ERROR disabling /dev/rtc periodic interrupts");
                perror("system");
//                exit(errno);
        }
close(raydium_timecall_devrtc_handle);
#endif
}


unsigned long raydium_timecall_devrtc_init(void)
{
#ifndef WIN32
 unsigned long freq;
 raydium_timecall_devrtc_clocks=0;
 
 if((raydium_timecall_devrtc_handle = open ("/dev/rtc", O_RDONLY)) == -1 )
 {
  raydium_log("timecall: ERROR: /dev/rtc unavailable ! (chmod a+rx /dev/rtc ?)");
  perror("system");
//  exit(errno);
  return 0;
 }

 // ok, it's now open, so let's read actual rate
 if (ioctl(raydium_timecall_devrtc_handle, RTC_IRQP_READ, &freq) == -1 )
 {
  raydium_log("timecall: ERROR reading /dev/rtc rate");
  perror("system");
//  exit(errno);
  return 0;
 }
 raydium_log("timecall: /dev/rtc rate is %lu Hz",freq);

 if(freq<RAYDIUM_TIMECALL_FREQ_MIN)
  {
   raydium_log("timecall: /dev/rtc rate (%i Hz) too low (min: %i)",freq,RAYDIUM_TIMECALL_FREQ_MIN);
   if(!raydium_timecall_devrtc_rate_change(RAYDIUM_TIMECALL_FREQ_PREFERED)) return 0;
   else freq=RAYDIUM_TIMECALL_FREQ_PREFERED; // need to verify this new value ?
  }
 
 if(freq<RAYDIUM_TIMECALL_FREQ_PREFERED)
  {
   raydium_log("timecall: /dev/rtc rate (%i Hz) is low (prefered: %i)",freq,RAYDIUM_TIMECALL_FREQ_PREFERED);
   if(raydium_timecall_devrtc_rate_change(RAYDIUM_TIMECALL_FREQ_PREFERED)) 
      freq=RAYDIUM_TIMECALL_FREQ_PREFERED; // need to verify this new value ?
  }

    // Enable periodic interrupts
    if(ioctl(raydium_timecall_devrtc_handle, RTC_PIE_ON, 0) == -1)
    {
	    raydium_log("timecall: ERROR enabling /dev/rtc periodic interrupts !");
	    raydium_log("timecall: is /proc/sys/dev/rtc/max-user-freq allowing %lu Hz ?",freq);
            perror("system");
//            exit(errno);
	    return 0;
    }

raydium_atexit(raydium_timecall_devrtc_close);
return freq;
#else
raydium_log("timecall: FAILED: /dev/rtc only available for Linux");
return 0;
#endif
}

/*
int raydium_timecall_detect_max_clock_frequency(void)
{
int i;
clock_t first,second;
float accu,max;

first=second=clock();
while(first==second)
{
i++;
second=clock();
}

accu=((second-first)/(float)CLOCKS_PER_SEC)*1000;
max=1.0/(accu/1000);
raydium_log("timecall: clock() accuracy = %.2f ms (%.2f Hz)",accu,max);
return (int)max;
}
*/

int raydium_timecall_detect_frequency(void)
{
int i=0;
unsigned long first,second;
float accu,max;

first=second=raydium_timecall_clock();
while(first==second)
{
i++;
second=raydium_timecall_clock();
}

raydium_log("timer: detection: %li iterations: diff: %li steps (%li/sec)",i,second-first,raydium_timecall_clocks_per_sec);
accu=((second-first)/(float)raydium_timecall_clocks_per_sec)*1000;
max=1.0/(accu/1000);
raydium_log("timecall: method accuracy = %.3f ms (%.2f Hz)",accu,max);
return (int)max;
}


void raydium_timecall_init(void)
{
int i;
unsigned long tmp;
#ifdef WIN32
LARGE_INTEGER t;
#endif

//default
raydium_timecall_method=RAYDIUM_TIMECALL_METHOD_CLOCK;
raydium_timecall_clocks_per_sec=__GETTIMEOFDAY_USEC;

#ifdef WIN32
raydium_timecall_w32_divmodulo=raydium_timecall_internal_w32_divmodulo_find();
QueryPerformanceFrequency(&t);
t.QuadPart>>=raydium_timecall_w32_divmodulo;
raydium_timecall_clocks_per_sec=t.LowPart;
#endif

raydium_timecall_max_frequency=raydium_timecall_detect_frequency();


if(raydium_timecall_max_frequency<RAYDIUM_TIMECALL_FREQ_PREFERED)
{
raydium_log("timecall: basic method accuracy is low , trying /dev/rtc ...");
tmp=raydium_timecall_devrtc_init();
if(tmp) {
	raydium_timecall_method=RAYDIUM_TIMECALL_METHOD_DEVRTC;
	raydium_timecall_max_frequency=tmp;
	}
}

if(raydium_timecall_method==RAYDIUM_TIMECALL_METHOD_CLOCK)
{
raydium_log("timecall: Using basic gettimeofday() method");
raydium_timecall_clocks_per_sec=__GETTIMEOFDAY_USEC;
#ifdef WIN32
//QueryPerformanceFrequency(&t);
raydium_timecall_clocks_per_sec=t.LowPart;
//printf("%i %i %i\n",t.LowPart,t.HighPart,t.QuadPart);
#endif

}

if(raydium_timecall_method==RAYDIUM_TIMECALL_METHOD_DEVRTC)
{
raydium_log("timecall: Using /dev/rtc method");
raydium_timecall_clocks_per_sec=raydium_timecall_max_frequency;
//raydium_timecall_method_test();
}

raydium_timecall_index=0;
for(i=0;i<RAYDIUM_MAX_TIMECALLS;i++)
    {
    raydium_timecall_funct[i]=NULL;
    raydium_timecall_soft_call[i]=0;
    raydium_timecall_interval[i]=0;
    raydium_timecall_next[i]=0;
    }
raydium_log("timecall: OK (%lu Hz)",raydium_timecall_max_frequency);
raydium_timecall_add(raydium_timecall_raydium,-1);
}


// Utility function only: No test is done here, please, secure your calls
void raydium_timecall_freq_change(int callback, GLint hz)
{
if(!hz)
 {
 raydium_log("timecall: WARNING ! 0 Hz callback (num %i)",callback);
 raydium_timecall_interval[callback]=0;
 }
else
 raydium_timecall_interval[callback]=raydium_timecall_clocks_per_sec/abs(hz);

raydium_timecall_soft_call[callback]=(hz<0);
raydium_timecall_next[callback]=raydium_timecall_clock();

if(abs(hz)>raydium_timecall_max_frequency && hz>0)
 raydium_log("timecall: WARNING ! this callback refresh rate (%i Hz) is probably too high for this system clock (detected at %i Hz)",hz,raydium_timecall_max_frequency);

if(hz>0)
 raydium_log("timecall: callback %i: %i Hz (%i clocks interval)",callback,hz,raydium_timecall_interval[callback]);

if(hz<0)
 raydium_log("timecall: softcall %i: %i Hz (%i clocks interval)",callback,-hz,raydium_timecall_interval[callback]);
}

int raydium_timecall_add(void *funct, GLint hz)
{
if(raydium_timecall_index>=RAYDIUM_MAX_TIMECALLS)
    {
    raydium_log("timecall: ERROR ! Too much timecalls, exiting.");
    exit(29);
    }

raydium_timecall_funct[raydium_timecall_index]=funct;
raydium_timecall_freq_change(raydium_timecall_index,hz);
return raydium_timecall_index++;
}



void raydium_timecall_callback(void)
{
int i,j,steps;
GLfloat stepsf;
unsigned long now,phase;
static unsigned long last;
void (*f)();
void (*ff)(GLfloat);

// workaround for time modulos
now=raydium_timecall_clock();
if(last>now)
    {
    raydium_log("timecall: warning: time modulo detected: workarounding");
    for(i=0;i<raydium_timecall_index;i++)
        if(!raydium_timecall_soft_call[i] && 
            raydium_timecall_interval[i])
            {
              // reset timecall next value
              raydium_timecall_next[i]=now+raydium_timecall_interval[i];
            }
    }
last=now;
// end of workaround


for(i=0;i<raydium_timecall_index;i++)
 {
 now=raydium_timecall_clock();

   if(!raydium_timecall_soft_call[i] && now>=raydium_timecall_next[i] && raydium_timecall_interval[i])
    {
    steps=((now-raydium_timecall_next[i])/raydium_timecall_interval[i])+1;
    phase=(now-raydium_timecall_next[i])-((steps-1)*raydium_timecall_interval[i]);
    //raydium_log("current phase overload for timecall %i: %i (total interval = %i",i,phase,raydium_timecall_interval[i]);
    raydium_timecall_next[i]=now+raydium_timecall_interval[i]-phase;

#ifdef DEBUG_MOVIE
    steps=(float)raydium_timecall_interval[i]/((float)raydium_timecall_interval[i]*(1/(float)DEBUG_MOVIE));
#endif

    if(steps>1000) { // DEBUG ! need to calculate this value
		    steps=100;
		    raydium_log("WARNING: timecall's too long");
		 }    
    
//    raydium_log("debug: need %i steps",steps);
    f=raydium_timecall_funct[i];

    for(j=0;j<steps;j++)
     f();
    }
    else if(raydium_timecall_soft_call[i]) // this an "elastic-timed" callback
    {
     stepsf=(now-raydium_timecall_next[i])/(GLfloat)raydium_timecall_interval[i];
     ff=raydium_timecall_funct[i];
     raydium_timecall_next[i]=raydium_timecall_clock();
     //raydium_log("debug: soft call: step factor: %.2f",stepsf);

#ifdef DEBUG_MOVIE
    stepsf=((float)raydium_timecall_interval[i]*(1/(float)DEBUG_MOVIE))/(float)raydium_timecall_interval[i];
#endif

     ff(stepsf);
    }
 }
}
