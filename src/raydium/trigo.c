/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/trigo.h"
#endif 

GLfloat raydium_trigo_cos(GLfloat i)
{
return( (GLfloat)cos(i*PI/180) );
}

GLfloat raydium_trigo_sin(GLfloat i)
{
return( (GLfloat)sin(i*PI/180) );
}

GLfloat raydium_trigo_cos_inv(GLfloat i)
{
return(acos(i)*180/PI);
}

GLfloat raydium_trigo_sin_inv(GLfloat i)
{
return(asin(i)*180/PI);
}

void raydium_trigo_rotate(GLfloat *p, GLfloat rx, GLfloat ry, GLfloat rz, GLfloat *res)
{
res[0]= (p[0]*raydium_trigo_cos(ry)+(p[2]*raydium_trigo_cos(rx)+p[1]*raydium_trigo_sin(rx))*raydium_trigo_sin(ry))*raydium_trigo_cos(rz) + (p[1]*raydium_trigo_cos(rx)-p[2]*raydium_trigo_sin(rx))*raydium_trigo_sin(rz);
res[1]=-(p[0]*raydium_trigo_cos(ry)+(p[2]*raydium_trigo_cos(rx)+p[1]*raydium_trigo_sin(rx))*raydium_trigo_sin(ry))*raydium_trigo_sin(rz) + (p[1]*raydium_trigo_cos(rx)-p[2]*raydium_trigo_sin(rx))*raydium_trigo_cos(rz);
res[2]= (p[2]*raydium_trigo_cos(rx)+ p[1]*raydium_trigo_sin(rx))*raydium_trigo_cos(ry)-p[0]*raydium_trigo_sin(ry);
}


