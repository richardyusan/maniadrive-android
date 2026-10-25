/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

// C prototype to PHP function wrappers:
// PHP_ReturnType_Arg1TypeArg2Type...(function to wrap)
// v stands for: void
// i stands for: int
// f stands for: float
// s stands for: char *
// svaria stands for: (char *, ...)
// Don't forget to register your functions, too ! (see register.c or docs)

#ifndef PHP_WRAPPERS_C
#define PHP_WRAPPERS_C

// use this macro when registering your functions
#define C2PHP ZEND_FN

// void f(void)
#define PHP_v_v(fname)\
ZEND_FUNCTION(fname)\
{\
fname();\
}

// void f(int)
#define PHP_v_i(fname)\
ZEND_FUNCTION(fname)\
{\
long a;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "l", &a) == FAILURE)  return;\
fname(a);\
}

// void f(float)
#define PHP_v_f(fname)\
ZEND_FUNCTION(fname)\
{\
double a;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "d", &a) == FAILURE)  return;\
fname(a);\
}

// int f(void)
#define PHP_i_v(fname)\
ZEND_FUNCTION(fname)\
{\
RETURN_LONG(fname());\
}

// int f(int)
#define PHP_i_i(fname)\
ZEND_FUNCTION(fname)\
{\
long a;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "l", &a) == FAILURE)  return;\
RETURN_LONG(fname(a));\
}

// int f(int, int)
#define PHP_i_ii(fname)\
ZEND_FUNCTION(fname)\
{\
long a,b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ll", &a, &b) == FAILURE)  return;\
RETURN_LONG(fname(a,b));\
}

// void f(int, int)
#define PHP_v_ii(fname)\
ZEND_FUNCTION(fname)\
{\
long a,b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ll", &a, &b) == FAILURE)  return;\
fname(a,b);\
}

// void f(int,float)
#define PHP_v_if(fname)\
ZEND_FUNCTION(fname)\
{\
long a;\
double b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ld", &a, &b) == FAILURE)  return;\
fname(a,b);\
}

// int f(int,float)
#define PHP_i_if(fname)\
ZEND_FUNCTION(fname)\
{\
long a;\
double b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ld", &a, &b) == FAILURE)  return;\
RETURN_LONG(fname(a,b));\
}
  
// void f(float, float, float) - double, too
#define PHP_v_fff(fname)\
ZEND_FUNCTION(fname)\
{\
double a,b,c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
    "ddd", &a, &b, &c) == FAILURE)  return;\
fname(a,b,c);\
}

// void f(float, float, float, float)
#define PHP_v_ffff(fname)\
ZEND_FUNCTION(fname)\
{\
double a,b,c,d;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
    "dddd", &a, &b, &c, &d) == FAILURE)  return;\
fname(a,b,c,d);\
}

// int f(char *)
#define PHP_i_s(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "s", &a, &s_len) == FAILURE)  return;\
RETURN_LONG(fname(a));\
}

// int f(char *, char*)
#define PHP_i_ss(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
char *b;\
long s_len;\
long s_len2;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ss", &a, &s_len, &b, &s_len2) == FAILURE)  return;\
RETURN_LONG(fname(a,b));\
}

// float f(char *, int)
#define PHP_f_si(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
long b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sl", &a, &s_len, &b) == FAILURE)  return;\
RETURN_DOUBLE(fname(a,b));\
}

// int f(char *, int)
#define PHP_i_si(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
long b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sl", &a, &s_len, &b) == FAILURE)  return;\
RETURN_LONG(fname(a,b));\
}

// int f(char *, int, int)
#define PHP_i_sii(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
long b,c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sll", &a, &s_len, &b, &c) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c));\
}


// void f(char *)
#define PHP_v_s(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "s", &a, &s_len) == FAILURE)  return;\
fname(a);\
}

// int f(char *, char*, char *)
#define PHP_i_sss(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
char *b;\
char *c;\
long s_len;\
long s_len2;\
long s_len3;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sss", &a, &s_len, &b, &s_len2, &c, &s_len3) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c));\
}

// void f(char *, char*, char *)
#define PHP_v_sss(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
char *b;\
char *c;\
long s_len;\
long s_len2;\
long s_len3;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sss", &a, &s_len, &b, &s_len2, &c, &s_len3) == FAILURE)  return;\
fname(a,b,c));\
}

// void f(char *, ...) - (printf style)
#define PHP_v_svaria(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "s", &a, &s_len) == FAILURE)  return;\
fname("%s",a);\
}


// int f(char *, int, float, float, char *)
#define PHP_i_siffs(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
long s_len2;\
char *a;\
long b;\
double c,d;\
char *e;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sldds", &a, &s_len1, &b, &c, &d, &e, &s_len2) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e));\
}

