#pragma once
#define MOPORGIC_MATH_H_
/*
 * mopomath.h
 * moporgic/math.h
 *
 *  Created on: 2015/07/09
 *      Author: moporgic
 */

#include "half.h"
#include <math.h>
#include <cmath>

#ifndef DEBUG

#define DLOG(msg,...)
#define __constexpr constexpr

#else /* DEBUG */

#define DLOG(msg,...) printf(msg, ##__VA_ARGS__)
#define __constexpr

#endif /* DEBUG */

namespace moporgic {

template<typename T>
inline void swap(T& v1, T& v2) {
	T t = v1;
	v1 = v2;
	v2 = t;
}

namespace math {
// ref:  The Aggregate Magic Algorithms

/**
 * Bit Reversal
 * Reversing the bits in an integer x is somewhat painful, but here's a SWAR algorithm for a 32-bit value
 */
inline unsigned int reverse(register unsigned int x) {
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return ((x >> 16) | (x << 16));
}
/**
 * Bit Reversal (re-write Bit Reversal algorithm to use 4 instead of 8 constants)
 * Reversing the bits in an integer x is somewhat painful, but here's a SWAR algorithm for a 32-bit value
 */
inline unsigned int reverse_v2(register unsigned int x) {
	register unsigned int y = 0x55555555;
	x = (((x >> 1) & y) | ((x & y) << 1));
	y = 0x33333333;
	x = (((x >> 2) & y) | ((x & y) << 2));
	y = 0x0f0f0f0f;
	x = (((x >> 4) & y) | ((x & y) << 4));
	y = 0x00ff00ff;
	x = (((x >> 8) & y) | ((x & y) << 8));
	return ((x >> 16) | (x << 16));
}
/**
 * Is Power of 2
 * A non-negative binary integer value x is a power of 2 iff (x&(x-1)) is 0 using 2's complement arithmetic.
 */
inline
bool ispw2(register unsigned int x) {
	return (x & (x - 1)) == 0;
}

/**
 * Population Count (Ones Count)
 * The population count of a binary integer value x is the number of one bits in the value.
 * Although many machines have single instructions for this,
 * the single instructions are usually microcoded loops that test a bit per cycle;
 * a log-time algorithm coded in C is often faster.
 * The following code uses a variable-precision SWAR algorithm to perform a tree
 * reduction adding the bits in a 32-bit value:
 */
inline unsigned int ones32(register unsigned int x) {
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return (x & 0x0000003f);
}

/**
 * Leading Zero Count
 * Some machines have had single instructions that count the number of leading zero bits in an integer;
 * such an operation can be an artifact of having floating point normalization hardware around. Clearly,
 * floor of base 2 log of x is (WORDBITS-lzc(x)). In any case, this operation has found its way
 * into quite a few algorithms, so it is useful to have an efficient implementation:
 */
inline
unsigned int lzc(register unsigned int x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (sizeof(unsigned int) - ones32(x));
}

/**
 * Least Significant 1 Bit
 * This can be useful for extracting the lowest numbered element of a bit set.
 * Given a 2's complement binary integer value x, (x&-x) is the least significant 1 bit.
 * (This was pointed-out by Tom May.) The reason this works is that it is equivalent to (x & ((~x) + 1));
 *  any trailing zero bits in x become ones in ~x, adding 1 to that carries into the following bit,
 * and AND with x yields only the flipped bit... the original position of the least significant 1 bit.
 * Alternatively, since (x&(x-1)) is actually x stripped of its least significant 1 bit,
 * the least significant 1 bit is also (x^(x&(x-1))).
 *
 */
inline
unsigned int lsb32(register unsigned int x) {
	return (x ^ (x & (x - 1)));
}

/**
 * Most Significant 1 Bit
 * Given a binary integer value x, the most significant 1 bit
 * (highest numbered element of a bit set) can be computed using a SWAR algorithm that recursively "folds"
 * the upper bits into the lower bits. This process yields a bit vector with
 * the same most significant 1 as x, but all 1's below it. Bitwise AND of the original value with
 * the complement of the "folded" value shifted down by one yields the most significant bit. For a 32-bit value:
 */
inline
unsigned int msb32(register unsigned int x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (x & ~(x >> 1));
}

/**
 * Log2 of an Integer
 * Given a binary integer value x, the floor of the base 2 log of that number efficiently
 * can be computed by the application of two variable-precision SWAR algorithms.
 * The first "folds" the upper bits into the lower bits to construct a bit vector with the
 * same most significant 1 as x, but all 1's below it. The second SWAR algorithm is population count,
 * defined elsewhere in this document. However, we must consider the issue of what the log2(0) should be;
 * the log of 0 is undefined, so how that value should be handled is unclear.
 * The following code for handling a 32-bit value gives two options:
 * if LOG0UNDEFINED, this code returns -1 for log2(0); otherwise, it returns 0 for log2(0). For a 32-bit value:
 *
 */
inline
unsigned int log2(register unsigned int x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
#ifdef	LOG0UNDEFINED
	return (ones32(x) - 1);
#else
	return (ones32(x >> 1));
#endif
}

/**
 * Log2 of an Integer
 * Suppose instead that you want the ceiling of the base 2 log.
 * The floor and ceiling are identical if x is a power of two; otherwise,
 * the result is 1 too small. This can be corrected using the power of 2 test
 * followed with the comparison-to-mask shift used in integer minimum/maximum.
 * The result is:
 */
inline
unsigned int log2_unstable(register unsigned int x) {
	register int y = (x & (x - 1));

	y |= -y;
	y >>= (sizeof(unsigned int) - 1);
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
#ifdef	LOG0UNDEFINED
	return (ones(x) - 1 - y);
#else
	return (ones32(x >> 1) - y);
#endif
}

/**
 * Next Largest Power of 2
 *  Given a binary integer value x, the next largest power of 2 can be computed
 *  by a SWAR algorithm that recursively "folds" the upper bits into the lower bits.
 *  This process yields a bit vector with the same most significant 1 as x, but all 1's below it.
 *  Adding 1 to that value yields the next largest power of 2. For a 32-bit value:
 */
inline
unsigned int nlpo2(register unsigned int x) {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (x + 1);
}

/**
 * Swap without temporary
 */
template<typename T>
inline
void swap(T& x, T& y) {
	x ^= y; /* x' = (x^y) */
	y ^= x; /* y' = (y^(x^y)) = x */
	x ^= y; /* x' = (x^y)^x = y */
}

/**
 * Swap without temporary
 */
template<typename T>
inline
void swap_v2(T& x, T& y) {
	x += y; /* x' = (x+y) */
	y = x - y; /* y' = (x+y)-y = x */
	x -= y; /* x' = (x+y)-x = y */
}

/**
 * Trailing Zero Count
 * Given the Least Significant 1 Bit and Population Count (Ones Count) algorithms,
 * it is trivial to combine them to construct a trailing zero count (as pointed-out by Joe Bowbeer):
 */
inline
unsigned int tzc(register int x) {
	return (ones32((x & -x) - 1));
}


class half {
public:
	half(const half& num);
	half(const f32& num = 0.0f);
	half(const f64& num);
	half(const u64& raw);
	half(const i64& raw);
	half(const u32& raw);
	half(const i32& raw);
	half(const u16& raw);
	half(const i16& raw);
	inline operator f32() const;
	template<typename N> inline f32 operator +(const N& f) const;
	template<typename N> inline f32 operator -(const N& f) const;
	template<typename N> inline f32 operator *(const N& f) const;
	template<typename N> inline f32 operator /(const N& f) const;
	inline f32 operator ++(int);
	inline f32 operator --(int);
	inline f32 operator ++();
	inline f32 operator --();
	inline half& operator  =(const half& f);
	template<typename N> inline half& operator  =(const N& f);
	template<typename N> inline half& operator +=(const N& f);
	template<typename N> inline half& operator -=(const N& f);
	template<typename N> inline half& operator *=(const N& f);
	template<typename N> inline half& operator /=(const N& f);
	template<typename N> inline bool operator ==(const N& f) const;
	template<typename N> inline bool operator !=(const N& f) const;
	template<typename N> inline bool operator <=(const N& f) const;
	template<typename N> inline bool operator >=(const N& f) const;
	template<typename N> inline bool operator < (const N& f) const;
	template<typename N> inline bool operator > (const N& f) const;

