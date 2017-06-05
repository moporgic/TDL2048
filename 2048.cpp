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
	weight() : id(0), length(0), value(nullptr) {}
	weight(const weight& w) = default;
	~weight() {}

	inline u64 sign() const { return id; }
	inline u64 size() const { return length; }
	inline numeric& operator [](const u64& i) { return value[(i << 1) + i]; }
	inline numeric& operator ()(const u64& i, const numeric& alpha, const numeric& error) {
		numeric  aupdv = alpha * error;
//		numeric  coher = std::abs(accum[i]) / updvu[i];
//		value[i] += std::isnan(coher) ? aupdv : (coher * aupdv);
		numeric* block = value + ((i << 1) + i);
		numeric& value = block[0];
		numeric& accum = block[1];
		numeric& updvu = block[2];
		value += updvu ? (std::abs(accum) / updvu) * aupdv : aupdv;
		accum += error;
		updvu += std::abs(error);
		return value;
	}
	inline numeric* raw(const u64& i = 0) { return value + ((i << 1) + i); }

	inline bool operator ==(const weight& w) const { return id == w.id; }
	inline bool operator !=(const weight& w) const { return id != w.id; }

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		auto& id = w.id;
		auto& length = w.length;
		auto& value = w.value;
		u32 code = 128;
		write_cast<u8>(out, code);
		switch (code) {
		case 128:
			write_cast<u32>(out, id);
			write_cast<u32>(out, 0);
			write_cast<u16>(out, sizeof(numeric));
			write_cast<u64>(out, length);
			write_cast<numeric>(out, cast<block<0>*>(value), cast<block<0>*>(value) + length);
			write_cast<u16>(out, sizeof(numeric));
			write_cast<u64>(out, length + length);
			write_cast<numeric>(out, cast<block<1>*>(value), cast<block<1>*>(value) + length); // accum
			write_cast<numeric>(out, cast<block<2>*>(value), cast<block<2>*>(value) + length); // updvu
			write_cast<u16>(out, 0);
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at ostream << weight, ";
			std::cerr << "use default (4) instead..." << std::endl;
			// no break
		case 4:
			write_cast<u32>(out, id);
			write_cast<u32>(out, 0);
			write_cast<u16>(out, sizeof(numeric));
			write_cast<u64>(out, length);
			write_cast<numeric>(out, cast<block<0>*>(value), cast<block<0>*>(value) + length);
			write_cast<u16>(out, 0);
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, weight& w) {
    	auto& id = w.id;
    	auto& length = w.length;
    	block<0>* value = nullptr;
    	block<1>* accum = nullptr;
    	block<2>* updvu = nullptr;
    	auto value_init = [&](size_t len) {
    		w.value = weight::alloc(len * 3);
    		value = cast<block<0>*>(w.value);
    		accum = cast<block<1>*>(w.value);
    		updvu = cast<block<2>*>(w.value);
    	};
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
			value_init(length);
			switch (code) {
			case 4: read_cast<f32>(in, value, value + length); break;
			case 8: read_cast<f64>(in, value, value + length); break;
			}
			break;
		case 127:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			value_init(length);
			read_cast<u16>(in, code);
			switch (code) {
			case 4:
				read_cast<f32>(in, value, value + length);
				read_cast<f32>(in, accum, accum + length);
				read_cast<f32>(in, updvu, updvu + length);
				break;
			case 8:
				read_cast<f64>(in, value, value + length);
				read_cast<f64>(in, accum, accum + length);
				read_cast<f64>(in, updvu, updvu + length);
				break;
			}
			break;
		case 128:
			read_cast<u32>(in, id);
			read_cast<u32>(in, code);
			read_cast<u16>(in, code);
			read_cast<u64>(in, length);
			value_init(length);
			switch (code) {
			case 4: read_cast<f32>(in, value, value + length); break;
			case 8: read_cast<f64>(in, value, value + length); break;
			}
			read_cast<u16>(in, code);
			in.ignore(8); // == length + length
			switch (code) {
			case 4:
				read_cast<f32>(in, accum, accum + length);
				read_cast<f32>(in, updvu, updvu + length);
				break;
			case 8:
				read_cast<f64>(in, accum, accum + length);
				read_cast<f64>(in, updvu, updvu + length);
				break;
			}
			while (read_cast<u16>(in, code) && code) {
				u64 skip; read_cast<u64>(in, skip);
				in.ignore(code * skip);
			}
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
			value_init(length);
			switch (code) {
			case 4: read_cast<f32>(in, value, value + length); break;
			case 8: read_cast<f64>(in, value, value + length); break;
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
				out << w, succ++;
			break;
		}
		out.flush();
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
				weight w; in >> w, succ++;
				wghts().push_back(w);
			}
			break;
		}
		return succ;
	}

	static weight& make(const u32& sign, const size_t& size) {
		wghts().push_back(weight(sign, size));
		return wghts().back();
	}
	typedef std::vector<weight>::iterator iter;
	static inline const std::vector<weight>& list() { return wghts(); }
	static inline iter begin() { return wghts().begin(); }
	static inline iter end()   { return wghts().end(); }
	static inline iter find(const u32& sign, const iter& first = begin(), const iter& last = end()) {
		return std::find_if(first, last, [=](const weight& w) { return w.sign() == sign; });
	}
	static inline weight& at(const u32& sign, const iter& first = begin(), const iter& last = end()) {
		const auto it = find(sign, first, last);
		if (it != last) return (*it);
		throw std::out_of_range("weight::at");
	}
	static inline iter erase(const iter& it) {
		if (it->value) free(it->value);
		return wghts().erase(it);
	}
	static inline std::vector<weight> transfer(const iter& first = begin(), const iter& last = end()) {
		std::vector<weight> ws(first, last);
		wghts().erase(first, last);
		return ws;
	}
