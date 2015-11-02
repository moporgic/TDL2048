#pragma once
#define MOPORGIC_TYPE_H_

/*
 *  mopotype.h type.h
 *  Created on: 2012/10/30
 *      Author: moporgic
 */

typedef long long int ll;
typedef long double llf;
typedef long double quadruple;
typedef long double quadra;
//typedef long long long int i128;
typedef long long int i64;
typedef int i32;
//typedef long long long unsigned int u128;
typedef long long unsigned int u64;
typedef long long unsigned int ull;
typedef unsigned int u32;
typedef short i16;
typedef unsigned short u16;
typedef long double f128;
typedef double f64;
typedef float f32;
typedef char i8;
typedef unsigned char u8;
typedef unsigned char byte;
typedef short life; // life is short

//#define raw_cast(type, var) (*(type*)(&var))
template<typename T> inline T& raw_cast(const unsigned int& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const signed int& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const unsigned short& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const signed short& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const unsigned long long& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const signed long long& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const unsigned char& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const signed char& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const long double& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const double& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const float& a) { return *((T*) &a); }
template<typename T> inline T& raw_cast(const bool& a) { return *((T*) &a); }

struct u8c {
	byte v;
	inline u8c(const u32& b = 0);
	inline u8c(const u8c& b);
	inline u8c& operator =(const u32& b);
	inline u8c& operator -=(const u32& b);
	inline u8c& operator +=(const u32& b);
	inline u8c& operator <<=(const u32& b);
	inline u8c& operator >>=(const u32& b);
	inline u8c& operator &=(const u32& b);
	inline u8c& operator |=(const u32& b);
	inline bool operator ==(const u32& b) const;
	inline bool operator !=(const u32& b) const;
	inline bool operator <(const u32& b) const;
	inline bool operator >(const u32& b) const;
	inline bool operator <=(const u32& b) const;
	inline bool operator >=(const u32& b) const;
	inline bool operator !() const;
	inline u32 operator +(const u32& b) const;
	inline u32 operator -(const u32& b) const;
	inline u32 operator <<(const u32& s) const;
	inline u32 operator >>(const u32& s) const;
	inline u32 operator &(const u32& b) const;
	inline u32 operator |(const u32& b) const;
	inline u32 operator ~() const;
	inline u32 operator ++(int);
	inline u32 operator --(int);
	inline u32 operator ++();
	inline u32 operator --();
	inline operator u32() const;
	inline operator i32() const;
	inline operator bool() const;
	inline operator u8*() const;
	inline operator i8*() const;
};
union r16 {
	u16 v_u16;
	u8c v_u8c[2];
	u8 v_u8[2];
	inline r16(const u16& v = 0);
	inline r16(const r16& r);
	inline r16(const u8* b, const int& e = 0);
	inline r16(const i8* b, const int& e = 0);
	inline u8c& operator[](const int& i);
	inline operator u8*() const;
	inline operator i8*() const;
	inline operator u16() const;
	inline operator i16() const;
	inline operator u32() const;
	inline operator i32() const;
	inline operator u8() const;
	inline operator i8() const;
	inline operator bool() const;
	inline r16 endian(const int& e = 0) const;
	inline r16 to_le() const;
	inline r16 to_be() const;
};
union r32 {
	u32 v_u32;
	f32 v_f32;
	r16 v_r16[2];
	u8c v_u8c[4];
	u8 v_u8[4];
	inline r32(const u32& v = 0);
	inline r32(const r32& v);
	inline r32(const r16* v, const int& e = 0);
	inline r32(const u8* b, const int& e = 0);
	inline r32(const i8* b, const int& e = 0);
	inline u8c& operator[](const int& i);
	inline operator u8*() const;
	inline operator i8*() const;
	inline operator u32() const;
	inline operator i32() const;
	inline operator f32() const;
	inline operator u16() const;
	inline operator i16() const;
	inline operator u8() const;
	inline operator i8() const;
	inline operator bool() const;
	inline r32 endian(const int& e = 0) const;
	inline r32 to_le() const;
	inline r32 to_be() const;
};
union r64 {
	u64 v_u64;
	f64 v_f64;
	r16 v_r16[4];
	r32 v_r32[2];
	u8c v_u8c[8];
	u8 v_u8[8];
	inline r64(const u64& v = 0);
	inline r64(const r64& v);
	inline r64(const r16* v, const int& e = 0);
	inline r64(const r32* v, const int& e = 0);
	inline r64(const u8* b, const int& e = 0);
	inline r64(const i8* b, const int& e = 0);
	inline u8c& operator[](const int& i);
	inline operator u8*() const;
	inline operator i8*() const;
	inline operator u64() const;
	inline operator i64() const;
	inline operator f64() const;
	inline operator u32() const;
	inline operator i32() const;
	inline operator f32() const;
	inline operator u16() const;
	inline operator i16() const;
	inline operator u8() const;
	inline operator i8() const;
	inline operator bool() const;
	inline r64 endian(const int& e = 0) const;
	inline r64 to_le() const;
	inline r64 to_be() const;
};