	template<typename N> inline half& operator >>(N& v) const;
	template<typename N> inline half& operator <<(const N& v);
private:
	u16 hf;
};
half::half(const half& num) : hf(num.hf) {}
half::half(const f32& num) : hf(to_half(num)) {}
half::half(const f64& num) : half(f32(num)) {}
half::half(const u64& num) : half(f32(num)) {}
half::half(const i64& num) : half(f32(num)) {}
half::half(const u32& num) : half(f32(num)) {}
half::half(const i32& num) : half(f32(num)) {}
half::half(const u16& num) : half(f32(num)) {}
half::half(const i16& num) : half(f32(num)) {}
inline half::operator f32() const { return to_float(hf); }
template<typename N> inline f32 half::operator +(const N& f) const { return operator f32() + f; }
template<typename N> inline f32 half::operator -(const N& f) const { return operator f32() - f; }
template<typename N> inline f32 half::operator *(const N& f) const { return operator f32() * f; }
template<typename N> inline f32 half::operator /(const N& f) const { return operator f32() / f; }
inline f32 half::operator ++(int) { f32 f = to_float(hf); hf = to_half(f + 1); return f; }
inline f32 half::operator --(int) { f32 f = to_float(hf); hf = to_half(f - 1); return f; }
inline f32 half::operator ++() { f32 f = to_float(hf) + 1; hf = to_half(f); return f; }
inline f32 half::operator --() { f32 f = to_float(hf) - 1; hf = to_half(f); return f; }
inline half& half::operator  =(const half& f) { hf = f.hf; return *this; }
template<typename N> inline half& half::operator  =(const N& f) { hf = to_float(f32(f)); return *this; }
template<typename N> inline half& half::operator +=(const N& f) { return operator =(operator +(f)); }
template<typename N> inline half& half::operator -=(const N& f) { return operator =(operator -(f)); }
template<typename N> inline half& half::operator *=(const N& f) { return operator =(operator *(f)); }
template<typename N> inline half& half::operator /=(const N& f) { return operator =(operator /(f)); }
template<typename N> inline bool half::operator ==(const N& f) const { return operator f32() == f; }
template<typename N> inline bool half::operator !=(const N& f) const { return operator f32() != f; }
template<typename N> inline bool half::operator <=(const N& f) const { return operator f32() <= f; }
template<typename N> inline bool half::operator >=(const N& f) const { return operator f32() >= f; }
template<typename N> inline bool half::operator < (const N& f) const { return operator f32() <  f; }
template<typename N> inline bool half::operator > (const N& f) const { return operator f32() >  f; }
inline u16& hfat(const f32& v) { return ((u16*)&v)[endian::be]; }
inline u32& hfat(const f64& v) { return ((u32*)&v)[endian::be]; }
template<typename N> inline half& half::operator >>(N& v) const { f32 f = 0; math::hfat(f) = hf; v = f; return *this; }
template<typename N> inline half& half::operator <<(const N& v) { f32 f(v); hf = hfat(f); return *this; }

template<typename N, typename P> __constexpr
N pow_tail(N v, N b, P p) noexcept { return p ? pow_tail(v * b, b, p - 1) : v; }

template<typename N, typename P> __constexpr inline
N pow(N b, P p) noexcept { return pow_tail(N(1), b, p); }

#undef __constexpr
} /* math */
}
