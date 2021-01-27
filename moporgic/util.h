#pragma once
#define MOPORGIC_UTIL_H_
/*
 * util.h
 *
 *  Created on: 2015/10/24
 *      Author: moporgic
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <random>
#include <utility>

#if defined(__GNUC__)
#define inline_always inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define inline_always __forceinline
#else
#define inline_always inline
#endif

#define VA_ARG_0(V, ...) V
#define VA_ARG_1(V, ...) VA_ARG_0(__VA_ARGS__)
#define VA_ARG_2(V, ...) VA_ARG_1(__VA_ARGS__)
#define VA_ARG_3(V, ...) VA_ARG_2(__VA_ARGS__)
#define VA_ARG_4(V, ...) VA_ARG_3(__VA_ARGS__)
#define VA_ARG_5(V, ...) VA_ARG_4(__VA_ARGS__)
#define VA_ARG_6(V, ...) VA_ARG_5(__VA_ARGS__)
#define VA_ARG_7(V, ...) VA_ARG_6(__VA_ARGS__)
#define VA_ARG_8(V, ...) VA_ARG_7(__VA_ARGS__)
#define VA_ARG_9(V, ...) VA_ARG_8(__VA_ARGS__)
#define VA_PASS(...) __VA_ARGS__

#define PASTE_PASS(a, b) a##b
#define PASTE(a, b) PASTE_PASS(a, b)

#define declare_alias_spec(alias, name, head, tail, ...)\
template <typename... types> VA_PASS(head inline) \
auto alias(types&&... args) VA_PASS(tail -> decltype(name(std::forward<types>(args)...)))\
{ return name(std::forward<types>(args)...); }
#define declare_alias(alias, name, ...)\
declare_alias_spec(alias, name, __VA_ARGS__,)


#define declare_comparators_rel(type, ...)\
VA_PASS(__VA_ARGS__ bool operator) !=(type v) const { return !(*this == v); }\
VA_PASS(__VA_ARGS__ bool operator) > (type v) const { return  (v < *this); }\
VA_PASS(__VA_ARGS__ bool operator) <=(type v) const { return !(v < *this); }\
VA_PASS(__VA_ARGS__ bool operator) >=(type v) const { return !(*this < v); }

#define declare_comparators_with(type, lhs, rhs, ...)\
VA_PASS(__VA_ARGS__ bool operator) ==(type v) const { return lhs == rhs; }\
VA_PASS(__VA_ARGS__ bool operator) < (type v) const { return lhs <  rhs; }\
declare_comparators_rel(type, __VA_ARGS__)

#define declare_comparators(type, cmp, ...)\
declare_comparators_with(type, cmp, v.cmp, __VA_ARGS__)


#define declare_extern_comparators_rel(ltype, rtype, ...)\
VA_PASS(__VA_ARGS__ inline bool operator) !=(ltype lv, rtype rv) { return !(lv == rv); }\
VA_PASS(__VA_ARGS__ inline bool operator) > (ltype lv, rtype rv) { return  (rv < lv); }\
VA_PASS(__VA_ARGS__ inline bool operator) <=(ltype lv, rtype rv) { return !(rv < lv); }\
VA_PASS(__VA_ARGS__ inline bool operator) >=(ltype lv, rtype rv) { return !(lv < rv); }

#define declare_extern_comparators_with(ltype, rtype, lhs, rhs, ...)\
VA_PASS(__VA_ARGS__ inline bool operator) ==(ltype lv, rtype rv) { return lhs == rhs; }\
VA_PASS(__VA_ARGS__ inline bool operator) < (ltype lv, rtype rv) { return lhs <  rhs; }\
declare_extern_comparators_rel(ltype, rtype, __VA_ARGS__)

#define declare_extern_comparators(ltype, rtype, cmp, ...)\
declare_extern_comparators_with(ltype, rtype, lv.cmp, rv.cmp, __VA_ARGS__)

namespace moporgic {
template<typename run>
struct invoke_on_destruct_t {
	invoke_on_destruct_t(run fx = run()) : fx(fx) {}
	invoke_on_destruct_t(const invoke_on_destruct_t&) = delete;
	invoke_on_destruct_t(invoke_on_destruct_t&& move) { fx = std::exchange(move.fx, run()); }
	~invoke_on_destruct_t() { if (fx) fx(); }
	invoke_on_destruct_t& operator =(run x) { fx = x; return *this; }
	invoke_on_destruct_t& operator =(const invoke_on_destruct_t&) = delete;
	invoke_on_destruct_t& operator =(invoke_on_destruct_t&& move) { fx = std::exchange(move.fx, run()); return *this; }
	run fx;
};

template<typename run> static inline
invoke_on_destruct_t<run> invoke_on_destruct(run fx) { return invoke_on_destruct_t<run>(fx); }
} /* moporgic */

