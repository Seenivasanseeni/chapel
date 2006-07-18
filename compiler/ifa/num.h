/* -*-Mode: c++;-*-
*/

#ifndef _num_h_
#define _num_h_

#include "defs.h"
#include "prim_data.h"

enum IF1_num_kind {
  NUM_KIND_NONE, NUM_KIND_UINT, NUM_KIND_INT, NUM_KIND_FLOAT, NUM_KIND_COMPLEX
};

enum IF1_const_kind {
  CONST_KIND_STRING = NUM_KIND_COMPLEX + 1, CONST_KIND_SYMBOL
};

enum IF1_int_type { 
  INT_SIZE_1, INT_SIZE_8, INT_SIZE_16, INT_SIZE_32, INT_SIZE_64,
  INT_SIZE_NUM
};

enum IF1_float_type { 
  FLOAT_SIZE_16, FLOAT_SIZE_32, FLOAT_SIZE_48, FLOAT_SIZE_64, 
  FLOAT_SIZE_80, FLOAT_SIZE_96, FLOAT_SIZE_112, FLOAT_SIZE_128, 
  FLOAT_SIZE_NUM
};

class Immediate : public gc { public:
  unsigned int const_kind : 4;
  unsigned int num_index : 3;
  union {
    bool       v_bool;
    int8       v_int8;
    int16      v_int16;
    int32      v_int32;
    int64      v_int64;
    // int128     v_int128;
    uint8      v_uint8;
    uint16     v_uint16;
    uint32     v_uint32;
    uint64     v_uint64;
    // uint128    v_uint128;
    float32    v_float32;
    float64    v_float64;
    float128   v_float128;
    complex32  v_complex32;
    complex64  v_complex64;
    complex128 v_complex128;
    char *v_string;
  };

  int64  int_value( void);
  uint64 uint_value( void);

  void set_int(int64 l, IF1_int_type int_size=INT_SIZE_64) {
    const_kind = NUM_KIND_INT;
    num_index = int_size;
    switch( int_size) {
    case INT_SIZE_8:  v_int8  = l; break;
    case INT_SIZE_16: v_int16 = l; break;
    case INT_SIZE_32: v_int32 = l; break;
    case INT_SIZE_64: v_int64 = l; break;
    default:
      assert(!"unsupported int IF1_int_type");
    }
  }

  void set_uint(uint64 ul, IF1_int_type uint_size=INT_SIZE_64) {
    const_kind = NUM_KIND_UINT;
    num_index = uint_size;
    switch( uint_size) {
    case INT_SIZE_8:  v_uint8  = ul; break;
    case INT_SIZE_16: v_uint16 = ul; break;
    case INT_SIZE_32: v_uint32 = ul; break;
    case INT_SIZE_64: v_uint64 = ul; break;
    default:
      assert(!"unsupported uint IF1_int_type");
    }
  }

  void set_float(float128 f, IF1_float_type fp_size=FLOAT_SIZE_64) {
    const_kind = NUM_KIND_FLOAT;
    num_index = fp_size;
    switch( fp_size) {
    case FLOAT_SIZE_32:  v_float32 = f;  break;
    case FLOAT_SIZE_64:  v_float64 = f;  break;
    case FLOAT_SIZE_128: v_float128 = f; break;
    default:
      assert(!"unsupported float IF1_float_type");
    }
  }

  void set_complex(complex128 c, IF1_float_type comp_size=FLOAT_SIZE_64) {
    const_kind = NUM_KIND_COMPLEX;
    num_index = comp_size;
    switch( comp_size) {
    case FLOAT_SIZE_32:  
      v_complex32.r = c.r;  
      v_complex32.i = c.i;  
      break;
    case FLOAT_SIZE_64:  
      v_complex64.r = c.r;  
      v_complex64.i = c.i;  
      break;
    case FLOAT_SIZE_128: 
      v_complex128.r = c.r; 
      v_complex128.i = c.i; 
      break;
    default:
      INT_FATAL("unsupported complex size enum %d", comp_size);
    }
  }

