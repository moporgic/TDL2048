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
#include "type.h"
#include <math.h>
#include <cmath>
#include <numeric>
#include <iterator>
#include <algorithm>

#if !defined(__cplusplus) || __cplusplus < 201103L
#define constexpr
#define noexcept
#endif

namespace moporgic {

namespace math {
/*	reference:  The Aggregate Magic Algorithms
 *
 *	@techreport{magicalgorithms,
 *	author={Henry Gordon Dietz},
 *	title={{The Aggregate Magic Algorithms}},
 *	institution={University of Kentucky},
 *	howpublished={Aggregate.Org online technical report},
 *	URL={http://aggregate.org/MAGIC/}
 *	}
 *
 */

/**
 * Absolute Value of a Float
 * IEEE floating point uses an explicit sign bit,
 * so the absolute value can be taken by a bitwise AND with the complement of the sign bit.
 * For IA32 32-bit, the sign bit is an int value of 0x80000000,
 * for IA32 64-bit, the sign bit is the long long int value 0x8000000000000000. Of course,
 * if you prefer to use just int values, the IA32 64-bit sign bit is 0x80000000 at an int address offset of +1.
 */
static inline constexpr
float abs(register float x) noexcept {
	*((int *) &x) &= 0x7fffffff;
	return x;
}

static inline constexpr
float abs(register double x) noexcept {
	*(((int *) &x) + 1) &= 0x7fffffff;
	return x;
}

/**
 * Average of Integers
 * This is actually an extension of the "well known" fact that for binary integer values x and y, (x+y) equals ((x&y)+(x|y)) equals ((x^y)+2*(x&y)).
 * Given two integer values x and y, the (floor of the) average normally would be computed by (x+y)/2; unfortunately,
 * this can yield incorrect results due to overflow. A very sneaky alternative is to use (x&y)+((x^y)/2).
 * If we are aware of the potential non-portability due to the fact that C does not specify if shifts are signed, this can be simplified to (x&y)+((x^y)>>1).
 * In either case, the benefit is that this code sequence cannot overflow.
 */
static inline constexpr
unsigned int avg(register unsigned int x, register unsigned int y) noexcept {
	return (x&y)+((x^y)>>1);
}
static inline constexpr
int avg(register int x, register int y) noexcept {
	return (x&y)+((x^y)>>1);
}
static inline constexpr
unsigned long long int avg(register unsigned long long int x, register unsigned long long int y) noexcept {
	return (x&y)+((x^y)>>1);
}
static inline constexpr
long long int avg(register long long int x, register long long int y) noexcept {
	return (x&y)+((x^y)>>1);
}

/**
 * Bit Reversal
 * Reversing the bits in an integer x is somewhat painful, but here's a SWAR algorithm for a 32-bit value
 */
static inline constexpr
unsigned int reverse(register unsigned int x) noexcept {
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
static inline constexpr
unsigned int reverse_v2(register unsigned int x) noexcept {
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
 * Comparison of Float Values
 * IEEE floating point has a number of nice properties,
 * including the ability to use 2's complement integer comparisons to compare floating point values,
 * provided the native byte order is consistent between float and integer values.
 * The only complication is the use of sign+magnitude representation in floats.
 * The AMD Athlon Processor x86 Code Optimization Guide gives a nice summary on Page 43. Here's a set of C routines that embody the same logic:
 *
 */
#define float_FasI(f)  (*((int *) &(f)))
#define float_FasUI(f) (*((unsigned int *) &(f)))

#define	float_lt0(f)	(float_FasUI(f) > 0x80000000U)
#define	float_le0(f)	(float_FasI(f) <= 0)
#define	float_gt0(f)	(float_FasI(f) > 0)
#define	float_ge0(f)	(float_FasUI(f) <= 0x80000000U)

/**
 * Integer Selection
 * A branchless, lookup-free, alternative to code like if (a<b) x=c; else x=d; is ((((a-b) >> (WORDBITS-1)) & (c^d)) ^ d).
 * This code assumes that the shift is signed, which, of course, C does not promise.
 */
static inline constexpr
int select_lt(register int a, register int b, register int c, register int d) noexcept {
	return ((((a-b) >> (32-1)) & (c^d)) ^ d);
}
static inline constexpr
long long int select_lt(register long long int a, register long long int b, register long long int c, register long long int d) noexcept {
	return ((((a-b) >> (64-1)) & (c^d)) ^ d);
}

/**
 * Integer Minimum or Maximum
 * Given 2's complement integer values x and y, the minimum can be computed without any branches as x+(((y-x)>>(WORDBITS-1))&(y-x)).
 * Logically, this works because the shift by (WORDBITS-1) replicates the sign bit to create a mask -- be aware,
 * however, that the C language does not require that shifts are signed even if their operands are signed, so there is a potential portability problem.
 * Additionally, one might think that a shift by any number greater than or equal to WORDBITS would have the same effect,
 * but many instruction sets have shifts that behave strangely when such shift distances are specified.
 *
 * Of course, maximum can be computed using the same trick: x-(((x-y)>>(WORDBITS-1))&(x-y)).
 *
 * Actually, the Integer Selection coding trick is just as efficient in encoding minimum and maximum....
 *
 */
static inline constexpr
int min(register int x, register int y) noexcept {
	return x+(((y-x)>>(32-1))&(y-x));
//	return select_lt(x, y, x, y);
}
static inline constexpr
long long int min(register long long int x, register long long int y) noexcept {
	return x+(((y-x)>>(64-1))&(y-x));
//	return select_lt(x, y, x, y);
}
static inline constexpr
int max(register int x, register int y) noexcept {
	return x-(((x-y)>>(32-1))&(x-y));
//	return select_lt(x, y, y, x);
}
static inline constexpr
long long int max(register long long int x, register long long int y) noexcept {
	return x-(((x-y)>>(64-1))&(x-y));
//	return select_lt(x, y, y, x);
}

/**
 * Is Power of 2
 * A non-negative binary integer value x is a power of 2 iff (x&(x-1)) is 0 using 2's complement arithmetic.
 */
static inline constexpr
bool ispw2(register unsigned int x) noexcept {
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
static inline constexpr
unsigned int ones32(register unsigned int x) noexcept {
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return (x & 0x0000003f);
}

static inline constexpr
unsigned int ones64(register unsigned long long x) noexcept {
	x = (x & 0x5555555555555555ull) + ((x >> 1) & 0x5555555555555555ull);
	x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
	return (((x + (x >> 4)) & 0x0f0f0f0f0f0f0f0full) * 0x0101010101010101ull) >> 56;
}
static inline constexpr
unsigned int ones(register unsigned int x) noexcept { return ones32(x); }

static inline constexpr
unsigned int ones(register unsigned long long x) noexcept { return ones64(x); }

static inline constexpr
unsigned int ones16(register unsigned int x) noexcept {
	x -= ((x >> 1) & 0x5555);
	x = (((x >> 2) & 0x3333) + (x & 0x3333));
	x = (((x >> 4) + x) & 0x0f0f);
	x += (x >> 8);
	return (x & 0x001f);
}

static inline constexpr
unsigned int ones8(register unsigned int x) noexcept {
	x -= ((x >> 1) & 0x5555);
	x = (((x >> 2) & 0x3333) + (x & 0x3333));
	x = (((x >> 4) + x) & 0x0f0f);
	return (x & 0x000f);
}

static inline constexpr
unsigned int ones4(register unsigned int x) noexcept {
	x = ((x & 0b1010) >> 1) + (x & 0b0101);
	return (((x & 0b1100) >> 2) + (x & 0b0011)) & 0b0111;
}


/**
 * Leading Zero Count
 * Some machines have had single instructions that count the number of leading zero bits in an integer;
 * such an operation can be an artifact of having floating point normalization hardware around. Clearly,
 * floor of base 2 log of x is (WORDBITS-lzc(x)). In any case, this operation has found its way
 * into quite a few algorithms, so it is useful to have an efficient implementation:
 */
static inline constexpr
unsigned int lzc32(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (32 - ones32(x));
}
static inline constexpr
unsigned int lzc64(register unsigned long long int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x |= (x >> 32);
	return (64 - ones64(x));
}
static inline constexpr
unsigned int lzc(register unsigned int x) noexcept { return lzc32(x); }
static inline constexpr
unsigned int lzc(register unsigned long long int x) noexcept { return lzc64(x); }

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
static inline constexpr
unsigned int lsb32(register unsigned int x) noexcept {
	return (x & -x);
//	return (x ^ (x & (x - 1)));
}
static inline constexpr
unsigned long long int lsb64(register unsigned long long int x) noexcept {
	return (x & -x);
//	return (x ^ (x & (x - 1)));
}
static inline constexpr
unsigned int lsb(register unsigned int x) noexcept { return lsb32(x); }
static inline constexpr
unsigned long long int lsb(register unsigned long long int x) noexcept { return lsb64(x); }

/**
 * Most Significant 1 Bit
 * Given a binary integer value x, the most significant 1 bit
 * (highest numbered element of a bit set) can be computed using a SWAR algorithm that recursively "folds"
 * the upper bits into the lower bits. This process yields a bit vector with
 * the same most significant 1 as x, but all 1's below it. Bitwise AND of the original value with
 * the complement of the "folded" value shifted down by one yields the most significant bit. For a 32-bit value:
 */
static inline constexpr
unsigned int msb32(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return (x & ~(x >> 1));
}

static inline constexpr
unsigned long long int msb64(register unsigned long long int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x |= (x >> 32);
	return (x & ~(x >> 1));
}
static inline constexpr
unsigned int msb(register unsigned int x) noexcept { return msb32(x); }

static inline constexpr
unsigned long long int msb(register unsigned long long int x) noexcept { return msb64(x); }

static inline constexpr
unsigned int msb16(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	return (x & ~(x >> 1));
}

static inline constexpr
unsigned int msb8(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	return (x & ~(x >> 1));
}

static inline constexpr
unsigned int msb4(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
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
static inline constexpr
unsigned int log2(register unsigned int x) noexcept {
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

static inline constexpr
unsigned int lg(register unsigned int x) noexcept {
	return log2(x);
}

static inline constexpr
unsigned int lg32(register unsigned int x) noexcept {
	return log2(x);
}

static inline constexpr
unsigned int lg64(register unsigned long long int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x |= (x >> 32);
#ifdef	LOG0UNDEFINED
	return (ones64(x) - 1);
#else
	return (ones64(x >> 1));
#endif
}

static inline constexpr
unsigned int lg16(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
#ifdef	LOG0UNDEFINED
	return (ones16(x) - 1);
#else
	return (ones16(x >> 1));
#endif
}

static inline constexpr
unsigned int lg8(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
#ifdef	LOG0UNDEFINED
	return (ones8(x) - 1);
#else
	return (ones8(x >> 1));
#endif
}

static inline constexpr
unsigned int lg4(register unsigned int x) noexcept {
	x |= (x >> 1);
	x |= (x >> 2);
#ifdef	LOG0UNDEFINED
	return (ones4(x) - 1);
#else
	return (ones4(x >> 1));
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
static inline constexpr
unsigned int log2_unstable(register unsigned int x) noexcept {
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
static inline constexpr
unsigned int nlpo2(register unsigned int x) noexcept {
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
template<typename T> static inline constexpr
void swap(T& x, T& y) noexcept {
	x ^= y; /* x' = (x^y) */
	y ^= x; /* y' = (y^(x^y)) = x */
	x ^= y; /* x' = (x^y)^x = y */
}

/**
 * Swap without temporary
 */
template<typename T> static inline constexpr
void swap_v2(T& x, T& y) noexcept {
	x += y; /* x' = (x+y) */
	y = x - y; /* y' = (x+y)-y = x */
	x -= y; /* x' = (x+y)-x = y */
}

/**
 * Trailing Zero Count
 * Given the Least Significant 1 Bit and Population Count (Ones Count) algorithms,
 * it is trivial to combine them to construct a trailing zero count (as pointed-out by Joe Bowbeer):
 */
static inline constexpr
unsigned int tzc32(register unsigned int x) noexcept {
	return (ones32((x & -x) - 1));
}
static inline constexpr
unsigned int tzc64(register unsigned long long int x) noexcept {
	return (ones64((x & -x) - 1));
}
static inline constexpr
unsigned int tzc(register unsigned int x) noexcept { return tzc32(x); }
static inline constexpr
unsigned int tzc(register unsigned long long int x) noexcept { return tzc64(x); }

/**
 * Tail recursive pow
 */
template<typename N, typename P> static inline constexpr
N pow_tail(N v, N b, P p) noexcept { return p ? pow_tail(v * b, b, p - 1) : v; }

/**
 * Tail recursive pow
 */
template<typename N, typename P> static inline constexpr
N pow(N b, P p) noexcept { return pow_tail(N(1), b, p); }

/**
 * mean
 */
template<typename numeric, typename iter> static inline constexpr
numeric mean(iter first, iter last, numeric init = 0) noexcept {
	return numeric(std::accumulate(first, last, init)) / std::distance(first, last);
}

/**
 * standard deviation
 */
template<typename numeric, typename iter> static inline constexpr
numeric deviation(iter first, iter last, numeric mean) noexcept {
	numeric sumofsqr = 0;
	for (iter it = first; it != last; std::advance(it, 1)) sumofsqr += std::pow(*it - mean, 2);
	return std::sqrt(sumofsqr / std::distance(first, last));
}

/**
 * standard deviation
 */
template<typename numeric, typename iter> static inline constexpr
numeric deviation(iter first, iter last) noexcept {
	return deviation<numeric>(first, last, mean<numeric>(first, last));
}

// reference: https://github.com/aappleby/smhasher/wiki/MurmurHash3
//            https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

/**
 * Finalization mix - force all bits of a hash block to avalanche
 */
static inline constexpr
uint32_t fmix32 (register uint32_t h) {
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

/**
 * Finalization mix - force all bits of a hash block to avalanche
 */
static inline constexpr
uint64_t fmix64 (register uint64_t h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdull;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53ull;
	h ^= h >> 33;
	return h;
}

} /* math */

/**
 * the full declaration of half is inside type.h
 * here is the 16-bit floating point operation
 */
constexpr half::half(f32 num) noexcept : hf(to_half(num)) {}
constexpr half::operator f32() const noexcept { return to_float(hf); }
constexpr half half::operator +(half f) const noexcept { return half::as(half_add(hf, f.hf)); }
constexpr half half::operator -(half f) const noexcept { return half::as(half_sub(hf, f.hf)); }
constexpr half half::operator *(half f) const noexcept { return half::as(half_mul(hf, f.hf)); }
constexpr half half::operator /(half f) const noexcept { return half::as(half_div(hf, f.hf)); }

} /* moporgic */
