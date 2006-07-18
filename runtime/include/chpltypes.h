#ifndef _chpltypes_H_
#define _chpltypes_H_

#define nil 0
#define _nilType void*

/* macros for specifying the correct C constant type */
#define INT8( i)   (i)
#define INT16( i)  (i)
#define INT32( i)  (i ## L)
#define INT64( i)  (i ## LL)
#define UINT8( i)  (i ## U)
#define UINT16( i) (i ## U)
#define UINT32( i) (i ## UL)
#define UINT64( i) (i ## ULL)

typedef enum __bool {
  false = 0,
  true = 1
} _bool;

typedef char                _int8;
typedef short int           _int16;
typedef int                 _int32;
typedef long long int       _int64;
typedef unsigned char       _uint8;
typedef unsigned short int  _uint16;
typedef unsigned int        _uint32;
typedef unsigned long long  _uint64;
typedef float               _float32;
typedef double              _float64;
typedef long double         _float128;
typedef struct __complex32  {_float32 re; _float32 im;} _complex32;
typedef struct __complex64  {_float64 re; _float64 im;} _complex64;
typedef struct __complex128 {_float128 re; _float128 im;} _complex128;
typedef char*               _string;
typedef _int64              _symbol;


#define ascii(s) ((_int8)(*s))

#define string_copy(rhs) (_glom_strings(1, rhs))

char* _glom_strings(int numstrings, ...);

char* _chpl_tostring_bool(_bool x, char* format);
char* _chpl_tostring_int(_int64 x, char* format);
char* _chpl_tostring_float(_float64 x, char* format);
char* _chpl_tostring_complex(_complex64 x, char* format);

_bool string_contains(_string x, _string y);
_string string_concat(_string x, _string y);
_string string_index(_string x, int i);
_string string_select(_string x, int low, int high);
_string string_strided_select(_string x, int low, int high, int stride);
_bool string_equal(_string x, _string y);
_int64 string_length(_string x);

// Construction and assignment of complex numbers
_complex32  _chpl_complex32( _float32 r, _float32 i);
_complex64  _chpl_complex64( _float64 r, _float64 i);
_complex128 _chpl_complex128( _float128 r, _float128 i);
#define     _chpl_complex_real(c)               c.re
#define     _chpl_complex_imag(c)               c.im
#define     _chpl_complex_set_real(c, r)        c.re=r
#define     _chpl_complex_set_imag(c, i)        c.im=i

#endif