  void set_bool(bool b) {
    const_kind = NUM_KIND_UINT;
    num_index = INT_SIZE_1;
    v_bool = b;
  }
  Immediate& operator=(const Immediate&);
  Immediate& operator=(bool b) {
    const_kind = NUM_KIND_UINT;
    num_index = INT_SIZE_1;
    v_bool = b;
    return *this;
  }
  Immediate& operator=(char *s) {
    const_kind = CONST_KIND_STRING;
    v_string = s;
    return *this;
  }
  Immediate(bool b) {
    memset(this, 0, sizeof(*this));
    const_kind = NUM_KIND_UINT;
    num_index = INT_SIZE_1;
    v_bool = b;
  }
  Immediate(char *s) {
    memset(this, 0, sizeof(*this));
    const_kind = CONST_KIND_STRING;
    v_string = s;
  }
  Immediate();
  Immediate(const Immediate &im);
};


inline int64
Immediate::int_value( void) {
  int64 val;
  switch (num_index) {
  case INT_SIZE_8 : val = v_int8;  break;
  case INT_SIZE_16: val = v_int16; break;
  case INT_SIZE_32: val = v_int32; break;
  case INT_SIZE_64: val = v_int64; break;
  default:
    assert(!"unknown int size");
  }
  return val;
}


inline uint64
Immediate::uint_value( void) {
  uint64 val;
  switch (num_index) {
  case INT_SIZE_1 : val = v_bool;  break;
  case INT_SIZE_8 : val = v_uint8;  break;
  case INT_SIZE_16: val = v_uint16; break;
  case INT_SIZE_32: val = v_uint32; break;
  case INT_SIZE_64: val = v_uint64; break;
  default:
    assert(!"unknown uint size");
  }
  return val;
}


class ImmHashFns { public:
  static unsigned int hash(Immediate *);
  static int equal(Immediate *, Immediate *);
};

#define CPP_IS_LAME {1,8,16,32,64}
IFA_EXTERN int int_type_precision[5] IFA_EXTERN_INIT(CPP_IS_LAME);
#undef CPP_IS_LAME

#define CPP_IS_LAME {16,32,48,64,80,96,112,128}
IFA_EXTERN int float_type_precision[8] IFA_EXTERN_INIT(CPP_IS_LAME);
#undef CPP_IS_LAME

#define CPP_IS_LAME {{0,0,0,0,0,0,0,0}, {"bool","uint8","uint16","uint32","uint64",0,0,0}, {"bool","int8","int16","int32","int64",0,0,0}, {0,"float32",0,"float64",0,0,0,"float128"}}
IFA_EXTERN char *num_kind_string[4][8] IFA_EXTERN_INIT(CPP_IS_LAME);
#undef CPP_IS_LAME

inline Immediate& Immediate::operator=(const Immediate& imm) {
  memcpy(this, &imm, sizeof(imm));
  return *this;
}

inline Immediate::Immediate(const Immediate& imm) {
  memcpy(this, &imm, sizeof(imm));
}

inline Immediate::Immediate() {
  memset(this, 0, sizeof(*this));
}

inline unsigned int
ImmHashFns::hash(Immediate *imm) {
  unsigned int h = 0;
  for (int i = 0; i < (int)(sizeof(*imm)/sizeof(unsigned int)); i++)
    h = h + open_hash_multipliers[i] * ((unsigned int*)imm)[i];
  return h;
}

inline int
ImmHashFns::equal(Immediate *imm1, Immediate *imm2) {
  return !memcmp(imm1, imm2, sizeof(*imm1));
}

int fprint_imm(FILE *fp, Immediate &imm);
int sprint_imm(char *s, Immediate &imm);
int sprint_imm(char *str, char *control_string, Immediate &imm);
void coerce_immediate(Immediate *from, Immediate *to);
void fold_result(Immediate *imm1, Immediate *imm2, Immediate *imm);
void fold_constant(int op, Immediate *im1, Immediate *im2, Immediate *imm);
void convert_string_to_immediate(char *str, Immediate *imm);

#endif