private:
	weight(const u32& sign, const size_t& size) : id(sign), length(size), value(alloc(size * 3)) {}
	static inline std::vector<weight>& wghts() { static std::vector<weight> w; return w; }

	static inline numeric* alloc(const size_t& size) {
		return new numeric[size]();
	}
	static inline void free(numeric* v) {
		delete[] v;
	}

	template<size_t i>
	struct block {
		numeric value[3];
		inline operator numeric&()  { return value[i]; }
		inline operator f32() const { return value[i]; }
		inline operator f64() const { return value[i]; }
		inline numeric& operator =(const f32& f) { value[i] = f; return value[i]; }
		inline numeric& operator =(const f64& f) { value[i] = f; return value[i]; }
	};

	u32 id;
	size_t length;
	numeric* value;
};

class indexer {
public:
	indexer() : id(0), map(nullptr) {}
	indexer(const indexer& i) = default;
	~indexer() {}

	typedef std::function<u64(const board&)> mapper;

	inline u64 sign() const { return id; }
	inline mapper index() const { return map; }
	inline u64 operator ()(const board& b) const { return map(b); }

	inline bool operator ==(const indexer& i) const { return id == i.id; }
	inline bool operator !=(const indexer& i) const { return id != i.id; }

	static indexer& make(const u32& sign, mapper map) {
		idxrs().push_back(indexer(sign, map));
		return idxrs().back();
	}
	typedef std::vector<indexer>::iterator iter;
	static inline const std::vector<indexer>& list() { return idxrs(); }
	static inline iter begin() { return idxrs().begin(); }
	static inline iter end() { return idxrs().end(); }
	static inline iter find(const u32& sign, const iter& first = begin(), const iter& last = end()) {
		return std::find_if(first, last, [=](const indexer& i) { return i.sign() == sign; });
	}
	static inline indexer& at(const u32& sign, const iter& first = begin(), const iter& last = end()) {
		const auto it = find(sign, first, last);
		if (it != last) return (*it);
		throw std::out_of_range("indexer::at");
	}
private:
	indexer(const u32& sign, mapper map) : id(sign), map(map) {}
	static inline std::vector<indexer>& idxrs() { static std::vector<indexer> i; return i; }

	u32 id;
	mapper map;
};

class feature {
public:
	feature() {}
	feature(const feature& t) = default;
	~feature() {}

	inline u64 sign() const { return (value.sign() << 32) | index.sign(); }
	inline numeric& operator [](const board& b) { return value[index(b)]; }
	inline numeric& operator [](const u64& idx) { return value[idx]; }
	inline u64 operator ()(const board& b) const { return index(b); }

	inline operator indexer() const { return index; }
	inline operator weight() const { return value; }

	inline bool operator ==(const feature& f) const { return sign() == f.sign(); }
	inline bool operator !=(const feature& f) const { return sign() != f.sign(); }

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
			for (feature f : feature::list())
				out << f, succ++;
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::save" << std::endl;
			break;
		}
		out.flush();
		return succ;
	}
	static u32 load(std::istream& in) {
		u32 succ = 0;
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
			for (read_cast<u32>(in, code); code; code--) {
				feature f; in >> f, succ++;
				feats().push_back(f);
			}
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::load" << std::endl;
			break;
		}
		return succ;
	}

	static feature& make(const u32& wgt, const u32& idx) {
		feats().push_back(feature(weight::at(wgt), indexer::at(idx)));
		return feats().back();
	}
	typedef std::vector<feature>::iterator iter;
	static inline const std::vector<feature>& list() { return feats(); }
	static inline iter begin() { return feats().begin(); }
	static inline iter end()   { return feats().end(); }
	static inline iter find(const u32& wght, const u32& idxr, const iter& first = begin(), const iter& last = end()) {
		return std::find_if(first, last,
			[=](const feature& f) { return weight(f).sign() == wght && indexer(f).sign() == idxr; });
	}
	static feature& at(const u32& wgt, const u32& idx, const iter& first = begin(), const iter& last = end()) {
		const auto it = find(wgt, idx, first, last);
		if (it != last) return (*it);
		throw std::out_of_range("feature::at");
	}
	static inline iter erase(const iter& it) { return feats().erase(it); }
	static inline std::vector<feature> transfer(const iter& first = begin(), const iter& last = end()) {
		std::vector<feature> ft(first, last);
		feats().erase(first, last);
		return ft;
	}

