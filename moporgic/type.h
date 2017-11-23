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
//typedef char     byte;
struct  byte;

typedef __int128          i128;
typedef unsigned __int128 u128;
typedef unsigned char     uchar;
typedef unsigned long int ulint;

typedef short life; // life is short
typedef long double llf;
typedef long double quadruple;
typedef long double quadra;
typedef long double f128;
typedef double f64;
typedef float  f32;

typedef intptr_t  iptr;
typedef uintptr_t uptr;

#if !defined(__cplusplus) || __cplusplus < 201103L
#define constexpr
#define noexcept
#endif

template<typename dst, typename src> static inline constexpr
dst  cast(src ptr) noexcept { return (dst) ptr; /*return reinterpret_cast<dst>(p);*/ }

template<typename dst, typename src> static inline constexpr
dst* pointer_cast(src* ptr) noexcept { return cast<dst*>(ptr); }

template<typename dst, typename src> static inline constexpr
dst& reference_cast(src& ref) noexcept { return *(pointer_cast<dst>(&ref)); }

template<typename dst, int off = 0, typename src> static inline constexpr
dst& raw_cast(src& ref, int sh = 0) noexcept { return *(pointer_cast<dst>(&ref) + off + sh); }

struct byte {
	unsigned char v;
	constexpr byte(unsigned char v = 0) noexcept : v(v) {}
	constexpr operator unsigned char&() noexcept { return v; }
	constexpr operator unsigned char*() noexcept { return &v; }
	constexpr operator char*() noexcept { return cast<char*>(&v); }
	constexpr operator const unsigned char&() const noexcept { return v; }
	constexpr operator const unsigned char*() const noexcept { return &v; }
	constexpr operator const char*() const noexcept { return cast<char*>(&v); }
	friend std::ostream& operator <<(std::ostream& os, const byte& b) {
		register auto hi = (b.v >> 4) & 0xf;
		register auto lo = (b.v >> 0) & 0xf;
		os << cast<char>(hi < 10 ? hi + '0' : hi + 'a' - 10);
		os << cast<char>(lo < 10 ? lo + '0' : lo + 'a' - 10);
		return os;
	}
	friend std::istream& operator >>(std::istream& is, const byte& b) {
		register unsigned char hi, lo;
		is >> hi >> lo;
		hi = (hi & 15) + (hi >> 6) * 9;
		lo = (lo & 15) + (lo >> 6) * 9;
		if (is) const_cast<byte&>(b).v = (hi << 4) | lo;
		return is;
	}
};

struct u8c {
	u8 v;
	constexpr u8c(const u32& b = 0) noexcept : v(b) {}
	constexpr u8c(const u8c& b) noexcept : v(b.v) {}
	constexpr u8c& operator =(const u32& b) noexcept { v = b; return *this; }
	constexpr u8c& operator -=(const u32& b) noexcept { v -= byte(b); return *this; }
	constexpr u8c& operator +=(const u32& b) noexcept { v += byte(b); return *this; }
	constexpr u8c& operator <<=(const u32& s) noexcept { v <<= s; return *this; }
	constexpr u8c& operator >>=(const u32& s) noexcept { v >>= s; return *this; }
	constexpr u8c& operator &=(const u32& b) noexcept { v &= byte(b); return *this; }
	constexpr u8c& operator |=(const u32& b) noexcept { v |= byte(b); return *this; }
	constexpr bool operator ==(const u32& b) const noexcept { return v == byte(b); }
	constexpr bool operator !=(const u32& b) const noexcept { return v != byte(b); }
	constexpr bool operator <(const u32& b) const noexcept { return v < byte(b); }
	constexpr bool operator >(const u32& b) const noexcept { return v > byte(b); }
	constexpr bool operator <=(const u32& b) const noexcept { return v <= byte(b); }
	constexpr bool operator >=(const u32& b) const noexcept { return v >= byte(b); }
	constexpr bool operator !() const noexcept { return (v & 0xff) == 0; }
	constexpr u32 operator +(const u32& b) const noexcept { return u32(v) + b; }
	constexpr u32 operator -(const u32& b) const noexcept { return u32(v) - b; }
	constexpr u32 operator <<(const u32& s) const noexcept { return u32(v) << s; }
	constexpr u32 operator >>(const u32& s) const noexcept { return u32(v) >> s; }
	constexpr u32 operator &(const u32& b) const noexcept { return u32(v) & b; }
	constexpr u32 operator |(const u32& b) const noexcept { return u32(v) | b; }
	constexpr u32 operator ~() const noexcept { return ~u32(v); }
	constexpr u32 operator ++(int) noexcept { u32 b = v++; return b; }
	constexpr u32 operator --(int) noexcept { u32 b = v--; return b; }
	constexpr u32 operator ++() noexcept { return ++v; }
	constexpr u32 operator --() noexcept { return ++v; }
	constexpr operator u32() const noexcept { return v; }
	constexpr operator i32() const noexcept { return v; }
	constexpr operator bool() const noexcept { return (v & 0xff) != 0; }
	constexpr operator u8*() const noexcept { return cast<u8*>(&v); }
	constexpr operator i8*() const noexcept { return cast<i8*>(&v); }
	constexpr operator byte*() const noexcept { return cast<byte*>(&v); }
};

