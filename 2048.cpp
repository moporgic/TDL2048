//============================================================================
// Name        : 2048.cpp
// Author      : Hung Guei
// Version     : beta
// Description : 2048
//============================================================================

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "moporgic/type.h"
#include "moporgic/util.h"
#include "moporgic/math.h"
#include "moporgic/io.h"
#include "board.h"
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <cmath>
#include <ctime>
#include <tuple>
#include <string>
#include <numeric>
#include <thread>
#include <limits>
#include <cctype>
#include <iterator>
#include <sstream>
#include <list>

namespace moporgic {

typedef double numeric;
numeric alpha = 0.0025;
const u64 base = 16;

class weight {
public:
	weight() : sign(0), size(0), value(nullptr) {}
	weight(const weight& w) : sign(w.sign), size(w.size), value(w.value) {}
	~weight() {}

	inline u32 signature() const { return sign; }
	inline size_t length() const { return size; }
	inline numeric& operator [](const u64& i) { return value[i]; }

	inline bool operator ==(const weight& w) const { return sign == w.sign; }
	inline bool operator !=(const weight& w) const { return sign != w.sign; }

	void operator >>(std::ostream& out) const {
		const char serial = 1;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(sign).le(), 4);
			out.write(r64(size).le(), 8);
			write<r32>(out);
			break;
		case 1:
			out.write(r32(sign).le(), 4);
			out.write(r64(size).le(), 8);
			switch (sizeof(numeric)) {
			case 4: write<r32>(out); break;
			case 8: write<r64>(out); break;
			}
			break;
		default:
			std::cerr << "unknown serial at weight::>>" << std::endl;
			break;
		}
	}
	void operator <<(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		switch (*load(1)) {
		case 0:
			sign = r32(load(4)).le();
			size = r64(load(8)).le();
			value = alloc(size);
			read<r32>(in);
			break;
		case 1:
			sign = r32(load(4)).le();
			size = r64(load(8)).le();
			value = alloc(size);
			switch (sizeof(numeric)) {
			case 4: read<r32>(in); break;
			case 8: read<r64>(in); break;
			}
			break;
		default:
			std::cerr << "unknown serial at weight::<<" << std::endl;
			break;
		}
	}

	static void save(std::ostream& out) {
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(u32(wghts().size())).le(), 4);
			for (weight w : wghts())
				w >> out;
			break;
		default:
			std::cerr << "unknown serial at weight::save" << std::endl;
			break;
		}
		out.flush();
	}
	static void load(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		char serial;
		in.read(&serial, 1);
		switch (serial) {
		case 0:
			in.read(buf, 4);
			for (u32 size = r32(buf).le(); size; size--) {
				weight w; w << in;
				wghts().push_back(w);
			}
			break;
		default:
			std::cerr << "unknown serial at weight::load" << std::endl;
			break;
		}
	}

	static weight make(const u32& sign, const size_t& size) {
		wghts().push_back(weight(sign, size));
		return wghts().back();
	}
	static inline weight at(const u32& sign) {
		const auto it = find(sign);
		if (it != end()) return (*it);
		throw std::out_of_range("weight::at");
	}
	typedef std::vector<weight>::iterator iter;
	static inline const std::vector<weight>& list() { return wghts(); }
	static inline iter begin() { return wghts().begin(); }
	static inline iter end() { return wghts().end(); }
	static inline iter find(const u32& sign) {
		return std::find_if(begin(), end(), [=](const weight& w) { return w.sign == sign; });
	}
private:
	weight(const u32& sign, const size_t& size) : sign(sign), size(size), value(alloc(size)) {}
	static inline std::vector<weight>& wghts() { static std::vector<weight> w; return w; }

	static inline numeric* alloc(const size_t& size) {
		return new numeric[size]();
	}

	template<typename rxx> void write(std::ostream& out) const {
		for (size_t i = 0; i < size; i++)
			out.write(rxx(value[i]).le(), sizeof(rxx));
	}
	template<typename rxx> void read(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		for (size_t i = 0; i < size; i++)
			value[i] = rxx(load(sizeof(rxx))).le();
	}

	u32 sign;
	size_t size;
	numeric* value;
};

class indexer {
public:
	indexer() : sign(0), map(nullptr) {}
	indexer(const indexer& i) : sign(i.sign), map(i.map) {}
	~indexer() {}

	typedef std::function<u64(const board&)> mapper;

	inline u32 signature() const { return sign; }
	inline mapper index() const { return map; }
	inline u64 operator ()(const board& b) const { return map(b); }

	inline bool operator ==(const indexer& i) const { return sign == i.sign; }
	inline bool operator !=(const indexer& i) const { return sign != i.sign; }

	static indexer make(const u32& sign, mapper map) {
		idxrs().push_back(indexer(sign, map));
		return idxrs().back();
	}
	static inline indexer at(const u32& sign) {
		const auto it = find(sign);
		if (it != end()) return (*it);
		throw std::out_of_range("indexer::at");
	}
	typedef std::vector<indexer>::iterator iter;
	static inline const std::vector<indexer>& list() { return idxrs(); }
	static inline iter begin() { return idxrs().begin(); }
	static inline iter end() { return idxrs().end(); }
	static inline iter find(const u32& sign) {
		return std::find_if(begin(), end(), [=](const indexer& i) { return i.sign == sign; });
	}
private:
	indexer(const u32& sign, mapper map) : sign(sign), map(map) {}
	static inline std::vector<indexer>& idxrs() { static std::vector<indexer> i; return i; }

	u32 sign;
	mapper map;
};

class feature {
public:
	feature() {}
	feature(const feature& t) : index(t.index), value(t.value) {}
	~feature() {}

	inline u64 signature() const { return make_sign(value.signature(), index.signature()); }
	inline operator indexer() const { return index; }
	inline operator weight() const { return value; }
	inline numeric& operator [](const board& b) { return value[index(b)]; }
	inline numeric& operator [](const u64& idx) { return value[idx]; }
	inline u64 operator ()(const board& b) const { return index(b); }

	inline bool operator ==(const feature& f) const { return signature() == f.signature(); }
	inline bool operator !=(const feature& f) const { return signature() != f.signature(); }

