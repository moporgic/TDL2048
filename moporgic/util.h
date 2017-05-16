#pragma once
#define MOPORGIC_UTIL_H_
/*
 * util.h
 *
 *  Created on: 2015/10/24
 *      Author: moporgic
 */

#ifndef DEBUG

#define DLOG(msg,...)
#define constexpr constexpr

#else /* DEBUG */

#define DLOG(msg,...) printf(msg, ##__VA_ARGS__)
#define constexpr

#endif /* DEBUG */

#include <cstdint>
#include <chrono>
#include <string>
#include <cstdio>
#include <iostream>
#include <functional>

namespace moporgic {

inline uint64_t millisec() {
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	return ms.count();
}
inline uint64_t microsec() {
	auto now = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	return us.count();
}

constexpr
uint32_t to_hash_tail(const char* str, const uint32_t& hash) noexcept {
	if (*str) return to_hash_tail(str + 1, (hash << 5) - hash + (*str));
	return hash;
}
constexpr inline
uint32_t to_hash(const char* str) noexcept {
	return to_hash_tail(str, 0); // i' = 31 * i + c
}

inline
uint32_t to_hash(const std::string& str) noexcept {
	return to_hash(str.c_str());
}

inline
uint32_t rand24() {
#if   RAND_MAX == 0x7fffffff
	return rand() & 0x00ffffff;
#elif RAND_MAX == 0xffffffff
	return rand() & 0x00ffffff;
#elif RAND_MAX == 0x7fff
    return (rand() << 9) + (rand() >> 6);
#elif RAND_MAX == 0xffff
	return (rand() << 8) + (rand() >> 8);
#else
    return ((rand() << 9) + (rand() >> 6)) & 0x00ffffff;
#endif
}

inline float random() {
#if RAND_MAX == 0x7fff
	return static_cast<float>(rand24()) / static_cast<float>(0x01000000);
#else
	return static_cast<float>(static_cast<uint32_t>(rand())) / static_cast<float>(RAND_MAX);
#endif
}

inline
uint32_t rand16() {
#if RAND_MAX == 0x7fff
    return (rand() << 1) + (rand() & 1);
#else
    return rand() & 0xffff;
#endif
}

inline
uint32_t rand31() {
#if   RAND_MAX == 0x7fffffff
	return rand();
#elif RAND_MAX == 0xffffffff
	return rand() & 0x7fffffff;
#elif RAND_MAX == 0x7fff
    return (rand() << 16) + (rand() << 1) + (rand() & 1);
#elif RAND_MAX == 0xffff
	return (rand() << 16) + rand();
#else
    return (rand() << 16) + (rand() << 1) + (rand() & 1);
#endif
}

inline
uint32_t rand32() {
#if   RAND_MAX == 0x7fffffff
	return (rand() << 1) + (rand() & 1);
#elif RAND_MAX == 0xffffffff
	return rand();
#elif RAND_MAX == 0x7fff
    return (rand() << 17) + (rand() << 2) + (rand() & 3);
#elif RAND_MAX == 0xffff
	return (rand() << 16) + rand();
#else
    return (rand() << 17) + (rand() << 2) + (rand() & 3);
#endif
}

inline
uint64_t rand64() {
#if   RAND_MAX == 0x7fffffff
	return (static_cast<uint64_t>(rand()) << 33) + (static_cast<uint64_t>(rand()) << 2) + (static_cast<uint64_t>(rand()) & 3);
#elif RAND_MAX == 0xffffffff
	return (static_cast<uint64_t>(rand()) << 32) + (static_cast<uint64_t>(rand()));
#elif RAND_MAX == 0x7fff
    return (static_cast<uint64_t>(rand()) << 49) + (static_cast<uint64_t>(rand()) << 34)
    	 + (static_cast<uint64_t>(rand()) << 19) + (static_cast<uint64_t>(rand()) << 4) + (static_cast<uint64_t>(rand()) & 15);
#elif RAND_MAX == 0xffff
	return (static_cast<uint64_t>(rand()) << 48) + (static_cast<uint64_t>(rand()) << 32)
		 + (static_cast<uint64_t>(rand()) << 16) + (static_cast<uint64_t>(rand()));
#else
    return (static_cast<uint64_t>(rand()) << 49) + (static_cast<uint64_t>(rand()) << 34)
    	 + (static_cast<uint64_t>(rand()) << 19) + (static_cast<uint64_t>(rand()) << 4) + (static_cast<uint64_t>(rand()) & 15);
#endif
}

#if   RAND_MAX == 0x7fffffff
#define RANDX_MAX RAND_MAX
#elif RAND_MAX == 0xffffffff
#define RANDX_MAX RAND_MAX
#elif RAND_MAX == 0x7fff
#define RANDX_MAX 0x3fffffff
#elif RAND_MAX == 0xffff
#define RANDX_MAX 0xffffffff
#else
#define RANDX_MAX RAND_MAX
#endif

#if   RAND_MAX == 0x7fff
#define RANDX_MAX 0x3fffffff
#elif RAND_MAX == 0xffff
#define RANDX_MAX 0xffffffff
#else
#define RANDX_MAX RAND_MAX
#endif

inline
uint32_t randx() {
#if   RAND_MAX == 0x7fffffff
	return rand();
#elif RAND_MAX == 0xffffffff
	return rand();
#elif RAND_MAX == 0x7fff
    return (rand() << 15) + (rand());
#elif RAND_MAX == 0xffff
	return (rand() << 16) + (rand());
#else
	return rand();
#endif
}

inline unsigned long long rdtsc() {
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

#define alias_declare(alias, name)\
template <typename... types>\
auto alias(types&&... args) -> decltype(name(std::forward<types>(args)...)) {\
	return name(std::forward<types>(args)...);\
}

} /* moporgic */

#undef constexpr

namespace moporgic {

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
#define _post_procn(u, b, i, p, f)\
for (i = p = f - 1; i >= b; u /= 10, i--) if ( (*i = u % 10) != 0) p = i;

#define _postu_fptoff	32
#define _postlu_fptoff	64
#define _postf_fptoff	64
#define _postf_vtype	unsigned long long
#define _postf_defprec	6
#define _moporgic_buf_size		64
char _buf[_moporgic_buf_size];

inline void skipch(unsigned n) {
	while (n--) getchar();
}
inline unsigned nextu() {
	unsigned v = 0;
	int ch;
	_appif_avail(v, ch);
	return v;
}
inline long long unsigned int nextlu() {
	int ch;
	long long unsigned int v = 0;
	_appif_avail(v, ch);
	return v;
}
inline int nextd() {
	int v = 0, ch;
	_appif_avail(v, ch);
	if (ch != '-') return v;
	_appif_avail(v, ch);
	return -v;
}
inline long long int nextld() {
	int ch;
	long long int v = 0;
	_appif_avail(v, ch);
	if (ch != '-') return v;
	_appif_avail(v, ch);
	return -v;
}
inline float nextf() {
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
inline double nextlf() {
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
inline int next(char buf[], unsigned len) {
	int ch;
	if ( _getch(ch) == EOF) return EOF;
	unsigned idx = 0;
	for (; idx < len && ch != EOF && ch > ' '; _getch(ch)) buf[idx++] = ch;
	if (idx < len) buf[idx] = '\0';
	return idx;
}
inline int nextln(char buf[], unsigned len) {
	int ch;
	if ( _getch(ch) == EOF) return EOF;
	unsigned idx = 0;
	for (; idx < len && ch != EOF && ch != '\n'; _getch(ch)) buf[idx++] = ch;
	if (idx < len) buf[idx] = '\0';
	return idx;
}
inline void postu(unsigned u) {
	char *ipt, *bpt, *fpt = _buf + _postu_fptoff;
	_post_procn(u, _buf, ipt, bpt, fpt);
	_flushbufn(bpt, fpt);
}
inline void postlu(unsigned long long u) {
	char *ipt, *bpt, *fpt = _buf + _postlu_fptoff;
	_post_procn(u, _buf, ipt, bpt, fpt);
	_flushbufn(bpt, fpt);
}
inline void postd(int d) {
	_post_lt0(d);
	postu(d);
}
inline void postld(long long d) {
	_post_lt0(d);
	postlu(d);
}
inline void postlf(double v, unsigned prec = _postf_defprec) {
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
inline void postf(float v, unsigned prec = _postf_defprec) {
	postlf(v, prec);
}
inline void post(const char *buf, unsigned len) {
	for (const char *lim = buf + len; buf < lim; buf++) putchar(*buf);
}

template<typename type> inline
std::ostream& write(std::ostream& out, const type& v, const size_t& len = sizeof(type)) {
	return out.write(reinterpret_cast<char*>(&const_cast<type&>(v)), len);
}

template<typename type> inline
std::istream& read(std::istream& in, type& v, const size_t& len = sizeof(type)) {
	return in.read(reinterpret_cast<char*>(&v), len);
}

template<typename type> inline
std::ostream& write(std::ostream& out, const type* begin, const type* end, const size_t& len = sizeof(type)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		write(out, *value, len);
	return out;
}

template<typename type> inline
std::istream& read(std::istream& in, const type* begin, const type* end, const size_t& len = sizeof(type)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		read(in, *value, len);
	return in;
}

template<typename cast, typename type> inline
std::ostream& write_cast(std::ostream& out, const type& v, const size_t& len = sizeof(cast)) {
	return write(out, cast(v), len);
}

template<typename cast, typename type> inline
std::istream& read_cast(std::istream& in, type& v, const size_t& len = sizeof(cast)) {
	cast buf; read(in, buf, len); v = buf;
	return in;
}

template<typename cast, typename type> inline
std::ostream& write_cast(std::ostream& out, const type* begin, const type* end, const size_t& len = sizeof(cast)) {
	for (type* value = const_cast<type*>(begin); value != end; value++)
		write(out, cast(*value), len);
	return out;
}

template<typename cast, typename type> inline
std::istream& read_cast(std::istream& in, const type* begin, const type* end, const size_t& len = sizeof(cast)) {
	cast buf;
	for (type* value = const_cast<type*>(begin); value != end; *value = buf, value++)
		read(in, buf, len);
	return in;
}

#ifdef _GLIBCXX_FUNCTIONAL

typedef std::function<char*(const unsigned&)> load_t;

char* istream_load(std::istream& in, char* buf, const unsigned& len, const int& on_fail) {
	in.read(buf, len);
	if (!in) {
		if (on_fail & 0x1) std::cerr << "error: load failure" << std::endl;
		if (on_fail & 0x2) std::exit(on_fail);
		if (on_fail & 0x4) throw std::out_of_range("error: load failure");
	}
	return buf;
}

load_t make_load(std::istream& in, char* buf, int on_fail = 0x3) {
	return std::bind(istream_load, std::ref(in), buf, std::placeholders::_1, on_fail);
}

#endif /* _GLIBCXX_FUNCTIONAL */


} /* moporgic */