// pos: GLfloat[3], m: GLfloat[16]
void raydium_trigo_pos_to_matrix(GLfloat *pos, GLfloat *m)
{
m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = pos[0];
m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = pos[1];
m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = pos[2];
m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

// res: GLfloat[3]
void raydium_trigo_pos_get_modelview(GLfloat *res)
{
GLfloat tmp[16];
glGetFloatv(GL_MODELVIEW_MATRIX,tmp);
res[0]=tmp[12];
res[1]=tmp[13];
res[2]=tmp[14];
}

/* Unfinished !
// pos == res is safe
void raydium_trigo_carth_to_sphere(GLfloat *pos, GLfloat *res)
{
GLfloat r,G,T;

r=sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
G=atan(pos[1]/pos[0]);
T=acos(pos[2]/r);

//printf("%f\n",r);
r=1;

if(pos[0]<0)
    res[0]=-r*cos(G);
else
    res[0]=+r*cos(G);

//res[1]=r*sin(G);
if(pos[0]<0)
    res[0]=-r*sin(T)*cos(G);
else
    res[0]=r*sin(T)*cos(G);
res[1]=r*sin(G)*cos(G);
res[2]=r*cos(T);

if(isnan(res[0])) res[0]=0;
if(isnan(res[1])) res[1]=0;
if(isnan(res[2])) res[2]=0;

}
*/

int raydium_trigo_pow2_next(int value)
{
int pows[]={0,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};
int pows_count=17; // 16 + first (0)
int i;

if(value>65536 || value<0)
    {
    raydium_log("trigo: value is outside of limits of this ugly function :/");
    }

for(i=0;i<pows_count;i++)
    if(pows[i]>=value)
	return pows[i];


// should never hit this point
raydium_log("trigo: raydium_trigo_pow2_next: ?!!");
return -1;
}


/** function which returns the determinant of a given matrix */
double raydium_matrix_determinant(matrix4x4 matrix)
{
	return raydium_matrix_internal_determinant(matrix,4);
}

/**  function which returns the adjoint of a given matrix.      */
matrix4x4 raydium_matrix_adjoint(matrix4x4 matrix)
{
	return raydium_matrix_internal_adjoint(matrix,4);
}
/* Function to find the product of two matrices   */
/* beware: the order is important. matrix1xmatrix2 != matrix2xmatrix1*/
matrix4x4 raydium_matrix_multiply(matrix4x4 matrix1, matrix4x4 matrix2)
{
	return raydium_matrix_internal_multiply(matrix1,matrix2,4);
}

/*  Function to find the inverse of a given matrix */
matrix4x4 raydium_matrix_inverse(matrix4x4 matrix)
{
	matrix4x4 matrix_adj;
	double determinant;
	
	determinant	=	raydium_matrix_determinant(matrix);
	matrix_adj	=	raydium_matrix_adjoint(matrix);
	return raydium_matrix_internal_inverse(matrix_adj, determinant, 4);
}

/** Function which returns the determinant of a given matrix */
double raydium_matrix_internal_determinant(matrix4x4 matrix, int dimension)
{
	static int tot;
  int    i, orig_mat_row, orig_mat_col, temp_mat_row, temp_mat_col;
  double det;

  matrix4x4 temp_matrix;

  if(dimension == 2)
  {
    //det = ( (matrix.ray[0][0]) * (matrix.ray[1][1]) ) - ( (matrix.ray[0][1]) * (matrix.ray[1][0]) );
	  det = ( (matrix.ray[0]) * (matrix.ray[3]) ) - ( (matrix.ray[1]) * (matrix.ray[2]) );
    return(det);
  }

  else {
    for( i = 0; i <= dimension - 1; i++ ) {
      temp_mat_row = 0;
      temp_mat_col = 0;
      for(orig_mat_row = 1; orig_mat_row <= dimension - 1; orig_mat_row++) {
				for(orig_mat_col = 0; orig_mat_col <= dimension - 1; orig_mat_col++) {
	  			if( orig_mat_col != i ) {
	    			//temp_matrix.ray[temp_mat_row][temp_mat_col] = matrix.ray[orig_mat_row][orig_mat_col];
					temp_matrix.ray[(temp_mat_row*dimension)+temp_mat_col] = matrix.ray[(orig_mat_row*dimension)+orig_mat_col];
	    			temp_mat_col++;
				  }
				}
				temp_mat_row++;
				temp_mat_col = 0;
      }
      //det = (matrix.ray[0][i] * determinant(temp_matrix,dimension-1));
	  det = (matrix.ray[(0*dimension)+i] * raydium_matrix_internal_determinant(temp_matrix,dimension-1));
      tot = tot + det * pow(-1,i+1);
    }
    return(-1 * tot);
  }

}


/**  Function which returns the adjoint of a given matrix.      */
matrix4x4 raydium_matrix_internal_adjoint(matrix4x4 matrix, int dimension)
{
  int row, col, orig_mat_row, orig_mat_col, temp_mat_row, temp_mat_col;
  matrix4x4 temp_matrix, conjugate_matrix, transpose_matrix;

  if(dimension == 2) {
    //conjugate_matrix.ray[0][0] = matrix.ray[1][1];
	  conjugate_matrix.ray[0] = matrix.ray[3];
    //conjugate_matrix.ray[0][1] = -1 * matrix.ray[0][1];
	  conjugate_matrix.ray[1] = -1 * matrix.ray[1];
    //conjugate_matrix.ray[1][0] = -1 * matrix.ray[1][0];
	  conjugate_matrix.ray[2] = -1 * matrix.ray[2];
    //conjugate_matrix.ray[1][1] = matrix.ray[0][0];
	  conjugate_matrix.ray[3] = matrix.ray[0];

    return(conjugate_matrix);
  }
  else
  {
    for(row = 0; row < dimension; row++)
	{
      for(col = 0; col < dimension; col++)
	  {	
				temp_mat_row = 0;
				temp_mat_col = 0;
	
				for(orig_mat_row = 0; orig_mat_row < dimension; orig_mat_row++)
				{
					for(orig_mat_col = 0; orig_mat_col < dimension ; orig_mat_col++)
					{
	    				if(orig_mat_row != row && orig_mat_col != col)
						{
				      		//temp_matrix.ray[temp_mat_row][temp_mat_col] = matrix.ray[orig_mat_row][orig_mat_col];
							temp_matrix.ray[(temp_mat_row*dimension)+temp_mat_col] = matrix.ray[(orig_mat_row*dimension)+orig_mat_col];
	    			  		temp_mat_col++;
						}
					}
	  				if( temp_mat_col > (dimension - 2) )
					{
	    				temp_mat_row++;
	    				temp_mat_col = 0;
				  	}
				}
				//conjugate_matrix.ray[row][col] = determinant(temp_matrix,dimension - 1) * pow(-1,row+col+2);
				conjugate_matrix.ray[(row*dimension)+col] = raydium_matrix_internal_determinant(temp_matrix,dimension - 1) * pow(-1,row+col+2);
      }

    	for(row = 0; row < dimension; row++)
		{
     		for(col = 0; col < dimension; col++)
			{
				//transpose_matrix.ray[col][row] = conjugate_matrix.ray[row][col];
				transpose_matrix.ray[(col*dimension)+row] = conjugate_matrix.ray[(row*dimension)+col];
			}
		} 		
	}
	return(transpose_matrix);
  }	

}



/* Function to find the product of two matrices   */

matrix4x4 raydium_matrix_internal_multiply(matrix4x4 matrix_one, matrix4x4 matrix_two, int dimension)
{
  matrix4x4 result_matrix;
  int i,j,k;

  for(i=0;i<dimension;i++)
  {
    for(j=0;j<dimension;j++)
	{	
		//result_matrix.ray[i][j] = 0;
		result_matrix.ray[(i*dimension)+j] = 0;
		for(k=0;k<dimension;k++)
		{
			//result_matrix.ray[i][j] = result_matrix.ray[i][j] + ( matrix_one.ray[i][k] * matrix_two.ray[k][j] );
			result_matrix.ray[(i*dimension)+j] = result_matrix.ray[(i*dimension)+j] + ( matrix_one.ray[(i*dimension)+k] * matrix_two.ray[(k*dimension)+j] );
		}
    }
  }
  return(result_matrix);
}



/*  Function to find the inverse of a given matrix */
// Fixme: code should be modified to take an ordinary matrix and call adjoint function itself.

matrix4x4 raydium_matrix_internal_inverse(matrix4x4 adjoint_matrix,double det,int dimension)
{
  int row, col;
  matrix4x4 inverse_matrix;

  for(row = 0; row < dimension; row++)
    for(col = 0; col < dimension; col++)
	{
      //inverse_matrix.ray[row][col] = (adjoint_matrix.ray[row][col]) / det ;
	  //beware ¡¡¡
		inverse_matrix.ray[(row*dimension)+col] =  (adjoint_matrix.ray[(row*dimension)+col]) / det ;
	}
  return(inverse_matrix);
}


// Our matrix_inverse seems broken. 
// This code works, thanks to Alexander Zaprjagaev (frustum@public.tsu.ru)
int _raydium_trigo_MatrixInverse(const float *m,float *out) {
    float   det;
    det = m[0] * m[5] * m[10];
    det += m[4] * m[9] * m[2];
    det += m[8] * m[1] * m[6];
    det -= m[8] * m[5] * m[2];
    det -= m[4] * m[1] * m[10];
    det -= m[0] * m[9] * m[6];
    if(det * det < 1e-25) return 0;
    det = 1.0 / det;    
    out[0] =    (m[5] * m[10] - m[9] * m[6]) * det;
    out[1] =  - (m[1] * m[10] - m[9] * m[2]) * det;
    out[2] =    (m[1] * m[6] -  m[5] * m[2]) * det;
    out[3] = 0.0;
    out[4] =  - (m[4] * m[10] - m[8] * m[6]) * det;
    out[5] =    (m[0] * m[10] - m[8] * m[2]) * det;
    out[6] =  - (m[0] * m[6] -  m[4] * m[2]) * det;
    out[7] = 0.0;
    out[8] =    (m[4] * m[9] -  m[8] * m[5]) * det;
    out[9] =  - (m[0] * m[9] -  m[8] * m[1]) * det;
    out[10] =   (m[0] * m[5] -  m[4] * m[1]) * det;
    out[11] = 0.0;
    out[12] = - (m[12] * out[0] + m[13] * out[4] + m[14] * out[8]);
    out[13] = - (m[12] * out[1] + m[13] * out[5] + m[14] * out[9]);
    out[14] = - (m[12] * out[2] + m[13] * out[6] + m[14] * out[10]);
    out[15] = 1.0;
    return 1;
}
