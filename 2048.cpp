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

typedef float numeric;

class weight {
public:
	inline weight() : id(0), length(0), raw(nullptr) {}
	inline weight(const weight& w) = default;
	inline ~weight() {}

	typedef u64 sign_t;

	inline sign_t sign() const { return id; }
	inline size_t size() const { return length; }
	inline size_t stride() const { return 1ull; }
	inline numeric& operator [](const u64& i) { return raw[i]; }
	inline numeric* data(const u64& i = 0) { return raw + i; }
	inline clip<numeric> value() const { return { raw, raw + length }; }
	declare_comparators(weight, sign());

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		auto& id = w.id;
		auto& length = w.length;
		auto& raw = w.raw;
		u32 code = 4;
		write_cast<u8>(out, code);
		switch (code) {
		default:
			std::cerr << "unknown serial (" << code << ") at ostream << weight, ";
			std::cerr << "use default (4) instead..." << std::endl;
			// no break
		case 4:
			write_cast<u32>(out, id);
			write_cast<u32>(out, 0);
			write_cast<u16>(out, sizeof(numeric));
			write_cast<u64>(out, length);
			write_cast<numeric>(out, raw, raw + length);
			write_cast<u16>(out, 0);
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, weight& w) {
		auto& id = w.id;
		auto& length = w.length;
		auto& raw = w.raw;
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
		case 1:
		case 2:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			if (code == 2)
				read_cast<u16>(in, code);
			else
				code = code == 1 ? 8 : 4;
			raw = alloc(length);
			switch (code) {
			case 4: read_cast<f32>(in, raw, raw + length); break;
			case 8: read_cast<f64>(in, raw, raw + length); break;
			}
			break;
		case 127:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			read_cast<u16>(in, code);
			raw = alloc(length);
			switch (code) {
			case 4: read_cast<f32>(in, raw, raw + length); break;
			case 8: read_cast<f64>(in, raw, raw + length); break;
			}
			in.ignore(code * length * 2);
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at istream >> weight, ";
			std::cerr << "use default (4) instead..." << std::endl;
			// no break
		case 4:
			read_cast<u32>(in, id);
			read_cast<u32>(in, code);
			read_cast<u16>(in, code);
			read_cast<u64>(in, length);
			raw = alloc(length);
			switch (code) {
			case 2: read_cast<f16>(in, raw, raw + length); break;
			case 4: read_cast<f32>(in, raw, raw + length); break;
			case 8: read_cast<f64>(in, raw, raw + length); break;
			}
			while (read_cast<u16>(in, code) && code) {
				u64 skip; read_cast<u64>(in, skip);
				in.ignore(code * skip);
			}
			break;
		}
		return in;
	}

	static u32 save(std::ostream& out) {
		u32 succ = 0;
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		default:
			std::cerr << "unknown serial (" << code << ") at weight::save, ";
			std::cerr << "use default (0) instead..." << std::endl;
			// no break
		case 0:
			write_cast<u32>(out, wghts().size());
			for (weight w : wghts())
				if (out << w) succ++;
			break;
		}
		out.flush();
		if (!out) {
			std::cerr << "output failed at weight::save" << std::endl;
		}
		return succ;
	}
	static u32 load(std::istream& in) {
		u32 succ = 0;
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		default:
			std::cerr << "unknown serial (" << code << ") at weight::load, ";
			std::cerr << "use default (0) instead..." << std::endl;
			// no break
		case 0:
			for (read_cast<u32>(in, code); code; code--) {
				list<weight>::as(wghts()).emplace_back();
				if (in >> wghts().back()) succ++;
			}
			break;
		}
		if (!in) {
			std::cerr << "input failed at weight::load" << std::endl;
			std::exit(-1);
		}
		return succ;
	}

	static inline clip<weight>& wghts() { static clip<weight> w; return w; }

	static inline weight& make(const sign_t& sign, const size_t& size) {
		list<weight>::as(wghts()).push_back(weight(sign, size));
		return wghts().back();
	}
	static inline weight* find(const sign_t& sign, const clip<weight>& range = wghts()) {
		return std::find_if(range.begin(), range.end(), [=](const weight& w) { return w.sign() == sign; });
	}
	static inline weight& at(const sign_t& sign, const clip<weight>& range = wghts()) {
		auto it = find(sign, range);
		if (it != range.end()) return (*it);
		throw std::out_of_range("weight::at");
	}
	static inline weight erase(const sign_t& sign, const bool& del = true) {
		weight w = at(sign);
		if (del) free(w.data());
		list<weight>::as(wghts()).erase(find(sign));
		return w;
	}

private:
	inline weight(const sign_t& sign, const size_t& size) : id(sign), length(size), raw(alloc(size)) {}

	static inline numeric* alloc(const size_t& size) {
		return new numeric[size]();
	}
	static inline void free(numeric* v) {
		delete[] v;
	}

	sign_t id;
	size_t length;
	numeric* raw;
};

class indexer {
public:
	inline indexer() : id(0), map(nullptr) {}
	inline indexer(const indexer& i) = default;
	inline ~indexer() {}

	typedef u64 sign_t;
	typedef std::function<u64(const board&)> mapper;

	inline sign_t sign() const { return id; }
	inline mapper index() const { return map; }
	inline u64 operator ()(const board& b) const { return map(b); }
	declare_comparators(indexer, sign());

	static inline clip<indexer>& idxrs() { static clip<indexer> i; return i; }

	static inline indexer& make(const sign_t& sign, mapper map) {
		list<indexer>::as(idxrs()).push_back(indexer(sign, map));
		return idxrs().back();
	}
	static inline indexer* find(const sign_t& sign, const clip<indexer>& range = idxrs()) {
		return std::find_if(range.begin(), range.end(), [=](const indexer& i) { return i.sign() == sign; });
	}
	static inline indexer& at(const sign_t& sign, const clip<indexer>& range = idxrs()) {
		const auto it = find(sign, range);
		if (it != range.end()) return (*it);
		throw std::out_of_range("indexer::at");
	}
	static inline indexer erase(const sign_t& sign) {
		indexer i = at(sign);
		list<indexer>::as(idxrs()).erase(find(sign));
		return i;
	}

private:
	inline indexer(const sign_t& sign, mapper map) : id(sign), map(map) {}

	sign_t id;
	mapper map;
};

class feature {
public:
	inline feature() {}
	inline feature(const feature& t) = default;
	inline ~feature() {}

	typedef u64 sign_t;

	inline sign_t sign() const { return (value.sign() << 32) | index.sign(); }
	inline numeric& operator [](const board& b) { return value[index(b)]; }
	inline numeric& operator [](const u64& idx) { return value[idx]; }
	inline u64 operator ()(const board& b) const { return index(b); }

	inline operator indexer() const { return index; }
	inline operator weight() const { return value; }
	declare_comparators(feature, sign());

	friend std::ostream& operator <<(std::ostream& out, const feature& f) {
		auto& index = f.index;
		auto& value = f.value;
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		case 0:
			write_cast<u32>(out, index.sign());
			write_cast<u32>(out, value.sign());
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at ostream << feature" << std::endl;
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, feature& f) {
		auto& index = f.index;
		auto& value = f.value;
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
			read_cast<u32>(in, code);
			index = indexer::at(code);
			read_cast<u32>(in, code);
			value = weight::at(code);
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at istream >> feature" << std::endl;
			break;
		}
		return in;
	}

	static u32 save(std::ostream& out) {
		u32 succ = 0;
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		case 0:
			write_cast<u32>(out, feats().size());
			for (feature f : feature::feats())
				out << f, succ++;
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::save" << std::endl;
			break;
		}
		out.flush();
		if (!out) {
			std::cerr << "output failed at feature::save" << std::endl;
		}
		return succ;
	}
	static u32 load(std::istream& in) {
		u32 succ = 0;
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
			for (read_cast<u32>(in, code); code; code--) {
				list<feature>::as(feats()).emplace_back();
				if (in >> feats().back()) succ++;
			}
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::load" << std::endl;
			break;
		}
		if (!in) {
			std::cerr << "input failed at feature::load" << std::endl;
			std::exit(-1);
		}
		return succ;
	}

	static inline clip<feature>& feats() { static clip<feature> f; return f; }

	static inline feature& make(const sign_t& wgt, const sign_t& idx) {
		list<feature>::as(feats()).push_back(feature(weight::at(wgt), indexer::at(idx)));
		return feats().back();
	}
	static inline feature* find(const sign_t& wght, const sign_t& idxr, const clip<feature>& range = feats()) {
		return std::find_if(range.begin(), range.end(),
			[=](const feature& f) { return weight(f).sign() == wght && indexer(f).sign() == idxr; });
	}
	static inline feature& at(const sign_t& wgt, const sign_t& idx, const clip<feature>& range = feats()) {
		const auto it = find(wgt, idx, range);
		if (it != range.end()) return (*it);
		throw std::out_of_range("feature::at");
	}
	static inline feature erase(const sign_t& wgt, const sign_t& idx) {
		feature f = at(wgt, idx);
		list<feature>::as(feats()).erase(find(wgt, idx));
		return f;
	}

private:
	inline feature(const weight& value, const indexer& index) : index(index), value(value) {}

	indexer index;
	weight value;
};

namespace utils {

class options {
public:
	options() {}
	options(const options& opts) : opts(opts.opts) {}
	using vector = std::vector<std::string>;

	class opinion {
		friend class option;
	public:
		opinion() = delete;
		opinion(const opinion& i) = default;
		opinion(std::string& token) : token(token) {}
		std::string label() const { return token.substr(0, token.find('=')); }
		std::string value() const { return token.substr(token.find('=') + 1); }
		operator std::string() const { return value(); }
		friend std::ostream& operator <<(std::ostream& out, const opinion& i) { return out << i.value(); }
		opinion& operator =(const opinion& opt) { token  = opt.token; return (*this); }
		opinion& operator =(const numeric& val) { return operator =(ntos(val)); }
		opinion& operator =(const std::string& val) { token = label() + (val.size() ? ("=" + val) : ""); return (*this); }
		opinion& operator =(const vector& vec) { return operator  =(vtos(vec)); }
		bool operator ==(const std::string& val) const { return value() == val; }
		bool operator !=(const std::string& val) const { return value() != val; }
		bool operator ()(const std::string& val) const { return value().find(val) != std::string::npos; }
		static bool comp(std::string token, std::string label) { return opinion(token).label() == label; }
	private:
		std::string& token;
	};

