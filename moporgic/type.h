#pragma once
#define MOPORGIC_TYPE_H_

/*
 *  mopotype.h type.h
 *  Created on: 2012/10/30
 *      Author: moporgic
 */

#include <cstddef>
#include <cstdint>

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
typedef char     byte;

typedef __int128          i128;
typedef unsigned __int128 u128;

typedef short life; // life is short
typedef long double llf;
typedef long double quadruple;
typedef long double quadra;
typedef long double f128;
typedef double f64;
typedef float  f32;

typedef intptr_t  iptr;
typedef uintptr_t uptr;

//#define raw_cast(type, var) (*(type*)(&var))

template<typename dst, typename src> inline constexpr
dst ptr_cast(src p) { return (dst) p; /*return reinterpret_cast<dst>(p);*/ }

template<typename dst, typename src> inline constexpr
dst& raw_cast(src& v, const int off = 0) { return *(ptr_cast<dst*>(&v) + off); }

struct u8c {
	byte v;
	inline constexpr u8c(const u32& b = 0);
	inline constexpr u8c(const u8c& b);
	inline constexpr u8c& operator =(const u32& b);
	inline constexpr u8c& operator -=(const u32& b);
	inline constexpr u8c& operator +=(const u32& b);
	inline constexpr u8c& operator <<=(const u32& b);
	inline constexpr u8c& operator >>=(const u32& b);
	inline constexpr u8c& operator &=(const u32& b);
	inline constexpr u8c& operator |=(const u32& b);
	inline constexpr bool operator ==(const u32& b) const;
	inline constexpr bool operator !=(const u32& b) const;
	inline constexpr bool operator <(const u32& b) const;
	inline constexpr bool operator >(const u32& b) const;
	inline constexpr bool operator <=(const u32& b) const;
	inline constexpr bool operator >=(const u32& b) const;
	inline constexpr bool operator !() const;
	inline constexpr u32 operator +(const u32& b) const;
	inline constexpr u32 operator -(const u32& b) const;
	inline constexpr u32 operator <<(const u32& s) const;
	inline constexpr u32 operator >>(const u32& s) const;
	inline constexpr u32 operator &(const u32& b) const;
	inline constexpr u32 operator |(const u32& b) const;
	inline constexpr u32 operator ~() const;
	inline constexpr u32 operator ++(int);
	inline constexpr u32 operator --(int);
	inline constexpr u32 operator ++();
	inline constexpr u32 operator --();
	inline constexpr operator u32() const;
	inline constexpr operator i32() const;
	inline constexpr operator bool() const;
	inline constexpr operator u8*() const;
	inline constexpr operator i8*() const;
	inline constexpr operator byte*() const;
};
union r16 {
	u16 v_u16;
	u8c v_u8c[2];
	u8 v_u8[2];
	inline constexpr r16(const u16& v = 0);
	inline constexpr r16(const i16& v);
	inline constexpr r16(const u32& v);
	inline constexpr r16(const i32& v);
	inline constexpr r16(const u64& v);
	inline constexpr r16(const i64& v);
	inline constexpr r16(const r16& v);
	inline constexpr r16(const u8& v);
	inline constexpr r16(const i8& v);
	inline constexpr r16(const u8* b);
	inline constexpr r16(const i8* b);
	inline r16(const u8* b, const int& e);
	inline r16(const i8* b, const int& e);
	inline constexpr r16(const byte* b);
	inline r16(const byte* b, const int& e);
	inline constexpr r16(u8* b);
	inline constexpr r16(i8* b);
	inline r16(u8* b, const int& e);
	inline r16(i8* b, const int& e);
//	inline constexpr r16(const size_t& v);
	inline constexpr u8c& operator[](const int& i);
	inline constexpr operator u8*() const;
	inline constexpr operator i8*() const;
	inline constexpr operator u16() const;
	inline constexpr operator i16() const;
	inline constexpr operator u32() const;
	inline constexpr operator i32() const;
	inline constexpr operator u8() const;
	inline constexpr operator i8() const;
	inline constexpr operator bool() const;
	inline constexpr operator byte*() const;
	inline r16 endian(const int& e = 0) const;
	inline r16 le() const;
	inline r16 be() const;
};
union r32 {
	u32 v_u32;
	f32 v_f32;
	r16 v_r16[2];
	u8c v_u8c[4];
	u8 v_u8[4];
	inline constexpr r32(const u32& v = 0);
	inline constexpr r32(const i32& v);
	inline constexpr r32(const u16& v);
	inline constexpr r32(const i16& v);
	inline constexpr r32(const u64& v);
	inline constexpr r32(const i64& v);
	inline constexpr r32(const f32& v);
	inline constexpr r32(const f64& v);
	inline constexpr r32(const r32& v);
	inline constexpr r32(const u8& v);
	inline constexpr r32(const i8& v);
	inline constexpr r32(const u8* b);
	inline constexpr r32(const i8* b);
	inline r32(const r16* v, const int& e);
	inline r32(const u8* b, const int& e);
	inline r32(const i8* b, const int& e);
	inline constexpr r32(const byte* b);
	inline r32(const byte* b, const int& e);
	inline constexpr r32(u8* b);
	inline constexpr r32(i8* b);
	inline r32(r16* v, const int& e);
	inline r32(u8* b, const int& e);
	inline r32(i8* b, const int& e);
//	inline constexpr r32(const size_t& v);
	inline constexpr u8c& operator[](const int& i);
	inline constexpr operator u8*() const;
	inline constexpr operator i8*() const;
	inline constexpr operator u32() const;
	inline constexpr operator i32() const;
	inline constexpr operator f32() const;
	inline constexpr operator u16() const;
	inline constexpr operator i16() const;
	inline constexpr operator u8() const;
	inline constexpr operator i8() const;
	inline constexpr operator bool() const;
	inline constexpr operator byte*() const;
	inline r32 endian(const int& e = 0) const;
	inline r32 le() const;
	inline r32 be() const;
};
union r64 {
	u64 v_u64;
	f64 v_f64;
	r16 v_r16[4];
	r32 v_r32[2];
	u8c v_u8c[8];
	u8 v_u8[8];
	inline constexpr r64(const u64& v = 0);
	inline constexpr r64(const i64& v);
	inline constexpr r64(const u32& v);
	inline constexpr r64(const i32& v);
	inline constexpr r64(const u16& v);
	inline constexpr r64(const i16& v);
	inline constexpr r64(const f32& v);
	inline constexpr r64(const f64& v);
	inline constexpr r64(const r64& v);
	inline constexpr r64(const u8& v);
	inline constexpr r64(const i8& v);
	inline constexpr r64(const u8* b);
	inline constexpr r64(const i8* b);
	inline r64(const r16* v, const int& e);
	inline r64(const r32* v, const int& e);
	inline r64(const u8* b, const int& e);
	inline r64(const i8* b, const int& e);
	inline constexpr r64(const byte* b);
	inline r64(const byte* b, const int& e);
	inline constexpr r64(u8* b);
	inline constexpr r64(i8* b);
	inline r64(r16* v, const int& e);
	inline r64(r32* v, const int& e);
	inline r64(u8* b, const int& e);
	inline r64(i8* b, const int& e);
//	inline constexpr r64(const size_t& v);
	inline constexpr u8c& operator[](const int& i);
	inline constexpr operator u8*() const;
	inline constexpr operator i8*() const;
	inline constexpr operator u64() const;
	inline constexpr operator i64() const;
	inline constexpr operator f64() const;
	inline constexpr operator u32() const;
	inline constexpr operator i32() const;
	inline constexpr operator f32() const;
	inline constexpr operator u16() const;
	inline constexpr operator i16() const;
	inline constexpr operator u8() const;
	inline constexpr operator i8() const;
	inline constexpr operator bool() const;
	inline constexpr operator byte*() const;
	inline r64 endian(const int& e = 0) const;
	inline r64 le() const;
	inline r64 be() const;
};