	void operator >>(std::ostream& out) const {
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(index.signature()).le(), 4);
			out.write(r32(value.signature()).le(), 4);
			break;
		default:
			std::cerr << "unknown serial at feature::>>" << std::endl;
			break;
		}
	}
	void operator <<(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		switch (*load(1)) {
		case 0:
			index = indexer::at(r32(load(4)).le());
			value = weight::at(r32(load(4)).le());
			break;
		default:
			std::cerr << "unknown serial at feature::<<" << std::endl;
			break;
		}
	}

	static void save(std::ostream& out) {
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(u32(feats().size())).le(), 4);
			for (u32 i = 0, size = feats().size(); i < size; i++)
				feats()[i] >> out;
			break;
		default:
			std::cerr << "unknown serial at feature::save" << std::endl;
			break;
		}
		out.flush();
	}
	static void load(std::istream& in) {
		char buf[4];
		char serial;
		in.read(&serial, 1);
		switch (serial) {
		case 0:
			in.read(buf, 4);
			for (u32 size = r32(buf).le(); size; size--) {
				feats().push_back(feature());
				feats().back() << in;
			}
			break;
		default:
			std::cerr << "unknown serial at feature::load" << std::endl;
			break;
		}
	}

	static feature make(const u32& wgt, const u32& idx) {
		feats().push_back(feature(weight::at(wgt), indexer::at(idx)));
		return feats().back();
	}
	static feature at(const u32& wgt, const u32& idx) {
		const auto it = find(wgt, idx);
		if (it != end()) return (*it);
		throw std::out_of_range("feature::at");
	}
	typedef std::vector<feature>::iterator iter;
	static inline const std::vector<feature>& list() { return feats(); }
	static inline iter begin() { return feats().begin(); }
	static inline iter end() { return feats().end(); }
	static inline iter find(const u32& wgt, const u32& idx) {
		const u32 sign = make_sign(wgt, idx);
		return std::find_if(begin(), end(), [=](const feature& f) { return f.signature() == sign; });
	}

private:
	feature(const weight& value, const indexer& index) : index(index), value(value) {}
	static inline std::vector<feature>& feats() { static std::vector<feature> f; return f; }
	static inline u64 make_sign(const u32& wgt, const u32& idx) { return (u64(wgt) << 32) | idx; }

	indexer index;
	weight value;
};

namespace utils {

struct options : public std::list<std::string> {
	options() : std::list<std::string>() {}
	options(const std::list<std::string>& opts) : std::list<std::string>(opts) {}

	options& operator +=(const std::string& opt) {
		push_back(opt);
		return *this;
	}
	options& operator -=(const std::string& opt) {
		auto it = std::find(begin(), end(), opt);
		if (it != end()) this->erase(it);
		return *this;
	}

	bool exists(const std::string& opt) const {
		return std::find(begin(), end(), opt) != end();
	}
	std::string& operator [](const std::string& opt) {
		auto it = std::find(begin(), end(), opt);
		if (it == end()) return (operator +=(opt))[opt];
		if (++it == end()) return (operator +=(""))[opt];
		return (*it);
	}

	operator std::string() const {
		std::stringstream ss;
		std::copy(begin(), end(), std::ostream_iterator<std::string>(ss, " "));
		return ss.str();
	}
};

inline u32 hashpatt(const std::vector<int>& patt) {
	u32 hash = 0;
	for (auto tile : patt) hash = (hash << 4) | tile;
	return hash;
}
inline std::vector<int> hashpatt(const std::string& hashs) {
	u32 hash; std::stringstream(hashs) >> std::hex >> hash;
	std::vector<int> patt(hashs.size());
	for (auto it = patt.rbegin(); it != patt.rend(); it++, hash >>= 4)
		(*it) = hash & 0x0f;
	return patt;
}

template<int p0, int p1, int p2, int p3, int p4, int p5>
u64 index6t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	index += b.at(p4) << 16;
	index += b.at(p5) << 20;
	return index;
}
template<int p0, int p1, int p2, int p3>
u64 index4t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	return index;
}
template<int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7>
u64 index8t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	index += b.at(p4) << 16;
	index += b.at(p5) << 20;
	index += b.at(p6) << 24;
	index += b.at(p7) << 28;
	return index;
}
template<int p0, int p1, int p2, int p3, int p4, int p5, int p6>
u64 index7t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	index += b.at(p4) << 16;
	index += b.at(p5) << 20;
	index += b.at(p6) << 24;
	return index;
}
template<int p0, int p1, int p2, int p3, int p4>
u64 index5t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	index += b.at(p4) << 16;
	return index;
}

u64 indexnta(const board& b, const std::vector<int>& p) {
	register u64 index = 0;
	for (size_t i = 0; i < p.size(); i++)
		index += b.at(p[i]) << (i << 2);
	return index;
}

u64 indexmerge0(const board& b) { // 16-bit
	board q = b; q.transpose();
	register u32 hori = 0, vert = 0;
	hori |= b.query(0).merge << 0;
	hori |= b.query(1).merge << 2;
	hori |= b.query(2).merge << 4;
	hori |= b.query(3).merge << 6;
	vert |= q.query(0).merge << 0;
	vert |= q.query(1).merge << 2;
	vert |= q.query(2).merge << 4;
	vert |= q.query(3).merge << 6;
	return hori | (vert << 8);
}

template<int transpose>
u64 indexmerge1(const board& b) { // 8-bit
	register u32 merge = 0;
	board k = b; if (transpose) k.transpose();
	merge |= k.query(0).merge << 0;
	merge |= k.query(1).merge << 2;
	merge |= k.query(2).merge << 4;
	merge |= k.query(3).merge << 6;
	return merge;
}

u64 indexnum0(const board& b) { // 10-bit
	// 2k ~ 32k, 2-bit ea.
	u16 num[16];
	b.count(num, 11, 16);
	register u64 index = 0;
	index += (num[11] & 0x03) << 0;
	index += (num[12] & 0x03) << 2;
	index += (num[13] & 0x03) << 4;
	index += (num[14] & 0x03) << 6;
	index += (num[15] & 0x03) << 8;
	return index;
}

u64 indexnum1(const board& b) { // 25-bit
	u16 num[16];
	b.count(num, 5, 16);
	register u64 index = 0;
	index += ((num[5] + num[6]) & 0x0f) << 0; // 32 & 64, 4-bit
	index += (num[7] & 0x07) << 4; // 128, 3-bit
	index += (num[8] & 0x07) << 7; // 256, 3-bit
	index += (num[9] & 0x07) << 10; // 512, 3-bit
	index += (num[10] & 0x03) << 13; // 1k ~ 32k, 2-bit ea.
	index += (num[11] & 0x03) << 15;
	index += (num[12] & 0x03) << 17;
	index += (num[13] & 0x03) << 19;
	index += (num[14] & 0x03) << 21;
	index += (num[15] & 0x03) << 23;
	return index;
}

