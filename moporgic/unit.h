#pragma once
#define MOPORGIC_UNIT_H_

/**
 * unit.h @ 2019-06-23
 * moporgic
 */

#include <cstddef>
#include <cstdint>
#include <cinttypes>

typedef int64_t  ll;
typedef uint64_t ull;

typedef int64_t  i64;
typedef uint64_t u64;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int8_t   i8;
typedef uint8_t  u8;

typedef __int128          i128;
typedef unsigned __int128 u128;
typedef unsigned char     uchar;
typedef unsigned long int ulint;

typedef long  art;  // art is long
typedef short life; // life is short

typedef long double llf;
typedef long double quadruple;
typedef long double quadra;
typedef long double f128;
typedef double f64;
typedef float  f32;

typedef intptr_t  iptr;
typedef uintptr_t uptr;

template<typename dst, typename src> static inline constexpr
dst  cast(src ptr) noexcept { return (dst) ptr; /*return reinterpret_cast<dst>(p);*/ }

template<typename dst, typename src> static inline constexpr
dst* pointer_cast(src* ptr) noexcept { return cast<dst*>(ptr); }

template<typename dst, typename src> static inline constexpr
dst& reference_cast(src& ref) noexcept { return *(pointer_cast<dst>(&ref)); }

template<typename dst, int off = 0, typename src> static inline constexpr
dst& raw_cast(src& ref, int sh = 0) noexcept { return *(pointer_cast<dst>(&ref) + off + sh); }
