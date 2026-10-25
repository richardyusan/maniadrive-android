/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/normal.h"
#endif 

void raydium_normal_generate_lastest_triangle(int default_visu)
{
GLfloat vect[2][3];
GLfloat norm[3];
GLfloat len;
int i;
GLuint index=raydium_vertex_index;

// let's find "vectors" of the triangle's plane ...
vect[0][0]=
 raydium_vertex_x[index-1] -
 raydium_vertex_x[index-2] ;
vect[0][1]=
 raydium_vertex_y[index-1] -
 raydium_vertex_y[index-2] ;
vect[0][2]=
 raydium_vertex_z[index-1] -
 raydium_vertex_z[index-2] ;

vect[1][0]=
 raydium_vertex_x[index-1] -
 raydium_vertex_x[index-3] ;
vect[1][1]=
 raydium_vertex_y[index-1] -
 raydium_vertex_y[index-3] ;
vect[1][2]=
 raydium_vertex_z[index-1] -
 raydium_vertex_z[index-3] ;

// ... and now, the normal ...

norm[0]=vect[0][1]*vect[1][2] - vect[0][2]*vect[1][1];
norm[1]=vect[0][2]*vect[1][0] - vect[0][0]*vect[1][2];
norm[2]=vect[0][0]*vect[1][1] - vect[0][1]*vect[1][0];

// ... wich we normalize.

len=sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);

for(i=1;i<=3;i++) 
{ 
raydium_vertex_normal_x[index-i]=-norm[0]/len;
if(default_visu) raydium_vertex_normal_visu_x[index-i]=raydium_vertex_normal_x[index-i];
}

for(i=1;i<=3;i++) 
{
raydium_vertex_normal_y[index-i]=-norm[1]/len;
if(default_visu) raydium_vertex_normal_visu_y[index-i]=raydium_vertex_normal_y[index-i];
}

for(i=1;i<=3;i++)
{
raydium_vertex_normal_z[index-i]=-norm[2]/len;
if(default_visu) raydium_vertex_normal_visu_z[index-i]=raydium_vertex_normal_z[index-i];
}

//raydium_log("norm [%f,%f,%f]/%f",norm[0],norm[1],norm[2],len);
}


void raydium_normal_restore_all(void)
{
GLuint i;
for(i=0;i<raydium_vertex_index;i++)
 {
 raydium_vertex_normal_visu_x[i]=raydium_vertex_normal_x[i];
 raydium_vertex_normal_visu_y[i]=raydium_vertex_normal_y[i];
 raydium_vertex_normal_visu_z[i]=raydium_vertex_normal_z[i];
 }
raydium_log("normal: Normals restaured.");
}

void raydium_normal_regenerate_all(void)
{
GLuint save=raydium_vertex_index;

if(save<3) return;

for(raydium_vertex_index=3;raydium_vertex_index<=save;raydium_vertex_index+=3)
    raydium_normal_generate_lastest_triangle(1);

raydium_vertex_index=save;
raydium_log("normal: Normals regenerated.");
}

void raydium_normal_smooth_from_to(GLuint from, GLuint to)
{
GLuint i,j;
GLfloat x,y,z;
GLfloat sum_x;
GLfloat sum_y;
GLfloat sum_z;
GLuint n;
GLuint debug_time;
char *tag; // 1 means "already done", 2 means "used for last normals search"
GLuint total;

total=to-from;

tag=malloc(total);
if(!tag) { raydium_log("normal: Not enought memory for normal smoothing, giving up."); return; }
memset(tag,0,total);

debug_time=0;

for(i=from;i<to;i++)
{
if(tag[i-from]) continue;
x=raydium_vertex_x[i];
y=raydium_vertex_y[i];
z=raydium_vertex_z[i];
sum_x=0;
sum_y=0;
sum_z=0;
n=0;

for(j=from;j<to;j++)
 {
 if(raydium_vertex_x[j]==x && raydium_vertex_y[j]==y && raydium_vertex_z[j]==z)
  { 
  sum_x+=raydium_vertex_normal_x[i];
  sum_y+=raydium_vertex_normal_y[i];
  sum_z+=raydium_vertex_normal_z[i];
  n++;
  tag[j-from]=2;  
  }
 }
sum_x/=(GLfloat)n;
sum_y/=(GLfloat)n;
sum_z/=(GLfloat)n;

for(j=from;j<to;j++)
if(tag[j-from]==2)
 {
 raydium_vertex_normal_visu_x[j]=sum_x;
 raydium_vertex_normal_visu_y[j]=sum_y;
 raydium_vertex_normal_visu_z[j]=sum_z;
 tag[j-from]=1;
 }


debug_time++;
//if(debug_time>100) { raydium_log("normal: smoothing: %i/%i",i-from,total); debug_time=0; }
}

free(tag);
raydium_log("normal: smoothing done.");
}


void raydium_normal_smooth_all(void)
{
raydium_normal_smooth_from_to(0,raydium_vertex_index);
}