u64 indexnum2(const board& b) { // 25-bit
	u16 num[16];
	b.count(num, 0, 16);
	register u64 index = 0;
	index += ((num[1] + num[2]) & 0x07) << 0; // 2 & 4, 3-bit
	index += ((num[3] + num[4]) & 0x07) << 3; // 8 & 16, 3-bit
	index += ((num[5] + num[6]) & 0x07) << 6; // 32 & 64, 3-bit
	index += ((num[7] + num[8]) & 0x07) << 9; // 126 & 256, 3-bit
	index += ((num[9] + num[10]) & 0x07) << 12; // 512 & 1k, 3-bit
	index += ((num[11]) & 0x03) << 15; // 2k ~ 32k, 2-bit ea.
	index += ((num[12]) & 0x03) << 17;
	index += ((num[13]) & 0x03) << 19;
	index += ((num[14]) & 0x03) << 21;
	index += ((num[15]) & 0x03) << 23;

	return index;
}

template<int transpose, int qu0, int qu1>
u64 indexnum2x(const board& b) { // 25-bit
	board o = b;
	if (transpose) o.transpose();
	auto& m = o.query(qu0).count;
	auto& n = o.query(qu1).count;

	register u64 index = 0;
	index += ((m[1] + n[1] + m[2] + n[2]) & 0x07) << 0; // 2 & 4, 3-bit
	index += ((m[3] + n[3] + m[4] + n[4]) & 0x07) << 3; // 8 & 16, 3-bit
	index += ((m[5] + n[5] + m[6] + n[6]) & 0x07) << 6; // 32 & 64, 3-bit
	index += ((m[7] + n[7] + m[8] + n[8]) & 0x07) << 9; // 126 & 256, 3-bit
	index += ((m[9] + n[9] + m[10] + n[10]) & 0x07) << 12; // 512 & 1k, 3-bit
	index += ((m[11] + n[11]) & 0x03) << 15; // 2k ~ 32k, 2-bit ea.
	index += ((m[12] + n[12]) & 0x03) << 17;
	index += ((m[13] + n[13]) & 0x03) << 19;
	index += ((m[14] + n[14]) & 0x03) << 21;
	index += ((m[15] + n[15]) & 0x03) << 23;

	return index;
}

template<int isomorphic>
u64 indexmono(const board& b) { // 24-bit
	board k = b;
	k.rotate(isomorphic);
	if (isomorphic >= 4) k.mirror();
	return k.mono();
}

template<u32 tile, int isomorphic>
u64 indexmask(const board& b) { // 16-bit
	board k = b;
	k.rotate(isomorphic);
	if (isomorphic >= 4) k.mirror();
	return k.mask(tile);
}

template<int isomorphic>
u64 indexmax(const board& b) { // 16-bit
	board k = b;
	k.rotate(isomorphic);
	if (isomorphic >= 4) k.mirror();
	return k.mask(k.max());
}