// int f(char *, int, float, float, char *, int)
#define PHP_i_siffsi(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
long s_len2;\
char *a;\
long b;\
double c,d;\
char *e;\
long f;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "slddsl", &a, &s_len1, &b, &c, &d, &e, &s_len2, &f) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f));\
}

// int f(char *, int, float, float, char *, float, float, float)
#define PHP_i_siffsfff(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
long s_len2;\
char *a;\
long b;\
double c,d;\
char *e;\
double f,g,h;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "slddsddd", &a, &s_len1, &b, &c, &d, &e, &s_len2, &f, &g, &h) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g,h));\
}


// int f(char *, int, float, float, int, int, char *)
#define PHP_i_siffiis(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
long s_len2;\
char *a;\
long b;\
double c,d;\
long e,f;\
char *g;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "slddlls", &a, &s_len1, &b, &c, &d, &e, &f, &g, &s_len2) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g));\
}

#define PHP_i_siffiii(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
char *a;\
long b;\
double c,d;\
long e,f;\
long g;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "slddlll", &a, &s_len1, &b, &c, &d, &e, &f, &g) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g));\
}

// int f(char *, int, float, float, float, float, int, int, char *)
#define PHP_i_siffffiis(fname)\
ZEND_FUNCTION(fname)\
{\
long s_len1;\
long s_len2;\
char *a;\
long b;\
double c,d,e,f;\
long g,h;\
char *i;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "slddddlls", &a, &s_len1, &b, &c, &d, &e, &f, &g, &h, &i, &s_len2) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g,h,i));\
}


// void f(char *, int)
#define PHP_v_si(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
long b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sl", &a, &s_len, &b) == FAILURE)  return;\
fname(a,b);\
}

// int f(char *, float)
#define PHP_i_sf(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
double b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sd", &a, &s_len, &b) == FAILURE)  return;\
RETURN_LONG(fname(a,b));\
}


// int f(char *, float, float)
#define PHP_i_sff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
double b,c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sdd", &a, &s_len, &b, &c) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c));\
}

// void f(char *, float, float)
#define PHP_v_sff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
double b,c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sdd", &a, &s_len, &b, &c) == FAILURE)  return;\
fname(a,b,c);\
}

// void f(char *, float, float, float)
#define PHP_v_sfff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
double b,c,d;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sddd", &a, &s_len, &b, &c, &d) == FAILURE)  return;\
fname(a,b,c,d);\
}

// void f(char *, float, float, float, float, float, float)
#define PHP_v_sffffff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len;\
double b,c,d;\
double e,f,g;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sdddddd", &a, &s_len, &b, &c, &d, &e, &f, &g) == FAILURE)  return;\
fname(a,b,c,d,e,f,g);\
}


// void f(char *, char *, int)
#define PHP_v_ssi(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
long c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ssl", &a, &s_len1, &b, &s_len2, &c) == FAILURE)  return;\
fname(a,b,c);\
}

// void f(char *, char *, float)
#define PHP_v_ssf(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
double c;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ssd", &a, &s_len1, &b, &s_len2, &b, &c) == FAILURE)  return;\
fname(a,b,c);\
}

// void f(char *, char *, float, float, float)
#define PHP_v_ssfff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
double c,d,e;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ssddd", &a, &s_len1, &b, &s_len2, &c, &d, &e) == FAILURE)  return;\
fname(a,b,c,d,e);\
}


// void f(char *, char *)
#define PHP_v_ss(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ss", &a, &s_len1, &b, &s_len2, &b) == FAILURE)  return;\
fname(a,b);\
}


// void f(char *, float)
#define PHP_v_sf(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
double b;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sd", &a, &s_len1, &b) == FAILURE)  return;\
fname(a,b);\
}

// int f(char *, char *, char *, float, float, float, float, float, float)
#define PHP_i_sssffffff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
char *c;\
long s_len3;\
double d,e,f;\
double g,h,i;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sssdddddd", &a, &s_len1, &b, &s_len2,&c, &s_len3, &d, &e ,&f, &g, &h, &i) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g,h,i));\
}

// int f(char *, float, float, float, float)
#define PHP_i_sffff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
double b,c,d,e;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sdddd", &a, &s_len1, &b, &c, &d, &e) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e));\
}

// int f(char *, char *, float, float, float, float)
#define PHP_i_ssffff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
double c,d,e,f;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "ssdddd", &a, &s_len1, &b, &s_len2, &c, &d, &e ,&f) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f));\
}

// int f(char *, char *, float, float, float, float)
#define PHP_i_sssffff(fname)\
ZEND_FUNCTION(fname)\
{\
char *a;\
long s_len1;\
char *b;\
long s_len2;\
char *c;\
long s_len3;\
double d,e,f;\
double g;\
if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,\
  "sssdddd", &a, &s_len1, &b, &s_len2,&c, &s_len3, &d, &e ,&f, &g) == FAILURE)  return;\
RETURN_LONG(fname(a,b,c,d,e,f,g));\
}

#endif
