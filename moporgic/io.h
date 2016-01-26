#pragma once
#define MOPORGIC_IO_H_
/*
 * mopoio.h
 *
 *  Created on: 2013/4/27
 *      Author: moporgic
 */

#include <cstdio>
#include <iostream>
#include <functional>

namespace moporgic {

//#define putln()			putchar('\n')
//#define putsp()			putchar(' ')
//#define puttab()		putchar('\t')
//#define print(...)		printf(__VA_ARGS__)
//#define println(...)	printf(__VA_ARGS__); putchar('\n')
#define removebuf()		setvbuf(stdout, NULL, _IONBF, 0)
#define	randinit()		srand(time(nullptr))
#define mallof(T, size) ((T*) malloc(sizeof(T) * size))
#define callof(T, size) ((T*) calloc(size, sizeof(T))

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

#endif

template<typename type> inline
std::ostream& write(std::ostream& out, const type& v) {
	return out.write(reinterpret_cast<char*>(&const_cast<type&>(v)), sizeof(v));
}

template<typename type> inline
std::istream& read(std::istream& in, type& v) {
	return in.read(reinterpret_cast<char*>(&v), sizeof(v));
}

#ifdef MOPORGIC_MACRO
#define postusp(v)	moporgic::postu(v); putsp();
#define postlusp(v)	moporgic::postlu(v); putsp();
#define postdsp(v)	moporgic::postd(v); putsp();
#define postldsp(v)	moporgic::postld(v); putsp();
#define postfsp(v)	moporgic::postf(v); putsp();
#define postlfsp(v)	moporgic::postlf(v); putsp();
#define postsp(b,l)	moporgic::post(b, l); putsp();

#define postuln(v)	moporgic::postu(v); putln();
#define postluln(v)	moporgic::postlu(v); putln();
#define postdln(v)	moporgic::postd(v); putln();
#define postldln(v)	moporgic::postld(v); putln();
#define postfln(v)	moporgic::postf(v); putln();
#define postlfln(v)	moporgic::postlf(v); putln();
#define postln(b,l)	moporgic::post(b, l); putln();

#define putu(v)		moporgic::postu(v); putln();
#define putlu(v)	moporgic::postlu(v); putln();
#define putd(v)		moporgic::postd(v); putln();
#define putld(v)	moporgic::postld(v); putln();
#define putf(v)		moporgic::postf(v); putln();
#define putlf(v)	moporgic::postlf(v); putln();
#endif

#undef _getch
#undef _appendn
#undef _appendf
#undef _appif_avail
#undef _appif_avifp
#undef _flushbuf
#undef _flushbufn
#undef _post_lt0
#undef _post_procn
#undef _postu_fptoff
#undef _postlu_fptoff
#undef _postf_fptoff
#undef _postf_vtype
#undef _postf_defprec

} // namespace moporgic

namespace moporgic {

#define setwf(w, f) std::setw(w) << std::setfill(f)
#define sethex() std::hex << setwf(2, '0')
#define setpw(p, w) p << std::setw(w)
#define setpwf(p, w, f) setpw(p, w) << std::setfill(f)

//inline std::ostream& setwf(std::ostream& out, int w, char f) {
//	return out << std::setw(w) << std::setfill(f);
//}
//inline std::ostream& sethex(std::ostream& out) {
//	return out << std::hex << setwf(out, 2, '0');
//}
//inline std::ostream& setpw(std::ostream& out, std::ios_base& (*p) (std::ios_base&), int w) {
//	return out << p << std::setw(w);
//}
//inline std::ostream& setpwf(std::ostream& out, std::ios_base& (*p) (std::ios_base&), int w, char f) {
//	return setpw(out, p, w) << std::setfill(f);
//}

} // namespace moporgic