namespace moporgic {

template<typename... types>
static inline std::string format(const std::string& spec, types&&... args) {
	size_t n = std::snprintf(nullptr, 0, spec.c_str(), std::forward<types>(args)...);
	std::string buf(n + 1, '\0');
	buf.resize(std::snprintf(&buf[0], buf.capacity(), spec.c_str(), std::forward<types>(args)...));
	return buf;
}

static inline uint64_t millisec() {
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	return ms.count();
}
static inline uint64_t microsec() {
	auto now = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	return us.count();
}
static __attribute__((unused)) std::string put_time(std::time_t t) {
	std::stringstream buf;
#if SIZE_MAX == UINT64_MAX
	if (t >= std::time_t(253402300799000ull)) { // is microseconds
		buf << "YYYY-MM-DD HH:MM:SS." << std::setfill('0') << std::setw(6) << (std::exchange(t, t / 1000000) % 1000000);
	} else if (t >= std::time_t(253402300799ull)) { // is milliseconds
		buf << "YYYY-MM-DD HH:MM:SS." << std::setfill('0') << std::setw(3) << (std::exchange(t, t / 1000) % 1000);
	} // else is seconds, do nothing
#endif
	buf.seekp(0) << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
	return buf.str();
}
#if SIZE_MAX == UINT32_MAX // 32-bit platform whose std::time_t only support seconds
static __attribute__((unused)) std::string put_time(uint64_t t) {
	if (t >= 253402300799000ull)   t /= 1000000; // is microseconds
	else if (t >= 253402300799ull) t /= 1000;    // is milliseconds
	return put_time(std::time_t(t));
}
#endif

#define __DATE_ISO__ ({ std::tm t = {}; std::string DATE(__DATE__); if (DATE[4] == ' ') DATE[4] = '0'; \
std::istringstream(DATE) >> std::get_time(&t, "%b %d %Y"); std::put_time(&t, "%Y-%m-%d");})

static inline constexpr uint32_t to_hash(const char* str) noexcept {
	uint32_t hash = 0;
	for (const char* c = str; (*c) != 0; c++)
		hash = (hash << 5) - hash + (*c); // i' = 31 * i + c
	return hash;
}
static inline uint32_t to_hash(const std::string& str) noexcept {
	return to_hash(str.c_str());
}

class random {
public:
	template<typename engine_t = std::mt19937>
	inline operator uint16_t() const { return next<engine_t>(); }
	template<typename engine_t = std::mt19937>
	inline operator uint32_t() const { return next<engine_t>(); }
	template<typename engine_t = std::mt19937_64>
	inline operator uint64_t() const { return next<engine_t>(); }
	template<typename engine_t = std::mt19937>
	inline operator float() const { return std::uniform_real_distribution<float>(0.0f, 1.0f)(engine_ref<engine_t>()); }
	template<typename engine_t = std::mt19937_64>
	inline operator double() const { return std::uniform_real_distribution<double>(0.0, 1.0)(engine_ref<engine_t>()); }

	template<typename engine_t = std::mt19937>
	static inline auto& engine_ref() { return *(engine_ptr<engine_t>()); }
	template<typename engine_t = std::mt19937>
	static inline auto& engine_ptr() { static engine_t* engine = nullptr; return engine; }
	template<typename engine_t = std::mt19937>
	static inline auto next() { return (engine_ref<engine_t>())(); }

	template<typename engine_t = std::mt19937, typename seed_t = decltype(engine_t::default_seed)>
	static inline void init(seed_t seed = engine_t::default_seed) {
		delete std::exchange(engine_ptr<engine_t>(), new engine_t(seed));
	}
	template<typename engine_t = std::mt19937, typename seed_t = decltype(engine_t::default_seed)>
	static inline void seed(seed_t seed = engine_t::default_seed) {
		if (engine_ptr<engine_t>()) engine_ref<engine_t>().seed(seed);
		else                        init<engine_t>(seed);
	}
protected:
	static __attribute__((constructor)) void __init__(void) {
		init<std::mt19937>(std::mt19937::default_seed);
		init<std::mt19937_64>(std::mt19937_64::default_seed);
	}
	static __attribute__((destructor))  void __exit__(void) {}
};

