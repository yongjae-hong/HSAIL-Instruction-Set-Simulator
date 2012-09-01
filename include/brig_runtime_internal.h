#ifndef BRIG_RUNTIME_INTERNAL_H
#define BRIG_RUNTIME_INTERNAL_H

#include "brig_runtime.h"

namespace hsa {
namespace brig {

template<class T, class B, unsigned L>
struct VecPolicy {
  typedef VecPolicy<T, B, L> Self;
  typedef T Type;
  typedef B Base;
  static const unsigned Len = L;
  typedef Base (*UMapFn)(Base);
  typedef Base (*BMapFn)(Base, Base);
  typedef void (*UForEachFn)(Base);
  typedef void (*BForEachFn)(Base, Base);
  typedef void (*TForEachFn)(Base, Base, Base);

  VecPolicy(Type &t) : t_(t) {}

  Base &operator[](unsigned i) {
    return reinterpret_cast<Base *>(&t_)[i];
  }

  static Type &S(Type &t) {
    for(unsigned i = 1; i < Len; ++i) {
      ((Self) t)[i] = ((Self) t)[0];
    }
    return t;
  }

  static Type &P(Type &t) {
    return t;
  }

 private:
  Type &t_;
};

template<class T> struct Vec;

template<class T>
static T map(typename Vec<T>::UMapFn MapFn, T t) {
  for(unsigned i = 0; i < Vec<T>::Len; ++i) {
    ((Vec<T>) t)[i] = MapFn(((Vec<T>) t)[i]);
  }
  return t;
}

template<class T>
static T map(typename Vec<T>::BMapFn MapFn, T x, T y) {
  for(unsigned i = 0; i < Vec<T>::Len; ++i) {
    ((Vec<T>) x)[i] = MapFn(((Vec<T>) x)[i], ((Vec<T>) y)[i]);
  }
  return x;
}


template<class T>
static void ForEach(typename Vec<T>::UForEachFn MapFn, T t) {
  for(unsigned i = 0; i < Vec<T>::Len; ++i) {
    MapFn(((Vec<T>) t)[i]);
  }
}

template<class T>
static void ForEach(typename Vec<T>::BForEachFn MapFn, T x, T y) {
  for(unsigned i = 0; i < Vec<T>::Len; ++i) {
    MapFn(((Vec<T>) x)[i], ((Vec<T>) y)[i]);
  }
}

template<class T>
static void ForEach(typename Vec<T>::TForEachFn MapFn, T x, T y, T z) {
  for(unsigned i = 0; i < Vec<T>::Len; ++i) {
    MapFn(((Vec<T>) x)[i], ((Vec<T>) y)[i], ((Vec<T>) z)[i]);
  }
}

#define defineVec(T,LEN)                                            \
  template<> struct Vec<T ## x ## LEN> :                            \
    public VecPolicy<T ## x ## LEN, T, LEN> {                       \
    Vec(T ## x ## LEN &t) : VecPolicy<T ## x ## LEN, T, LEN>(t) {}  \
  };

defineVec(u8, 4)
defineVec(s8, 4)
defineVec(u8, 8)
defineVec(s8, 8)
defineVec(u8, 16)
defineVec(s8, 16)
defineVec(u16, 2)
defineVec(s16, 2)
// defineVec(f16, 2)
defineVec(u16, 4)
defineVec(s16, 4)
// defineVec(f16, 4)
defineVec(u16, 8)
defineVec(s16, 8)
// defineVec(f16, 8)
defineVec(u32, 2)
defineVec(s32, 2)
defineVec(f32, 2)
defineVec(u32, 4)
defineVec(s32, 4)
defineVec(f32, 4)
defineVec(u64, 2)
defineVec(s64, 2)
defineVec(f64, 2)

#define BitInst(D,INST,NARY)                    \
  D ## NARY(INST, b32)                          \
  D ## NARY(INST, b64)

#define SignedInst(D,INST,NARY)                 \
  D ## NARY(INST, s32)                          \
  D ## NARY(INST, s64)

#define UnsignedInst(D,INST,NARY)               \
  D ## NARY(INST, u32)                          \
  D ## NARY(INST, u64)

#define FloatInst(D,INST,NARY)                  \
  /* D ## NARY(INST, f16) */                    \
  D ## NARY(INST, f32)                          \
  D ## NARY(INST, f64)

#define SignedVectorInst(D,INST,NARY)           \
  NARY ## Vector(D, INST, s8x4)                 \
  NARY ## Vector(D, INST, s8x8)                 \
  NARY ## Vector(D, INST, s16x2)                \
  NARY ## Vector(D, INST, s16x4)                \
  NARY ## Vector(D, INST, s32x2)

#define UnsignedVectorInst(D,INST,NARY)         \
  NARY ## Vector(D, INST, u8x4)                 \
  NARY ## Vector(D, INST, u8x8)                 \
  NARY ## Vector(D, INST, u16x2)                \
  NARY ## Vector(D, INST, u16x4)                \
  NARY ## Vector(D, INST, u32x2)

#define FloatVectorInst(D,INST,NARY)            \
  /* NARY ## Vector(D, INST, f16x2) */          \
  /* NARY ## Vector(D, INST, f16x4) */          \
  NARY ## Vector(D, INST, f32x2)

#define UnaryVector(D,FUNC,TYPE)                \
  D ## UnaryVectorPacking(FUNC,TYPE,P)          \
  D ## UnaryVectorPacking(FUNC,TYPE,S)

#define BinaryVector(D,FUNC,TYPE)               \
  D ## BinaryVectorPacking(FUNC, TYPE, P, P)    \
  D ## BinaryVectorPacking(FUNC, TYPE, P, S)    \
  D ## BinaryVectorPacking(FUNC, TYPE, S, P)    \
  D ## BinaryVectorPacking(FUNC, TYPE, S, S)

#define defineUnary(FUNC,TYPE)                  \
  extern "C" TYPE FUNC ## _ ## TYPE (TYPE t) {  \
    return FUNC(t);                             \
  }

#define defineBinary(FUNC,TYPE)                         \
  extern "C" TYPE FUNC ## _ ## TYPE (TYPE t, TYPE u) {  \
    return FUNC(t, u);                                  \
  }