namespace moporgic {
namespace endian {
constexpr int b2endian[2][2] = { { 0,1 }, { 1,0 } };
constexpr int b4endian[3][4] = { { 0,1,2,3 }, { 3,2,1,0 }, { 1,0,3,2 } };
constexpr int b8endian[4][8] = { { 0,1,2,3,4,5,6,7 }, { 7,6,5,4,3,2,1,0 }, { 3,2,1,0,7,6,5,4 }, { 1,0,3,2,5,4,7,6 } };
const int le = r32(0x01)[3];
const int be = r32(0x01)[0];
inline bool is_le() { return le == 0; } // on a little-endian machine, LE will be 0 and BE will be 1
inline bool is_be() { return be == 0; } // on a big-endian machine,    LE will be 1 and BE will be 0
template<typename T> inline constexpr T repos(const T& v, const int& i, const int& p) { return ((v >> (i << 3)) & 0xff) << (p << 3); }
template<typename T> inline constexpr T swpos(const T& v, const int& i, const int& p) { return repos(v, i, p) | repos(v, p, i); }
inline u16 to_le(const u16& v) { return is_le() ? v : swpos(v, 0, 1); }
inline u32 to_le(const u32& v) { return is_le() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
inline u64 to_le(const u64& v) { return is_le() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
inline u16 to_be(const u16& v) { return is_be() ? v : swpos(v, 0, 1); }
inline u32 to_be(const u32& v) { return is_be() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
inline u64 to_be(const u64& v) { return is_be() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
}
}

inline constexpr u8c::u8c(const u32& b) : v(b) {}
inline constexpr u8c::u8c(const u8c& b) : v(b.v) {}
inline constexpr u8c& u8c::operator =(const u32& b) { v = b; return *this; }
inline constexpr u8c& u8c::operator -=(const u32& b) { v -= byte(b); return *this; }
inline constexpr u8c& u8c::operator +=(const u32& b) { v += byte(b); return *this; }
inline constexpr u8c& u8c::operator <<=(const u32& s) { v <<= s; return *this; }
inline constexpr u8c& u8c::operator >>=(const u32& s) { v >>= s; return *this; }
inline constexpr u8c& u8c::operator &=(const u32& b) { v &= byte(b); return *this; }
inline constexpr u8c& u8c::operator |=(const u32& b) { v |= byte(b); return *this; }
inline constexpr bool u8c::operator ==(const u32& b) const { return v == byte(b); }
inline constexpr bool u8c::operator !=(const u32& b) const { return v != byte(b); }
inline constexpr bool u8c::operator <(const u32& b) const { return v < byte(b); }
inline constexpr bool u8c::operator >(const u32& b) const { return v > byte(b); }
inline constexpr bool u8c::operator <=(const u32& b) const { return v <= byte(b); }
inline constexpr bool u8c::operator >=(const u32& b) const { return v >= byte(b); }
inline constexpr bool u8c::operator !() const { return (v & 0xff) == 0; }
inline constexpr u32 u8c::operator +(const u32& b) const { return u32(v) + b; }
inline constexpr u32 u8c::operator -(const u32& b) const { return u32(v) - b; }
inline constexpr u32 u8c::operator <<(const u32& s) const { return u32(v) << s; }
inline constexpr u32 u8c::operator >>(const u32& s) const { return u32(v) >> s; }
inline constexpr u32 u8c::operator &(const u32& b) const { return u32(v) & b; }
inline constexpr u32 u8c::operator |(const u32& b) const { return u32(v) | b; }
inline constexpr u32 u8c::operator ~() const { return ~u32(v); }
inline constexpr u32 u8c::operator ++(int) { u32 b = v++; return b; }
inline constexpr u32 u8c::operator --(int) { u32 b = v--; return b; }
inline constexpr u32 u8c::operator ++() { return ++v; }
inline constexpr u32 u8c::operator --() { return ++v; }
inline constexpr u8c::operator u32() const { return v; }
inline constexpr u8c::operator i32() const { return v; }
inline constexpr u8c::operator bool() const { return (v & 0xff) != 0; }
inline constexpr u8c::operator u8*() const { return ptr_cast<u8*>(&v); }
inline constexpr u8c::operator i8*() const { return ptr_cast<i8*>(&v); }
inline constexpr u8c::operator byte*() const { return ptr_cast<byte*>(&v); }

inline constexpr r16::r16(const u16& v) : v_u16(v) {}
inline constexpr r16::r16(const i16& v) : v_u16(v) {}
inline constexpr r16::r16(const u32& v) : v_u16(v) {}
inline constexpr r16::r16(const i32& v) : v_u16(v) {}
inline constexpr r16::r16(const u64& v) : v_u16(v) {}
inline constexpr r16::r16(const i64& v) : v_u16(v) {}
inline constexpr r16::r16(const r16& v) : v_u16(v) {}
inline constexpr r16::r16(const u8& v)  : v_u16(v) {}
inline constexpr r16::r16(const i8& v)  : v_u16(v) {}
inline constexpr r16::r16(const u8* b)  : r16(const_cast<u8*>(b)) {}
inline constexpr r16::r16(const i8* b)  : r16(const_cast<i8*>(b)) {};
inline r16::r16(const u8* b, const int& e) : r16(const_cast<u8*>(b), e) {}
inline r16::r16(const i8* b, const int& e) : r16(const_cast<i8*>(b), e) {}
inline constexpr r16::r16(const byte* b)  : r16(ptr_cast<i8*>(b)) {};
inline r16::r16(const byte* b, const int& e) : r16(ptr_cast<i8*>(b), e) {}
inline constexpr r16::r16(u8* b)  : v_u16(*ptr_cast<u16*>(b)) {}
inline constexpr r16::r16(i8* b)  : v_u16(*ptr_cast<u16*>(b)) {}
inline r16::r16(u8* b, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_u8c[i] = b[endian[i]];
}
inline r16::r16(i8* b, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_u8c[i] = b[endian[i]];
}
//inline constexpr r16::r16(const size_t& v) : v_u16(v) {}
inline constexpr u8c& r16::operator[](const int& i) { return v_u8c[i]; }
inline constexpr r16::operator u8*() const { return ptr_cast<u8*>(v_u8); }
inline constexpr r16::operator i8*() const { return ptr_cast<i8*>(v_u8); }
inline constexpr r16::operator u16() const { return v_u16; }
inline constexpr r16::operator i16() const { return v_u16; }
inline constexpr r16::operator u32() const { return v_u16; }
inline constexpr r16::operator i32() const { return v_u16; }
inline constexpr r16::operator u8() const { return v_u8[0]; }
inline constexpr r16::operator i8() const { return v_u8[0]; }
inline constexpr r16::operator bool() const { return static_cast<bool>(v_u16); }
inline constexpr r16::operator byte*() const { return ptr_cast<byte*>(v_u8); }
inline r16 r16::endian(const int& e) const { return r16(operator u8*(), e); }
inline r16 r16::le() const { return moporgic::endian::to_le(v_u16); }
inline r16 r16::be() const { return moporgic::endian::to_be(v_u16); }

inline constexpr r32::r32(const u32& v) : v_u32(v) {}
inline constexpr r32::r32(const i32& v) : v_u32(v) {}
inline constexpr r32::r32(const u16& v) : v_u32(v) {}
inline constexpr r32::r32(const i16& v) : v_u32(v) {}
inline constexpr r32::r32(const u64& v) : v_u32(v) {}
inline constexpr r32::r32(const i64& v) : v_u32(v) {}
inline constexpr r32::r32(const f32& v) : v_f32(v) {}
inline constexpr r32::r32(const f64& v) : v_f32(v) {}
inline constexpr r32::r32(const r32& v) : v_u32(v) {}
inline constexpr r32::r32(const u8& v)  : v_u32(v) {}
inline constexpr r32::r32(const i8& v)  : v_u32(v) {}
inline constexpr r32::r32(const u8* b)  : r32(const_cast<u8*>(b)) {}
inline constexpr r32::r32(const i8* b)  : r32(const_cast<i8*>(b)) {}
inline r32::r32(const r16* v, const int& e) : r32(const_cast<r16*>(v), e) {}
inline r32::r32(const u8* b,  const int& e) : r32(const_cast<u8*>(b),  e) {}
inline r32::r32(const i8* b,  const int& e) : r32(const_cast<i8*>(b),  e) {}
inline constexpr r32::r32(const byte* b)  : r32(ptr_cast<i8*>(b)) {};
inline r32::r32(const byte* b, const int& e) : r32(ptr_cast<i8*>(b), e) {}
inline constexpr r32::r32(u8* b)  : v_u32(*ptr_cast<u32*>(b)) {}
inline constexpr r32::r32(i8* b)  : v_u32(*ptr_cast<u32*>(b)) {}
inline r32::r32(r16* v, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_r16[i] = v[endian[i]];
}
inline r32::r32(u8* b, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_u8c[i] = b[endian[i]];
}
inline r32::r32(i8* b, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_u8c[i] = b[endian[i]];
}
//inline constexpr r32::r32(const size_t& v) : v_u32(v) {}
inline constexpr u8c& r32::operator[](const int& i) { return v_u8c[i]; }
inline constexpr r32::operator u8*() const { return ptr_cast<u8*>(v_u8); }
inline constexpr r32::operator i8*() const { return ptr_cast<i8*>(v_u8); }
inline constexpr r32::operator u32() const { return v_u32; }
inline constexpr r32::operator i32() const { return v_u32; }
inline constexpr r32::operator f32() const { return v_f32; }
inline constexpr r32::operator u16() const { return v_r16[0]; }
inline constexpr r32::operator i16() const { return v_r16[0]; }
inline constexpr r32::operator u8() const { return v_u8[0]; }
inline constexpr r32::operator i8() const { return v_u8[0]; }
inline constexpr r32::operator bool() const { return static_cast<bool>(v_u32); }
inline constexpr r32::operator byte*() const { return ptr_cast<byte*>(v_u8); }
inline r32 r32::endian(const int& e) const { return r32(operator u8*(), e); }
inline r32 r32::le() const { return moporgic::endian::to_le(v_u32); }
inline r32 r32::be() const { return moporgic::endian::to_be(v_u32); }

inline constexpr r64::r64(const u64& v) : v_u64(v) {}
inline constexpr r64::r64(const i64& v) : v_u64(v) {}
inline constexpr r64::r64(const u32& v) : v_u64(v) {}
inline constexpr r64::r64(const i32& v) : v_u64(v) {}
inline constexpr r64::r64(const u16& v) : v_u64(v) {}
inline constexpr r64::r64(const i16& v) : v_u64(v) {}
inline constexpr r64::r64(const f32& v) : v_f64(v) {}
inline constexpr r64::r64(const f64& v) : v_f64(v) {}
inline constexpr r64::r64(const r64& v) : v_u64(v) {}
inline constexpr r64::r64(const u8& v)  : v_u64(v) {}
inline constexpr r64::r64(const i8& v)  : v_u64(v) {}
inline constexpr r64::r64(const u8* b)  : r64(const_cast<u8*>(b)) {}
inline constexpr r64::r64(const i8* b)  : r64(const_cast<i8*>(b)) {}
inline r64::r64(const r16* v, const int& e) : r64(const_cast<r16*>(v), e) {}
inline r64::r64(const r32* v, const int& e) : r64(const_cast<r32*>(v), e) {}
inline r64::r64(const u8* b, const int& e)  : r64(const_cast<u8*>(b), e) {}
inline r64::r64(const i8* b, const int& e)  : r64(const_cast<i8*>(b), e) {}
inline constexpr r64::r64(const byte* b)  : r64(ptr_cast<i8*>(b)) {};
inline r64::r64(const byte* b, const int& e) : r64(ptr_cast<i8*>(b), e) {}
inline constexpr r64::r64(u8* b)  : v_u64(*ptr_cast<u64*>(b)) {}
inline constexpr r64::r64(i8* b)  : v_u64(*ptr_cast<u64*>(b)) {}
inline r64::r64(r16* v, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_r16[i] = v[endian[i]];
}
inline r64::r64(r32* v, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_r32[i] = v[endian[i]];
}
inline r64::r64(u8* b, const int& e) {
	const int* endian = moporgic::endian::b8endian[e];
	for (int i = 0; i < 8; i++) v_u8c[i] = b[endian[i]];
}
inline r64::r64(i8* b, const int& e) {
	const int* endian = moporgic::endian::b8endian[e];
	for (int i = 0; i < 8; i++) v_u8c[i] = b[endian[i]];
}
//inline constexpr r64::r64(const size_t& v) : v_u64(v) {}
inline constexpr u8c& r64::operator[](const int& i) { return v_u8c[i]; }
inline constexpr r64::operator u8*() const { return ptr_cast<u8*>(v_u8); }
inline constexpr r64::operator i8*() const { return ptr_cast<i8*>(v_u8); }
inline constexpr r64::operator u64() const { return v_u64; }
inline constexpr r64::operator i64() const { return v_u64; }
inline constexpr r64::operator f64() const { return v_f64; }
inline constexpr r64::operator u32() const { return v_r32[0]; }
inline constexpr r64::operator i32() const { return v_r32[0]; }
inline constexpr r64::operator f32() const { return v_r32[0]; }
inline constexpr r64::operator u16() const { return v_r32[0]; }
inline constexpr r64::operator i16() const { return v_r32[0]; }
inline constexpr r64::operator u8() const { return v_u8[0]; }
inline constexpr r64::operator i8() const { return v_u8[0]; }
inline constexpr r64::operator bool() const { return static_cast<bool>(v_u64); }
inline constexpr r64::operator byte*() const { return ptr_cast<byte*>(v_u8); }
inline r64 r64::endian(const int& e) const { return r64(operator u8*(), e); }
inline r64 r64::le() const { return moporgic::endian::to_le(v_u64); }
inline r64 r64::be() const { return moporgic::endian::to_le(v_u64); }

#ifdef MOPORGIC_MACRO
#define DECLARE_OP_TCMP(T, op, v) inline bool operator op(const T& t) const { return v op t.v; }
#define DECLARE_OP_NEQ(T) inline bool operator !=(const T& t) const { return !(operator ==(t)); }
#define DECLARE_OP_CMP(T, op) inline bool operator op(const T& v) const { return operator T() op v; }
#endif