static inline void srand(uint32_t seed = to_hash("moporgic")) {
	random::seed<std::mt19937>(seed);
	random::seed<std::mt19937_64>(seed);
}
static inline uint32_t rand()   { return uint32_t(random()); }
static inline uint32_t rand16() { return uint32_t(random()) & 0xffffu; }
static inline uint32_t rand32() { return uint32_t(random()); }
static inline uint32_t rand31() { return uint32_t(random()) & 0x7fffffffu; }
static inline uint64_t rand64() { return uint64_t(random()); }
static inline uint64_t rand63() { return uint64_t(random()) & 0x7fffffffffffffffull; }
static inline uint32_t rand24() { return uint32_t(random()) & 0x00ffffffu; }
static inline uint32_t randx()  { return rand32(); }

static inline auto rdtsc() {
#if defined __GNUC__ && defined __x86_64__
	register uint32_t lo, hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t) hi << 32) | lo;
#elif defined __GNUC__ && defined __i386__
	register uint32_t lo;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo));
    return lo;
#elif defined _MSC_VER
    // #include <intrin.h>
    return __rdtsc();
#else
    return -1;
#endif
}

} /* moporgic */

namespace moporgic {

template<typename type> static inline
std::ostream& write(std::ostream& out, const type& v, const size_t& len = sizeof(type)) {
	return out.write(reinterpret_cast<char*>(&const_cast<type&>(v)), len);
}

template<typename type> static inline
std::istream& read(std::istream& in, type& v, const size_t& len = sizeof(type)) {
	return in.read(reinterpret_cast<char*>(&v), len);
}

template<typename type> static inline
type read(std::istream& in) {
	type buf; read(in, buf); return buf;
}

template<typename type> static inline
std::ostream& write(std::ostream& out, const type* begin, const type* end, const size_t& len = sizeof(type)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		write(out, *value, len);
	return out;
}

template<typename type> static inline
std::istream& read(std::istream& in, const type* begin, const type* end, const size_t& len = sizeof(type)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		read(in, *value, len);
	return in;
}

template<typename cast, typename type> static inline
std::ostream& write_cast(std::ostream& out, const type& v, const size_t& len = sizeof(cast)) {
	return write(out, cast(v), len);
}

template<typename cast, typename type> static inline
std::istream& read_cast(std::istream& in, type& v, const size_t& len = sizeof(cast)) {
	cast buf; read(in, buf, len); v = buf;
	return in;
}

template<typename cast, typename type> static inline
type read_cast(std::istream& in) {
	type buf; read_cast<cast>(in, buf); return buf;
}

template<typename cast, typename type> static inline
std::ostream& write_cast(std::ostream& out, const type* begin, const type* end, const size_t& len = sizeof(cast)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		write(out, cast(*value), len);
	return out;
}

template<typename cast, typename type> static inline
std::istream& read_cast(std::istream& in, const type* begin, const type* end, const size_t& len = sizeof(cast)) {
	cast buf;
	for (type* value = const_cast<type*>(begin); value != end; *value = buf, value++)
		read(in, buf, len);
	return in;
}

} /* moporgic */

namespace moporgic { // reference://wordaligned.org/articles/cpp-streambufs

class redirector {
public:
	redirector(std::ostream& dst, std::ostream& src) : src(src), sbuf(src.rdbuf(dst.rdbuf())) {}
	~redirector() { src.rdbuf(sbuf); }
private:
	std::ostream& src;
	std::streambuf* const sbuf;
};

class teestreambuf : public std::streambuf {
public:
	teestreambuf(std::streambuf* sb1, std::streambuf* sb2) : sb1(sb1), sb2(sb2) {}
	virtual int sync() {
		const auto r1 = sb1->pubsync();
		const auto r2 = sb2->pubsync();
		return ((r1 == 0) & (r2 == 0)) ? 0 : -1;
	}
	virtual int overflow(int c) {
		constexpr auto eof = std::char_traits<char>::eof();
		if (c == eof) return !eof;
		const auto r1 = sb1->sputc(c);
		const auto r2 = sb2->sputc(c);
		return ((r1 == eof) | (r2 == eof)) ? eof : c;
	}
protected:
	virtual std::streamsize xsputn(const char_type* s, std::streamsize n) {
		const auto z1 = sb1->sputn(s, n);
		const auto z2 = sb2->sputn(s, n);
		return std::min(z1, z2);
	}
private:
	std::streambuf* sb1;
	std::streambuf* sb2;
};

class teestream : public std::ostream {
public:
	teestream(std::ostream& o1, std::ostream& o2 = std::cout) : std::ostream(&tbuf) , tbuf(o1.rdbuf(), o2.rdbuf()) {}
private:
	teestreambuf tbuf;
};
typedef teestream oostream;

} /* moporgic */

namespace std {
static inline std::ostream& lf(std::ostream& os) { return os.put(os.widen('\n')); }
}

namespace moporgic {

__attribute__((constructor))
static void __util_init__(void) {}

__attribute__((destructor))
static void __util_exit__(void) {}

} /* moporgic */
