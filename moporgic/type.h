#pragma once
#define MOPORGIC_TYPE_H_

/*
 *  mopotype.h type.h
 *  Created on: 2012/10/30
 *      Author: moporgic
 */

#include "unit.h"
#include <type_traits>
#include <iterator>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <utility>

namespace moporgic {
class byte;
class half;
class hexadeca;
class bihexadeca;
typedef hexadeca hex;
typedef bihexadeca hexa;
}
typedef moporgic::byte byte;
typedef moporgic::half f16;

namespace moporgic {

#define declare_enable_if_with(trait, name, detail...)\
template<typename test> using enable_if_##name = typename std::enable_if<std::trait<detail>::value>::type;\
template<typename test> using enable_if_not_##name = typename std::enable_if<not std::trait<detail>::value>::type;
#define declare_enable_if(trait)\
declare_enable_if_with(trait, trait, test);

declare_enable_if(is_arithmetic);
declare_enable_if(is_integral);
declare_enable_if(is_floating_point);
declare_enable_if(is_function);
declare_enable_if(is_fundamental);
declare_enable_if(is_reference);
declare_enable_if(is_object);
declare_enable_if(is_member_pointer);
declare_enable_if(is_pointer);
declare_enable_if(is_array);
declare_enable_if(is_scalar);
declare_enable_if(is_compound);
declare_enable_if(is_union);
declare_enable_if(is_class);
declare_enable_if(is_signed);
declare_enable_if(is_unsigned);

declare_enable_if_with(is_convertible, is_iterator_convertible, typename std::iterator_traits<test>::iterator_category, std::input_iterator_tag);

#define declare_trait(trait, detail...)\
template<> struct std::trait<detail> : public std::true_type {};

template<typename base, typename test> using enable_if_is_base_of = typename std::enable_if<std::is_base_of<base, test>::value>::type;
template<typename base, typename test> using enable_if_not_is_base_of = typename std::enable_if<not std::is_base_of<base, test>::value>::type;

template<typename from, typename to> using enable_if_is_convertible = typename std::enable_if<std::is_convertible<from, to>::value>::type;
template<typename from, typename to> using enable_if_not_is_convertible = typename std::enable_if<not std::is_convertible<from, to>::value>::type;

} /* namespace moporgic */

declare_trait(is_integral, moporgic::hexadeca);
declare_trait(is_unsigned, moporgic::hexadeca);
declare_trait(is_array, moporgic::hexadeca);
declare_trait(is_integral, moporgic::byte);
declare_trait(is_unsigned, moporgic::byte);
declare_trait(is_floating_point, moporgic::half);
declare_trait(is_signed, moporgic::half);

namespace moporgic {

class byte {
public:
	constexpr byte(u32 v = 0) noexcept : val(v) {}
	constexpr operator u8&() noexcept { return val; }
	constexpr operator u32() const noexcept { return val; }