namespace moporgic {
namespace endian {
static inline constexpr bool is_le() noexcept { register u32 v = 0x01; return raw_cast<byte, 0>(v); }
static inline constexpr bool is_be() noexcept { register u32 v = 0x01; return raw_cast<byte, 3>(v); }
template<typename T> static inline constexpr T repos(T v, int i, int p) noexcept { return ((v >> (i << 3)) & 0xff) << (p << 3); }
template<typename T> static inline constexpr T swpos(T v, int i, int p) noexcept { return repos(v, i, p) | repos(v, p, i); }
static inline constexpr u16 to_le(u16 v) noexcept { return is_le() ? v : swpos(v, 0, 1); }
static inline constexpr u32 to_le(u32 v) noexcept { return is_le() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
static inline constexpr u64 to_le(u64 v) noexcept { return is_le() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
static inline constexpr u16 to_be(u16 v) noexcept { return is_be() ? v : swpos(v, 0, 1); }
static inline constexpr u32 to_be(u32 v) noexcept { return is_be() ? v : swpos(v, 0, 3) | swpos(v, 1, 2); }
static inline constexpr u64 to_be(u64 v) noexcept { return is_be() ? v : swpos(v, 0, 7) | swpos(v, 1, 6) | swpos(v, 2, 5) | swpos(v, 3, 4); }
} // namespace endian
} // namespace moporgic

union r16 {
	u16 v_u16;
	u8c v_u8c[2];
	u8 v_u8[2];
	constexpr r16(const u16& v = 0) noexcept : v_u16(v) {}
	constexpr r16(const i16& v) noexcept : v_u16(v) {}
	constexpr r16(const u32& v) noexcept : v_u16(v) {}
	constexpr r16(const i32& v) noexcept : v_u16(v) {}
	constexpr r16(const u64& v) noexcept : v_u16(v) {}
	constexpr r16(const i64& v) noexcept : v_u16(v) {}
	constexpr r16(const r16& v) noexcept : v_u16(v) {}
	constexpr r16(const u8& v) noexcept  : v_u16(v) {}
	constexpr r16(const i8& v) noexcept  : v_u16(v) {}
	constexpr r16(const u8* b) noexcept  : r16(const_cast<u8*>(b)) {}
	constexpr r16(const i8* b) noexcept  : r16(const_cast<i8*>(b)) {};
	constexpr r16(const byte* b) noexcept  : r16(cast<i8*>(b)) {};
	constexpr r16(u8* b) noexcept  : v_u16(*cast<u16*>(b)) {}
	constexpr r16(i8* b) noexcept  : v_u16(*cast<u16*>(b)) {}
	constexpr u8c& operator[](const int& i) noexcept { return v_u8c[i]; }
	constexpr operator u8*() const noexcept { return cast<u8*>(v_u8); }
	constexpr operator i8*() const noexcept { return cast<i8*>(v_u8); }
	constexpr operator u64() const noexcept { return v_u16; }
	constexpr operator i64() const noexcept { return v_u16; }
	constexpr operator f64() const noexcept { return v_u16; }
	constexpr operator u32() const noexcept { return v_u16; }
	constexpr operator i32() const noexcept { return v_u16; }
	constexpr operator f32() const noexcept { return v_u16; }
	constexpr operator u16() const noexcept { return v_u16; }
	constexpr operator i16() const noexcept { return v_u16; }
	constexpr operator u8() const noexcept { return v_u8[0]; }
	constexpr operator i8() const noexcept { return v_u8[0]; }
	constexpr operator bool() const noexcept { return static_cast<bool>(v_u16); }
	constexpr operator byte*() const noexcept { return cast<byte*>(v_u8); }
	constexpr r16 le() const noexcept { return moporgic::endian::to_le(v_u16); }
	constexpr r16 be() const noexcept { return moporgic::endian::to_be(v_u16); }
};

union r32 {
	u32 v_u32;
	f32 v_f32;
	r16 v_r16[2];
	u8c v_u8c[4];
	u8 v_u8[4];
	constexpr r32(const u32& v = 0) noexcept : v_u32(v) {}
	constexpr r32(const i32& v) noexcept : v_u32(v) {}
	constexpr r32(const u16& v) noexcept : v_u32(v) {}
	constexpr r32(const i16& v) noexcept : v_u32(v) {}
	constexpr r32(const u64& v) noexcept : v_u32(v) {}
	constexpr r32(const i64& v) noexcept : v_u32(v) {}
	constexpr r32(const f32& v) noexcept : v_f32(v) {}
	constexpr r32(const f64& v) noexcept : v_f32(v) {}
	constexpr r32(const r32& v) noexcept : v_u32(v) {}
	constexpr r32(const u8& v) noexcept  : v_u32(v) {}
	constexpr r32(const i8& v) noexcept  : v_u32(v) {}
	constexpr r32(const u8* b) noexcept  : r32(const_cast<u8*>(b)) {}
	constexpr r32(const i8* b) noexcept  : r32(const_cast<i8*>(b)) {}
	constexpr r32(const byte* b) noexcept  : r32(cast<i8*>(b)) {};
	constexpr r32(u8* b) noexcept  : v_u32(*cast<u32*>(b)) {}
	constexpr r32(i8* b) noexcept  : v_u32(*cast<u32*>(b)) {}
	constexpr u8c& operator[](const int& i) noexcept { return v_u8c[i]; }
	constexpr operator u8*() const noexcept { return cast<u8*>(v_u8); }
	constexpr operator i8*() const noexcept { return cast<i8*>(v_u8); }
	constexpr operator u64() const noexcept { return v_u32; }
	constexpr operator i64() const noexcept { return v_u32; }
	constexpr operator f64() const noexcept { return v_f32; }
	constexpr operator u32() const noexcept { return v_u32; }
	constexpr operator i32() const noexcept { return v_u32; }
	constexpr operator f32() const noexcept { return v_f32; }
	constexpr operator u16() const noexcept { return v_r16[0]; }
	constexpr operator i16() const noexcept { return v_r16[0]; }
	constexpr operator u8() const noexcept { return v_u8[0]; }
	constexpr operator i8() const noexcept { return v_u8[0]; }
	constexpr operator bool() const noexcept { return static_cast<bool>(v_u32); }
	constexpr operator byte*() const noexcept { return cast<byte*>(v_u8); }
	constexpr r32 le() const noexcept { return moporgic::endian::to_le(v_u32); }
	constexpr r32 be() const noexcept { return moporgic::endian::to_be(v_u32); }
};

union r64 {
	u64 v_u64;
	f64 v_f64;
	r16 v_r16[4];
	r32 v_r32[2];
	u8c v_u8c[8];
	u8 v_u8[8];
	constexpr r64(const u64& v) noexcept : v_u64(v) {}
	constexpr r64(const i64& v) noexcept : v_u64(v) {}
	constexpr r64(const u32& v) noexcept : v_u64(v) {}
	constexpr r64(const i32& v) noexcept : v_u64(v) {}
	constexpr r64(const u16& v) noexcept : v_u64(v) {}
	constexpr r64(const i16& v) noexcept : v_u64(v) {}
	constexpr r64(const f32& v) noexcept : v_f64(v) {}
	constexpr r64(const f64& v) noexcept : v_f64(v) {}
	constexpr r64(const r64& v) noexcept : v_u64(v) {}
	constexpr r64(const u8& v) noexcept  : v_u64(v) {}
	constexpr r64(const i8& v) noexcept  : v_u64(v) {}
	constexpr r64(const u8* b) noexcept  : r64(const_cast<u8*>(b)) {}
	constexpr r64(const i8* b) noexcept  : r64(const_cast<i8*>(b)) {}
	constexpr r64(const byte* b) noexcept  : r64(cast<i8*>(b)) {};
	constexpr r64(u8* b) noexcept  : v_u64(*cast<u64*>(b)) {}
	constexpr r64(i8* b) noexcept  : v_u64(*cast<u64*>(b)) {}
	constexpr u8c& operator[](const int& i) noexcept { return v_u8c[i]; }
	constexpr operator u8*() const noexcept { return cast<u8*>(v_u8); }
	constexpr operator i8*() const noexcept { return cast<i8*>(v_u8); }
	constexpr operator u64() const noexcept { return v_u64; }
	constexpr operator i64() const noexcept { return v_u64; }
	constexpr operator f64() const noexcept { return v_f64; }
	constexpr operator u32() const noexcept { return v_r32[0]; }
	constexpr operator i32() const noexcept { return v_r32[0]; }
	constexpr operator f32() const noexcept { return v_r32[0]; }
	constexpr operator u16() const noexcept { return v_r32[0]; }
	constexpr operator i16() const noexcept { return v_r32[0]; }
	constexpr operator u8() const noexcept { return v_u8[0]; }
	constexpr operator i8() const noexcept { return v_u8[0]; }
	constexpr operator bool() const noexcept { return static_cast<bool>(v_u64); }
	constexpr operator byte*() const noexcept { return cast<byte*>(v_u8); }
	constexpr r64 le() const noexcept { return moporgic::endian::to_le(v_u64); }
	constexpr r64 be() const noexcept { return moporgic::endian::to_le(v_u64); }
};

namespace moporgic {
/**
 * half-precision half operation is defined in math.h
 */
class half {
public:
	constexpr half() noexcept : hf(0) {}
	constexpr half(const f32& num) noexcept; /* : hf(to_half(num)) {} */
	constexpr half(const f64& num) noexcept : half(f32(num)) {}
	constexpr half(const u64& num) noexcept : half(f32(num)) {}
	constexpr half(const i64& num) noexcept : half(f32(num)) {}
	constexpr half(const u32& num) noexcept : half(f32(num)) {}
	constexpr half(const i32& num) noexcept : half(f32(num)) {}
	constexpr half(const u16& num) noexcept : half(f32(num)) {}
	constexpr half(const i16& num) noexcept : half(f32(num)) {}
	constexpr half(const half& num) noexcept : hf(num.hf) {}
public:
	constexpr operator f32() const noexcept; /* { return to_float(hf); } */
	constexpr half operator +(const half& f) const noexcept; /* { return half::as(half_add(hf, f.hf)); } */
	constexpr half operator -(const half& f) const noexcept; /* { return half::as(half_sub(hf, f.hf)); } */
	constexpr half operator *(const half& f) const noexcept; /* { return half::as(half_mul(hf, f.hf)); } */
	constexpr half operator /(const half& f) const noexcept; /* { return half::as(half_div(hf, f.hf)); } */
	constexpr f32  operator +(const f32& f) const noexcept { return operator f32() + f; }
	constexpr f32  operator -(const f32& f) const noexcept { return operator f32() - f; }
	constexpr f32  operator *(const f32& f) const noexcept { return operator f32() * f; }
	constexpr f32  operator /(const f32& f) const noexcept { return operator f32() / f; }
public:
	constexpr half  operator ++(int) noexcept { half v(*this); operator +=(1); return v; }
	constexpr half  operator --(int) noexcept { half v(*this); operator -=(1); return v; }
	constexpr half& operator ++() noexcept { operator =(operator f32() + 1); return *this; }
	constexpr half& operator --() noexcept { operator =(operator f32() - 1); return *this; }
	constexpr half& operator  =(const half& f) noexcept { hf = f.hf; return *this; }
	constexpr half& operator +=(const half& f) noexcept { return operator =(operator +(f)); }
	constexpr half& operator -=(const half& f) noexcept { return operator =(operator -(f)); }
	constexpr half& operator *=(const half& f) noexcept { return operator =(operator *(f)); }
	constexpr half& operator /=(const half& f) noexcept { return operator =(operator /(f)); }
	constexpr half& operator  =(const f32& f) noexcept { return operator =(half(f)); }
	constexpr half& operator +=(const f32& f) noexcept { return operator =(operator +(f)); }
	constexpr half& operator -=(const f32& f) noexcept { return operator =(operator -(f)); }
	constexpr half& operator *=(const f32& f) noexcept { return operator =(operator *(f)); }
	constexpr half& operator /=(const f32& f) noexcept { return operator =(operator /(f)); }
public:
	constexpr bool operator ==(const half& f) const noexcept { return hf == f.hf; }
	constexpr bool operator !=(const half& f) const noexcept { return hf != f.hf; }
	constexpr bool operator ==(const f32& f) const noexcept { return operator f32() == f; }
	constexpr bool operator !=(const f32& f) const noexcept { return operator f32() != f; }
	constexpr bool operator <=(const f32& f) const noexcept { return operator f32() <= f; }
	constexpr bool operator >=(const f32& f) const noexcept { return operator f32() >= f; }
	constexpr bool operator < (const f32& f) const noexcept { return operator f32() <  f; }
	constexpr bool operator > (const f32& f) const noexcept { return operator f32() >  f; }
public:
	static constexpr half& as(const u16& raw) noexcept { return raw_cast<half>(raw); }
private:
	u16 hf;
};

} /* namespace moporgic */

typedef moporgic::half f16;

namespace moporgic {

template<typename type>
class clip {
public:
	constexpr clip(type *first, type *last) noexcept : first(first), last(last) {}
	constexpr clip(const clip& c) noexcept : first(c.first), last(c.last) {}
	constexpr clip() noexcept : first(nullptr), last(nullptr) {}
public:
	typedef type value_type;
	typedef type* iterator, pointer;
	typedef type& reference;
	typedef const type* const_iterator, const_pointer;
	typedef const type& const_reference;
	typedef size_t size_type, difference_type;
public:
	constexpr type* data() const noexcept { return first; }
	constexpr type* begin() const noexcept { return first; }
	constexpr type* end() const noexcept { return last; }
	constexpr const type* cbegin() const noexcept { return begin(); }
	constexpr const type* cend() const noexcept { return end(); }
	constexpr type& front() const noexcept { return *(begin()); }
	constexpr type& back()  const noexcept { return *(end() - 1); }
	constexpr type& operator[](size_t i) const noexcept { return *(begin() + i); }
	constexpr type& at(size_t i) const noexcept { return operator[](i); }
	constexpr size_t size() const noexcept { return end() - begin(); }
	constexpr size_t capacity() const noexcept { return size(); }
	constexpr size_t max_size() const noexcept { return -1ull; }
	constexpr bool empty() const noexcept { return size() == 0; }
public:
	constexpr bool operator==(const clip<type>& c) const noexcept { return begin() == c.begin() && end() == c.end(); }
	constexpr bool operator!=(const clip<type>& c) const noexcept { return begin() != c.begin() || end() != c.end(); }
	constexpr bool operator< (const clip<type>& c) const noexcept { return begin() <  c.begin(); }
	constexpr bool operator<=(const clip<type>& c) const noexcept { return begin() <= c.begin(); }
	constexpr bool operator> (const clip<type>& c) const noexcept { return begin() >  c.begin(); }
	constexpr bool operator>=(const clip<type>& c) const noexcept { return begin() >= c.begin(); }
public:
	constexpr void swap(clip<type>& c) noexcept { std::swap(first, c.first); std::swap(last, c.last); }
	constexpr clip<type>& operator=(const clip<type>& c) noexcept = default;
protected:
	type *first, *last;
};

template<typename type, typename alloc = std::allocator<type>>
class list : public clip<type> {
	template<typename iter>
	using enable_if_iterator_convertible = typename std::enable_if<std::is_convertible<
		typename std::iterator_traits<iter>::iterator_category, std::input_iterator_tag>::value>::type;
public:
	constexpr
	list() noexcept : clip<type>() {}
	list(const clip<type>& c) : list(c.begin(), c.end()) {}
	list(const list<type>& l) : list(l.begin(), l.end()) {}
	list(list<type>&& l) noexcept : list() { clip<type>::swap(l); }
	template<typename iter, typename = enable_if_iterator_convertible<iter>>
	list(iter first, iter last) : list() { insert(clip<type>::cend(), first, last); }
	list(size_t n, const type& v = {}) : list() { insert(clip<type>::cend(), n, v); }
	list(std::initializer_list<type> init) : list(init.begin(), init.end()) {}
	~list() { clear(); }
public:
	void clear() { set(nullptr, nullptr); }
	void shrink_to_fit() noexcept /* unnecessary */ {}
	void reserve(size_t n) noexcept /* unavailable */ {}
	void resize(size_t n, const type& v = {}) {
		if (n > clip<type>::size()) insert(clip<type>::cend(), n - clip<type>::size(), v);
		else if (n < clip<type>::size()) erase(clip<type>::cbegin() + n, clip<type>::cend());
	}
	type* insert(const type* p, size_t n, const type& v) {
		type* buf = alloc().allocate(clip<type>::size() + n);
		new (buf) type[clip<type>::size() + n]();
		type* pos = std::copy(clip<type>::cbegin(), p, buf);
		std::fill(pos, pos + n, v);
		std::copy(p, clip<type>::cend(), pos + n);
		set(buf, buf + clip<type>::size() + n);
		return pos;
	}
	template<typename iter, typename = enable_if_iterator_convertible<iter>>
	type* insert(const type* p, iter i, iter j) {
		type* pos = insert(p, std::distance(i, j), type());
		std::copy(i, j, pos);
		return pos;
	}
	type* insert(const type* p, const type& v) { return insert(p, &v, &v + 1); }
	type* erase(const type* i, const type* j) {
		list<type> tmp(clip<type>::size() - std::distance(i, j));
		std::copy(j, clip<type>::cend(), std::copy(clip<type>::cbegin(), i, tmp.begin()));
		clip<type>::swap(tmp);
		return clip<type>::begin() + (i - tmp.begin());
	}
	type* erase(const type* p) { return erase(p, p + 1); }
	void push_front(const type& v) { insert(clip<type>::cbegin(), v); }
	void push_back(const type& v) { insert(clip<type>::cend(), v); }
	void pop_front() { erase(clip<type>::cbegin()); }
	void pop_back() { erase(clip<type>::cend() - 1); }
	template<typename... args> /* fake emplace */
	type* emplace(const type* p, args&&... a) { return insert(p, type(std::forward<args>(a)...)); }
	template<typename... args> /* fake emplace */
	type& emplace_back(args&&... a) { return *emplace(clip<type>::cend(), std::forward<args>(a)...); }
	void assign(size_t n, const type& v) { operator=(list<type>(n, v)); }
	template<typename iter, typename = enable_if_iterator_convertible<iter>>
	void assign(iter i, iter j) { operator=(list<type>(i, j)); }
	list<type>& operator=(const clip<type>& c) { assign(c.cbegin(), c.cend()); return *this; }
	list<type>& operator=(const list<type>& l) { assign(l.cbegin(), l.cend()); return *this; }
	list<type>& operator=(list<type>&& l) { clip<type>::swap(l); return *this; }
public:
	static constexpr
	list<type>& as(clip<type>& c) noexcept { return raw_cast<list<type>>(c); }
	static constexpr
	const list<type>& as(const clip<type>& c) noexcept { return raw_cast<list<type>>(c); }
protected:
	void set(type* first, type* last) {
		alloc().deallocate(clip<type>::begin(), clip<type>::size());
		clip<type>::operator=(clip<type>(first, last));
	}
};

} /* namespace moporgic */

