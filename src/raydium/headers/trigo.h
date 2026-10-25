#ifndef _TRIGO_H
#define _TRIGO_H

/*=
Maths
200
*/

// Little introduction to trigo.c
/**
This section is mostly designed for internal uses, but provides some
usefull maths functions, mostly for trigonometrical uses.
**/

__rayapi GLfloat raydium_trigo_cos (GLfloat i);
/**
Obvious (degrees)
**/

__rayapi GLfloat raydium_trigo_sin (GLfloat i);
/**
Obvious (degrees)
**/

__rayapi GLfloat raydium_trigo_cos_inv (GLfloat i);
/**
Obvious (degrees)
**/

__rayapi GLfloat raydium_trigo_sin_inv (GLfloat i);
/**
Obvious (degrees)
**/

#define raydium_trigo_abs(a) ( (a) < (0) ? (-a) : (a) )
/**
Obvious
**/

#define raydium_trigo_min(a,b) ( (a) < (b) ? (a) : (b) )
/**
Obvious
**/

#define raydium_trigo_max(a,b) ( (a) > (b) ? (a) : (b) )
/**
Obvious
**/

#define raydium_trigo_isfloat(a) ( (!isnan(a) && !isinf(a)) ? 1 : 0)
/**
Test two cases : "Not a Number" and "Infinite"
**/

#define raydium_trigo_round(a) ((int)((a)>0?((a)+0.5):((a)-0.5)))
/**
Will obviously "round" ##a## instead of the default C floor behaviour
**/

__rayapi void raydium_trigo_rotate (GLfloat * p, GLfloat rx, GLfloat ry, GLfloat rz, GLfloat * res);
/**
Rotate p (GLfloat * 3) by (rx,ry,rx) angles (degrees).
Result is stored in res (GLfloat * 3)
**/

__rayapi void raydium_trigo_pos_to_matrix (GLfloat * pos, GLfloat * m);
/**
Generates a ODE style matrix (16 Glfloat) from pos (GLfloat * 3)
**/

__rayapi void raydium_trigo_pos_get_modelview (GLfloat * res);
/**
Stores the current OpenGL MODELVIEW matrix in res (16 GLfloat)
**/

__rayapi int raydium_trigo_pow2_next(int value);
/**
Returns next power of two of ##value##. Ugly.
**/

//Matrix functions
/**
Here there are a few functions also designed for internal uses that aims 
only at matrices. Really the main objective of these functions is give support
for the inverse function.
The data type matrix4x4 is really an 16 double array.
**/


__rayapi double raydium_matrix_determinant(matrix4x4 matrix);
/**
Returns the ##determinant## of the given matrix.
**/

__rayapi matrix4x4 raydium_matrix_adjoint(matrix4x4 matrix);
/**
Returns the ##adjoint matrix## of the given matrix.
**/

__rayapi matrix4x4 raydium_matrix_multiply(matrix4x4 matrix1, matrix4x4 matrix2);
/**
Returns the resulting matrix of the multiplication of 2 matrices.
Remeber that the multiplication of matrices doesn't have the conmutative 
property, so is not equal ##matrix1 X matrix2## than ##matrix2 x matrix1##.
**/

__rayapi matrix4x4 raydium_matrix_inverse(matrix4x4 matrix);
/**
Returns the inverse matrix of a given matrix.
**/

__rayapi double raydium_matrix_internal_determinant(matrix4x4 matrix, int dimension);
/**
internal, don't use.
**/

__rayapi matrix4x4 raydium_matrix_internal_adjoint(matrix4x4 matrix, int dimension);
/**
internal, don't use.
**/
__rayapi matrix4x4 raydium_matrix_internal_multiply(matrix4x4 matrix_one, matrix4x4 matrix_two, int dimension);
/**
internal, don't use.
**/
__rayapi matrix4x4 raydium_matrix_internal_inverse(matrix4x4 adjoint_matrix,double det,int dimension);
/**
internal, don't use.
**/

__rayapi int _raydium_trigo_MatrixInverse(const float *m,float *out);
/*
Our matrix_inverse seems broken. 
This code works, thanks to Alexander Zaprjagaev (frustum@public.tsu.ru)
This code is not native
*/

#endif