void make_indexers(const std::string& res = "") {
	auto imake = [](u32 sign, indexer::mapper func) {
		if (indexer::find(sign) == indexer::end()) indexer::make(sign, func);
	};

	imake(0x00012367, utils::index6t<0x0,0x1,0x2,0x3,0x6,0x7>);
	imake(0x0037bfae, utils::index6t<0x3,0x7,0xb,0xf,0xa,0xe>);
	imake(0x00fedc98, utils::index6t<0xf,0xe,0xd,0xc,0x9,0x8>);
	imake(0x00c84051, utils::index6t<0xc,0x8,0x4,0x0,0x5,0x1>);
	imake(0x00fb7362, utils::index6t<0xf,0xb,0x7,0x3,0x6,0x2>);
	imake(0x00cdefab, utils::index6t<0xc,0xd,0xe,0xf,0xa,0xb>);
	imake(0x00048c9d, utils::index6t<0x0,0x4,0x8,0xc,0x9,0xd>);
	imake(0x00321054, utils::index6t<0x3,0x2,0x1,0x0,0x5,0x4>);
	imake(0x004567ab, utils::index6t<0x4,0x5,0x6,0x7,0xa,0xb>);
	imake(0x0026ae9d, utils::index6t<0x2,0x6,0xa,0xe,0x9,0xd>);
	imake(0x00ba9854, utils::index6t<0xb,0xa,0x9,0x8,0x5,0x4>);
	imake(0x00d95162, utils::index6t<0xd,0x9,0x5,0x1,0x6,0x2>);
	imake(0x00ea6251, utils::index6t<0xe,0xa,0x6,0x2,0x5,0x1>);
	imake(0x0089ab67, utils::index6t<0x8,0x9,0xa,0xb,0x6,0x7>);
	imake(0x00159dae, utils::index6t<0x1,0x5,0x9,0xd,0xa,0xe>);
	imake(0x00765498, utils::index6t<0x7,0x6,0x5,0x4,0x9,0x8>);
	imake(0x00012456, utils::index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	imake(0x0037b26a, utils::index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	imake(0x00fedba9, utils::index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	imake(0x00c84d95, utils::index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	imake(0x00fb7ea6, utils::index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	imake(0x00cde89a, utils::index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	imake(0x00048159, utils::index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	imake(0x00321765, utils::index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	imake(0x0045689a, utils::index6t<0x4,0x5,0x6,0x8,0x9,0xa>);
	imake(0x0026a159, utils::index6t<0x2,0x6,0xa,0x1,0x5,0x9>);
	imake(0x00ba9765, utils::index6t<0xb,0xa,0x9,0x7,0x6,0x5>);
	imake(0x00d95ea6, utils::index6t<0xd,0x9,0x5,0xe,0xa,0x6>);
	imake(0x00ea6d95, utils::index6t<0xe,0xa,0x6,0xd,0x9,0x5>);
	imake(0x0089a456, utils::index6t<0x8,0x9,0xa,0x4,0x5,0x6>);
	imake(0x0015926a, utils::index6t<0x1,0x5,0x9,0x2,0x6,0xa>);
	imake(0x00765ba9, utils::index6t<0x7,0x6,0x5,0xb,0xa,0x9>);
	imake(0x00000123, utils::index4t<0x0,0x1,0x2,0x3>);
	imake(0x00004567, utils::index4t<0x4,0x5,0x6,0x7>);
	imake(0x000089ab, utils::index4t<0x8,0x9,0xa,0xb>);
	imake(0x0000cdef, utils::index4t<0xc,0xd,0xe,0xf>);
	imake(0x000037bf, utils::index4t<0x3,0x7,0xb,0xf>);
	imake(0x000026ae, utils::index4t<0x2,0x6,0xa,0xe>);
	imake(0x0000159d, utils::index4t<0x1,0x5,0x9,0xd>);
	imake(0x0000048c, utils::index4t<0x0,0x4,0x8,0xc>);
	imake(0x0000fedc, utils::index4t<0xf,0xe,0xd,0xc>);
	imake(0x0000ba98, utils::index4t<0xb,0xa,0x9,0x8>);
	imake(0x00007654, utils::index4t<0x7,0x6,0x5,0x4>);
	imake(0x00003210, utils::index4t<0x3,0x2,0x1,0x0>);
	imake(0x0000c840, utils::index4t<0xc,0x8,0x4,0x0>);
	imake(0x0000d951, utils::index4t<0xd,0x9,0x5,0x1>);
	imake(0x0000ea62, utils::index4t<0xe,0xa,0x6,0x2>);
	imake(0x0000fb73, utils::index4t<0xf,0xb,0x7,0x3>);
	imake(0x00000145, utils::index4t<0x0,0x1,0x4,0x5>);
	imake(0x00001256, utils::index4t<0x1,0x2,0x5,0x6>);
	imake(0x00002367, utils::index4t<0x2,0x3,0x6,0x7>);
	imake(0x00004589, utils::index4t<0x4,0x5,0x8,0x9>);
	imake(0x0000569a, utils::index4t<0x5,0x6,0x9,0xa>);
	imake(0x000067ab, utils::index4t<0x6,0x7,0xa,0xb>);
	imake(0x000089cd, utils::index4t<0x8,0x9,0xc,0xd>);
	imake(0x00009ade, utils::index4t<0x9,0xa,0xd,0xe>);
	imake(0x0000abef, utils::index4t<0xa,0xb,0xe,0xf>);
	imake(0x00003726, utils::index4t<0x3,0x7,0x2,0x6>);
	imake(0x00007b6a, utils::index4t<0x7,0xb,0x6,0xa>);
	imake(0x0000bfae, utils::index4t<0xb,0xf,0xa,0xe>);
	imake(0x00002615, utils::index4t<0x2,0x6,0x1,0x5>);
	imake(0x00006a59, utils::index4t<0x6,0xa,0x5,0x9>);
	imake(0x0000ae9d, utils::index4t<0xa,0xe,0x9,0xd>);
	imake(0x00001504, utils::index4t<0x1,0x5,0x0,0x4>);
	imake(0x00005948, utils::index4t<0x5,0x9,0x4,0x8>);
	imake(0x00009d8c, utils::index4t<0x9,0xd,0x8,0xc>);
	imake(0x0000feba, utils::index4t<0xf,0xe,0xb,0xa>);
	imake(0x0000eda9, utils::index4t<0xe,0xd,0xa,0x9>);
	imake(0x0000dc98, utils::index4t<0xd,0xc,0x9,0x8>);
	imake(0x0000ba76, utils::index4t<0xb,0xa,0x7,0x6>);
	imake(0x0000a965, utils::index4t<0xa,0x9,0x6,0x5>);
	imake(0x00009854, utils::index4t<0x9,0x8,0x5,0x4>);
	imake(0x00007632, utils::index4t<0x7,0x6,0x3,0x2>);
	imake(0x00006521, utils::index4t<0x6,0x5,0x2,0x1>);
	imake(0x00005410, utils::index4t<0x5,0x4,0x1,0x0>);
	imake(0x0000c8d9, utils::index4t<0xc,0x8,0xd,0x9>);
	imake(0x00008495, utils::index4t<0x8,0x4,0x9,0x5>);
	imake(0x00004051, utils::index4t<0x4,0x0,0x5,0x1>);
	imake(0x0000d9ea, utils::index4t<0xd,0x9,0xe,0xa>);
	imake(0x000095a6, utils::index4t<0x9,0x5,0xa,0x6>);
	imake(0x00005162, utils::index4t<0x5,0x1,0x6,0x2>);
	imake(0x0000eafb, utils::index4t<0xe,0xa,0xf,0xb>);
	imake(0x0000a6b7, utils::index4t<0xa,0x6,0xb,0x7>);
	imake(0x00006273, utils::index4t<0x6,0x2,0x7,0x3>);
	imake(0x00003276, utils::index4t<0x3,0x2,0x7,0x6>);
	imake(0x00002165, utils::index4t<0x2,0x1,0x6,0x5>);
	imake(0x00001054, utils::index4t<0x1,0x0,0x5,0x4>);
	imake(0x000076ba, utils::index4t<0x7,0x6,0xb,0xa>);
	imake(0x000065a9, utils::index4t<0x6,0x5,0xa,0x9>);
	imake(0x00005498, utils::index4t<0x5,0x4,0x9,0x8>);
	imake(0x0000bafe, utils::index4t<0xb,0xa,0xf,0xe>);
	imake(0x0000a9ed, utils::index4t<0xa,0x9,0xe,0xd>);
	imake(0x000098dc, utils::index4t<0x9,0x8,0xd,0xc>);
	imake(0x0000fbea, utils::index4t<0xf,0xb,0xe,0xa>);
	imake(0x0000b7a6, utils::index4t<0xb,0x7,0xa,0x6>);
	imake(0x00007362, utils::index4t<0x7,0x3,0x6,0x2>);
	imake(0x0000ead9, utils::index4t<0xe,0xa,0xd,0x9>);
	imake(0x0000a695, utils::index4t<0xa,0x6,0x9,0x5>);
	imake(0x00006251, utils::index4t<0x6,0x2,0x5,0x1>);
	imake(0x0000d9c8, utils::index4t<0xd,0x9,0xc,0x8>);
	imake(0x00009584, utils::index4t<0x9,0x5,0x8,0x4>);
	imake(0x00005140, utils::index4t<0x5,0x1,0x4,0x0>);
	imake(0x0000cd89, utils::index4t<0xc,0xd,0x8,0x9>);
	imake(0x0000de9a, utils::index4t<0xd,0xe,0x9,0xa>);
	imake(0x0000efab, utils::index4t<0xe,0xf,0xa,0xb>);
	imake(0x00008945, utils::index4t<0x8,0x9,0x4,0x5>);
	imake(0x00009a56, utils::index4t<0x9,0xa,0x5,0x6>);
	imake(0x0000ab67, utils::index4t<0xa,0xb,0x6,0x7>);
	imake(0x00004501, utils::index4t<0x4,0x5,0x0,0x1>);
	imake(0x00005612, utils::index4t<0x5,0x6,0x1,0x2>);
	imake(0x00006723, utils::index4t<0x6,0x7,0x2,0x3>);
	imake(0x00000415, utils::index4t<0x0,0x4,0x1,0x5>);
	imake(0x00004859, utils::index4t<0x4,0x8,0x5,0x9>);
	imake(0x00008c9d, utils::index4t<0x8,0xc,0x9,0xd>);
	imake(0x00001526, utils::index4t<0x1,0x5,0x2,0x6>);
	imake(0x0000596a, utils::index4t<0x5,0x9,0x6,0xa>);
	imake(0x00009dae, utils::index4t<0x9,0xd,0xa,0xe>);
	imake(0x00002637, utils::index4t<0x2,0x6,0x3,0x7>);
	imake(0x00006a7b, utils::index4t<0x6,0xa,0x7,0xb>);
	imake(0x0000aebf, utils::index4t<0xa,0xe,0xb,0xf>);
	imake(0x01234567, utils::index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	imake(0x456789ab, utils::index8t<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	imake(0x37bf26ae, utils::index8t<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	imake(0x26ae159d, utils::index8t<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	imake(0xfedcba98, utils::index8t<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	imake(0xba987654, utils::index8t<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	imake(0xc840d951, utils::index8t<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	imake(0xd951ea62, utils::index8t<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	imake(0x32107654, utils::index8t<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	imake(0x7654ba98, utils::index8t<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	imake(0xfb73ea62, utils::index8t<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	imake(0xea62d951, utils::index8t<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	imake(0xcdef89ab, utils::index8t<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	imake(0x89ab4567, utils::index8t<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	imake(0x048c159d, utils::index8t<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	imake(0x159d26ae, utils::index8t<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	imake(0x00123456, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x6>);
	imake(0x037bf26a, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x6,0xa>);
	imake(0x0fedcba9, utils::index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x9>);
	imake(0x0c840d95, utils::index7t<0xc,0x8,0x4,0x0,0xd,0x9,0x5>);
	imake(0x03210765, utils::index7t<0x3,0x2,0x1,0x0,0x7,0x6,0x5>);
	imake(0x0fb73ea6, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xa,0x6>);
	imake(0x0cdef89a, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x9,0xa>);
	imake(0x0048c159, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x9>);
	imake(0x0456789a, utils::index7t<0x4,0x5,0x6,0x7,0x8,0x9,0xa>);
	imake(0x026ae159, utils::index7t<0x2,0x6,0xa,0xe,0x1,0x5,0x9>);
	imake(0x0ba98765, utils::index7t<0xb,0xa,0x9,0x8,0x7,0x6,0x5>);
	imake(0x0d951ea6, utils::index7t<0xd,0x9,0x5,0x1,0xe,0xa,0x6>);
	imake(0x07654ba9, utils::index7t<0x7,0x6,0x5,0x4,0xb,0xa,0x9>);
	imake(0x0ea62d95, utils::index7t<0xe,0xa,0x6,0x2,0xd,0x9,0x5>);
	imake(0x089ab456, utils::index7t<0x8,0x9,0xa,0xb,0x4,0x5,0x6>);
	imake(0x0159d26a, utils::index7t<0x1,0x5,0x9,0xd,0x2,0x6,0xa>);
	imake(0x00001234, utils::index5t<0x0,0x1,0x2,0x3,0x4>);
	imake(0x00037bf2, utils::index5t<0x3,0x7,0xb,0xf,0x2>);
	imake(0x000fedcb, utils::index5t<0xf,0xe,0xd,0xc,0xb>);
	imake(0x000c840d, utils::index5t<0xc,0x8,0x4,0x0,0xd>);
	imake(0x00032107, utils::index5t<0x3,0x2,0x1,0x0,0x7>);
	imake(0x000fb73e, utils::index5t<0xf,0xb,0x7,0x3,0xe>);
	imake(0x000cdef8, utils::index5t<0xc,0xd,0xe,0xf,0x8>);
	imake(0x000048c1, utils::index5t<0x0,0x4,0x8,0xc,0x1>);
	imake(0x00045678, utils::index5t<0x4,0x5,0x6,0x7,0x8>);
	imake(0x00026ae1, utils::index5t<0x2,0x6,0xa,0xe,0x1>);
	imake(0x000ba987, utils::index5t<0xb,0xa,0x9,0x8,0x7>);
	imake(0x000d951e, utils::index5t<0xd,0x9,0x5,0x1,0xe>);
	imake(0x0007654b, utils::index5t<0x7,0x6,0x5,0x4,0xb>);
	imake(0x000ea62d, utils::index5t<0xe,0xa,0x6,0x2,0xd>);
	imake(0x00089ab4, utils::index5t<0x8,0x9,0xa,0xb,0x4>);
	imake(0x000159d2, utils::index5t<0x1,0x5,0x9,0xd,0x2>);
	imake(0xff000000, utils::indexmerge0);
	imake(0xff000001, utils::indexmerge1<0>);
	imake(0xff000011, utils::indexmerge1<1>);
	imake(0xfe000000, utils::indexnum0);
	imake(0xfe000001, utils::indexnum1);
	imake(0xfe000002, utils::indexnum2);
	imake(0xfe000082, utils::indexnum2x<0, 0, 1>);
	imake(0xfe000092, utils::indexnum2x<0, 2, 3>);
	imake(0xfe0000c2, utils::indexnum2x<1, 0, 1>);
	imake(0xfe0000d2, utils::indexnum2x<1, 2, 3>);
	imake(0xfd000000, utils::indexmono<0>);
	imake(0xfd000010, utils::indexmono<1>);
	imake(0xfd000020, utils::indexmono<2>);
	imake(0xfd000030, utils::indexmono<3>);
	imake(0xfd000040, utils::indexmono<4>);
	imake(0xfd000050, utils::indexmono<5>);
	imake(0xfd000060, utils::indexmono<6>);
	imake(0xfd000070, utils::indexmono<7>);
	imake(0xfc000000, utils::indexmax<0>);
	imake(0xfc000010, utils::indexmax<1>);
	imake(0xfc000020, utils::indexmax<2>);
	imake(0xfc000030, utils::indexmax<3>);
	imake(0xfc000040, utils::indexmax<4>);
	imake(0xfc000050, utils::indexmax<5>);
	imake(0xfc000060, utils::indexmax<6>);
	imake(0xfc000070, utils::indexmax<7>);

	// patt(012367) num(-)
	std::string in(res);
	while (in.find_first_of(":()[],") != std::string::npos)
		in[in.find_first_of(":()[],")] = ' ';
	if (in.find("default") != std::string::npos)
		in.replace(in.find("default"), 7, "");
	std::stringstream idxin(in);
	std::string type, sign;
	while (idxin >> type && idxin >> sign) {
		u32 idxr = 0;
		using moporgic::to_hash;
		switch (to_hash(type)) {
		case to_hash("p"):
		case to_hash("patt"):
		case to_hash("pattern"):
		case to_hash("tuple"):
			std::stringstream(sign) >> std::hex >> idxr;
			if (indexer::find(idxr) == indexer::end()) {
				auto patt = new std::vector<int>(hashpatt(sign)); // will NOT be deleted
				indexer::make(idxr, std::bind(utils::indexnta, std::placeholders::_1, std::cref(*patt)));
			} else {
				std::cerr << "warning: redefined indexer " << sign << std::endl;
			}
			break;
		case to_hash("n"):
		case to_hash("num"):
		case to_hash("count"):
			std::cerr << "error: unsupported custom indexer type " << type << std::endl;
			break;
		default:
			std::cerr << "error: unknown custom indexer type " << type << std::endl;
			break;
		}
	}
}

bool save_weights(const std::string& path) {
	std::ofstream out;
	char buf[1 << 20];
	out.rdbuf()->pubsetbuf(buf, sizeof(buf));
	out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return false;
	weight::save(out);
	out.flush();
	out.close();
	return true;
}
bool load_weights(const std::string& path) {
	std::ifstream in;
	char buf[1 << 20];
	in.rdbuf()->pubsetbuf(buf, sizeof(buf));
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return false;
	weight::load(in);
	in.close();
	return true;
}
void make_weights(const std::string& res = "") {
	auto wmake = [](u32 sign, u64 size) {
		if (weight::find(sign) == weight::end()) weight::make(sign, size);
	};

	// weight:size weight(size) weight[size]
	std::string in(res);
	while (in.find_first_of(":()[],") != std::string::npos)
		in[in.find_first_of(":()[],")] = ' ';
	if (in.find("default") != std::string::npos) {
		in.replace(in.find("default"), 7, "");

		// make default weights
		std::vector<std::vector<int>> defpatt = {
			{ 0x0, 0x1, 0x2, 0x3, 0x6, 0x7 },
			{ 0x4, 0x5, 0x6, 0x7, 0xa, 0xb },
			{ 0x0, 0x1, 0x2, 0x4, 0x5, 0x6 },
			{ 0x4, 0x5, 0x6, 0x8, 0x9, 0xa },
		};
		for (const auto& patt : defpatt) {
			wmake(utils::hashpatt(patt), std::pow(u64(base), patt.size()));
		}
		wmake(0xfe000001, 1 << 25);
		wmake(0xff000000, 1 << 16);
	}
	std::stringstream wghtin(in);
	std::string signs, sizes;
	while (wghtin >> signs && wghtin >> sizes) {
		u32 sign = 0; u64 size = 0;
		std::stringstream(signs) >> std::hex >> sign;
		std::stringstream(sizes) >> std::dec >> size;
		wmake(sign, size);
	}
}

bool save_features(const std::string& path) {
	std::ofstream out;
	out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return false;
	feature::save(out);
	out.flush();
	out.close();
	return true;
}
bool load_features(const std::string& path) {
	std::ifstream in;
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return false;
	feature::load(in);
	in.close();
	return true;
}
void make_features(const std::string& res = "") {
	auto fmake = [](u32 wght, u32 idxr) {
		if (feature::find(wght, idxr) == feature::end()) feature::make(wght, idxr);
	};

	// weight:indexer weight(indexer) weight[indexer]
	std::string in(res);
	while (in.find_first_of(":()[],") != std::string::npos)
		in[in.find_first_of(":()[],")] = ' ';
	if (in.find("default") != std::string::npos) {
		in.replace(in.find("default"), 7, "");

		// make default features
		std::vector<std::vector<int>> defpatt = {
			{ 0x0, 0x1, 0x2, 0x3, 0x6, 0x7 },
			{ 0x4, 0x5, 0x6, 0x7, 0xa, 0xb },
			{ 0x0, 0x1, 0x2, 0x4, 0x5, 0x6 },
			{ 0x4, 0x5, 0x6, 0x8, 0x9, 0xa },
		};
		for (const auto& patt : defpatt) {
			auto xpatt = patt;
			for (u32 iso = 0; iso < 8; iso++) {
				std::transform(patt.begin(), patt.end(), xpatt.begin(), [=](int i) -> int {
					board x(0xfedcba9876543210);
					x.isomorphic(-iso);
					return x[i];
				});
				fmake(utils::hashpatt(patt), utils::hashpatt(xpatt));
			}
		}
		fmake(0xfe000001, 0xfe000001);
		fmake(0xff000000, 0xff000000);
	}
	std::stringstream featin(in);
	std::string wghts, idxrs;
	while (featin >> wghts && featin >> idxrs) {
		u32 wght = 0, idxr = 0;
		std::stringstream(wghts) >> std::hex >> wght;
		std::stringstream(idxrs) >> std::hex >> idxr;
		if (weight::find(wght) == weight::end()) {
			std::cerr << "warning: undefined weight " << wghts;
			std::cerr << " [assume " << (wghts.size()) << "-tile pattern]" << std::endl;
			weight::make(wght, std::pow(u64(base), wghts.size()));
		}
		if (indexer::find(idxr) == indexer::end()) {
			std::cerr << "warning: undefined indexer " << idxrs;
			std::cerr << " [assume " << (idxrs.size()) << "-tile pattern]" << std::endl;
			auto patt = new std::vector<int>(hashpatt(idxrs)); // will NOT be deleted
			indexer::make(idxr, std::bind(utils::indexnta, std::placeholders::_1, std::cref(*patt)));
		}
		fmake(wght, idxr);
	}
}

inline numeric estimate(const board& state,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	register numeric esti = 0;
	for (auto f = begin; f != end; f++)
		esti += (*f)[state];
	return esti;
}

inline numeric update(const board& state, const numeric& updv,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	register numeric esti = 0;
	for (auto f = begin; f != end; f++)
		esti += ((*f)[state] += updv);
	return esti;
}

inline numeric update(const board& state,
		const numeric& curr, const numeric& accu, const numeric& alpha = moporgic::alpha,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	return update(state, alpha * (accu - curr), begin, end);
}

void list_mapping() {
	for (weight w : weight::list()) {
		u32 usageK = (sizeof(numeric) * w.length()) >> 10;
		u32 usageM = usageK >> 10;
		char buf[64];
		snprintf(buf, sizeof(buf), "weight(%08x)[%llu] = %d%c", w.signature(), w.length(),
				usageM ? usageM : usageK, usageM ? 'M' : 'K');
		std::cout << buf;
		std::string feats;
		for (feature f : feature::list()) {
			if (weight(f) == w) {
				snprintf(buf, sizeof(buf), " %08x", indexer(f).signature());
				feats += buf;
			}
		}
		if (feats.size()) std::cout << " :" << feats;
		std::cout << std::endl;
	}
}

} // utils


struct state {
	board move;
	i32 (board::*oper)();
	i32 score;
	numeric esti;
	state() : state(nullptr) {}
	state(i32 (board::*oper)()) : oper(oper), score(-1), esti(0) {}
	state(const state& s) = default;

	inline void assign(const board& b) {
		move = b;
		score = (move.*oper)();
	}
	inline numeric estimate(const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
		if (score >= 0) {
			esti = score + utils::estimate(move, begin, end);
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
		return esti;
	}
	inline numeric update(const numeric& accu, const numeric& alpha = moporgic::alpha,
			const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
		esti = score + utils::update(move, alpha * (accu - (esti - score)), begin, end);
		return esti;
	}
	inline numeric optimize(const numeric& accu, const numeric& alpha = moporgic::alpha,
			const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
		esti = score + utils::estimate(move, begin, end);
		return update(accu, alpha, begin, end);
	}

	inline void operator <<(const board& b) {
		assign(b);
		estimate();
	}
	inline numeric operator +=(const numeric& v) {
		return update(v);
	}
	inline numeric operator +=(const state& s) {
		return operator +=(s.esti);
	}

	inline void operator >>(board& b) const { b = move; }
	inline bool operator >(const state& s) const { return esti > s.esti; }

	void operator >>(std::ostream& out) const {
		move >> out;
		moporgic::write(out, score);
	}
	void operator <<(std::istream& in) {
		move << in;
		moporgic::read(in, score);
	}
	inline operator bool() const { return score >= 0; }
};
struct select {
	state move[4];
	state *best;
	select() : best(move) {
		move[0] = state(&board::up);
		move[1] = state(&board::right);
		move[2] = state(&board::down);
		move[3] = state(&board::left);
	}
	inline select& operator ()(const board& b) {
		move[0] << b;
		move[1] << b;
		move[2] << b;
		move[3] << b;
		return update();
	}
	inline select& operator ()(const board& b, const feature::iter begin, const feature::iter end) {
		move[0].assign(b);
		move[1].assign(b);
		move[2].assign(b);
		move[3].assign(b);
		move[0].estimate(begin, end);
		move[1].estimate(begin, end);
		move[2].estimate(begin, end);
		move[3].estimate(begin, end);
		return update();
	}
	inline select& update() {
		return update_rand();
		best = move;
		if (move[1] > *best) best = move + 1;
		if (move[2] > *best) best = move + 2;
		if (move[3] > *best) best = move + 3;
		return *this;
	}
	inline select& update_rand() {
		const u32 i = std::rand() % 4;
		best = move + i;
		if (move[(i + 1) % 4] > *best) best = move + ((i + 1) % 4);
		if (move[(i + 2) % 4] > *best) best = move + ((i + 2) % 4);
		if (move[(i + 3) % 4] > *best) best = move + ((i + 3) % 4);
		return *this;
	}
	inline void shuffle() { std::random_shuffle(move, move + 4); }
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(state& s) const { s = (*best); }
	inline void operator >>(board& b) const { *best >> b; }
	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }
};
struct statistic {
	u64 limit;
	u64 loop;
	u64 check;

	struct record {
		u64 score;
		u64 win;
		u64 time;
		u64 opers;
		u32 max;
		u32 hash;
		std::array<u32, 32> count;
	} total, local;

	void init(const u64& max, const u64& chk = 1000) {
		limit = max * chk;
		loop = 1;
		check = chk;

		total = {};
		local = {};
		local.time = moporgic::millisec();
	}
	u64 operator++(int) { return (++loop) - 1; }
	u64 operator++() { return (++loop); }
	operator bool() const { return loop <= limit; }
	bool checked() const { return (loop % check) == 0; }

	void update(const u32& score, const u32& hash, const u32& opers) {
		local.score += score;
		local.hash |= hash;
		local.opers += opers;
		if (hash >= 2048) local.win++;
		local.max = std::max(local.max, score);

		if ((loop % check) != 0) return;

		u64 currtimept = moporgic::millisec();
		u64 elapsedtime = currtimept - local.time;
		total.score += local.score;
		total.win += local.win;
		total.time += elapsedtime;
		total.opers += local.opers;
		total.hash |= local.hash;
		total.max = std::max(total.max, local.max);

		std::cout << std::endl;
		char buf[64];
		snprintf(buf, sizeof(buf), "%03llu/%03llu %llums %.2fops",
				loop / check,
				limit / check,
				elapsedtime,
				local.opers * 1000.0 / elapsedtime);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), "local:  avg=%llu max=%u tile=%u win=%.2f%%",
				local.score / check,
				local.max,
				math::msb32(local.hash),
				local.win * 100.0 / check);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / loop,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / loop);
		std::cout << buf << std::endl;

		local = {};
		local.time = currtimept;
	}

	inline void updatec(const u32& score, const u32& hash, const u32& opers) {
		update(score, hash, opers);
		u32 max = std::log2(hash);
		local.count[max]++;
		total.count[max]++;
		if ((loop % check) != 0) return;
		local.count = {};
	}

	void summary(const u32& begin = 1, const u32& end = 17) {
		std::cout << "max tile summary" << std::endl;

		auto iter = total.count.begin();
		double sum = std::accumulate(iter + begin, iter + end, 0);
		char buf[64];
		for (u32 i = begin; i < end; ++i) {
			snprintf(buf, sizeof(buf), "%d:\t%8d%8.2f%%%8.2f%%",
					(1 << i) & 0xfffffffeu, total.count[i],
					(total.count[i] * 100.0 / sum),
					(std::accumulate(iter + begin, iter + i + 1, 0) * 100.0 / sum));
			std::cout << buf << std::endl;
		}
	}
};

int main(int argc, const char* argv[]) {
	u32 train = 100;
	u32 test = 10;
	u32 timestamp = std::time(nullptr);
	u32 seed = timestamp;
	numeric& alpha = moporgic::alpha;
	utils::options wopts;
	utils::options fopts;
	utils::options opts;


	auto valueof = [&](int& i, const char* def) -> const char* {
		if (i + 1 < argc && *(argv[i + 1]) != '-') return argv[++i];
		if (def != nullptr) return def;
		std::cerr << "invalid: " << argv[i] << std::endl;
		std::exit(1);
	};
	for (int i = 1; i < argc; i++) {
		switch (to_hash(argv[i])) {
		case to_hash("-a"):
		case to_hash("--alpha"):
			alpha = std::stod(valueof(i, nullptr));
			break;
		case to_hash("-a/"):
		case to_hash("--alpha-divide"):
			opts["alpha-divide"] = valueof(i, nullptr);
			alpha /= std::stod(opts["alpha-divide"]);
			break;
		case to_hash("-t"):
		case to_hash("--train"):
			train = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-T"):
		case to_hash("--test"):
			test = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
		case to_hash("--srand"):
			seed = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-wio"):
		case to_hash("--weight-input-output"):
			wopts["input"] = valueof(i, "tdl2048.weight");
			wopts["output"] = wopts["input"];
			break;
		case to_hash("-wi"):
		case to_hash("--weight-input"):
			wopts["input"] = valueof(i, "tdl2048.weight");
			break;
		case to_hash("-wo"):
		case to_hash("--weight-output"):
			wopts["output"] = valueof(i, "tdl2048.weight");
			break;
		case to_hash("-fio"):
		case to_hash("--feature-input-output"):
			fopts["input"] = valueof(i, "tdl2048.feature");
			fopts["output"] = fopts["input"];
			break;
		case to_hash("-fi"):
		case to_hash("--feature-input"):
			fopts["input"] = valueof(i, "tdl2048.feature");
			break;
		case to_hash("-fo"):
		case to_hash("--feature-output"):
			fopts["output"] = valueof(i, "tdl2048.feature");
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
		case to_hash("--weight-value"):
			for (std::string w; (w = valueof(i, "")).size(); )
				wopts["value"] += (w += ',');
			break;
		case to_hash("-f"):
		case to_hash("--feature"):
		case to_hash("--feature-value"):
			for (std::string f; (f = valueof(i, "")).size(); )
				fopts["value"] += (f += ',');
			break;
		case to_hash("-u"):
		case to_hash("--util"):
		case to_hash("--utils"):
			break;
		case to_hash("-o"):
		case to_hash("--option"):
		case to_hash("--options"):
		case to_hash("--extra"):
			for (std::string o; (o = valueof(i, "")).size(); )
				opts += o;
			break;
		case to_hash("-tt"):
		case to_hash("--train-type"):
			opts["train-type"] = valueof(i, "");
			break;
		case to_hash("-Tt"):
		case to_hash("-TT"):
		case to_hash("--test-type"):
			opts["test-type"] = valueof(i, "");
			break;
		case to_hash("-c"):
		case to_hash("--comment"):
			opts["comment"] = valueof(i, "");
			break;
		default:
			std::cerr << "unknown: " << argv[i] << std::endl;
			std::exit(1);
			break;
		}
	}

	std::srand(seed);
	std::cout << "TDL2048+ LOG" << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "timestamp = " << timestamp << std::endl;
	std::cout << "srand = " << seed << std::endl;
	std::cout << "alpha = " << alpha << std::endl;
//	printf("board::look[%d] = %lluM", (1 << 20), ((sizeof(board::cache) * (1 << 20)) >> 20));
	std::cout << std::endl;


	utils::make_indexers();

	if (utils::load_weights(wopts["input"]) == false) {
		if (wopts["input"].size())
			std::cerr << "warning: " << wopts["input"] << " not loaded!" << std::endl;
		if (wopts["value"].empty())
			wopts["value"] = "default";
	}
	utils::make_weights(wopts["value"]);

	if (utils::load_features(fopts["input"]) == false) {
		if (fopts["input"].size())
			std::cerr << "warning: " << fopts["input"] << " not loaded!" << std::endl;
		if (fopts["value"].empty())
			fopts["value"] = "default";
	}
	utils::make_features(fopts["value"]);

	utils::list_mapping();


	board b;
	state last;
	select best;
	statistic stats;
	std::vector<state> path;
	path.reserve(20000);

	if (train) std::cout << std::endl << "start training..." << std::endl;
	switch (to_hash(opts["train-type"])) {

	case to_hash("backward"):
	case to_hash("backward-reesti"):
		for (stats.init(train); stats; stats++) {

			register u32 score = 0;
			register u32 opers = 0;

			for (b.init(); best << b; b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			for (numeric v = 0; path.size(); path.pop_back()) {
				v = path.back().optimize(v);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	default:
	case to_hash("online"):
	case to_hash("forward"):
		for (stats.init(train); stats; stats++) {

			register u32 score = 0;
			register u32 opers = 0;

			b.init();
			best << b;
			score += best.score();
			opers += 1;
			best >> last;
			best >> b;
			b.next();
			while (best << b) {
				last += best.esti();
				score += best.score();
				opers += 1;
				best >> last;
				best >> b;
				b.next();
			}
			last += 0;

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	utils::save_weights(wopts["output"]);
	utils::save_features(fopts["output"]);




	if (test) std::cout << std::endl << "start testing..." << std::endl;
	for (stats.init(test); stats; stats++) {

		register u32 score = 0;
		register u32 opers = 0;

		for (b.init(); best << b; b.next()) {
			score += best.score();
			opers += 1;
			best >> b;
		}

		stats.update(score, b.hash(), opers);
	}

	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