#define defineUnaryVectorPacking(FUNC,TYPE,PACKING)             \
  extern "C" TYPE FUNC ## _ ## PACKING ## _ ## TYPE (TYPE t) {  \
    return FUNC ## Vector(Vec<TYPE>::PACKING(t));               \
  }

#define defineBinaryVectorPacking(FUNC,TYPE,P1,P2)                      \
  extern "C" TYPE FUNC ## _ ## P1 ## P2 ## _ ## TYPE (TYPE t, TYPE u) { \
    return FUNC ## Vector(Vec<TYPE>::P1(t), Vec<TYPE>::P2(u));          \
  }

#define declareUnary(FUNC,TYPE)                 \
  extern "C" TYPE FUNC ## _ ## TYPE (TYPE t);

#define declareBinary(FUNC,TYPE)                      \
  extern "C" TYPE FUNC ## _ ## TYPE (TYPE t, TYPE u);

#define declareUnaryVectorPacking(FUNC,TYPE,PACKING)          \
  extern "C" TYPE FUNC ## _ ## PACKING ## _ ## TYPE (TYPE t);

#define declareBinaryVectorPacking(FUNC,TYPE,P1,P2)                     \
  extern "C" TYPE FUNC ## _ ## P1 ## P2 ## _ ## TYPE (TYPE t, TYPE u);

template <bool S> struct IntTypes;
template<> struct IntTypes<true> {
  typedef s64 Int64Ty;
  typedef s32 Int32Ty;
  typedef s16 Int16Ty;
  typedef s8  Int8Ty;
};

template<> struct IntTypes<false> {
  typedef u64 Int64Ty;
  typedef u32 Int32Ty;
  typedef u16 Int16Ty;
  typedef u8  Int8Ty;
};

template<class T, class U, bool S> struct IntPolicy : public IntTypes<S> {
  typedef U Unsigned;
  static const bool isSigned = S;
  static const unsigned Bits = 8 * sizeof(T);
  static const T HighBit = T(T(1) << (Bits - 1));
  static const T Min = isSigned ? HighBit :  T(0);
  static const T Max = ~Min;
  static bool isNeg(T t) { return isSigned && (t & HighBit); }
};

template<class T> struct Int;

template<> struct Int<u8>  : public IntPolicy<u8,  u8,  false> {};
template<> struct Int<u16> : public IntPolicy<u16, u16, false> {};
template<> struct Int<u32> : public IntPolicy<u32, u32, false> {};
template<> struct Int<u64> : public IntPolicy<u64, u64, false> {};
template<> struct Int<s8>  : public IntPolicy<s8,  u8,  true>  {};
template<> struct Int<s16> : public IntPolicy<s16, u16, true>  {};
template<> struct Int<s32> : public IntPolicy<s32, u32, true>  {};
template<> struct Int<s64> : public IntPolicy<s64, u64, true>  {};

template <class T> inline bool isNan(T t) { return t != t; }

} // namespace brig
} // namespace hsa

#endif // BRIG_RUNTIME_INTERNAL_H