private:
	feature(const weight& value, const indexer& index) : index(index), value(value) {}
	static inline std::vector<feature>& feats() { static std::vector<feature> f; return f; }

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
		opinion& operator =(const numeric& v) { return operator =(std::to_string(v)); }
		opinion& operator =(const std::string& v) { token = v.size() ? (label() + "=" + v) : label(); return (*this); }
		opinion& operator =(const vector& vec) { return operator  =(vtos(vec)); }
		bool operator ==(const std::string& v) const { return value() == v; }
		bool operator !=(const std::string& v) const { return value() != v; }
		bool operator ()(const std::string& v) const { return value().find(v) != std::string::npos; }
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
		option& operator  =(const numeric& v) { return operator =(std::to_string(v)); }
		option& operator  =(const std::string& v) { clear(); return operator +=(v); }
		option& operator +=(const std::string& v) { push_back(v); return *this; }
		option& operator  =(const vector& vec) { clear(); return operator +=(vec); }
		option& operator +=(const vector& vec) { insert(end(), vec.begin(), vec.end()); return *this; }
		bool operator ==(const std::string& v) const { return value() == v; }
		bool operator !=(const std::string& v) const { return value() != v; }

		bool operator ()(const std::string& ext) const {
			return std::find_if(cbegin(), cend(), std::bind(opinion::comp, std::placeholders::_1, ext)) != cend();
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
		return operator ()(opt) ? const_cast<options&>(*this)[opt](ext) : false;
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

	static std::vector<std::string> stov(const std::string& str) {
		std::stringstream ss(str);
		std::istream_iterator<std::string> begin(ss), end;
		return std::vector<std::string>(begin, end);
	}

	static std::string vtos(const vector& vec) {
		std::string str = std::accumulate(vec.cbegin(), vec.cend(), std::string(),
		    [](std::string& r, const std::string& v){ return std::move(r) + v + " "; });
		if (str.size()) str.pop_back();
		return str;
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
	return (board::lookup(h0).left.mono) | (board::lookup(h1).left.mono << 12);
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
	auto imake = [&](u32 sign, indexer::mapper func) {
		if (indexer::find(sign) != indexer::end()) return;
		indexer::make(sign, func); succ++;
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
	imake(0x00012345, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	imake(0x0037bf26, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	imake(0x00fedcba, utils::index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	imake(0x00c840d9, utils::index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	imake(0x00321076, utils::index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	imake(0x00fb73ea, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	imake(0x00cdef89, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	imake(0x00048c15, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	imake(0x00456789, utils::index6t<0x4,0x5,0x6,0x7,0x8,0x9>);
	imake(0x0026ae15, utils::index6t<0x2,0x6,0xa,0xe,0x1,0x5>);
	imake(0x00ba9876, utils::index6t<0xb,0xa,0x9,0x8,0x7,0x6>);
	imake(0x00d951ea, utils::index6t<0xd,0x9,0x5,0x1,0xe,0xa>);
	imake(0x007654ba, utils::index6t<0x7,0x6,0x5,0x4,0xb,0xa>);
	imake(0x00ea62d9, utils::index6t<0xe,0xa,0x6,0x2,0xd,0x9>);
	imake(0x0089ab45, utils::index6t<0x8,0x9,0xa,0xb,0x4,0x5>);
	imake(0x00159d26, utils::index6t<0x1,0x5,0x9,0xd,0x2,0x6>);
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
	imake(0x00123567, utils::index6t<0x1,0x2,0x3,0x5,0x6,0x7>);
	imake(0x007bf6ae, utils::index6t<0x7,0xb,0xf,0x6,0xa,0xe>);
	imake(0x00edca98, utils::index6t<0xe,0xd,0xc,0xa,0x9,0x8>);
	imake(0x00840951, utils::index6t<0x8,0x4,0x0,0x9,0x5,0x1>);
	imake(0x00210654, utils::index6t<0x2,0x1,0x0,0x6,0x5,0x4>);
	imake(0x00b73a62, utils::index6t<0xb,0x7,0x3,0xa,0x6,0x2>);
	imake(0x00def9ab, utils::index6t<0xd,0xe,0xf,0x9,0xa,0xb>);
	imake(0x0048c59d, utils::index6t<0x4,0x8,0xc,0x5,0x9,0xd>);
	imake(0x005679ab, utils::index6t<0x5,0x6,0x7,0x9,0xa,0xb>);
	imake(0x006ae59d, utils::index6t<0x6,0xa,0xe,0x5,0x9,0xd>);
	imake(0x00a98654, utils::index6t<0xa,0x9,0x8,0x6,0x5,0x4>);
	imake(0x00951a62, utils::index6t<0x9,0x5,0x1,0xa,0x6,0x2>);
	imake(0x00654a98, utils::index6t<0x6,0x5,0x4,0xa,0x9,0x8>);
	imake(0x00a62951, utils::index6t<0xa,0x6,0x2,0x9,0x5,0x1>);
	imake(0x009ab567, utils::index6t<0x9,0xa,0xb,0x5,0x6,0x7>);
	imake(0x0059d6ae, utils::index6t<0x5,0x9,0xd,0x6,0xa,0xe>);
	imake(0x0001237b, utils::index6t<0x0,0x1,0x2,0x3,0x7,0xb>);
	imake(0x0037bfed, utils::index6t<0x3,0x7,0xb,0xf,0xe,0xd>);
	imake(0x00fedc84, utils::index6t<0xf,0xe,0xd,0xc,0x8,0x4>);
	imake(0x00c84012, utils::index6t<0xc,0x8,0x4,0x0,0x1,0x2>);
	imake(0x00321048, utils::index6t<0x3,0x2,0x1,0x0,0x4,0x8>);
	imake(0x00fb7321, utils::index6t<0xf,0xb,0x7,0x3,0x2,0x1>);
	imake(0x00cdefb7, utils::index6t<0xc,0xd,0xe,0xf,0xb,0x7>);
	imake(0x00048cde, utils::index6t<0x0,0x4,0x8,0xc,0xd,0xe>);
	imake(0x00012348, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x8>);
	imake(0x0037bf21, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x1>);
	imake(0x00fedcb7, utils::index6t<0xf,0xe,0xd,0xc,0xb,0x7>);
	imake(0x00c840de, utils::index6t<0xc,0x8,0x4,0x0,0xd,0xe>);
	imake(0x0032107b, utils::index6t<0x3,0x2,0x1,0x0,0x7,0xb>);
	imake(0x00fb73ed, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xd>);
	imake(0x00cdef84, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x4>);
	imake(0x00048c12, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x2>);
	imake(0x0089abcd, utils::index6t<0x8,0x9,0xa,0xb,0xc,0xd>);
	imake(0x00159d04, utils::index6t<0x1,0x5,0x9,0xd,0x0,0x4>);
	imake(0x00765432, utils::index6t<0x7,0x6,0x5,0x4,0x3,0x2>);
	imake(0x00ea62fb, utils::index6t<0xe,0xa,0x6,0x2,0xf,0xb>);
	imake(0x00ba98fe, utils::index6t<0xb,0xa,0x9,0x8,0xf,0xe>);
	imake(0x00d951c8, utils::index6t<0xd,0x9,0x5,0x1,0xc,0x8>);
	imake(0x00456701, utils::index6t<0x4,0x5,0x6,0x7,0x0,0x1>);
	imake(0x0026ae37, utils::index6t<0x2,0x6,0xa,0xe,0x3,0x7>);

	// k.matsuzaki
	imake(0x00012456, utils::index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	imake(0x0037b26a, utils::index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	imake(0x00fedba9, utils::index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	imake(0x00c84d95, utils::index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	imake(0x00321765, utils::index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	imake(0x00fb7ea6, utils::index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	imake(0x00cde89a, utils::index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	imake(0x00048159, utils::index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	imake(0x0012569d, utils::index6t<0x1,0x2,0x5,0x6,0x9,0xd>);
	imake(0x007b6a54, utils::index6t<0x7,0xb,0x6,0xa,0x5,0x4>);
	imake(0x00eda962, utils::index6t<0xe,0xd,0xa,0x9,0x6,0x2>);
	imake(0x008495ab, utils::index6t<0x8,0x4,0x9,0x5,0xa,0xb>);
	imake(0x002165ae, utils::index6t<0x2,0x1,0x6,0x5,0xa,0xe>);
	imake(0x00b7a698, utils::index6t<0xb,0x7,0xa,0x6,0x9,0x8>);
	imake(0x00de9a51, utils::index6t<0xd,0xe,0x9,0xa,0x5,0x1>);
	imake(0x00485967, utils::index6t<0x4,0x8,0x5,0x9,0x6,0x7>);
	imake(0x00012345, utils::index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	imake(0x0037bf26, utils::index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	imake(0x00fedcba, utils::index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	imake(0x00c840d9, utils::index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	imake(0x00321076, utils::index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	imake(0x00fb73ea, utils::index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	imake(0x00cdef89, utils::index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	imake(0x00048c15, utils::index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	imake(0x0001567a, utils::index6t<0x0,0x1,0x5,0x6,0x7,0xa>);
	imake(0x00376ae9, utils::index6t<0x3,0x7,0x6,0xa,0xe,0x9>);
	imake(0x00fea985, utils::index6t<0xf,0xe,0xa,0x9,0x8,0x5>);
	imake(0x00c89516, utils::index6t<0xc,0x8,0x9,0x5,0x1,0x6>);
	imake(0x00326549, utils::index6t<0x3,0x2,0x6,0x5,0x4,0x9>);
	imake(0x00fba625, utils::index6t<0xf,0xb,0xa,0x6,0x2,0x5>);
	imake(0x00cd9ab6, utils::index6t<0xc,0xd,0x9,0xa,0xb,0x6>);
	imake(0x000459da, utils::index6t<0x0,0x4,0x5,0x9,0xd,0xa>);
	imake(0x0001259a, utils::index6t<0x0,0x1,0x2,0x5,0x9,0xa>);
	imake(0x0037b659, utils::index6t<0x3,0x7,0xb,0x6,0x5,0x9>);
	imake(0x00feda65, utils::index6t<0xf,0xe,0xd,0xa,0x6,0x5>);
	imake(0x00c849a6, utils::index6t<0xc,0x8,0x4,0x9,0xa,0x6>);
	imake(0x003216a9, utils::index6t<0x3,0x2,0x1,0x6,0xa,0x9>);
	imake(0x00fb7a95, utils::index6t<0xf,0xb,0x7,0xa,0x9,0x5>);
	imake(0x00cde956, utils::index6t<0xc,0xd,0xe,0x9,0x5,0x6>);
	imake(0x0004856a, utils::index6t<0x0,0x4,0x8,0x5,0x6,0xa>);
	imake(0x000159de, utils::index6t<0x0,0x1,0x5,0x9,0xd,0xe>);
	imake(0x00376548, utils::index6t<0x3,0x7,0x6,0x5,0x4,0x8>);
	imake(0x00fea621, utils::index6t<0xf,0xe,0xa,0x6,0x2,0x1>);
	imake(0x00c89ab7, utils::index6t<0xc,0x8,0x9,0xa,0xb,0x7>);
	imake(0x00326aed, utils::index6t<0x3,0x2,0x6,0xa,0xe,0xd>);
	imake(0x00fba984, utils::index6t<0xf,0xb,0xa,0x9,0x8,0x4>);
	imake(0x00cd9512, utils::index6t<0xc,0xd,0x9,0x5,0x1,0x2>);
	imake(0x0004567b, utils::index6t<0x0,0x4,0x5,0x6,0x7,0xb>);
	imake(0x0001589d, utils::index6t<0x0,0x1,0x5,0x8,0x9,0xd>);
	imake(0x00376154, utils::index6t<0x3,0x7,0x6,0x1,0x5,0x4>);
	imake(0x00fea762, utils::index6t<0xf,0xe,0xa,0x7,0x6,0x2>);
	imake(0x00c89eab, utils::index6t<0xc,0x8,0x9,0xe,0xa,0xb>);
	imake(0x00326bae, utils::index6t<0x3,0x2,0x6,0xb,0xa,0xe>);
	imake(0x00fbad98, utils::index6t<0xf,0xb,0xa,0xd,0x9,0x8>);
	imake(0x00cd9451, utils::index6t<0xc,0xd,0x9,0x4,0x5,0x1>);
	imake(0x00045267, utils::index6t<0x0,0x4,0x5,0x2,0x6,0x7>);
	imake(0x0001246a, utils::index6t<0x0,0x1,0x2,0x4,0x6,0xa>);
	imake(0x0037b2a9, utils::index6t<0x3,0x7,0xb,0x2,0xa,0x9>);
	imake(0x00fedb95, utils::index6t<0xf,0xe,0xd,0xb,0x9,0x5>);
	imake(0x00c84d56, utils::index6t<0xc,0x8,0x4,0xd,0x5,0x6>);
	imake(0x00321759, utils::index6t<0x3,0x2,0x1,0x7,0x5,0x9>);
	imake(0x00fb7e65, utils::index6t<0xf,0xb,0x7,0xe,0x6,0x5>);
	imake(0x00cde8a6, utils::index6t<0xc,0xd,0xe,0x8,0xa,0x6>);
	imake(0x0004819a, utils::index6t<0x0,0x4,0x8,0x1,0x9,0xa>);

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
	imake(0x00123567, utils::index7t<0x0,0x1,0x2,0x3,0x5,0x6,0x7>);
	imake(0x037bf6ae, utils::index7t<0x3,0x7,0xb,0xf,0x6,0xa,0xe>);
	imake(0x0fedca98, utils::index7t<0xf,0xe,0xd,0xc,0xa,0x9,0x8>);
	imake(0x0c840951, utils::index7t<0xc,0x8,0x4,0x0,0x9,0x5,0x1>);
	imake(0x03210654, utils::index7t<0x3,0x2,0x1,0x0,0x6,0x5,0x4>);
	imake(0x0fb73a62, utils::index7t<0xf,0xb,0x7,0x3,0xa,0x6,0x2>);
	imake(0x0cdef9ab, utils::index7t<0xc,0xd,0xe,0xf,0x9,0xa,0xb>);
	imake(0x0048c59d, utils::index7t<0x0,0x4,0x8,0xc,0x5,0x9,0xd>);
	imake(0x045679ab, utils::index7t<0x4,0x5,0x6,0x7,0x9,0xa,0xb>);
	imake(0x026ae59d, utils::index7t<0x2,0x6,0xa,0xe,0x5,0x9,0xd>);
	imake(0x0ba98654, utils::index7t<0xb,0xa,0x9,0x8,0x6,0x5,0x4>);
	imake(0x0d951a62, utils::index7t<0xd,0x9,0x5,0x1,0xa,0x6,0x2>);
	imake(0x07654a98, utils::index7t<0x7,0x6,0x5,0x4,0xa,0x9,0x8>);
	imake(0x0ea62951, utils::index7t<0xe,0xa,0x6,0x2,0x9,0x5,0x1>);
	imake(0x089ab567, utils::index7t<0x8,0x9,0xa,0xb,0x5,0x6,0x7>);
	imake(0x0159d6ae, utils::index7t<0x1,0x5,0x9,0xd,0x6,0xa,0xe>);
	imake(0x001237bf, utils::index7t<0x0,0x1,0x2,0x3,0x7,0xb,0xf>);
	imake(0x037bfedc, utils::index7t<0x3,0x7,0xb,0xf,0xe,0xd,0xc>);
	imake(0x0fedc840, utils::index7t<0xf,0xe,0xd,0xc,0x8,0x4,0x0>);
	imake(0x0c840123, utils::index7t<0xc,0x8,0x4,0x0,0x1,0x2,0x3>);
	imake(0x0321048c, utils::index7t<0x3,0x2,0x1,0x0,0x4,0x8,0xc>);
	imake(0x0fb73210, utils::index7t<0xf,0xb,0x7,0x3,0x2,0x1,0x0>);
	imake(0x0cdefb73, utils::index7t<0xc,0xd,0xe,0xf,0xb,0x7,0x3>);
	imake(0x0048cdef, utils::index7t<0x0,0x4,0x8,0xc,0xd,0xe,0xf>);
	imake(0x0012348c, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x8,0xc>);
	imake(0x037bf210, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x1,0x0>);
	imake(0x0fedcb73, utils::index7t<0xf,0xe,0xd,0xc,0xb,0x7,0x3>);
	imake(0x0c840def, utils::index7t<0xc,0x8,0x4,0x0,0xd,0xe,0xf>);
	imake(0x032107bf, utils::index7t<0x3,0x2,0x1,0x0,0x7,0xb,0xf>);
	imake(0x0fb73edc, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xd,0xc>);
	imake(0x0cdef840, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x4,0x0>);
	imake(0x0048c123, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x2,0x3>);
	imake(0x00123458, utils::index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x8>);
	imake(0x037bf261, utils::index7t<0x3,0x7,0xb,0xf,0x2,0x6,0x1>);
	imake(0x0fedcba7, utils::index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x7>);
	imake(0x0c840d9e, utils::index7t<0xc,0x8,0x4,0x0,0xd,0x9,0xe>);
	imake(0x0321076b, utils::index7t<0x3,0x2,0x1,0x0,0x7,0x6,0xb>);
	imake(0x0fb73ead, utils::index7t<0xf,0xb,0x7,0x3,0xe,0xa,0xd>);
	imake(0x0cdef894, utils::index7t<0xc,0xd,0xe,0xf,0x8,0x9,0x4>);
	imake(0x0048c152, utils::index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x2>);
	imake(0x0012367b, utils::index7t<0x0,0x1,0x2,0x3,0x6,0x7,0xb>);
	imake(0x037bfaed, utils::index7t<0x3,0x7,0xb,0xf,0xa,0xe,0xd>);
	imake(0x0fedc984, utils::index7t<0xf,0xe,0xd,0xc,0x9,0x8,0x4>);
	imake(0x0c840512, utils::index7t<0xc,0x8,0x4,0x0,0x5,0x1,0x2>);
	imake(0x03210548, utils::index7t<0x3,0x2,0x1,0x0,0x5,0x4,0x8>);
	imake(0x0fb73621, utils::index7t<0xf,0xb,0x7,0x3,0x6,0x2,0x1>);
	imake(0x0cdefab7, utils::index7t<0xc,0xd,0xe,0xf,0xa,0xb,0x7>);
	imake(0x0048c9de, utils::index7t<0x0,0x4,0x8,0xc,0x9,0xd,0xe>);
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
	imake(0xfe000003, utils::indexnum3);
	imake(0xfe000004, utils::indexnum4);
	imake(0xfe000005, utils::indexnum5lt);
	imake(0xfe000015, utils::indexnum5st);
	imake(0xfd012301, utils::indexmono<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	imake(0xfd37bf01, utils::indexmono<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	imake(0xfdfedc01, utils::indexmono<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	imake(0xfdc84001, utils::indexmono<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	imake(0xfd321001, utils::indexmono<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	imake(0xfdfb7301, utils::indexmono<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	imake(0xfdcdef01, utils::indexmono<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	imake(0xfd048c01, utils::indexmono<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	imake(0xfd456701, utils::indexmono<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	imake(0xfd26ae01, utils::indexmono<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	imake(0xfdba9801, utils::indexmono<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	imake(0xfdd95101, utils::indexmono<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	imake(0xfd765401, utils::indexmono<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	imake(0xfdea6201, utils::indexmono<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	imake(0xfd89ab01, utils::indexmono<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	imake(0xfd159d01, utils::indexmono<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	imake(0xfc000000, utils::indexmax<0>);
	imake(0xfc000010, utils::indexmax<1>);
	imake(0xfc000020, utils::indexmax<2>);
	imake(0xfc000030, utils::indexmax<3>);
	imake(0xfc000040, utils::indexmax<4>);
	imake(0xfc000050, utils::indexmax<5>);
	imake(0xfc000060, utils::indexmax<6>);
	imake(0xfc000070, utils::indexmax<7>);
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
	u32 succ = 0;
	auto wmake = [&](u32 sign, u64 size) {
		if (weight::find(sign) != weight::end()) return;
		weight::make(sign, size); succ++;
	};

	std::map<std::string, std::string> alias;
	alias["khyeh"] = "012345:patt 456789:patt 012456:patt 45689a:patt ";
	alias["patt/42-33"] = "012345:patt 456789:patt 89abcd:patt 012456:patt 45689a:patt ";
	alias["patt/4-22"] = "0123:patt 4567:patt 0145:patt 1256:patt 569a:patt ";
	alias["k.matsuzaki"] = "012456:? 12569d:? 012345:? 01567a:? 01259a:? 0159de:? 01589d:? 01246a:? ";
	alias["monotonic"] = "fd012301:^24 fd456701:^24 ";
	alias["default"] = alias["khyeh"] + alias["monotonic"] + "fe000005:^24 fe000015:^24 ";
	alias["4x6patt"] = alias["khyeh"];
	alias["5x6patt"] = alias["patt/42-33"];
	alias["8x6patt"] = alias["k.matsuzaki"];
	alias["5x4patt"] = alias["patt/4-22"];

	// weight:size weight(size) weight[size] weight:patt weight:? weight:^bit
	if (res.empty() && weight::list().empty())
		res = { "default" };
	for (auto predef : alias)
		if (res.find(predef.first) != std::string::npos) { // insert predefined weights
			res.insert(res.find(predef.first), predef.second);
			res.replace(res.find(predef.first), predef.first.size(), "");
		}
	while (res.find_first_of(":|()[],") != std::string::npos)
		res[res.find_first_of(":|()[],")] = ' ';

	std::stringstream in(res);
	std::string signs, sizes;
	while (in >> signs && in >> sizes) {
		u32 sign = 0; u64 size = 0;
		std::stringstream(signs) >> std::hex >> sign;
		if (sizes == "patt" || sizes == "?") {
			size = std::pow(16ull, signs.size());
		} else if (sizes.front() == '^') {
			size = std::pow(2ull, std::stol(sizes.substr(1)));
		} else {
			std::stringstream(sizes) >> std::dec >> size;
		}
		wmake(sign, size);
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
	u32 succ = 0;
	auto fmake = [&](u32 wght, u32 idxr) {
		if (feature::find(wght, idxr) != feature::end()) return;
		feature::make(wght, idxr); succ++;
	};

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
	alias["default"] = alias["khyeh"] + alias["monotonic"] + "fe000005[fe000005] fe000015[fe000015] ";
	alias["4x6patt"] = alias["khyeh"];
	alias["5x6patt"] = alias["patt/42-33"];
	alias["8x6patt"] = alias["k.matsuzaki"];
	alias["5x4patt"] = alias["patt/4-22"];
	alias["mono"] = alias["monotonic"];

	// weight:indexer weight(indexer) weight[indexer]
	if (res.empty() && feature::list().empty())
		res = { "default" };
	for (auto predef : alias)
		if (res.find(predef.first) != std::string::npos) { // insert predefined features
			res.insert(res.find(predef.first), predef.second);
			res.replace(res.find(predef.first), predef.first.size(), "");
		}
	while (res.find_first_of(":|()[],") != std::string::npos)
		res[res.find_first_of(":|()[],")] = ' ';

	std::stringstream in(res);
	std::string wghts, idxrs;
	while (in >> wghts && in >> idxrs) {
		u32 wght = 0, idxr = 0;

		std::stringstream(wghts) >> std::hex >> wght;
		if (weight::find(wght) == weight::end()) {
			std::cerr << "unknown weight (" << wghts << ") at make_features, ";
			std::cerr << "assume as pattern descriptor..." << std::endl;
			weight::make(wght, std::pow(16ull, wghts.size()));
		}

		std::vector<int> isomorphic = { 0 };
		for (; !std::isxdigit(idxrs.back()); idxrs.pop_back()) {
			if (idxrs.back() == '!') isomorphic = { 0, 1, 2, 3, 4, 5, 6, 7 };
		}
		for (int iso : isomorphic) {
			auto xpatt = utils::hashpatt(idxrs);
			board x(0xfedcba9876543210ull);
			x.isomorphic(-iso);
			for (size_t i = 0; i < xpatt.size(); i++)
				xpatt[i] = x[xpatt[i]];
			idxr = utils::hashpatt(xpatt);
			if (indexer::find(idxr) == indexer::end()) {
				std::cerr << "unknown indexer (" << idxrs << ") at make_features, ";
				std::cerr << "assume as pattern descriptor..." << std::endl;
				auto patt = new std::vector<int>(xpatt); // will NOT be deleted
				indexer::make(idxr, std::bind(utils::indexnta, std::placeholders::_1, std::cref(*patt)));
			}
			fmake(wght, idxr);
		}
	}
	return succ;
}

void list_mapping() {
	for (weight w : std::vector<weight>(weight::list())) {
		char buf[64];
		std::string feats;
		for (feature f : feature::list()) {
			if (weight(f) == w) {
				snprintf(buf, sizeof(buf), " %08llx", indexer(f).sign());
				feats += buf;
			}
		}
		if (feats.size()) {
			u32 usageK = (sizeof(numeric) * w.size()) >> 10;
			u32 usageM = usageK >> 10;
			u32 usageG = usageM >> 10;
			u32 usage = usageG ? usageG : (usageM ? usageM : usageK);
			char scale = usageG ? 'G' : (usageM ? 'M' : 'K');
			snprintf(buf, sizeof(buf), "weight(%08llx)[%llu] = %d%c x3", w.sign(), w.size(), usage, scale);
			std::cout << buf << " :" << feats << std::endl;
		} else {
			snprintf(buf, sizeof(buf), "%08llx", w.sign());
			std::cerr << "unused weight (" << buf << ") at list_mapping" << std::endl;
			weight::erase(weight::find(w.sign()));
		}
	}
}



inline numeric estimate(const board& state,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	register numeric esti = 0;
	for (auto f = begin; f != end; f++)
		esti += (*f)[state];
	return esti;
}

inline numeric update(const board& state, const numeric& alpha, const numeric& error,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	register numeric esti = 0;
	for (auto f = begin; f != end; f++) {
		u64 idx = (*f)(state);
		esti += weight(*f)(idx, alpha, error);
	}
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

	inline void assign(const board& b) {
		move = b;
		score = (move.*oper)();
	}

	inline numeric value() const { return esti - score; }
	inline numeric reward() const { return score; }

	inline numeric estimate(
			const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
		if (score >= 0) {
			esti = state::reward() + utils::estimate(move, begin, end);
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
		return esti;
	}
	inline numeric update(const numeric& accu, const numeric& alpha = state::alpha(),
			const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
//		esti = state::reward() + utils::update(move, alpha * (accu - state::value()), begin, end);
		esti = state::reward() + utils::update(move, alpha, accu - state::value(), begin, end);
		return esti;
	}

	inline void operator <<(const board& b) {
		assign(b);
		estimate();
	}
	inline numeric operator +=(const numeric& v) {
		return update(v);
	}
	inline numeric operator +=(const state& s) {
		return update(s.esti);
	}

	inline void operator >>(board& b) const { b = move; }
	inline bool operator >(const state& s) const { return esti > s.esti; }
	inline operator bool() const { return score >= 0; }
	inline operator board() const { return move; }

	void operator >>(std::ostream& out) const {
		move >> out;
		moporgic::write(out, score);
	}
	void operator <<(std::istream& in) {
		move << in;
		moporgic::read(in, score);
	}

	inline static numeric& alpha() {
		static numeric a = numeric(0.01);
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
		return update_ordered();
	}
	inline select& update_ordered() {
		best = move;
		if (move[1] > *best) best = move + 1;
		if (move[2] > *best) best = move + 2;
		if (move[3] > *best) best = move + 3;
		return *this;
	}
	inline select& update_random() {
		const u32 i = std::rand() % 4;
		best = move + i;
		if (move[(i + 1) % 4] > *best) best = move + ((i + 1) % 4);
		if (move[(i + 2) % 4] > *best) best = move + ((i + 2) % 4);
		if (move[(i + 3) % 4] > *best) best = move + ((i + 3) % 4);
		return *this;
	}
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(state& s) const { s = (*best); }
	inline void operator >>(board& b) const { *best >> b; }
	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }

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
		indexf = "%0" + std::to_string(dec) + "llu/%0" + std::to_string(dec) + "llu %llums %.2fops";
		localf = "local: " + std::string(dec * 2 - 5, ' ') + "avg=%llu max=%u tile=%u win=%.2f%%";
		totalf = "total: " + std::string(dec * 2 - 5, ' ') + "avg=%llu max=%u tile=%u win=%.2f%%";
		summaf = "summary" + std::string(dec * 2 - 5, ' ') + "%llums %.2fops";
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
		every.count[std::log2(hash)] += 1;
		every.score[std::log2(hash)] += score;
		every.opers[std::log2(hash)] += opers;

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

	void summary() const {
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
			try {
				opts["seed"] = find_opt(i, "moporgic");
				std::stoull(opts["seed"]);
			} catch (std::invalid_argument&) {
				opts["seed"] = to_hash(opts["seed"]);
			}
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
		case to_hash("-o"):
		case to_hash("--option"):
		case to_hash("--options"):
		case to_hash("--extra"):
			opts["temporary"] = find_opts(i);
			opts["options"] += opts["temporary"];
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
	board b;
	state last;
	select best;
	statistic stats;
	std::vector<state> path;
	path.reserve(65536);
	u32 score;
	u32 opers;

	switch (to_hash(opts["train"]["mode"])) {
	case to_hash("backward"):
	case to_hash("backward-best"):
		for (stats.init(opts["train"]); stats; stats++) {

			score = 0;
			opers = 0;

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

			score = 0;
			opers = 0;

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

	return stats;
}

statistic test(utils::options opts = {}) {
	board b;
	select best;
	statistic stats;
	u32 score;
	u32 opers;

	switch (to_hash(opts["test"]["mode"])) {
	default:
	case to_hash("best"):
		for (stats.init(opts["test"]); stats; stats++) {

			score = 0;
			opers = 0;

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
	if (!opts("train")) opts["train"] = 1000;
	if (!opts("test")) opts["test"] = 1000;
	if (!opts("alpha")) opts["alpha"] = 0.01;
	if (!opts("seed")) opts["seed"] = rdtsc();
	if (!opts("test", "info")) opts["test"]["info"] = "summary";

	std::cout << "TDL2048+ LOG" << std::endl;
	std::cout << "develop-coherence" << " build C++" << __cplusplus;
	std::cout << " " << __DATE__ << " " << __TIME__ << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << moporgic::millisec() << std::endl;
	std::cout << "seed = " << std::stoull(opts["seed"]) << std::endl;
	std::cout << "alpha = " << std::stod(opts["alpha"]) << std::endl;
	std::cout << std::endl;

	std::srand(std::stoull(opts["seed"]));
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
		if (opts["train"]["info"]("summary"))
			stat.summary();
	}

	utils::save_weights(opts["weight-output"]);
	utils::save_features(opts["feature-output"]);

	if (statistic(opts["test"])) {
		std::cout << std::endl << "start testing..." << std::endl;
		statistic stat = test(opts);
		if (opts["test"]["info"]("summary"))
			stat.summary();
	}

	std::cout << std::endl;
	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