	class option : public vector {
		friend class options;
	public:
		option(const vector& opt = {}) : vector(opt) {}
		std::string value() const { return vtos(*this); }
		operator std::string() const { return value(); }
		friend std::ostream& operator <<(std::ostream& out, const option& opt) { return out << opt.value(); }
		option& operator  =(const numeric& val) { return operator =(ntos(val)); }
		option& operator  =(const std::string& val) { clear(); return operator +=(val); }
		option& operator +=(const std::string& val) { push_back(val); return *this; }
		option& operator  =(const vector& vec) { clear(); return operator +=(vec); }
		option& operator +=(const vector& vec) { insert(end(), vec.begin(), vec.end()); return *this; }
		bool operator ==(const std::string& val) const { return value() == val; }
		bool operator !=(const std::string& val) const { return value() != val; }
		bool operator ()(const std::string& ext) const {
			return std::find_if(cbegin(), cend(), std::bind(opinion::comp, std::placeholders::_1, ext)) != cend();
		}
		bool operator ()(const std::string& ext, const std::string& val) const {
			return operator ()(ext) && const_cast<option&>(*this)[ext](val);
		}
		opinion operator [](const std::string& ext) {
			auto pos = std::find_if(begin(), end(), std::bind(opinion::comp, std::placeholders::_1, ext));
			return (pos != end()) ? opinion(*pos) : operator +=(ext)[ext];
		}
		std::string find(const std::string& ext, const std::string& val = {}) const {
			return operator() (ext) ? const_cast<option&>(*this)[ext] : val;
		}
	};

	bool operator ()(const std::string& opt) const {
		return opts.find(opt) != opts.end();
	}
	bool operator ()(const std::string& opt, const std::string& ext) const {
		return operator ()(opt) && const_cast<options&>(*this)[opt](ext);
	}
	bool operator ()(const std::string& opt, const std::string& ext, const std::string& val) const {
		return operator ()(opt, ext) && const_cast<options&>(*this)[opt][ext](val);
	}
	option& operator [](const std::string& opt) {
		if (opts.find(opt) == opts.end()) opts[opt] = option();
		return opts[opt];
	}
	std::string find(const std::string& opt, const std::string& val = {}) const {
		return operator()(opt) ? const_cast<options&>(*this)[opt] : val;
	}

private:
	std::map<std::string, option> opts;