	friend std::ostream& operator <<(std::ostream& os, const byte& b) {
		os << cast<char>("0123456789abcdef"[(b.val >> 4) & 0xf]);
		os << cast<char>("0123456789abcdef"[(b.val >> 0) & 0xf]);
		return os;
	}
	friend std::istream& operator >>(std::istream& is, const byte& b) {
		register char hi, lo;
		is >> hi >> lo;
		hi = (hi & 15) + (hi >> 6) * 9;
		lo = (lo & 15) + (lo >> 6) * 9;
		const_cast<byte&>(b) = is ? u32((hi << 4) | lo) : u32(b);
		return is;
	}
protected:
	u8 val;
};

/**
 * half-precision half operation is defined in math.h
 */
class half {
public:
	constexpr half() noexcept : hf(0) {}
	constexpr half(f32 num) noexcept; /* : hf(to_half(num)) {} */
	constexpr half(const half& num) noexcept : hf(num.hf) {}
public:
	constexpr operator f32() const noexcept; /* { return to_float(hf); } */
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr operator numeric() const noexcept { return operator f32(); }
	constexpr operator bool() const noexcept { return hf; }
	constexpr half operator +(half f) const noexcept; /* { return half::as(half_add(hf, f.hf)); } */
	constexpr half operator -(half f) const noexcept; /* { return half::as(half_sub(hf, f.hf)); } */
	constexpr half operator *(half f) const noexcept; /* { return half::as(half_mul(hf, f.hf)); } */
	constexpr half operator /(half f) const noexcept; /* { return half::as(half_div(hf, f.hf)); } */
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half operator +(numeric f) const noexcept { return operator f32() + f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half operator -(numeric f) const noexcept { return operator f32() - f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half operator *(numeric f) const noexcept { return operator f32() * f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half operator /(numeric f) const noexcept { return operator f32() / f; }
public:
	constexpr half  operator ++(int) noexcept { half v(*this); operator +=(1); return v; }
	constexpr half  operator --(int) noexcept { half v(*this); operator -=(1); return v; }
	constexpr half& operator ++() noexcept { operator =(operator f32() + 1); return *this; }
	constexpr half& operator --() noexcept { operator =(operator f32() - 1); return *this; }
public:
	constexpr half& operator  =(const half& f) noexcept { hf = f.hf; return *this; }
	constexpr half& operator +=(half f) noexcept { return operator =(operator +(f)); }
	constexpr half& operator -=(half f) noexcept { return operator =(operator -(f)); }
	constexpr half& operator *=(half f) noexcept { return operator =(operator *(f)); }
	constexpr half& operator /=(half f) noexcept { return operator =(operator /(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half& operator  =(numeric f) noexcept { return operator =(half(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half& operator +=(numeric f) noexcept { return operator =(operator +(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half& operator -=(numeric f) noexcept { return operator =(operator -(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half& operator *=(numeric f) noexcept { return operator =(operator *(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr half& operator /=(numeric f) noexcept { return operator =(operator /(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend numeric& operator +=(numeric& n, half f) { return n += numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend numeric& operator -=(numeric& n, half f) { return n -= numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend numeric& operator *=(numeric& n, half f) { return n *= numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend numeric& operator /=(numeric& n, half f) { return n /= numeric(f); }
public:
	constexpr bool operator ==(half f) const noexcept { return hf == f.hf; }
	constexpr bool operator !=(half f) const noexcept { return hf != f.hf; }
	constexpr bool operator <=(half f) const noexcept { return operator <=(f32(f)); }
	constexpr bool operator >=(half f) const noexcept { return operator >=(f32(f)); }
	constexpr bool operator < (half f) const noexcept { return operator < (f32(f)); }
	constexpr bool operator > (half f) const noexcept { return operator > (f32(f)); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator ==(numeric f) const noexcept { return operator numeric() == f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator !=(numeric f) const noexcept { return operator numeric() != f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator <=(numeric f) const noexcept { return operator numeric() <= f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator >=(numeric f) const noexcept { return operator numeric() >= f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator < (numeric f) const noexcept { return operator numeric() <  f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr bool operator > (numeric f) const noexcept { return operator numeric() >  f; }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator ==(numeric n, half f) { return n == numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator !=(numeric n, half f) { return n != numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator <=(numeric n, half f) { return n <= numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator >=(numeric n, half f) { return n >= numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator < (numeric n, half f) { return n <  numeric(f); }
	template<typename numeric, typename = enable_if_is_arithmetic<numeric>>
	constexpr friend bool operator > (numeric n, half f) { return n >  numeric(f); }

public:
	static constexpr half& as(const u16& raw) noexcept { return raw_cast<half>(raw); }
	friend std::ostream& operator <<(std::ostream& os, const half& hf) { return os << f32(hf); }
	friend std::istream& operator >>(std::istream& is, half& hf) { f32 k; if (is >> k) hf = k; return is; }
private:
	u16 hf;
};

} /* namespace moporgic */

namespace moporgic {
namespace endian {
static inline constexpr bool is_le() noexcept { const u32 v = 0x01; return raw_cast<u8, 0>(v); }
static inline constexpr bool is_be() noexcept { const u32 v = 0x01; return raw_cast<u8, 3>(v); }
static inline constexpr u16 to_le(u16 v) noexcept { return is_le() ? v : __builtin_bswap16(v); }
static inline constexpr u32 to_le(u32 v) noexcept { return is_le() ? v : __builtin_bswap32(v); }
static inline constexpr u64 to_le(u64 v) noexcept { return is_le() ? v : __builtin_bswap64(v); }
static inline constexpr u16 to_be(u16 v) noexcept { return is_be() ? v : __builtin_bswap16(v); }
static inline constexpr u32 to_be(u32 v) noexcept { return is_be() ? v : __builtin_bswap32(v); }
static inline constexpr u64 to_be(u64 v) noexcept { return is_be() ? v : __builtin_bswap64(v); }
} // namespace endian
} // namespace moporgic

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
	static constexpr size_t npos = size_t(-1ull);
public:
	constexpr type* data() const noexcept { return first; }
	constexpr type* begin() const noexcept { return first; }
	constexpr type* end() const noexcept { return last; }
	constexpr type* begin(type* it) noexcept { return std::exchange(first, it); }
	constexpr type* end(type* it) noexcept { return std::exchange(last, it); }
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
	constexpr clip<type> subc(size_t i, size_t n = npos) const noexcept { return {begin() + i, (n != npos) & (i + n < size()) ? begin() + i + n : end()}; }
public:
	constexpr bool operator==(const clip<type>& c) const noexcept { return begin() == c.begin() && end() == c.end(); }
	constexpr bool operator!=(const clip<type>& c) const noexcept { return begin() != c.begin() || end() != c.end(); }
	constexpr bool operator< (const clip<type>& c) const noexcept { return begin() <  c.begin(); }
	constexpr bool operator<=(const clip<type>& c) const noexcept { return begin() <= c.begin(); }
	constexpr bool operator> (const clip<type>& c) const noexcept { return begin() >  c.begin(); }
	constexpr bool operator>=(const clip<type>& c) const noexcept { return begin() >= c.begin(); }
public:
	constexpr operator type*() const noexcept { return data(); }
	constexpr void swap(clip<type>& c) noexcept { std::swap(first, c.first); std::swap(last, c.last); }
	constexpr clip<type>& operator=(const clip<type>& c) noexcept = default;
protected:
	type *first, *last;
};

template<typename type, typename alloc = std::allocator<type>>
class list : public clip<type> {
public:
	constexpr list() noexcept : clip<type>() {}
	list(const clip<type>& c) : list(c.begin(), c.end()) {}
	list(const list<type>& l) : list(l.begin(), l.end()) {}
	list(clip<type>&& c) noexcept : list() { clip<type>::swap(c); }
	list(list<type>&& l) noexcept : list() { clip<type>::swap(l); }
	template<typename iter, typename = enable_if_is_iterator_convertible<iter>>
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
	template<typename iter, typename = enable_if_is_iterator_convertible<iter>>
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
	template<typename iter, typename = enable_if_is_iterator_convertible<iter>>
	void assign(iter i, iter j) { operator=(list<type>(i, j)); }
	list<type>& operator=(const clip<type>& c) { assign(c.cbegin(), c.cend()); return *this; }
	list<type>& operator=(const list<type>& l) { assign(l.cbegin(), l.cend()); return *this; }
	list<type>& operator=(list<type>&& l) { clip<type>::swap(l); return *this; }
	list<type> subl(size_t i, size_t n = clip<type>::npos) const noexcept { return list<type>(clip<type>::subc(i, n)); }
public:
	static constexpr list<type>& as(clip<type>& c) noexcept { return raw_cast<list<type>>(c); }
	static constexpr const list<type>& as(const clip<type>& c) noexcept { return raw_cast<list<type>>(c); }
protected:
	void set(type* first, type* last) {
		for (type* it = clip<type>::begin(); it != clip<type>::end(); it++) it->~type();
		alloc().deallocate(clip<type>::begin(), clip<type>::size());
		clip<type>::operator=(clip<type>(first, last));
	}
};

} /* namespace moporgic */

namespace moporgic {

class hexadeca {
public:
	constexpr hexadeca(u64 hex = 0) noexcept : hex(hex) {}
	constexpr operator u64&() noexcept { return hex; }
	constexpr operator u64() const noexcept { return hex; }
	constexpr static hexadeca& as(u64& hex) noexcept { return raw_cast<hexadeca>(hex); }
public:
	class cell {
		friend class hexadeca;
	public:
		constexpr cell() noexcept = delete;
		constexpr cell(const cell& c) noexcept = default;
		constexpr cell(u64& ref, u32 idx) noexcept : ref(ref), idx(idx) {}
		constexpr operator u32() const noexcept { return (ref >> (idx << 2)) & 0x0f; }
		constexpr cell& operator =(u32 val) noexcept { ref = (ref & ~(0x0full << (idx << 2))) | ((val & 0x0full) << (idx << 2)); return *this; }
		constexpr cell& operator =(const cell& c) noexcept { return operator =(u32(c)); }
		constexpr cell& operator +=(u32 val) noexcept { return operator =(operator u32() + val); }
		constexpr cell& operator -=(u32 val) noexcept { return operator =(operator u32() - val); }
		constexpr cell& operator *=(u32 val) noexcept { return operator =(operator u32() * val); }
		constexpr cell& operator /=(u32 val) noexcept { return operator =(operator u32() / val); }
		constexpr cell& operator &=(u32 val) noexcept { return operator =(operator u32() & val); }
		constexpr cell& operator |=(u32 val) noexcept { return operator =(operator u32() | val); }
		constexpr cell& operator ^=(u32 val) noexcept { return operator =(operator u32() ^ val); }
		constexpr cell& operator ++() noexcept { return operator +=(1); }
		constexpr cell& operator --() noexcept { return operator -=(1); }
		constexpr u32 operator ++(int) noexcept { u32 v = operator u32(); operator ++(); return v; }
		constexpr u32 operator --(int) noexcept { u32 v = operator u32(); operator --(); return v; }
	protected:
		u64& ref;
		u32 idx;
	};
	class iter : cell {
		friend class hexadeca;
	public:
		typedef u32 value_type;
		typedef i32 difference_type;
		typedef cell& reference;
		typedef iter pointer;
		typedef std::forward_iterator_tag iterator_category;
		constexpr iter() noexcept = delete;
		constexpr iter(const cell& c) noexcept : cell(c) {};
		constexpr iter(u64& ref, u32 idx) noexcept : cell(ref, idx) {};
		constexpr cell& operator *() noexcept { return *this; }
		constexpr const cell& operator *() const noexcept { return *this; }
		constexpr cell  operator ->() const noexcept { return *this; }
		constexpr bool  operator ==(const iter& t) const noexcept { return ((ref == t.ref) & (idx == t.idx)); }
		constexpr bool  operator !=(const iter& t) const noexcept { return ((ref != t.ref) | (idx != t.idx)); }
		constexpr bool  operator < (const iter& t) const noexcept { return ((ref == t.ref) & (idx < t.idx)) | (ref < t.ref); }
		constexpr i32   operator - (const iter& t) const noexcept { return i32(idx) - i32(t.idx); }
		constexpr iter  operator + (u32 n) const noexcept { return iter(ref, idx + n); }
		constexpr iter  operator - (u32 n) const noexcept { return iter(ref, idx - n); }
		constexpr iter& operator +=(u32 n) noexcept { idx += n; return *this; }
		constexpr iter& operator -=(u32 n) noexcept { idx -= n; return *this; }
		constexpr iter& operator ++() noexcept { idx += 1; return *this; }
		constexpr iter& operator --() noexcept { idx -= 1; return *this; }
		constexpr iter  operator ++(int) noexcept { return iter(ref, ++idx - 1); }
		constexpr iter  operator --(int) noexcept { return iter(ref, --idx + 1); }
	};
public:
	constexpr cell operator [](u32 idx) const noexcept { return cell(const_cast<u64&>(hex), idx); }
	constexpr cell at(u32 idx) const noexcept { return operator [](idx); }
	constexpr size_t size() const noexcept { return at(15); }
	constexpr size_t size(u32 len) noexcept { u32 n = size(); at(15) = len; return n; }
	constexpr void resize(u32 len) noexcept { hex &= (len ? -1ull >> ((16 - len) << 2) : 0); size(len); }
	constexpr size_t capacity() const noexcept { return 15; }
	constexpr size_t max_size() const noexcept { return 16; }
	constexpr bool empty() const noexcept { return size() == 0; }
	constexpr void clear() { hex = 0; }
	constexpr iter begin() const noexcept { return iter(operator [](0)); }
	constexpr iter end() const noexcept { return iter(operator [](size())); }
	constexpr cell front() const noexcept { return at(0); }
	constexpr cell back() const noexcept { return at(size() - 1); }
	constexpr void push_front(u32 v) noexcept { u32 n = size(); hex <<= 4; at(0) = v; size(n + 1); }
	constexpr void push_back(u32 v) noexcept { at(size()) = v; size(size() + 1); }
	constexpr void pop_front() noexcept { u32 n = size(); hex >>= 4; resize(n - 1); }
	constexpr void pop_back() noexcept { resize(size() - 1); }
protected:
	u64 hex;
};

class bihexadeca : public hexadeca {
public:
	constexpr bihexadeca(u64 hex = 0, u64 ext = 0) noexcept : hexadeca(hex), ext(ext) {}
	constexpr hexadeca& exten() noexcept { return raw_cast<hexadeca>(ext); }
	constexpr hexadeca exten() const noexcept { return raw_cast<hexadeca>(ext); }
public:
	class iter : cell {
		friend class hexadeca;
	public:
		typedef u32 value_type;
		typedef i32 difference_type;
		typedef iter pointer;
		typedef std::forward_iterator_tag iterator_category;
		constexpr iter() noexcept = delete;
		constexpr iter(const cell& c) noexcept : cell(c) {};
		constexpr iter(u64& ref, u32 idx) noexcept : cell(ref, idx) {};
		constexpr cell  operator *() noexcept { return cell(cast<u64*>(&ref)[idx >> 4], idx & 0x0f); }
		constexpr cell  operator ->() const noexcept { return cell(cast<u64*>(&ref)[idx >> 4], idx & 0x0f); }
		constexpr bool  operator ==(const iter& t) const noexcept { return ((ref == t.ref) & (idx == t.idx)); }
		constexpr bool  operator !=(const iter& t) const noexcept { return ((ref != t.ref) | (idx != t.idx)); }
		constexpr bool  operator < (const iter& t) const noexcept { return ((ref == t.ref) & (idx < t.idx)) | (ref < t.ref); }
		constexpr i32   operator - (const iter& t) const noexcept { return i32(idx) - i32(t.idx); }
		constexpr iter  operator + (u32 n) const noexcept { return iter(ref, idx + n); }
		constexpr iter  operator - (u32 n) const noexcept { return iter(ref, idx - n); }
		constexpr iter& operator +=(u32 n) noexcept { idx += n; return *this; }
		constexpr iter& operator -=(u32 n) noexcept { idx -= n; return *this; }
		constexpr iter& operator ++() noexcept { idx += 1; return *this; }
		constexpr iter& operator --() noexcept { idx -= 1; return *this; }
		constexpr iter  operator ++(int) noexcept { return iter(ref, ++idx - 1); }
		constexpr iter  operator --(int) noexcept { return iter(ref, --idx + 1); }
	};
public:
	constexpr cell operator [](u32 idx) const noexcept { return cell(cast<u64*>(this)[idx >> 4], idx & 0x0f); }
	constexpr cell at(u32 idx) const noexcept { return operator [](idx); }
	constexpr size_t size() const noexcept { return raw_cast<u8, 7>(ext); }
	constexpr size_t size(u32 len) noexcept { u32 n = size(); raw_cast<u8, 7>(ext) = len; return n; }
	constexpr void resize(u32 len) noexcept {
		if (len > 16) ext &= (-1ull >> ((16 - (len - 16)) << 2));
		else ext = 0, hex &= (len ? -1ull >> ((16 - len) << 2) : 0);
		size(len);
	}
	constexpr size_t capacity() const noexcept { return 30; }
	constexpr size_t max_size() const noexcept { return 32; }
	constexpr bool empty() const noexcept { return size() == 0; }
	constexpr void clear() { hex = ext = 0; }
	constexpr iter begin() const noexcept { return iter(const_cast<u64&>(hex), 0); }
	constexpr iter end() const noexcept { return iter(const_cast<u64&>(hex), size()); }
	constexpr cell front() const noexcept { return at(0); }
	constexpr cell back() const noexcept { return at(size() - 1); }
	constexpr void push_front(u32 v) noexcept { u32 n = size(); ext <<= 4; at(16) = at(15); hex <<= 4; at(0) = v; size(n + 1); }
	constexpr void push_back(u32 v) noexcept { at(size()) = v; size(size() + 1); }
	constexpr void pop_front() noexcept { u32 n = size(); hex >>= 4; at(15) = at(16); ext >>= 4; resize(n - 1); }
	constexpr void pop_back() noexcept { resize(size() - 1); }
protected:
	u64 ext;
};

} /* namespace moporgic */

namespace std {
constexpr static inline void swap(moporgic::hexadeca::cell lc, moporgic::hexadeca::cell rc) noexcept { u32 t = lc; lc = rc; rc = t; }
constexpr static inline u32 exchange(moporgic::hexadeca::cell c, u32 val) noexcept { u32 o = c; c = val; return o; }
} /* namespace std */

namespace moporgic {

template<typename type,
	template<typename...> class list = moporgic::list,
	template<typename...> class alloc = std::allocator>
class segment {
public:
	constexpr segment() noexcept {}
	segment(const segment& seg) = delete;
	segment(segment&& seg) noexcept : space(std::move(seg.space)) {}
	segment(type* buf, size_t num) noexcept { deallocate(buf, num); }
	segment& operator =(const segment& seg) = delete;
	segment& operator =(segment& seg) { space = std::move(seg.space); return *this; }

	type* allocate(size_t num) {
		for (auto it = space.begin(); it != space.end(); it++) {
			if (it->size() >= num) {
				type* buf = it->begin(it->begin() + num);
				if (it->empty()) space.erase(it);
				return buf;
			}
		}
		return nullptr;
	}

	void deallocate(type* buf, size_t num) {
		clip<type> tok(buf, buf + num);
		auto it = std::lower_bound(space.begin(), space.end(), tok);
		auto pt = std::prev(it);

		if (it != space.begin()) {
			tok.begin(std::max(tok.begin(), pt->end()));
			tok.end(std::max(tok.end(), pt->end()));
		}
		if (it != space.end()) {
			while (it != space.end() && tok.end() > it->end())
				it = space.erase(it), pt = std::prev(it);
			if (it != space.end())
				tok.end(std::min(tok.end(), it->begin()));
		}

		if (it != space.begin() && pt->end() == tok.begin()) {
			if (it != space.end() && tok.end() == it->begin()) {
				it->begin(pt->begin());
				it = space.erase(pt);
			} else {
				pt->end(tok.end());
			}
		} else {
			if (it != space.end() && tok.end() == it->begin()) {
				it->begin(tok.begin());
			} else {
				it = space.insert(it, tok);
			}
		}
	}

protected:
	list<clip<type>, alloc<clip<type>>> space;
};

} /* namespace moporgic */


namespace moporgic {

template<typename type, typename = void>
class once { // for fundamental types
public:
	constexpr inline once() : data{}, pass(0) {}
	constexpr inline once(const type& v, u32 p = 0) : data{v}, pass(p) { reset(v); }
	constexpr inline once(const once& o) = default;
	constexpr inline operator type&() { return data; }
	constexpr inline operator const type&() const { return data; }
	constexpr inline type& operator =(const type& v) { return (data = (pass++ ? data : v)); }
	constexpr inline u32 reset(const type& v = {}) { data = v; return std::exchange(pass, 0); }
	constexpr inline u32 count() const { return pass; }
private:
	type data;
	u32 pass;
};

template<typename type>
class once<type, enable_if_is_class<type>> : public type { // for class types
public:
	constexpr inline once() : type(), pass(0) {}
	constexpr inline once(const type& v, u32 p = 0) : type(v), pass(p) { reset(v); }
	constexpr inline once(const once& o) = default;
	template<typename save>
	constexpr inline type& operator =(const save& v) { return pass++ ? *this : type::operator =(v); }
	constexpr inline type& operator =(const type& v) { return type::operator =(pass++ ? *this : v); }
	constexpr inline u32 reset(const type& v = {}) { type::operator =(v); return std::exchange(pass, 0); }
	constexpr inline u32 count() const { return pass; }
private:
	u32 pass;
};

template<typename type, typename scope = void>
class static_store {
public:
	constexpr inline static_store() {}
	constexpr inline static_store(const type& v) { instance() = v; }
	constexpr inline static_store(const static_store& s) = default;
	constexpr inline operator type&() { return instance(); }
	constexpr inline operator const type&() const { return instance(); }
	constexpr inline type& operator =(const type& v) { return instance() = v; }
	static inline type& instance() { static type v; return v;}
};

} /* namespace moporgic */

