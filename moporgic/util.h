#pragma once
#define MOPORGIC_UTIL_H_
/*
 * util.h
 *
 *  Created on: 2015/10/24
 *      Author: moporgic
 */

#include <cstdint>
#include <chrono>
#include <string>
#include <cstdio>
#include <iostream>
#include <utility>

#ifndef DEBUG
#define DLOG(msg,...)
#else /* DEBUG */
#define DLOG(msg,...) printf(msg, ##__VA_ARGS__)
#endif /* DEBUG */

#if !defined(__cplusplus) || __cplusplus < 201103L
#define constexpr
#define noexcept
#endif

#define VA_ARG_1(V, ...) V
#define VA_ARG_2(V, ...) VA_ARG_1(__VA_ARGS__)
#define VA_ARG_3(V, ...) VA_ARG_2(__VA_ARGS__)
#define VA_ARG_4(V, ...) VA_ARG_3(__VA_ARGS__)
#define VA_ARG_5(V, ...) VA_ARG_4(__VA_ARGS__)
#define VA_ARG_6(V, ...) VA_ARG_5(__VA_ARGS__)
#define VA_ARG_7(V, ...) VA_ARG_6(__VA_ARGS__)
#define VA_ARG_8(V, ...) VA_ARG_7(__VA_ARGS__)
#define VA_ARG_9(V, ...) VA_ARG_8(__VA_ARGS__)
#define VA_PASS(...) __VA_ARGS__


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

static inline constexpr
uint32_t to_hash_tail(const char* str, const uint32_t& hash) noexcept {
	if (*str) return to_hash_tail(str + 1, (hash << 5) - hash + (*str));
	return hash;
}
static inline constexpr
uint32_t to_hash(const char* str) noexcept {
	return to_hash_tail(str, 0); // i' = 31 * i + c
}

static inline
uint32_t to_hash(const std::string& str) noexcept {
	return to_hash(str.c_str());
}

static inline
uint32_t rand16() {
#if RAND_MAX == 0x7fff
    return (rand() << 1) | (rand() & 1);
#else
    return rand() & 0xffff;
#endif
}

static inline
uint32_t rand32() {
#if   RAND_MAX == 0x7fffffff
	return (rand() << 1) | (rand() & 1);
#elif RAND_MAX == 0x7fff
    return (rand() << 17) | (rand() << 2) | (rand() & 3);
#else
    return (rand16() << 16) | rand16();
#endif
}

static inline
uint32_t rand31() {
#if   RAND_MAX == 0x7fffffff
	return rand();
#elif RAND_MAX == 0x7fff
	return (rand() << 16) | (rand() << 1) | (rand() & 1);
#else
	return rand32() & 0x7fffffff;
#endif
}

static inline
uint64_t rand64() {
#if   RAND_MAX == 0x7fffffff
	return (static_cast<uint64_t>(rand()) << 33) | (static_cast<uint64_t>(rand()) << 2) | (static_cast<uint64_t>(rand()) & 3);
#elif RAND_MAX == 0x7fff
    return (static_cast<uint64_t>(rand()) << 49) | (static_cast<uint64_t>(rand()) << 34)
    	 | (static_cast<uint64_t>(rand()) << 19) | (static_cast<uint64_t>(rand()) << 4) | (static_cast<uint64_t>(rand()) & 15);
#else
    return (rand32() << 32) | rand32();
#endif
}

static inline
uint64_t rand63() {
	return rand64() & 0x7fffffffffffffffull;
}

static inline
uint32_t rand24() {
#if   RAND_MAX == 0x7fffffff
	return rand() & 0x00ffffff;
#elif RAND_MAX == 0x7fff
    return (rand() << 9) | (rand() >> 6);
#else
    return (rand32()) & 0x00ffffff;
#endif
}

#if RAND_MAX == 0x7fff
#define RANDX_MAX 0x3fffffff
#else
#define RANDX_MAX RAND_MAX
#endif

static inline
uint32_t randx() {
#if RAND_MAX == 0x7fff
    return (rand() << 15) | (rand());
#else
	return rand();
#endif
}

static inline
float random() {
	return static_cast<float>(randx()) / static_cast<float>(RANDX_MAX);
}