	static std::string vtos(const vector& vec) {
		std::string str = std::accumulate(vec.cbegin(), vec.cend(), std::string(),
		    [](std::string& r, const std::string& v){ return std::move(r) + v + " "; });
		if (str.size()) str.pop_back();
		return str;
	}
	static std::string ntos(const numeric& num) {
		std::string val = std::to_string(num);
		if (val.find('.') != std::string::npos) {
			while (val.back() == '0') val.pop_back();
			if (val.back() == '.') val.pop_back();
		}
		return val;
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
inline std::string hashpatt(const u32& hash, const size_t& n = 0) {
	std::stringstream ss; ss << std::hex << hash;
	std::string patt = ss.str();
	return std::string(std::max(n, patt.size()) - patt.size(), '0') + patt;
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
	auto num = b.numof();
	register u64 index = 0;
	index += (num[11] & 0x03) << 0;
	index += (num[12] & 0x03) << 2;
	index += (num[13] & 0x03) << 4;
	index += (num[14] & 0x03) << 6;
	index += (num[15] & 0x03) << 8;
	return index;
}

u64 indexnum1(const board& b) { // 25-bit
	auto num = b.numof();
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
	auto num = b.numof();
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
	auto& m = o.query(qu0).numof;
	auto& n = o.query(qu1).numof;

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

u64 indexnum3(const board& b) { // 28-bit
	auto num = b.numof();
	register u64 index = 0;
	index += ((num[0] + num[1] + num[2]) & 0x0f) << 0; // 0 & 2 & 4, 4-bit
	index += ((num[3] + num[4]) & 0x07) << 4; // 8 & 16, 3-bit
	index += ((num[5] + num[6]) & 0x07) << 7; // 32 & 64, 3-bit
	index += (num[7] & 0x03) << 10; // 128, 2-bit
	index += (num[8] & 0x03) << 12; // 256, 2-bit
	index += (num[9] & 0x03) << 14; // 512, 2-bit
	index += (num[10] & 0x03) << 16; // 1k ~ 32k, 2-bit ea.
	index += (num[11] & 0x03) << 18;
	index += (num[12] & 0x03) << 20;
	index += (num[13] & 0x03) << 22;
	index += (num[14] & 0x03) << 24;
	index += (num[15] & 0x03) << 26;
	return index;
}

u64 indexnum4(const board& b) { // 24-bit
	auto num = b.numof();
	register u64 index = 0;
	index |= (num[0] + num[1] + num[2] + num[3]) << 0; // 0+2+4+8, 4-bit
	index |= (num[4] + num[5] + num[6]) << 4; // 16+32+64, 4-bit
	index |= (num[7] + num[8]) << 8; // 128+256, 4-bit
	index |= std::min(num[9] + num[10], 7u) << 12; // 512+1024, 3-bit
	index |= std::min(num[11], 3u) << 15; // 2048~16384, 2-bit ea.
	index |= std::min(num[12], 3u) << 17;
	index |= std::min(num[13], 3u) << 19;
	index |= std::min(num[14], 3u) << 21;
	index |= std::min(num[15], 1u) << 23; // 32768, 1-bit
	return index;
}

u64 indexnum5lt(const board& b) { // 24-bit
	auto num = b.numof();
	register u64 index = 0;
	index |= std::min(num[8],  7u) <<  0; // 256, 3-bit
	index |= std::min(num[9],  7u) <<  3; // 512, 3-bit
	index |= std::min(num[10], 7u) <<  6; // 1024, 3-bit
	index |= std::min(num[11], 7u) <<  9; // 2048, 3-bit
	index |= std::min(num[12], 7u) << 12; // 4096, 3-bit
	index |= std::min(num[13], 7u) << 15; // 8192, 3-bit
	index |= std::min(num[14], 7u) << 18; // 16384, 3-bit
	index |= std::min(num[15], 7u) << 21; // 32768, 3-bit
	return index;
}

u64 indexnum5st(const board& b) { // 24-bit
	auto num = b.numof();
	register u64 index = 0;
	index |= std::min(num[0], 7u) <<  0; // 0, 3-bit
	index |= std::min(num[1], 7u) <<  3; // 2, 3-bit
	index |= std::min(num[2], 7u) <<  6; // 4, 3-bit
	index |= std::min(num[3], 7u) <<  9; // 8, 3-bit
	index |= std::min(num[4], 7u) << 12; // 16, 3-bit
	index |= std::min(num[5], 7u) << 15; // 32, 3-bit
	index |= std::min(num[6], 7u) << 18; // 64, 3-bit
	index |= std::min(num[7], 7u) << 21; // 128, 3-bit
	return index;
}

u64 indexnuma(const board& b, const std::vector<int>& n) {
	auto num = b.numof();
	register u64 index = 0;
	register u32 offset = 0;
	for (const int& code : n) {
		using moporgic::math::msb32;
		using moporgic::math::log2;
		// code: 0x00SSTTTT
		u32 size = (code >> 16);
		u32 tile = (code & 0xffff);
		u32 idx = log2(tile);
		u32 var = num[idx];
		while ((tile &= ~(1 << idx)) != 0) {
			idx = log2(tile);
			var += num[idx];
		}
		index += (var & ~(-1 << size)) << offset;
		offset += size;
	}
	return index;
}

template<int p0, int p1, int p2, int p3, int p4, int p5, int p6, int p7>
u64 indexmono(const board& b) { // 24-bit
	u32 h0 = (b.at(p0)) | (b.at(p1) << 4) | (b.at(p2) << 8) | (b.at(p3) << 12);
	u32 h1 = (b.at(p4)) | (b.at(p5) << 4) | (b.at(p6) << 8) | (b.at(p7) << 12);
	return (board::lookup[h0].left.mono) | (board::lookup[h1].left.mono << 12);
}

template<u32 tile, int isomorphic>
u64 indexmask(const board& b) { // 16-bit
	board k = b;
	k.isomorphic(isomorphic);
	return k.mask(tile);
}

template<int isomorphic>
u64 indexmax(const board& b) { // 16-bit
	board k = b;
	k.isomorphic(isomorphic);
	return k.mask(k.max());
}

u32 make_indexers(std::string res = "") {
	u32 succ = 0;
	auto make = [&](indexer::sign_t sign, indexer::mapper func) {
		if (indexer::find(sign) != indexer::idxrs().end()) return;
		indexer::make(sign, func); succ++;
	};

	make(0x00012367, utils::index6t<0x0,0x1,0x2,0x3,0x6,0x7>);
	make(0x0037bfae, utils::index6t<0x3,0x7,0xb,0xf,0xa,0xe>);
	make(0x00fedc98, utils::index6t<0xf,0xe,0xd,0xc,0x9,0x8>);
	make(0x00c84051, utils::index6t<0xc,0x8,0x4,0x0,0x5,0x1>);
	make(0x00fb7362, utils::index6t<0xf,0xb,0x7,0x3,0x6,0x2>);
	make(0x00cdefab, utils::index6t<0xc,0xd,0xe,0xf,0xa,0xb>);
	make(0x00048c9d, utils::index6t<0x0,0x4,0x8,0xc,0x9,0xd>);
	make(0x00321054, utils::index6t<0x3,0x2,0x1,0x0,0x5,0x4>);
	make(0x004567ab, utils::index6t<0x4,0x5,0x6,0x7,0xa,0xb>);
	make(0x0026ae9d, utils::index6t<0x2,0x6,0xa,0xe,0x9,0xd>);
	make(0x00ba9854, utils::index6t<0xb,0xa,0x9,0x8,0x5,0x4>);
	make(0x00d95162, utils::index6t<0xd,0x9,0x5,0x1,0x6,0x2>);
	make(0x00ea6251, utils::index6t<0xe,0xa,0x6,0x2,0x5,0x1>);
	make(0x0089ab67, utils::index6t<0x8,0x9,0xa,0xb,0x6,0x7>);
	make(0x00159dae, utils::index6t<0x1,0x5,0x9,0xd,0xa,0xe>);
	make(0x00765498, utils::index6t<0x7,0x6,0x5,0x4,0x9,0x8>);
	make(0x00012345, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	make(0x0037bf26, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	make(0x00fedcba, utils::index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	make(0x00c840d9, utils::index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	make(0x00321076, utils::index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	make(0x00fb73ea, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	make(0x00cdef89, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	make(0x00048c15, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	make(0x00456789, utils::index6t<0x4,0x5,0x6,0x7,0x8,0x9>);
	make(0x0026ae15, utils::index6t<0x2,0x6,0xa,0xe,0x1,0x5>);
	make(0x00ba9876, utils::index6t<0xb,0xa,0x9,0x8,0x7,0x6>);
	make(0x00d951ea, utils::index6t<0xd,0x9,0x5,0x1,0xe,0xa>);
	make(0x007654ba, utils::index6t<0x7,0x6,0x5,0x4,0xb,0xa>);
	make(0x00ea62d9, utils::index6t<0xe,0xa,0x6,0x2,0xd,0x9>);
	make(0x0089ab45, utils::index6t<0x8,0x9,0xa,0xb,0x4,0x5>);
	make(0x00159d26, utils::index6t<0x1,0x5,0x9,0xd,0x2,0x6>);
	make(0x00012456, utils::index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	make(0x0037b26a, utils::index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	make(0x00fedba9, utils::index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	make(0x00c84d95, utils::index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	make(0x00fb7ea6, utils::index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	make(0x00cde89a, utils::index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	make(0x00048159, utils::index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	make(0x00321765, utils::index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	make(0x0045689a, utils::index6t<0x4,0x5,0x6,0x8,0x9,0xa>);
	make(0x0026a159, utils::index6t<0x2,0x6,0xa,0x1,0x5,0x9>);
	make(0x00ba9765, utils::index6t<0xb,0xa,0x9,0x7,0x6,0x5>);
	make(0x00d95ea6, utils::index6t<0xd,0x9,0x5,0xe,0xa,0x6>);
	make(0x00ea6d95, utils::index6t<0xe,0xa,0x6,0xd,0x9,0x5>);
	make(0x0089a456, utils::index6t<0x8,0x9,0xa,0x4,0x5,0x6>);
	make(0x0015926a, utils::index6t<0x1,0x5,0x9,0x2,0x6,0xa>);
	make(0x00765ba9, utils::index6t<0x7,0x6,0x5,0xb,0xa,0x9>);
	make(0x00123567, utils::index6t<0x1,0x2,0x3,0x5,0x6,0x7>);
	make(0x007bf6ae, utils::index6t<0x7,0xb,0xf,0x6,0xa,0xe>);
	make(0x00edca98, utils::index6t<0xe,0xd,0xc,0xa,0x9,0x8>);
	make(0x00840951, utils::index6t<0x8,0x4,0x0,0x9,0x5,0x1>);
	make(0x00210654, utils::index6t<0x2,0x1,0x0,0x6,0x5,0x4>);
	make(0x00b73a62, utils::index6t<0xb,0x7,0x3,0xa,0x6,0x2>);
	make(0x00def9ab, utils::index6t<0xd,0xe,0xf,0x9,0xa,0xb>);
	make(0x0048c59d, utils::index6t<0x4,0x8,0xc,0x5,0x9,0xd>);
	make(0x005679ab, utils::index6t<0x5,0x6,0x7,0x9,0xa,0xb>);
	make(0x006ae59d, utils::index6t<0x6,0xa,0xe,0x5,0x9,0xd>);
	make(0x00a98654, utils::index6t<0xa,0x9,0x8,0x6,0x5,0x4>);
	make(0x00951a62, utils::index6t<0x9,0x5,0x1,0xa,0x6,0x2>);
	make(0x00654a98, utils::index6t<0x6,0x5,0x4,0xa,0x9,0x8>);
	make(0x00a62951, utils::index6t<0xa,0x6,0x2,0x9,0x5,0x1>);
	make(0x009ab567, utils::index6t<0x9,0xa,0xb,0x5,0x6,0x7>);
	make(0x0059d6ae, utils::index6t<0x5,0x9,0xd,0x6,0xa,0xe>);
	make(0x0001237b, utils::index6t<0x0,0x1,0x2,0x3,0x7,0xb>);
	make(0x0037bfed, utils::index6t<0x3,0x7,0xb,0xf,0xe,0xd>);
	make(0x00fedc84, utils::index6t<0xf,0xe,0xd,0xc,0x8,0x4>);
	make(0x00c84012, utils::index6t<0xc,0x8,0x4,0x0,0x1,0x2>);
	make(0x00321048, utils::index6t<0x3,0x2,0x1,0x0,0x4,0x8>);
	make(0x00fb7321, utils::index6t<0xf,0xb,0x7,0x3,0x2,0x1>);
	make(0x00cdefb7, utils::index6t<0xc,0xd,0xe,0xf,0xb,0x7>);
	make(0x00048cde, utils::index6t<0x0,0x4,0x8,0xc,0xd,0xe>);
	make(0x00012348, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x8>);
	make(0x0037bf21, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x1>);
	make(0x00fedcb7, utils::index6t<0xf,0xe,0xd,0xc,0xb,0x7>);
	make(0x00c840de, utils::index6t<0xc,0x8,0x4,0x0,0xd,0xe>);
	make(0x0032107b, utils::index6t<0x3,0x2,0x1,0x0,0x7,0xb>);
	make(0x00fb73ed, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xd>);
	make(0x00cdef84, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x4>);
	make(0x00048c12, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x2>);
	make(0x0089abcd, utils::index6t<0x8,0x9,0xa,0xb,0xc,0xd>);
	make(0x00159d04, utils::index6t<0x1,0x5,0x9,0xd,0x0,0x4>);
	make(0x00765432, utils::index6t<0x7,0x6,0x5,0x4,0x3,0x2>);
	make(0x00ea62fb, utils::index6t<0xe,0xa,0x6,0x2,0xf,0xb>);
	make(0x00ba98fe, utils::index6t<0xb,0xa,0x9,0x8,0xf,0xe>);
	make(0x00d951c8, utils::index6t<0xd,0x9,0x5,0x1,0xc,0x8>);
	make(0x00456701, utils::index6t<0x4,0x5,0x6,0x7,0x0,0x1>);
	make(0x0026ae37, utils::index6t<0x2,0x6,0xa,0xe,0x3,0x7>);

	// k.matsuzaki
	make(0x00012456, utils::index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	make(0x00c84d95, utils::index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	make(0x00fedba9, utils::index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	make(0x0037b26a, utils::index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	make(0x00321765, utils::index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	make(0x00fb7ea6, utils::index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	make(0x00cde89a, utils::index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	make(0x00048159, utils::index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	make(0x0012569d, utils::index6t<0x1,0x2,0x5,0x6,0x9,0xd>);
	make(0x008495ab, utils::index6t<0x8,0x4,0x9,0x5,0xa,0xb>);
	make(0x00eda962, utils::index6t<0xe,0xd,0xa,0x9,0x6,0x2>);
	make(0x007b6a54, utils::index6t<0x7,0xb,0x6,0xa,0x5,0x4>);
	make(0x002165ae, utils::index6t<0x2,0x1,0x6,0x5,0xa,0xe>);
	make(0x00b7a698, utils::index6t<0xb,0x7,0xa,0x6,0x9,0x8>);
	make(0x00de9a51, utils::index6t<0xd,0xe,0x9,0xa,0x5,0x1>);
	make(0x00485967, utils::index6t<0x4,0x8,0x5,0x9,0x6,0x7>);
	make(0x00012345, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	make(0x00c840d9, utils::index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	make(0x00fedcba, utils::index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	make(0x0037bf26, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	make(0x00321076, utils::index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	make(0x00fb73ea, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	make(0x00cdef89, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	make(0x00048c15, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	make(0x0001567a, utils::index6t<0x0,0x1,0x5,0x6,0x7,0xa>);
	make(0x00c89516, utils::index6t<0xc,0x8,0x9,0x5,0x1,0x6>);
	make(0x00fea985, utils::index6t<0xf,0xe,0xa,0x9,0x8,0x5>);
	make(0x00376ae9, utils::index6t<0x3,0x7,0x6,0xa,0xe,0x9>);
	make(0x00326549, utils::index6t<0x3,0x2,0x6,0x5,0x4,0x9>);
	make(0x00fba625, utils::index6t<0xf,0xb,0xa,0x6,0x2,0x5>);
	make(0x00cd9ab6, utils::index6t<0xc,0xd,0x9,0xa,0xb,0x6>);
	make(0x000459da, utils::index6t<0x0,0x4,0x5,0x9,0xd,0xa>);
	make(0x0001259a, utils::index6t<0x0,0x1,0x2,0x5,0x9,0xa>);
	make(0x00c849a6, utils::index6t<0xc,0x8,0x4,0x9,0xa,0x6>);
	make(0x00feda65, utils::index6t<0xf,0xe,0xd,0xa,0x6,0x5>);
	make(0x0037b659, utils::index6t<0x3,0x7,0xb,0x6,0x5,0x9>);
	make(0x003216a9, utils::index6t<0x3,0x2,0x1,0x6,0xa,0x9>);
	make(0x00fb7a95, utils::index6t<0xf,0xb,0x7,0xa,0x9,0x5>);
	make(0x00cde956, utils::index6t<0xc,0xd,0xe,0x9,0x5,0x6>);
	make(0x0004856a, utils::index6t<0x0,0x4,0x8,0x5,0x6,0xa>);
	make(0x000159de, utils::index6t<0x0,0x1,0x5,0x9,0xd,0xe>);
	make(0x00c89ab7, utils::index6t<0xc,0x8,0x9,0xa,0xb,0x7>);
	make(0x00fea621, utils::index6t<0xf,0xe,0xa,0x6,0x2,0x1>);
	make(0x00376548, utils::index6t<0x3,0x7,0x6,0x5,0x4,0x8>);
	make(0x00326aed, utils::index6t<0x3,0x2,0x6,0xa,0xe,0xd>);
	make(0x00fba984, utils::index6t<0xf,0xb,0xa,0x9,0x8,0x4>);
	make(0x00cd9512, utils::index6t<0xc,0xd,0x9,0x5,0x1,0x2>);
	make(0x0004567b, utils::index6t<0x0,0x4,0x5,0x6,0x7,0xb>);
	make(0x0001589d, utils::index6t<0x0,0x1,0x5,0x8,0x9,0xd>);
	make(0x00c89eab, utils::index6t<0xc,0x8,0x9,0xe,0xa,0xb>);
	make(0x00fea762, utils::index6t<0xf,0xe,0xa,0x7,0x6,0x2>);
	make(0x00376154, utils::index6t<0x3,0x7,0x6,0x1,0x5,0x4>);
	make(0x00326bae, utils::index6t<0x3,0x2,0x6,0xb,0xa,0xe>);
	make(0x00fbad98, utils::index6t<0xf,0xb,0xa,0xd,0x9,0x8>);
	make(0x00cd9451, utils::index6t<0xc,0xd,0x9,0x4,0x5,0x1>);
	make(0x00045267, utils::index6t<0x0,0x4,0x5,0x2,0x6,0x7>);
	make(0x0001246a, utils::index6t<0x0,0x1,0x2,0x4,0x6,0xa>);
	make(0x00c84d56, utils::index6t<0xc,0x8,0x4,0xd,0x5,0x6>);
	make(0x00fedb95, utils::index6t<0xf,0xe,0xd,0xb,0x9,0x5>);
	make(0x0037b2a9, utils::index6t<0x3,0x7,0xb,0x2,0xa,0x9>);
	make(0x00321759, utils::index6t<0x3,0x2,0x1,0x7,0x5,0x9>);
	make(0x00fb7e65, utils::index6t<0xf,0xb,0x7,0xe,0x6,0x5>);
	make(0x00cde8a6, utils::index6t<0xc,0xd,0xe,0x8,0xa,0x6>);
	make(0x0004819a, utils::index6t<0x0,0x4,0x8,0x1,0x9,0xa>);
	make(0x00456789, utils::index6t<0x4,0x5,0x6,0x7,0x8,0x9>);
	make(0x00d951ea, utils::index6t<0xd,0x9,0x5,0x1,0xe,0xa>);
	make(0x00ba9876, utils::index6t<0xb,0xa,0x9,0x8,0x7,0x6>);
	make(0x0026ae15, utils::index6t<0x2,0x6,0xa,0xe,0x1,0x5>);
	make(0x007654ba, utils::index6t<0x7,0x6,0x5,0x4,0xb,0xa>);
	make(0x00ea62d9, utils::index6t<0xe,0xa,0x6,0x2,0xd,0x9>);
	make(0x0089ab45, utils::index6t<0x8,0x9,0xa,0xb,0x4,0x5>);
	make(0x00159d26, utils::index6t<0x1,0x5,0x9,0xd,0x2,0x6>);
	make(0x00234569, utils::index6t<0x2,0x3,0x4,0x5,0x6,0x9>);
	make(0x0040d95a, utils::index6t<0x4,0x0,0xd,0x9,0x5,0xa>);
	make(0x00dcba96, utils::index6t<0xd,0xc,0xb,0xa,0x9,0x6>);
	make(0x00bf26a5, utils::index6t<0xb,0xf,0x2,0x6,0xa,0x5>);
	make(0x0010765a, utils::index6t<0x1,0x0,0x7,0x6,0x5,0xa>);
	make(0x0073ea69, utils::index6t<0x7,0x3,0xe,0xa,0x6,0x9>);
	make(0x00ef89a5, utils::index6t<0xe,0xf,0x8,0x9,0xa,0x5>);
	make(0x008c1596, utils::index6t<0x8,0xc,0x1,0x5,0x9,0x6>);
	make(0x00345678, utils::index6t<0x3,0x4,0x5,0x6,0x7,0x8>);
	make(0x000d951e, utils::index6t<0x0,0xd,0x9,0x5,0x1,0xe>);
	make(0x00cba987, utils::index6t<0xc,0xb,0xa,0x9,0x8,0x7>);
	make(0x00f26ae1, utils::index6t<0xf,0x2,0x6,0xa,0xe,0x1>);
	make(0x0007654b, utils::index6t<0x0,0x7,0x6,0x5,0x4,0xb>);
	make(0x003ea62d, utils::index6t<0x3,0xe,0xa,0x6,0x2,0xd>);
	make(0x00f89ab4, utils::index6t<0xf,0x8,0x9,0xa,0xb,0x4>);
	make(0x00c159d2, utils::index6t<0xc,0x1,0x5,0x9,0xd,0x2>);
	make(0x00134567, utils::index6t<0x1,0x3,0x4,0x5,0x6,0x7>);
	make(0x0080d951, utils::index6t<0x8,0x0,0xd,0x9,0x5,0x1>);
	make(0x00ecba98, utils::index6t<0xe,0xc,0xb,0xa,0x9,0x8>);
	make(0x007f26ae, utils::index6t<0x7,0xf,0x2,0x6,0xa,0xe>);
	make(0x00207654, utils::index6t<0x2,0x0,0x7,0x6,0x5,0x4>);
	make(0x00b3ea62, utils::index6t<0xb,0x3,0xe,0xa,0x6,0x2>);
	make(0x00df89ab, utils::index6t<0xd,0xf,0x8,0x9,0xa,0xb>);
	make(0x004c159d, utils::index6t<0x4,0xc,0x1,0x5,0x9,0xd>);
	make(0x0001489a, utils::index6t<0x0,0x1,0x4,0x8,0x9,0xa>);
	make(0x00c8dea6, utils::index6t<0xc,0x8,0xd,0xe,0xa,0x6>);
	make(0x00feb765, utils::index6t<0xf,0xe,0xb,0x7,0x6,0x5>);
	make(0x00372159, utils::index6t<0x3,0x7,0x2,0x1,0x5,0x9>);
	make(0x00327ba9, utils::index6t<0x3,0x2,0x7,0xb,0xa,0x9>);
	make(0x00fbed95, utils::index6t<0xf,0xb,0xe,0xd,0x9,0x5>);
	make(0x00cd8456, utils::index6t<0xc,0xd,0x8,0x4,0x5,0x6>);
	make(0x0004126a, utils::index6t<0x0,0x4,0x1,0x2,0x6,0xa>);

	make(0x00000123, utils::index4t<0x0,0x1,0x2,0x3>);
	make(0x00004567, utils::index4t<0x4,0x5,0x6,0x7>);
	make(0x000089ab, utils::index4t<0x8,0x9,0xa,0xb>);
	make(0x0000cdef, utils::index4t<0xc,0xd,0xe,0xf>);
	make(0x000037bf, utils::index4t<0x3,0x7,0xb,0xf>);
	make(0x000026ae, utils::index4t<0x2,0x6,0xa,0xe>);
	make(0x0000159d, utils::index4t<0x1,0x5,0x9,0xd>);
	make(0x0000048c, utils::index4t<0x0,0x4,0x8,0xc>);
	make(0x0000fedc, utils::index4t<0xf,0xe,0xd,0xc>);
	make(0x0000ba98, utils::index4t<0xb,0xa,0x9,0x8>);
	make(0x00007654, utils::index4t<0x7,0x6,0x5,0x4>);
	make(0x00003210, utils::index4t<0x3,0x2,0x1,0x0>);
	make(0x0000c840, utils::index4t<0xc,0x8,0x4,0x0>);
	make(0x0000d951, utils::index4t<0xd,0x9,0x5,0x1>);
	make(0x0000ea62, utils::index4t<0xe,0xa,0x6,0x2>);
	make(0x0000fb73, utils::index4t<0xf,0xb,0x7,0x3>);
	make(0x00000145, utils::index4t<0x0,0x1,0x4,0x5>);
	make(0x00001256, utils::index4t<0x1,0x2,0x5,0x6>);
	make(0x00002367, utils::index4t<0x2,0x3,0x6,0x7>);
	make(0x00004589, utils::index4t<0x4,0x5,0x8,0x9>);
	make(0x0000569a, utils::index4t<0x5,0x6,0x9,0xa>);
	make(0x000067ab, utils::index4t<0x6,0x7,0xa,0xb>);
	make(0x000089cd, utils::index4t<0x8,0x9,0xc,0xd>);
	make(0x00009ade, utils::index4t<0x9,0xa,0xd,0xe>);
	make(0x0000abef, utils::index4t<0xa,0xb,0xe,0xf>);
	make(0x00003726, utils::index4t<0x3,0x7,0x2,0x6>);
	make(0x00007b6a, utils::index4t<0x7,0xb,0x6,0xa>);
	make(0x0000bfae, utils::index4t<0xb,0xf,0xa,0xe>);
	make(0x00002615, utils::index4t<0x2,0x6,0x1,0x5>);
	make(0x00006a59, utils::index4t<0x6,0xa,0x5,0x9>);
	make(0x0000ae9d, utils::index4t<0xa,0xe,0x9,0xd>);
	make(0x00001504, utils::index4t<0x1,0x5,0x0,0x4>);
	make(0x00005948, utils::index4t<0x5,0x9,0x4,0x8>);
	make(0x00009d8c, utils::index4t<0x9,0xd,0x8,0xc>);
	make(0x0000feba, utils::index4t<0xf,0xe,0xb,0xa>);
	make(0x0000eda9, utils::index4t<0xe,0xd,0xa,0x9>);
	make(0x0000dc98, utils::index4t<0xd,0xc,0x9,0x8>);
	make(0x0000ba76, utils::index4t<0xb,0xa,0x7,0x6>);
	make(0x0000a965, utils::index4t<0xa,0x9,0x6,0x5>);
	make(0x00009854, utils::index4t<0x9,0x8,0x5,0x4>);
	make(0x00007632, utils::index4t<0x7,0x6,0x3,0x2>);
	make(0x00006521, utils::index4t<0x6,0x5,0x2,0x1>);
	make(0x00005410, utils::index4t<0x5,0x4,0x1,0x0>);
	make(0x0000c8d9, utils::index4t<0xc,0x8,0xd,0x9>);
	make(0x00008495, utils::index4t<0x8,0x4,0x9,0x5>);
	make(0x00004051, utils::index4t<0x4,0x0,0x5,0x1>);
	make(0x0000d9ea, utils::index4t<0xd,0x9,0xe,0xa>);
	make(0x000095a6, utils::index4t<0x9,0x5,0xa,0x6>);
	make(0x00005162, utils::index4t<0x5,0x1,0x6,0x2>);
	make(0x0000eafb, utils::index4t<0xe,0xa,0xf,0xb>);
	make(0x0000a6b7, utils::index4t<0xa,0x6,0xb,0x7>);
	make(0x00006273, utils::index4t<0x6,0x2,0x7,0x3>);
	make(0x00003276, utils::index4t<0x3,0x2,0x7,0x6>);
	make(0x00002165, utils::index4t<0x2,0x1,0x6,0x5>);
	make(0x00001054, utils::index4t<0x1,0x0,0x5,0x4>);
	make(0x000076ba, utils::index4t<0x7,0x6,0xb,0xa>);
	make(0x000065a9, utils::index4t<0x6,0x5,0xa,0x9>);
	make(0x00005498, utils::index4t<0x5,0x4,0x9,0x8>);
	make(0x0000bafe, utils::index4t<0xb,0xa,0xf,0xe>);
	make(0x0000a9ed, utils::index4t<0xa,0x9,0xe,0xd>);
	make(0x000098dc, utils::index4t<0x9,0x8,0xd,0xc>);
	make(0x0000fbea, utils::index4t<0xf,0xb,0xe,0xa>);
	make(0x0000b7a6, utils::index4t<0xb,0x7,0xa,0x6>);
	make(0x00007362, utils::index4t<0x7,0x3,0x6,0x2>);
	make(0x0000ead9, utils::index4t<0xe,0xa,0xd,0x9>);
	make(0x0000a695, utils::index4t<0xa,0x6,0x9,0x5>);
	make(0x00006251, utils::index4t<0x6,0x2,0x5,0x1>);
	make(0x0000d9c8, utils::index4t<0xd,0x9,0xc,0x8>);
	make(0x00009584, utils::index4t<0x9,0x5,0x8,0x4>);
	make(0x00005140, utils::index4t<0x5,0x1,0x4,0x0>);
	make(0x0000cd89, utils::index4t<0xc,0xd,0x8,0x9>);
	make(0x0000de9a, utils::index4t<0xd,0xe,0x9,0xa>);
	make(0x0000efab, utils::index4t<0xe,0xf,0xa,0xb>);
	make(0x00008945, utils::index4t<0x8,0x9,0x4,0x5>);
	make(0x00009a56, utils::index4t<0x9,0xa,0x5,0x6>);
	make(0x0000ab67, utils::index4t<0xa,0xb,0x6,0x7>);
	make(0x00004501, utils::index4t<0x4,0x5,0x0,0x1>);
	make(0x00005612, utils::index4t<0x5,0x6,0x1,0x2>);
	make(0x00006723, utils::index4t<0x6,0x7,0x2,0x3>);
	make(0x00000415, utils::index4t<0x0,0x4,0x1,0x5>);
	make(0x00004859, utils::index4t<0x4,0x8,0x5,0x9>);
	make(0x00008c9d, utils::index4t<0x8,0xc,0x9,0xd>);
	make(0x00001526, utils::index4t<0x1,0x5,0x2,0x6>);
	make(0x0000596a, utils::index4t<0x5,0x9,0x6,0xa>);
	make(0x00009dae, utils::index4t<0x9,0xd,0xa,0xe>);
	make(0x00002637, utils::index4t<0x2,0x6,0x3,0x7>);
	make(0x00006a7b, utils::index4t<0x6,0xa,0x7,0xb>);
	make(0x0000aebf, utils::index4t<0xa,0xe,0xb,0xf>);
	make(0x01234567, utils::index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	make(0x456789ab, utils::index8t<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	make(0x37bf26ae, utils::index8t<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	make(0x26ae159d, utils::index8t<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	make(0xfedcba98, utils::index8t<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	make(0xba987654, utils::index8t<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	make(0xc840d951, utils::index8t<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	make(0xd951ea62, utils::index8t<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	make(0x32107654, utils::index8t<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	make(0x7654ba98, utils::index8t<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	make(0xfb73ea62, utils::index8t<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	make(0xea62d951, utils::index8t<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	make(0xcdef89ab, utils::index8t<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	make(0x89ab4567, utils::index8t<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	make(0x048c159d, utils::index8t<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	make(0x159d26ae, utils::index8t<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	make(0x00123456, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x6>);
	make(0x037bf26a, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x6,0xa>);
	make(0x0fedcba9, utils::index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x9>);
	make(0x0c840d95, utils::index7t<0xc,0x8,0x4,0x0,0xd,0x9,0x5>);
	make(0x03210765, utils::index7t<0x3,0x2,0x1,0x0,0x7,0x6,0x5>);
	make(0x0fb73ea6, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xa,0x6>);
	make(0x0cdef89a, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x9,0xa>);
	make(0x0048c159, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x9>);
	make(0x0456789a, utils::index7t<0x4,0x5,0x6,0x7,0x8,0x9,0xa>);
	make(0x026ae159, utils::index7t<0x2,0x6,0xa,0xe,0x1,0x5,0x9>);
	make(0x0ba98765, utils::index7t<0xb,0xa,0x9,0x8,0x7,0x6,0x5>);
	make(0x0d951ea6, utils::index7t<0xd,0x9,0x5,0x1,0xe,0xa,0x6>);
	make(0x07654ba9, utils::index7t<0x7,0x6,0x5,0x4,0xb,0xa,0x9>);
	make(0x0ea62d95, utils::index7t<0xe,0xa,0x6,0x2,0xd,0x9,0x5>);
	make(0x089ab456, utils::index7t<0x8,0x9,0xa,0xb,0x4,0x5,0x6>);
	make(0x0159d26a, utils::index7t<0x1,0x5,0x9,0xd,0x2,0x6,0xa>);
	make(0x00123567, utils::index7t<0x0,0x1,0x2,0x3,0x5,0x6,0x7>);
	make(0x037bf6ae, utils::index7t<0x3,0x7,0xb,0xf,0x6,0xa,0xe>);
	make(0x0fedca98, utils::index7t<0xf,0xe,0xd,0xc,0xa,0x9,0x8>);
	make(0x0c840951, utils::index7t<0xc,0x8,0x4,0x0,0x9,0x5,0x1>);
	make(0x03210654, utils::index7t<0x3,0x2,0x1,0x0,0x6,0x5,0x4>);
	make(0x0fb73a62, utils::index7t<0xf,0xb,0x7,0x3,0xa,0x6,0x2>);
	make(0x0cdef9ab, utils::index7t<0xc,0xd,0xe,0xf,0x9,0xa,0xb>);
	make(0x0048c59d, utils::index7t<0x0,0x4,0x8,0xc,0x5,0x9,0xd>);
	make(0x045679ab, utils::index7t<0x4,0x5,0x6,0x7,0x9,0xa,0xb>);
	make(0x026ae59d, utils::index7t<0x2,0x6,0xa,0xe,0x5,0x9,0xd>);
	make(0x0ba98654, utils::index7t<0xb,0xa,0x9,0x8,0x6,0x5,0x4>);
	make(0x0d951a62, utils::index7t<0xd,0x9,0x5,0x1,0xa,0x6,0x2>);
	make(0x07654a98, utils::index7t<0x7,0x6,0x5,0x4,0xa,0x9,0x8>);
	make(0x0ea62951, utils::index7t<0xe,0xa,0x6,0x2,0x9,0x5,0x1>);
	make(0x089ab567, utils::index7t<0x8,0x9,0xa,0xb,0x5,0x6,0x7>);
	make(0x0159d6ae, utils::index7t<0x1,0x5,0x9,0xd,0x6,0xa,0xe>);
	make(0x001237bf, utils::index7t<0x0,0x1,0x2,0x3,0x7,0xb,0xf>);
	make(0x037bfedc, utils::index7t<0x3,0x7,0xb,0xf,0xe,0xd,0xc>);
	make(0x0fedc840, utils::index7t<0xf,0xe,0xd,0xc,0x8,0x4,0x0>);
	make(0x0c840123, utils::index7t<0xc,0x8,0x4,0x0,0x1,0x2,0x3>);
	make(0x0321048c, utils::index7t<0x3,0x2,0x1,0x0,0x4,0x8,0xc>);
	make(0x0fb73210, utils::index7t<0xf,0xb,0x7,0x3,0x2,0x1,0x0>);
	make(0x0cdefb73, utils::index7t<0xc,0xd,0xe,0xf,0xb,0x7,0x3>);
	make(0x0048cdef, utils::index7t<0x0,0x4,0x8,0xc,0xd,0xe,0xf>);
	make(0x0012348c, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x8,0xc>);
	make(0x037bf210, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x1,0x0>);
	make(0x0fedcb73, utils::index7t<0xf,0xe,0xd,0xc,0xb,0x7,0x3>);
	make(0x0c840def, utils::index7t<0xc,0x8,0x4,0x0,0xd,0xe,0xf>);
	make(0x032107bf, utils::index7t<0x3,0x2,0x1,0x0,0x7,0xb,0xf>);
	make(0x0fb73edc, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xd,0xc>);
	make(0x0cdef840, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x4,0x0>);
	make(0x0048c123, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x2,0x3>);
	make(0x00123458, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x8>);
	make(0x037bf261, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x6,0x1>);
	make(0x0fedcba7, utils::index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x7>);
	make(0x0c840d9e, utils::index7t<0xc,0x8,0x4,0x0,0xd,0x9,0xe>);
	make(0x0321076b, utils::index7t<0x3,0x2,0x1,0x0,0x7,0x6,0xb>);
	make(0x0fb73ead, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xa,0xd>);
	make(0x0cdef894, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x9,0x4>);
	make(0x0048c152, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x2>);
	make(0x0012367b, utils::index7t<0x0,0x1,0x2,0x3,0x6,0x7,0xb>);
	make(0x037bfaed, utils::index7t<0x3,0x7,0xb,0xf,0xa,0xe,0xd>);
	make(0x0fedc984, utils::index7t<0xf,0xe,0xd,0xc,0x9,0x8,0x4>);
	make(0x0c840512, utils::index7t<0xc,0x8,0x4,0x0,0x5,0x1,0x2>);
	make(0x03210548, utils::index7t<0x3,0x2,0x1,0x0,0x5,0x4,0x8>);
	make(0x0fb73621, utils::index7t<0xf,0xb,0x7,0x3,0x6,0x2,0x1>);
	make(0x0cdefab7, utils::index7t<0xc,0xd,0xe,0xf,0xa,0xb,0x7>);
	make(0x0048c9de, utils::index7t<0x0,0x4,0x8,0xc,0x9,0xd,0xe>);
	make(0x00001234, utils::index5t<0x0,0x1,0x2,0x3,0x4>);
	make(0x00037bf2, utils::index5t<0x3,0x7,0xb,0xf,0x2>);
	make(0x000fedcb, utils::index5t<0xf,0xe,0xd,0xc,0xb>);
	make(0x000c840d, utils::index5t<0xc,0x8,0x4,0x0,0xd>);
	make(0x00032107, utils::index5t<0x3,0x2,0x1,0x0,0x7>);
	make(0x000fb73e, utils::index5t<0xf,0xb,0x7,0x3,0xe>);
	make(0x000cdef8, utils::index5t<0xc,0xd,0xe,0xf,0x8>);
	make(0x000048c1, utils::index5t<0x0,0x4,0x8,0xc,0x1>);
	make(0x00045678, utils::index5t<0x4,0x5,0x6,0x7,0x8>);
	make(0x00026ae1, utils::index5t<0x2,0x6,0xa,0xe,0x1>);
	make(0x000ba987, utils::index5t<0xb,0xa,0x9,0x8,0x7>);
	make(0x000d951e, utils::index5t<0xd,0x9,0x5,0x1,0xe>);
	make(0x0007654b, utils::index5t<0x7,0x6,0x5,0x4,0xb>);
	make(0x000ea62d, utils::index5t<0xe,0xa,0x6,0x2,0xd>);
	make(0x00089ab4, utils::index5t<0x8,0x9,0xa,0xb,0x4>);
	make(0x000159d2, utils::index5t<0x1,0x5,0x9,0xd,0x2>);
	make(0xff000000, utils::indexmerge0);
	make(0xff000001, utils::indexmerge1<0>);
	make(0xff000011, utils::indexmerge1<1>);
	make(0xfe000000, utils::indexnum0);
	make(0xfe000001, utils::indexnum1);
	make(0xfe000002, utils::indexnum2);
	make(0xfe000082, utils::indexnum2x<0, 0, 1>);
	make(0xfe000092, utils::indexnum2x<0, 2, 3>);
	make(0xfe0000c2, utils::indexnum2x<1, 0, 1>);
	make(0xfe0000d2, utils::indexnum2x<1, 2, 3>);
	make(0xfe000003, utils::indexnum3);
	make(0xfe000004, utils::indexnum4);
	make(0xfe000005, utils::indexnum5lt);
	make(0xfe000015, utils::indexnum5st);
	make(0xfd012301, utils::indexmono<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	make(0xfd37bf01, utils::indexmono<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	make(0xfdfedc01, utils::indexmono<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	make(0xfdc84001, utils::indexmono<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	make(0xfd321001, utils::indexmono<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	make(0xfdfb7301, utils::indexmono<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	make(0xfdcdef01, utils::indexmono<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	make(0xfd048c01, utils::indexmono<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	make(0xfd456701, utils::indexmono<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	make(0xfd26ae01, utils::indexmono<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	make(0xfdba9801, utils::indexmono<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	make(0xfdd95101, utils::indexmono<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	make(0xfd765401, utils::indexmono<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	make(0xfdea6201, utils::indexmono<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	make(0xfd89ab01, utils::indexmono<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	make(0xfd159d01, utils::indexmono<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	make(0xfc000000, utils::indexmax<0>);
	make(0xfc000010, utils::indexmax<1>);
	make(0xfc000020, utils::indexmax<2>);
	make(0xfc000030, utils::indexmax<3>);
	make(0xfc000040, utils::indexmax<4>);
	make(0xfc000050, utils::indexmax<5>);
	make(0xfc000060, utils::indexmax<6>);
	make(0xfc000070, utils::indexmax<7>);
	return succ;
}

u32 save_weights(std::string path) {
	std::ofstream out;
	char buf[1 << 20];
	out.rdbuf()->pubsetbuf(buf, sizeof(buf));
	out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return 0;
	u32 succ = weight::save(out);
	out.flush();
	out.close();
	return succ;
}
u32 load_weights(std::string path) {
	std::ifstream in;
	char buf[1 << 20];
	in.rdbuf()->pubsetbuf(buf, sizeof(buf));
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return 0;
	u32 succ = weight::load(in);
	in.close();
	return succ;
}
u32 make_weights(std::string res = "") {
	std::map<std::string, std::string> alias;
	alias["khyeh"] = "012345 456789 012456 45689a ";
	alias["patt/42-33"] = "012345 456789 89abcd 012456 45689a ";
	alias["patt/4-22"] = "0123 4567 0145 1256 569a ";
	alias["k.matsuzaki"] = "012456 12569d 012345 01567a 01259a 0159de 01589d 01246a ";
	alias["k.matsuzaki-opt"] = "012456 456789 012345 234569 01259a 345678 134567 01489a ";
	alias["monotonic"] = "fd012301:^24 fd456701:^24 ";
	alias["default"] = alias["khyeh"] + alias["monotonic"] + "fe000005:^24 fe000015:^24 ";
	alias["4x6patt"] = alias["khyeh"];
	alias["5x6patt"] = alias["patt/42-33"];
	alias["8x6patt"] = alias["k.matsuzaki-opt"];
	alias["5x4patt"] = alias["patt/4-22"];

	if (res.empty() && weight::wghts().empty())
		res = { "default" };
	for (auto def : alias) { // insert predefined weights
		auto pos = (" " + res + " ").find(" " + def.first + " ");
		if (pos != std::string::npos)
			res.replace(pos, def.first.size(), def.second);
	}

	u32 succ = 0;
	std::stringstream split(res);
	std::string token;
	while (split >> token) {
		// weight:size weight(size) weight[size] weight:patt weight:? weight:^bit old=new old=new:size
		while (token.find_first_of(":()[],") != std::string::npos)
			token[token.find_first_of(":()[],")] = ' ';
		std::stringstream info(token);

		std::string signs;
		if (!(info >> signs)) continue;
		weight::sign_t prev = std::stoull(signs.substr(0), nullptr, 16);
		weight::sign_t sign = std::stoull(signs.substr(signs.find('=') + 1), nullptr, 16);
		if (prev != sign && weight::find(prev) != weight::wghts().end()) {
			std::cerr << "move weight to a new sign (" << signs << ")..." << std::endl;
			signs = signs.substr(signs.find('=') + 1);
			weight wsrc = weight::at(prev);
			weight wdst = weight::make(sign, wsrc.size());
			std::copy_n(wsrc.data(), wsrc.size() * wsrc.stride(), wdst.data());
			weight::erase(prev);
		}

		std::string sizes;
		if (!(info >> sizes)) sizes = "?";
		size_t size = 0;
		switch (sizes.front()) {
		case 'p':
		case '?':
			size = std::pow(16ull, signs.size());
			break;
		case '^':
			size = std::pow(2ull, std::stol(sizes.substr(1)));
			break;
		default:
			size = std::stoull(sizes, nullptr, 0);
			break;
		}

		if (weight::find(sign) == weight::wghts().end()) {
			weight::make(sign, size);
			succ++;
		} else if (weight::at(sign).size() != size) {
			std::cerr << "size mismatch for weight (" << signs << ") at make_weights, ";
			std::cerr << "override previous weight table..." << std::endl;
			weight::erase(sign);
			weight::make(sign, size);
			succ++;
		}
	}
	return succ;
}

u32 save_features(std::string path) {
	std::ofstream out;
	out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return 0;
	u32 succ = feature::save(out);
	out.flush();
	out.close();
	return succ;
}
u32 load_features(std::string path) {
	std::ifstream in;
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return 0;
	u32 succ = feature::load(in);
	in.close();
	return succ;
}
u32 make_features(std::string res = "") {
	std::map<std::string, std::string> alias;
	alias["khyeh"] = "012345[012345!] 456789[456789!] 012456[012456!] 45689a[45689a!] ";
	alias["patt/42-33"] = "012345[012345!] 456789[456789!] 89abcd[89abcd!] 012456[012456!] 45689a[45689a!] ";
	alias["patt/4-22"] = "0123[0123!] 4567[4567!] 0145[0145!] 1256[1256!] 569a[569a!] ";
	alias["monotonic"] = "fd012301[fd012301] fd012301[fd37bf01] fd012301[fdfedc01] fd012301[fdc84001] "
	                     "fd012301[fd321001] fd012301[fdfb7301] fd012301[fdcdef01] fd012301[fd048c01] "
	                     "fd456701[fd456701] fd456701[fd26ae01] fd456701[fdba9801] fd456701[fdd95101] "
	                     "fd456701[fd765401] fd456701[fdea6201] fd456701[fd89ab01] fd456701[fd159d01] ";
	alias["k.matsuzaki"] = "012456:012456! 12569d:12569d! 012345:012345! 01567a:01567a! "
	                       "01259a:01259a! 0159de:0159de! 01589d:01589d! 01246a:01246a! ";
	alias["k.matsuzaki-opt"] = "012456:012456! 456789:456789! 012345:012345! 234569:234569! "
	                           "01259a:01259a! 345678:345678! 134567:134567! 01489a:01489a! ";
	alias["default"] = alias["khyeh"] + alias["monotonic"] + "fe000005[fe000005] fe000015[fe000015] ";
	alias["4x6patt"] = alias["khyeh"];
	alias["5x6patt"] = alias["patt/42-33"];
	alias["8x6patt"] = alias["k.matsuzaki-opt"];
	alias["5x4patt"] = alias["patt/4-22"];
	alias["mono"] = alias["monotonic"];

	if (res.empty() && feature::feats().empty())
		res = { "default" };
	for (auto def : alias) { // insert predefined features
		auto pos = (" " + res + " ").find(" " + def.first + " ");
		if (pos != std::string::npos)
			res.replace(pos, def.first.size(), def.second);
	}

	u32 succ = 0;
	std::stringstream split(res);
	std::string token;
	while (split >> token) {
		// weight:indexer weight(indexer) weight[indexer] weight[and&or|pattern!]
		while (token.find_first_of(":()[],") != std::string::npos)
			token[token.find_first_of(":()[],")] = ' ';
		std::stringstream info(token);

		std::string wghts, idxrs;
		if (!(info >> wghts && info >> idxrs)) continue;

		weight::sign_t wght = std::stoull(wghts, nullptr, 16);
		if (weight::find(wght) == weight::wghts().end()) {
			std::cerr << "unknown weight (" << wghts << ") at make_features, ";
			std::cerr << "assume as pattern descriptor..." << std::endl;
			weight::make(wght, std::pow(16ull, wghts.size()));
		}

		std::string idxv;
		int isomorphic = 1;
		u64 op_bitand = -1ull, op_bitor = 0ull;
		while (idxv.empty() && std::isxdigit(idxrs[0])) {
			size_t pos = 0;
			u64 value = std::stoull(idxrs, &pos, 16);
			switch (idxrs[pos]) {
			case '!':
				isomorphic = 8;
				idxv = idxrs.substr(0, pos);
				break;
			default:
				idxv = idxrs.substr(0, pos);
				break;
			case '&':
				op_bitand = value;
				idxrs.erase(0, pos + 1);
				break;
			case '|':
				op_bitor = value;
				idxrs.erase(0, pos + 1);
				break;
			}
		}

		if (idxv.empty()) continue;

		for (int iso = 0; iso < isomorphic; iso++) {
			std::vector<int> xpatt = utils::hashpatt(idxv);
			std::transform(xpatt.begin(), xpatt.end(), xpatt.begin(), [=](int v) {
				board x(0xfedcba9876543210ull);
				x.isomorphic(-iso);
				return x.at(v);
			});

			indexer::sign_t idxr = (utils::hashpatt(xpatt) & op_bitand) | op_bitor;
			if (indexer::find(idxr) == indexer::idxrs().end()) {
				idxrs = utils::hashpatt(idxr, idxv.size());
				std::cerr << "unknown indexer (" << idxrs << ") at make_features, ";
				std::cerr << "assume as pattern descriptor..." << std::endl;
				std::vector<int>* npatt = new std::vector<int>(xpatt); // will NOT be deleted
				indexer::make(idxr, std::bind(utils::indexnta, std::placeholders::_1, std::cref(*npatt)));
			}

			if (feature::find(wght, idxr) == feature::feats().end()) {
				feature::make(wght, idxr);
				succ++;
			}
		}
	}
	return succ;
}

void list_mapping() {
	for (weight w : list<weight>(weight::wghts())) {
		char buf[64];
		std::string feats;
		for (feature f : feature::feats()) {
			if (weight(f) == w) {
				snprintf(buf, sizeof(buf), " %08" PRIx64, indexer(f).sign());
				feats += buf;
			}
		}
		if (feats.size()) {
			u32 usageK = (sizeof(numeric) * w.size()) >> 10;
			u32 usageM = usageK >> 10;
			u32 usageG = usageM >> 10;
			u32 usage = usageG ? usageG : (usageM ? usageM : usageK);
			char scale = usageG ? 'G' : (usageM ? 'M' : 'K');
			snprintf(buf, sizeof(buf), "weight(%08" PRIx64 ")[%zu] = %d%c", w.sign(), w.size(), usage, scale);
			std::cout << buf << " :" << feats << std::endl;
		} else {
			snprintf(buf, sizeof(buf), "%08" PRIx64, w.sign());
			weight::erase(w.sign());
			std::cerr << "unused weight (" << buf << ") at list_mapping, erased" << std::endl;
		}
	}
}



inline numeric estimate(const board& state,
		const clip<feature>& range = feature::feats()) {
	register numeric esti = 0;
	for (register feature& feat : range)
		esti += feat[state];
	return esti;
}

inline numeric update(const board& state, const numeric& updv,
		const clip<feature>& range = feature::feats()) {
	register numeric esti = 0;
	for (register feature& feat : range)
		esti += (feat[state] += updv);
	return esti;
}

} // utils


struct state {
	typedef i32 (board::*action)();
	board move;
	action oper;
	i32 score;
	numeric esti;
	state() : state(nullptr) {}
	state(action oper) : oper(oper), score(-1), esti(0) {}
	state(const state& s) = default;

	inline operator bool() const { return score >= 0; }
	inline operator board() const { return move; }

	inline numeric value() const { return esti - score; }
	inline numeric reward() const { return score; }

	inline void assign(const board& b) {
		move = b;
		score = (move.*oper)();
	}
	inline numeric estimate(
			const clip<feature>& range = feature::feats()) {
		if (score >= 0) {
			esti = state::reward() + utils::estimate(move, range);
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
		return esti;
	}
	inline numeric update(const numeric& accu, const numeric& alpha = state::alpha(),
			const clip<feature>& range = feature::feats()) {
		esti = state::reward() + utils::update(move, alpha * (accu - state::value()), range);
		return esti;
	}

	inline void operator <<(const board& b) { assign(b); estimate(); }
	inline void operator >>(board& b) const { b = move; }

	declare_comparators(state, esti);

	inline static numeric& alpha() {
		static numeric a = numeric(0.0025);
		return a;
	}
	inline static numeric& alpha(const numeric& a) {
		return (state::alpha() = a);
	}
};
struct select {
	state move[4];
	state *best;
	select(state::action up = &board::up, state::action right = &board::right,
		   state::action down = &board::down, state::action left = &board::left) : best(move) {
		move[0] = state(up);
		move[1] = state(right);
		move[2] = state(down);
		move[3] = state(left);
	}
	inline select& operator ()(const board& b, const clip<feature>& range = feature::feats()) {
		move[0].assign(b);
		move[1].assign(b);
		move[2].assign(b);
		move[3].assign(b);
		move[0].estimate(range);
		move[1].estimate(range);
		move[2].estimate(range);
		move[3].estimate(range);
		best = move;
		if (move[1] > *best) best = move + 1;
		if (move[2] > *best) best = move + 2;
		if (move[3] > *best) best = move + 3;
		return *this;
	}
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(state& s) const { s = (*best); }
	inline void operator >>(board& b) const { *best >> b; }

	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }
	inline u32 opcode() const { return best - move; }

	inline state* begin() { return move; }
	inline state* end() { return move + 4; }
};
struct statistic {
	u64 limit;
	u64 loop;
	u64 unit;
	u32 winv;

	class format_t : public std::array<char, 64> {
	public:
		inline void operator =(const std::string& s) { std::copy_n(s.begin(), s.size() + 1, begin()); }
		inline operator const char*() const { return &(operator[](0)); }
	};

	format_t indexf;
	format_t localf;
	format_t totalf;
	format_t summaf;

	struct record {
		u64 score;
		u64 win;
		u64 time;
		u64 opers;
		u32 max;
		u32 hash;
		record& operator +=(const record& rec) {
			score += rec.score;
			win += rec.win;
			time += rec.time;
			opers += rec.opers;
			hash |= rec.hash;
			max = std::max(max, rec.max);
			return (*this);
		}
	} total, local;

	struct each {
		std::array<u64, 32> score;
		std::array<u64, 32> opers;
		std::array<u64, 32> count;
		each& operator +=(const each& ea) {
			std::transform(count.begin(), count.end(), ea.count.begin(), count.begin(), std::plus<u64>());
			std::transform(score.begin(), score.end(), ea.score.begin(), score.begin(), std::plus<u64>());
			std::transform(opers.begin(), opers.end(), ea.opers.begin(), opers.begin(), std::plus<u64>());
			return (*this);
		}
	} every;

	statistic() : limit(0), loop(0), unit(0), winv(0), total({}), local({}), every({}) {}
	statistic(const utils::options::option& opt) : statistic() { init(opt); }
	statistic(const statistic& stat) = default;

	bool init(const utils::options::option& opt = {}) {
		loop = 1000;
		unit = 1000;
		winv = 2048;

		auto npos = std::string::npos;
		auto it = std::find_if(opt.begin(), opt.end(), [=](std::string v) { return v.find('=') == npos; });
		std::string res = (it != opt.end()) ? *it : "1000";
		try {
			loop = std::stol(res);
			if (res.find('x') != npos) unit = std::stol(res.substr(res.find('x') + 1));
			if (res.find(':') != npos) winv = std::stol(res.substr(res.find(':') + 1));
		} catch (std::invalid_argument&) {}

		loop = std::stol(opt.find("loop", std::to_string(loop)));
		unit = std::stol(opt.find("unit", std::to_string(unit)));
		winv = std::stol(opt.find("win",  std::to_string(winv)));

		limit = loop * unit;
		loop = 1;
		format();

		every = {};
		total = {};
		local = {};
		local.time = moporgic::millisec();

		return limit;
	}
	void format() {
//		indexf = "%03llu/%03llu %llums %.2fops";
//		localf = "local:  avg=%llu max=%u tile=%u win=%.2f%%";
//		totalf = "total:  avg=%llu max=%u tile=%u win=%.2f%%";
//		summaf = "summary %llums %.2fops";
		u32 dec = std::max(std::floor(std::log10(limit / unit)) + 1, 3.0);
		indexf = "%0" + std::to_string(dec) + PRIu64 "/%0" + std::to_string(dec) + PRIu64 " %" PRIu64 "ms %.2fops";
		localf = "local: " + std::string(dec * 2 - 5, ' ') + "avg=%" PRIu64 " max=%u tile=%u win=%.2f%%";
		totalf = "total: " + std::string(dec * 2 - 5, ' ') + "avg=%" PRIu64 " max=%u tile=%u win=%.2f%%";
		summaf = "summary" + std::string(dec * 2 - 5, ' ') + "%" PRIu64 "ms %.2fops";
	}

	u64 operator++(int) { return (++loop) - 1; }
	u64 operator++() { return (++loop); }
	operator bool() const { return loop <= limit; }
	bool checked() const { return (loop % unit) == 0; }

	void update(const u32& score, const u32& hash, const u32& opers) {
		local.score += score;
		local.hash |= hash;
		local.opers += opers;
		if (hash >= winv) local.win += 1;
		local.max = std::max(local.max, score);
		every.count[math::log2(hash)] += 1;
		every.score[math::log2(hash)] += score;
		every.opers[math::log2(hash)] += opers;

		if ((loop % unit) != 0) return;

		u64 current_time = moporgic::millisec();
		local.time = current_time - local.time;
		total.score += local.score;
		total.win += local.win;
		total.time += local.time;
		total.opers += local.opers;
		total.hash |= local.hash;
		total.max = std::max(total.max, local.max);

		std::cout << std::endl;
		char buf[64];
		snprintf(buf, sizeof(buf), indexf, // "%03llu/%03llu %llums %.2fops",
				loop / unit,
				limit / unit,
				local.time,
				local.opers * 1000.0 / local.time);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), localf, // "local:  avg=%llu max=%u tile=%u win=%.2f%%",
				local.score / unit,
				local.max,
				math::msb32(local.hash),
				local.win * 100.0 / unit);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), totalf, // "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / loop,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / loop);
		std::cout << buf << std::endl;

		local = {};
		local.time = current_time;
	}

	void summary(const utils::options::option& opt = {}) const {
		std::cout << std::endl;
		char buf[80];
		snprintf(buf, sizeof(buf), summaf,
				total.time,
				total.opers * 1000.0 / total.time);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), totalf, // "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / limit,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / limit);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), "%-6s"  "%8s"    "%8s"    "%8s"   "%9s"   "%9s",
		                           "tile", "count", "score", "move", "rate", "win");
		std::cout << buf << std::endl;
		const auto& count = every.count;
		const auto& score = every.score;
		const auto& opers = every.opers;
		auto total = std::accumulate(count.begin(), count.end(), 0);
		for (auto left = total, i = 0; left; left -= count[i++]) {
			if (count[i] == 0) continue;
			snprintf(buf, sizeof(buf), "%-6d" "%8d" "%8d" "%8d" "%8.2f%%" "%8.2f%%",
					(1 << (i)) & 0xfffffffeu, u32(count[i]),
					u32(score[i] / count[i]), u32(opers[i] / count[i]),
					count[i] * 100.0 / total, left * 100.0 / total);
			std::cout << buf << std::endl;
		}
	}

	statistic  operator + (const statistic& stat) const {
		return statistic(*this) += stat;
	}
	statistic& operator +=(const statistic& stat) {
		limit += stat.limit;
		loop += stat.loop;
		if (!unit) unit = stat.unit;
		if (!winv) winv = stat.winv;
		total += stat.total;
		local += stat.local;
		every += stat.every;
		format();
		return *this;
	}
};

inline utils::options parse(int argc, const char* argv[]) {
	utils::options opts;
	auto find_opt = [&](int& i, const std::string& v) -> std::string {
		return (i + 1 < argc && *(argv[i + 1]) != '-') ? argv[++i] : v;
	};
	auto find_opts = [&](int& i) -> std::vector<std::string> {
		std::vector<std::string> vec;
		for (std::string v; (v = find_opt(i, "")).size(); ) vec.push_back(v);
		return vec;
	};
	for (int i = 1; i < argc; i++) {
		switch (to_hash(argv[i])) {
		case to_hash("-a"):
		case to_hash("--alpha"):
			opts["alpha"] = find_opt(i, std::to_string(state::alpha()));
			break;
		case to_hash("-t"):
		case to_hash("--train"):
			opts["train"] = find_opt(i, "1000");
			opts["train"] += find_opts(i);
			break;
		case to_hash("-T"):
		case to_hash("-e"):
		case to_hash("--test"):
			opts["test"] = find_opt(i, "1000");
			opts["test"] += find_opts(i);
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
		case to_hash("--srand"):
			opts["seed"] = find_opt(i, "moporgic");
			break;
		case to_hash("-wio"):
		case to_hash("--weight-input-output"):
			opts["temporary"] = find_opts(i);
			opts["weight-input"] += opts["temporary"];
			opts["weight-output"] += opts["temporary"];
			break;
		case to_hash("-wi"):
		case to_hash("--weight-input"):
			opts["weight-input"] += find_opts(i);
			break;
		case to_hash("-wo"):
		case to_hash("--weight-output"):
			opts["weight-output"] += find_opts(i);
			break;
		case to_hash("-fio"):
		case to_hash("--feature-input-output"):
			opts["temporary"] = find_opts(i);
			opts["feature-input"] += opts["temporary"];
			opts["feature-output"] += opts["temporary"];
			break;
		case to_hash("-fi"):
		case to_hash("--feature-input"):
			opts["feature-input"] += find_opts(i);
			break;
		case to_hash("-fo"):
		case to_hash("--feature-output"):
			opts["feature-output"] += find_opts(i);
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
		case to_hash("--weight-value"):
			opts["weight-value"] += find_opts(i);
			break;
		case to_hash("-f"):
		case to_hash("--feature"):
		case to_hash("--feature-value"):
			opts["feature-value"] += find_opts(i);
			break;
		case to_hash("-wf"):
		case to_hash("-fw"):
			opts["temporary"] = find_opts(i);
			opts["feature-value"] += opts["temporary"];
			opts["weight-value"] += opts["temporary"];
			break;
		case to_hash("-i"):
		case to_hash("--info"):
			opts["info"] = find_opt(i, "full");
			opts["info"] += find_opts(i);
			break;
		case to_hash("-o"):
		case to_hash("--option"):
		case to_hash("--options"):
		case to_hash("--extra"):
			opts["options"] += find_opts(i);
			break;
		case to_hash("-tt"):
		case to_hash("-tm"):
		case to_hash("--train-mode"):
			opts["train"]["mode"] = find_opt(i, "bias");
			break;
		case to_hash("-Tt"):
		case to_hash("-et"):
		case to_hash("-em"):
		case to_hash("--test-mode"):
			opts["test"]["mode"] = find_opt(i, "bias");
			break;
		case to_hash("-tc"):
		case to_hash("-tu"):
		case to_hash("--train-check"):
		case to_hash("--train-check-interval"):
		case to_hash("--train-unit"):
			opts["train"]["unit"] = find_opt(i, "1000");
			break;
		case to_hash("-Tc"):
		case to_hash("-ec"):
		case to_hash("-eu"):
		case to_hash("--test-check"):
		case to_hash("--test-check-interval"):
		case to_hash("--test-unit"):
			opts["test"]["unit"] = find_opt(i, "1000");
			break;
		case to_hash("-v"):
		case to_hash("--win"):
			opts["train"]["win"] = opts["test"]["win"] = find_opt(i, "2048");
			break;
		case to_hash("-c"):
		case to_hash("--comment"):
			opts["comment"] = find_opts(i);
			break;
		default:
			std::cerr << "unknown: " << argv[i];
			for (auto& v : find_opts(i)) std::cerr << " " << v;
			std::cerr << std::endl;
			break;
		}
	}
	return opts;
}

statistic train(utils::options opts = {}) {
	std::vector<state> path;
	path.reserve(65536);
	statistic stats;
	select best;
	state last;
	board b;

	switch (to_hash(opts["train"]["mode"])) {
	case to_hash("backward"):
	case to_hash("backward-best"):
		for (stats.init(opts["train"]); stats; stats++) {

			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best << b; b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			for (numeric v = 0; path.size(); path.pop_back()) {
				path.back().estimate();
				v = path.back().update(v);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	default:
	case to_hash("forward"):
	case to_hash("forward-best"):
		for (stats.init(opts["train"]); stats; stats++) {

			u32 score = 0;
			u32 opers = 0;

			b.init();
			best << b;
			score += best.score();
			opers += 1;
			best >> last;
			best >> b;
			b.next();
			while (best << b) {
				last.update(best.esti());
				score += best.score();
				opers += 1;
				best >> last;
				best >> b;
				b.next();
			}
			last.update(0);

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

statistic test(utils::options opts = {}) {
	statistic stats;
	select best;
	board b;

	switch (to_hash(opts["test"]["mode"])) {
	default:
	case to_hash("best"):
		for (stats.init(opts["test"]); stats; stats++) {

			u32 score;
			u32 opers;

			for (b.init(); best << b; b.next()) {
				score += best.score();
				opers += 1;
				best >> b;
			}

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

int main(int argc, const char* argv[]) {
	utils::options opts = parse(argc, argv);
	if (!opts("train")) opts["train"] = opts("test") ? 0 : 1000;
	if (!opts("test")) opts["test"] = opts("train") ? 0 : 10;
	if (!opts("alpha")) opts["alpha"] = 0.0025;
	if (!opts("seed")) opts["seed"] = rdtsc();

	std::cout << "TDL2048+ LOG" << std::endl;
	std::cout << "develop" << " build C++" << __cplusplus;
	std::cout << " " << __DATE__ << " " << __TIME__ << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << moporgic::millisec() << std::endl;
	std::cout << "seed = " << opts["seed"] << std::endl;
	std::cout << "alpha = " << opts["alpha"] << std::endl;
	std::cout << std::endl;

	std::srand(moporgic::to_hash(opts["seed"]));
	state::alpha(std::stod(opts["alpha"]));

	utils::make_indexers();

	utils::load_weights(opts["weight-input"]);
	utils::make_weights(opts["weight-value"]);

	utils::load_features(opts["feature-input"]);
	utils::make_features(opts["feature-value"]);

	utils::list_mapping();

	if (statistic(opts["train"])) {
		std::cout << std::endl << "start training..." << std::endl;
		statistic stat = train(opts);
		if (opts["info"] == "full") stat.summary(opts["train"]);
	}

	utils::save_weights(opts["weight-output"]);
	utils::save_features(opts["feature-output"]);

	if (statistic(opts["test"])) {
		std::cout << std::endl << "start testing..." << std::endl;
		statistic stat = test(opts);
		if (opts["info"] != "none") stat.summary(opts["test"]);
	}

	std::cout << std::endl;
	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