namespace moporgic {
namespace endian {
const int b2endian[2][2] = { { 0,1 }, { 1,0 } };
const int b4endian[3][4] = { { 0,1,2,3 }, { 3,2,1,0 }, { 1,0,3,2 } };
const int b8endian[4][8] = { { 0,1,2,3,4,5,6,7 }, { 7,6,5,4,3,2,1,0 }, { 3,2,1,0,7,6,5,4 }, { 1,0,3,2,5,4,7,6 } };
const int le = r32(0x01)[sizeof(r32) - 1];
const int be = r32(0x01)[0];
inline bool is_le() { return le == 0; } // on a little-endian machine, LE will be 0 and BE will be 1
inline bool is_be() { return be == 0; } // on a big-endian machine,    LE will be 1 and BE will be 0
template<typename T> inline T repos(const T& v, const int& i, const int& p) { return ((v >> (i << 3)) & 0xf) << (p << 3); }
template<typename T> inline T swpos(const T& v, const int& i, const int& p) { return repos(v, i, p) | repos(v, p, i); }
inline u16 to_le(const u16& v) { return is_le() ? v : swpos(v, 0, 1); }
inline u32 to_le(const u32& v) { return is_le() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
inline u64 to_le(const u64& v) { return is_le() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
inline u16 to_be(const u16& v) { return is_be() ? v : swpos(v, 0, 1); }
inline u32 to_be(const u32& v) { return is_be() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
inline u64 to_be(const u64& v) { return is_be() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
}
}

inline u8c::u8c(const u32& b) : v(b & 0xff) {}
inline u8c::u8c(const u8c& b) : v(b.v) {}
inline u8c& u8c::operator =(const u32& b) { v = b & 0xff; return *this; }
inline u8c& u8c::operator -=(const u32& b) { v = (v - (b & 0xff)) & 0xff; return *this; }
inline u8c& u8c::operator +=(const u32& b) { v = (v + (b & 0xff)) & 0xff; return *this; }
inline u8c& u8c::operator <<=(const u32& s) { v = (v << s) & 0xff; return *this; }
inline u8c& u8c::operator >>=(const u32& s) { v = (v >> s) & 0xff; return *this; }
inline u8c& u8c::operator &=(const u32& b) { v = (v & b) & 0xff; return *this; }
inline u8c& u8c::operator |=(const u32& b) { v = (v | b) & 0xff; return *this; }
inline bool u8c::operator ==(const u32& b) const { return v == (b & 0xff); }
inline bool u8c::operator !=(const u32& b) const { return v != (b & 0xff); }
inline bool u8c::operator <(const u32& b) const { return v < (b & 0xff); }
inline bool u8c::operator >(const u32& b) const { return v > (b & 0xff); }
inline bool u8c::operator <=(const u32& b) const { return v <= (b & 0xff); }
inline bool u8c::operator >=(const u32& b) const { return v >= (b & 0xff); }
inline bool u8c::operator !() const { return (v & 0xff) == 0; }
inline u32 u8c::operator +(const u32& b) const { return u32(v) + b; }
inline u32 u8c::operator -(const u32& b) const { return u32(v) - b; }
inline u32 u8c::operator <<(const u32& s) const { return u32(v) << s; }
inline u32 u8c::operator >>(const u32& s) const { return u32(v) >> s; }
inline u32 u8c::operator &(const u32& b) const { return u32(v) & b; }
inline u32 u8c::operator |(const u32& b) const { return u32(v) | b; }
inline u32 u8c::operator ~() const { return ~u32(v); }
inline u32 u8c::operator ++(int) { u32 b = v++; return b; }
inline u32 u8c::operator --(int) { u32 b = v--; return b; }
inline u32 u8c::operator ++() { return ++v; }
inline u32 u8c::operator --() { return ++v; }
inline u8c::operator u32() const { return v; }
inline u8c::operator i32() const { return v; }
inline u8c::operator bool() const { return (v & 0xff) != 0; }
inline u8c::operator u8*() const { return (u8*) &v; }
inline u8c::operator i8*() const { return (i8*) &v; }

inline r16::r16(const u16& v) : v_u16(v) {}
inline r16::r16(const r16& r) : v_u16(r.v_u16) {}
inline r16::r16(const u8* b, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_u8c[i] = b[endian[i]];
}
inline r16::r16(const i8* b, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_u8c[i] = b[endian[i]];
}
inline u8c& r16::operator[](const int& i) { return v_u8c[i]; }
inline r16::operator u8*() const { return (u8*) v_u8; }
inline r16::operator i8*() const { return (i8*) v_u8; }
inline r16::operator u16() const { return v_u16; }
inline r16::operator i16() const { return v_u16; }
inline r16::operator u32() const { return v_u16; }
inline r16::operator i32() const { return v_u16; }
inline r16::operator u8() const { return v_u8[0]; }
inline r16::operator i8() const { return v_u8[0]; }
inline r16::operator bool() const { return static_cast<bool>(v_u16); }
inline r16 r16::endian(const int& e) const { return r16(operator u8*(), e); }
inline r16 r16::to_le() const { return moporgic::endian::to_le(v_u16); }
inline r16 r16::to_be() const { return moporgic::endian::to_be(v_u16); }

inline r32::r32(const u32& v) : v_u32(v) {}
inline r32::r32(const r32& r) : v_u32(r.v_u32) {}
inline r32::r32(const r16* v, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_r16[i] = v[endian[i]];
}
inline r32::r32(const u8* b, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_u8c[i] = b[endian[i]];
}
inline r32::r32(const i8* b, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_u8c[i] = b[endian[i]];
}
inline u8c& r32::operator[](const int& i) { return v_u8c[i]; }
inline r32::operator u8*() const { return (u8*) v_u8; }
inline r32::operator i8*() const { return (i8*) v_u8; }
inline r32::operator u32() const { return v_u32; }
inline r32::operator i32() const { return v_u32; }
inline r32::operator f32() const { return v_f32; }
inline r32::operator u16() const { return v_r16[0]; }
inline r32::operator i16() const { return v_r16[0]; }
inline r32::operator u8() const { return v_u8[0]; }
inline r32::operator i8() const { return v_u8[0]; }
inline r32::operator bool() const { return static_cast<bool>(v_u32); }
inline r32 r32::endian(const int& e) const { return r32(operator u8*(), e); }
inline r32 r32::to_le() const { return moporgic::endian::to_le(v_u32); }
inline r32 r32::to_be() const { return moporgic::endian::to_be(v_u32); }

inline r64::r64(const u64& v) : v_u64(v) {}
inline r64::r64(const r64& r) : v_u64(r.v_u64) {}
inline r64::r64(const r16* v, const int& e) {
	const int* endian = moporgic::endian::b4endian[e];
	for (int i = 0; i < 4; i++) v_r16[i] = v[endian[i]];
}
inline r64::r64(const r32* v, const int& e) {
	const int* endian = moporgic::endian::b2endian[e];
	for (int i = 0; i < 2; i++) v_r32[i] = v[endian[i]];
}
inline r64::r64(const u8* b, const int& e) {
	const int* endian = moporgic::endian::b8endian[e];
	for (int i = 0; i < 8; i++) v_u8c[i] = b[endian[i]];
}
inline r64::r64(const i8* b, const int& e) {
	const int* endian = moporgic::endian::b8endian[e];
	for (int i = 0; i < 8; i++) v_u8c[i] = b[endian[i]];
}
inline u8c& r64::operator[](const int& i) { return v_u8c[i]; }
inline r64::operator u8*() const { return (u8*) v_u8; }
inline r64::operator i8*() const { return (i8*) v_u8; }
inline r64::operator u64() const { return v_u64; }
inline r64::operator i64() const { return v_u64; }
inline r64::operator f64() const { return v_f64; }
inline r64::operator u32() const { return v_r32[0]; }
inline r64::operator i32() const { return v_r32[0]; }
inline r64::operator f32() const { return v_r32[0]; }
inline r64::operator u16() const { return v_r32[0]; }
inline r64::operator i16() const { return v_r32[0]; }
inline r64::operator u8() const { return v_u8[0]; }
inline r64::operator i8() const { return v_u8[0]; }
inline r64::operator bool() const { return static_cast<bool>(v_u64); }
inline r64 r64::endian(const int& e) const { return r64(operator u8*(), e); }
inline r64 r64::to_le() const { return moporgic::endian::to_le(v_u64); }
inline r64 r64::to_be() const { return moporgic::endian::to_le(v_u64); }

#ifdef MOPORGIC_MACRO
#define DECLARE_OP_TCMP(T, op, v) inline bool operator op(const T& t) const { return v op t.v; }
#define DECLARE_OP_NEQ(T) inline bool operator !=(const T& t) const { return !(operator ==(t)); }
#define DECLARE_OP_CMP(T, op) inline bool operator op(const T& v) const { return operator T() op v; }
#endif