static inline
unsigned long long rdtsc() {
#if defined __GNUC__ && defined __x86_64__
	register unsigned int lo, hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((unsigned long long) hi << 32) | lo;
#elif defined __GNUC__ && defined __i386__
	register unsigned int lo;
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

#ifdef DIRECTIO

//#define putln()			putchar('\n')
//#define putsp()			putchar(' ')
//#define puttab()		putchar('\t')
//#define print(...)		printf(__VA_ARGS__)
//#define println(...)	printf(__VA_ARGS__); putchar('\n')
#define removebuf()		setvbuf(stdout, NULL, _IONBF, 0)
//#define randinit()		srand(time(nullptr))
//#define mallof(T, size) ((T*) malloc(sizeof(T) * size))
//#define callof(T, size) ((T*) calloc(size, sizeof(T))

#define _getch(ch)				(ch = getchar())
#define _appendn(val, ch)		val = (val * 10) + (ch - '0')
#define _appendf(val, ch, f)	val += (ch - '0') / f
#define _appif_avail(v, ch)		while (_getch(ch) >= '0') {_appendn(v, ch);}
#define _appif_avifp(v, ch, f)	while (_getch(ch) >= '0') {_appendf(v, ch, f); f = f * 10;}
#define _flushbuf(ipt, fpt)		while (ipt < fpt) putchar((*ipt++));
#define _flushbufn(ipt, fpt)	while (ipt < fpt) putchar((*ipt++) + '0');
#define _post_lt0(v) 			if (v < 0) { v = -v; putchar('-'); }
#define _post_procn(u, b, i, p, f)  for (i = p = f - 1; i >= b; u /= 10, i--) if ( (*i = u % 10) != 0) p = i;

#define _postu_fptoff	32
#define _postlu_fptoff	64
#define _postf_fptoff	64
#define _postf_vtype	unsigned long long
#define _postf_defprec	6
#define _moporgic_buf_size		64
char _buf[_moporgic_buf_size];

static inline void skipch(unsigned n) {
	while (n--) getchar();
}
static inline unsigned nextu() {
	unsigned v = 0;
	int ch;
	_appif_avail(v, ch);
	return v;
}
static inline long long unsigned int nextlu() {
	int ch;
	long long unsigned int v = 0;
	_appif_avail(v, ch);
	return v;
}
static inline int nextd() {
	int v = 0, ch;
	_appif_avail(v, ch);
	if (ch != '-') return v;
	_appif_avail(v, ch);
	return -v;
}
static inline long long int nextld() {
	int ch;
	long long int v = 0;
	_appif_avail(v, ch);
	if (ch != '-') return v;
	_appif_avail(v, ch);
	return -v;
}
static inline float nextf() {
	int ch;
	float v = 0, f = 10;
	_appif_avail(v, ch);
	if (ch == '.') {
		_appif_avifp(v, ch, f);
		return v;
	}
	if (ch != '-') return v;
	_appif_avail(v, ch);
	if (ch != '.') return -v;
	_appif_avifp(v, ch, f);
	return v;
}
static inline double nextlf() {
	int ch;
	double v = 0, f = 10;
	_appif_avail(v, ch);
	if (ch == '.') {
		_appif_avifp(v, ch, f);
		return v;
	}
	if (ch != '-') return v;
	_appif_avail(v, ch);
	if (ch != '.') return -v;
	_appif_avifp(v, ch, f);
	return -v;
}
static inline int next(char buf[], unsigned len) {
	int ch;
	if ( _getch(ch) == EOF) return EOF;
	unsigned idx = 0;
	for (; idx < len && ch != EOF && ch > ' '; _getch(ch)) buf[idx++] = ch;
	if (idx < len) buf[idx] = '\0';
	return idx;
}
static inline int nextln(char buf[], unsigned len) {
	int ch;
	if ( _getch(ch) == EOF) return EOF;
	unsigned idx = 0;
	for (; idx < len && ch != EOF && ch != '\n'; _getch(ch)) buf[idx++] = ch;
	if (idx < len) buf[idx] = '\0';
	return idx;
}
static inline void postu(unsigned u) {
	char *ipt, *bpt, *fpt = _buf + _postu_fptoff;
	_post_procn(u, _buf, ipt, bpt, fpt);
	_flushbufn(bpt, fpt);
}
static inline void postlu(unsigned long long u) {
	char *ipt, *bpt, *fpt = _buf + _postlu_fptoff;
	_post_procn(u, _buf, ipt, bpt, fpt);
	_flushbufn(bpt, fpt);
}
static inline void postd(int d) {
	_post_lt0(d);
	postu(d);
}
static inline void postld(long long d) {
	_post_lt0(d);
	postlu(d);
}
static inline void postlf(double v, unsigned prec = _postf_defprec) {
	_post_lt0(v);
	_postf_vtype f, u, p = 1;
	unsigned int t = prec;
	char *ipt, *bpt, *fpt = _buf + _postf_fptoff;
	while (t--) p = (p << 3) + (p << 1);
	u = (_postf_vtype) (v = v * p);
	f = (v - u >= 0.5) ? (_postf_vtype) (v + 1) : u;
	u = f / p;
	_post_procn(u, _buf, ipt, bpt, fpt);
	_flushbufn(bpt, fpt);
	if (prec == 0) return;
	putchar('.');
	u = f % p;
	_post_procn(u, _buf, ipt, bpt, fpt);
	for (t = fpt - bpt; t < prec; t++) putchar('0');
	_flushbufn(bpt, fpt);
}
static inline void postf(float v, unsigned prec = _postf_defprec) {
	postlf(v, prec);
}
static inline void post(const char *buf, unsigned len) {
	for (const char *lim = buf + len; buf < lim; buf++) putchar(*buf);
}

#endif /* DIRECTIO */

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
