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

	typedef moporgic::numeric numeric;
	struct segment {
		numeric value;
		numeric accum;
		numeric updvu;
		inline constexpr segment() : value(0), accum(0), updvu(0) {}
		inline constexpr segment(const segment& s) = default;
		inline constexpr operator numeric&() { return value; }
		inline constexpr operator numeric() const { return value; }
		inline constexpr segment& operator =(const segment& s) = default;
		inline constexpr numeric& operator =(numeric val) { return value = val; }
		inline constexpr numeric& operator +=(numeric aupdv) {
			value += aupdv * (updvu ? (std::abs(accum) / updvu) : 1);
			accum += aupdv;
			updvu += std::abs(aupdv);
			return value;
		}
	};

	inline u64 sign() const { return id; }
	inline size_t size() const { return length; }
	inline segment& operator [](u64 i) { return raw[i]; }
	inline segment* data(u64 i = 0) { return raw + i; }

	template<size_t i>
	struct block : std::array<numeric, sizeof(segment) / sizeof(numeric)> {
		block() = delete;
		inline operator numeric&() { return operator [](i); }
		inline operator const numeric&() const { return operator [](i); }
		inline numeric& operator =(numeric v) { return (operator [](i) = v); }
		declare_comparators_with(const numeric&, operator [](i), v, inline);
	};
	inline clip<block<0>> value() const { return { cast<block<0>*>(raw), cast<block<0>*>(raw) + length }; }
	inline clip<block<1>> accum() const { return { cast<block<1>*>(raw), cast<block<1>*>(raw) + length }; }
	inline clip<block<2>> updvu() const { return { cast<block<2>*>(raw), cast<block<2>*>(raw) + length }; }
	inline operator bool() const { return raw; }
	declare_comparators(const weight&, sign(), inline);

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		u32 code = 128;
		write_cast<u8>(out, code);
		switch (code) {
		case 128:
			write_cast<u32>(out, w.sign());
			write_cast<u32>(out, 0);
			write_cast<u16>(out, sizeof(weight::numeric));
			write_cast<u64>(out, w.size());
			write_cast<numeric>(out, w.value().begin(), w.value().end());
			write_cast<u16>(out, sizeof(weight::numeric));
			write_cast<u64>(out, w.size() + w.size());
			write_cast<numeric>(out, w.accum().begin(), w.accum().end());
			write_cast<numeric>(out, w.updvu().begin(), w.updvu().end());
			write_cast<u16>(out, 0);
			break;
		default:
		case 4:
			write_cast<u32>(out, w.sign());
			write_cast<u32>(out, 0);
			write_cast<u16>(out, sizeof(weight::numeric));
			write_cast<u64>(out, w.size());
			write_cast<numeric>(out, w.value().begin(), w.value().end());
			write_cast<u16>(out, 0);
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, weight& w) {
		auto& id = w.id;
		auto& length = w.length;
		auto& raw = w.raw;
		u32 code = 4;
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
			raw = weight::alloc(length);
			switch (code) {
			case 4: read_cast<f32>(in, w.value().begin(), w.value().end()); break;
			case 8: read_cast<f64>(in, w.value().begin(), w.value().end()); break;
			}
			std::fill(w.accum().begin(), w.accum().end(), numeric(0));
			std::fill(w.updvu().begin(), w.updvu().end(), numeric(0));
			break;
		case 127:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			raw = weight::alloc(length);
			read_cast<u16>(in, code);
			switch (code) {
			case 4:
				read_cast<f32>(in, w.value().begin(), w.value().end());
				read_cast<f32>(in, w.accum().begin(), w.accum().end());
				read_cast<f32>(in, w.updvu().begin(), w.updvu().end());
				break;
			case 8:
				read_cast<f64>(in, w.value().begin(), w.value().end());
				read_cast<f64>(in, w.accum().begin(), w.accum().end());
				read_cast<f64>(in, w.updvu().begin(), w.updvu().end());
				break;
			}
			break;
		case 128:
			read_cast<u32>(in, id);
			read_cast<u32>(in, code);
			read_cast<u16>(in, code);
			read_cast<u64>(in, length);
			raw = weight::alloc(length);
			switch (code) {
			case 2: read_cast<f16>(in, w.value().begin(), w.value().end()); break;
			case 4: read_cast<f32>(in, w.value().begin(), w.value().end()); break;
			case 8: read_cast<f64>(in, w.value().begin(), w.value().end()); break;
			}
			read_cast<u16>(in, code);
			in.ignore(8); // == length + length
			switch (code) {
			case 2: read_cast<f16>(in, w.accum().begin(), w.accum().end()); break;
			case 4: read_cast<f32>(in, w.accum().begin(), w.accum().end()); break;
			case 8: read_cast<f64>(in, w.accum().begin(), w.accum().end()); break;
			}
			switch (code) {
			case 2: read_cast<f16>(in, w.updvu().begin(), w.updvu().end()); break;
			case 4: read_cast<f32>(in, w.updvu().begin(), w.updvu().end()); break;
			case 8: read_cast<f64>(in, w.updvu().begin(), w.updvu().end()); break;
			}
			while (read_cast<u16>(in, code) && code) {
				u64 skip; read_cast<u64>(in, skip);
				in.ignore(code * skip);
			}
			break;
		default:
		case 4:
			read_cast<u32>(in, id);
			read_cast<u32>(in, code);
			read_cast<u16>(in, code);
			read_cast<u64>(in, length);
			raw = weight::alloc(length);
			switch (code) {
			case 2: read_cast<f16>(in, w.value().begin(), w.value().end()); break;
			case 4: read_cast<f32>(in, w.value().begin(), w.value().end()); break;
			case 8: read_cast<f64>(in, w.value().begin(), w.value().end()); break;
			}
			while (read_cast<u16>(in, code) && code)
				in.ignore(code * read<u64>(in));
			break;
		}
		return in;
	}

	static void save(std::ostream& out) {
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		default:
		case 0:
			write_cast<u32>(out, wghts().size());
			for (weight w : wghts()) out << w;
			break;
		}
	}
	static void load(std::istream& in) {
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		default:
		case 0:
			for (read_cast<u32>(in, code); code; code--)
				in >> list<weight>::as(wghts()).emplace_back();
			break;
		}
	}

	class container : public clip<weight> {
	public:
		constexpr container() noexcept : clip<weight>() {}
		constexpr container(const clip<weight>& w) : clip<weight>(w) {}
	public:
		weight& make(u64 sign, size_t size) { return list<weight>::as(*this).emplace_back(weight(sign, size)); }
		size_t erase(u64 sign) { auto it = find(sign); return it != end() ? free(it->data()), list<weight>::as(*this).erase(it), erase(sign) + 1 : 0; }
		weight* find(u64 sign) const { return std::find_if(begin(), end(), [=](const weight& w) { return w.sign() == sign; }); }
		weight& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("weight::at"); }
		weight& operator[](u64 sign) const { return (*find(sign)); }
		weight operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : weight(); }
	};

	static inline weight::container& wghts() { static container w; return w; }
	static inline weight& make(u64 sign, size_t size, container& src = wghts()) { return src.make(sign, size); }
	static inline size_t erase(u64 sign, container& src = wghts()) { return src.erase(sign); }
	inline weight(u64 sign, const container& src = wghts()) : weight(src(sign)) {}

private:
	inline weight(u64 sign, size_t size) : id(sign), length(size), raw(alloc(size)) {}

	static inline segment* alloc(size_t size) { return new segment[size](); }
	static inline void free(segment* v) { delete[] v; }

	u64 id;
	size_t length;
	segment* raw;
};

class indexer {
public:
	inline indexer() : id(0), map(nullptr) {}
	inline indexer(const indexer& i) = default;
	inline ~indexer() {}

	typedef u64(*mapper)(const board&);

	inline u64 sign() const { return id; }
	inline mapper index() const { return map; }
	inline u64 operator ()(const board& b) const { return (*map)(b); }
	inline operator bool() const { return map; }
	declare_comparators(const indexer&, sign(), inline);

	class container : public clip<indexer> {
	public:
		constexpr container() noexcept : clip<indexer>() {}
		constexpr container(const clip<indexer>& i) : clip<indexer>(i) {}
	public:
		indexer& make(u64 sign, mapper map) { return list<indexer>::as(*this).emplace_back(indexer(sign, map)); }
		size_t erase(u64 sign) { auto it = find(sign); return it != end() ? list<indexer>::as(*this).erase(it), erase(sign) + 1 : 0; }
		indexer* find(u64 sign) const { return std::find_if(begin(), end(), [=](const indexer& i) { return i.sign() == sign; }); }
		indexer& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("indexer::at"); }
		indexer& operator[](u64 sign) const { return (*find(sign)); }
		indexer operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : indexer(); }
	};

	static inline indexer::container& idxrs() { static container i; return i; }
	static inline indexer& make(u64 sign, mapper map, container& src = idxrs()) { return src.make(sign, map); }
	static inline size_t erase(u64 sign, container& src = idxrs()) { return src.erase(sign); }
	inline indexer(u64 sign, const container& src = idxrs()) : indexer(src(sign)) {}

private:
	inline indexer(u64 sign, mapper map) : id(sign), map(map) {}

	u64 id;
	mapper map;
};

class feature {
public:
	inline feature() {}
	inline feature(const feature& t) = default;
	inline ~feature() {}

	inline u64 sign() const { return (raw.sign() << 32) | map.sign(); }
	inline weight::segment& operator [](const board& b) { return raw[map(b)]; }
	inline weight::segment& operator [](u64 idx) { return raw[idx]; }
	inline u64 operator ()(const board& b) const { return map(b); }

	inline indexer index() const { return map; }
	inline weight  value() const { return raw; }
	inline operator indexer() const { return map; }
	inline operator weight()  const { return raw; }
	inline operator bool() const { return map && raw; }
	declare_comparators(const feature&, sign(), inline);

	friend std::ostream& operator <<(std::ostream& out, const feature& f) {
		auto& index = f.map;
		auto& value = f.raw;
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		default:
		case 0:
			write_cast<u32>(out, index.sign());
			write_cast<u32>(out, value.sign());
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, feature& f) {
		auto& index = f.map;
		auto& value = f.raw;
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		default:
		case 0:
			read_cast<u32>(in, code);
			index = indexer(code);
			read_cast<u32>(in, code);
			value = weight(code);
			break;
		}
		return in;
	}

	static void save(std::ostream& out) {
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		default:
		case 0:
			write_cast<u32>(out, feats().size());
			for (feature f : feats()) out << f;
			break;
		}
	}
	static void load(std::istream& in) {
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		default:
		case 0:
			for (read_cast<u32>(in, code); code; code--)
				in >> list<feature>::as(feats()).emplace_back();
			break;
		}
	}

	class container : public clip<feature> {
	public:
		constexpr container() noexcept : clip<feature>() {}
		constexpr container(const clip<feature>& f) : clip<feature>(f) {}
	public:
		feature& make(u64 wgt, u64 idx) { return list<feature>::as(*this).emplace_back(feature(weight(wgt), indexer(idx))); }
		feature& make(u64 sign) { return make(u32(sign >> 32), u32(sign)); }
		size_t erase(u64 wgt, u64 idx) { return erase((wgt << 32) | idx); }
		size_t erase(u64 sign) { auto it = find(sign); return it != end() ? list<feature>::as(*this).erase(it), erase(sign) + 1 : 0; }
		feature* find(u64 wgt, u64 idx) const { return find((wgt << 32) | idx); }
		feature* find(u64 sign) const { return std::find_if(begin(), end(), [=](const feature& f) { return f.sign() == sign; }); }
		feature& at(u64 wgt, u64 idx) const { return at((wgt << 32) | idx); }
		feature& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("feature::at"); }
		feature& operator[](u64 sign) const { return (*find(sign)); }
		feature operator()(u64 wgt, u64 idx) const { return operator()((wgt << 32) | idx); }
		feature operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : feature(); }
	};

	static inline feature::container& feats() { static container f; return f; }
	static inline feature& make(u64 wgt, u64 idx, container& src = feats()) { return src.make(wgt, idx); }
	static inline size_t erase(u64 wgt, u64 idx, container& src = feats()) { return src.erase(wgt, idx); }
	inline feature(u64 wgt, u64 idx, const container& src = feats()) : feature(src(wgt, idx)) {}

private:
	inline feature(const weight& value, const indexer& index) : map(index), raw(value) {}

	indexer map;
	weight raw;
};

namespace utils {

class options {
public:
	options() {}
	options(const options& opts) : opts(opts.opts) {}
	typedef std::list<std::string> list;

	class opinion {
		friend class option;
	public:
		opinion() = delete;
		opinion(const opinion& i) = default;
		opinion(std::string& token) : token(token) {}
		operator std::string() const { return value(); }
		operator numeric() const { return std::stod(value()); }
		std::string label() const { return token.substr(0, token.find('=')); }
		std::string value() const { return token.find('=') != std::string::npos ? token.substr(token.find('=') + 1) : ""; }
		std::string operator +(const std::string& val) { return value() + val; }
		friend std::ostream& operator <<(std::ostream& out, const opinion& i) { return out << i.value(); }
		opinion& operator  =(const opinion& opi) { return operator =(opi.value()); }
		opinion& operator  =(const numeric& val) { return operator =(ntos(val)); }
		opinion& operator  =(const std::string& val) { token = label() + (val.size() ? ("=" + val) : ""); return (*this); }
		opinion& operator +=(const std::string& val) { return operator =(value() + val); }
		opinion& operator  =(const list& vec) { return operator =(vtos(vec)); }
		opinion& operator +=(const list& vec) { return operator =(value() + vtos(vec)); }
		bool operator ==(const std::string& val) const { return value() == val; }
		bool operator !=(const std::string& val) const { return value() != val; }
		bool operator ()(const std::string& val) const { return value().find(val) != std::string::npos; }
		static bool comp(std::string token, std::string label) { return opinion(token).label() == label; }
	private:
		std::string& token;
	};

	class option : public list {
		friend class options;
	public:
		option(const list& opt = {}) : list(opt) {}
		operator std::string() const { return value(); }
		operator numeric() const { return std::stod(value()); }
		std::string value() const { return vtos(*this); }
		std::string operator +(const std::string& val) { return value() + val; }
		friend std::ostream& operator <<(std::ostream& out, const option& opt) { return out << opt.value(); }
		option& operator  =(const numeric& val) { return operator =(ntos(val)); }
		option& operator  =(const std::string& val) { clear(); return operator +=(val); }
		option& operator +=(const std::string& val) { push_back(val); return *this; }
		option& operator  =(const list& vec) { clear(); return operator +=(vec); }
		option& operator +=(const list& vec) { insert(end(), vec.begin(), vec.end()); return *this; }
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

	static std::string vtos(const list& vec) {
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

void logging(utils::options::option opt) {
	static std::ofstream logout;
	if (logout.is_open()) return;
	std::string path = opt.size() ? opt.back() : "";
	logout.open(path, std::ios::out | std::ios::app);
	if (logout.is_open()) {
		static moporgic::teestream tee(std::cout, logout);
		static moporgic::redirector redirect(tee, std::cout);
	}
}

inline u32 hashpatt(const std::vector<u32>& patt) {
	u32 hash = 0;
	for (auto tile : patt) hash = (hash << 4) | tile;
	return hash;
}
inline std::vector<u32> hashpatt(const std::string& hashs, int iso = 0) {
	u32 hash; std::stringstream(hashs) >> std::hex >> hash;
	std::vector<u32> patt(hashs.size());
	for (auto it = patt.rbegin(); it != patt.rend(); it++, hash >>= 4)
		(*it) = hash & 0x0f;
	std::transform(patt.begin(), patt.end(), patt.begin(), [=](u32 v) {
		board x(0xfedcba9876543210ull);
		x.isomorphic(-iso);
		return x.at(v);
	});
	return patt;
}
inline std::string hashpatt(u32 hash, size_t n = 0) {
	std::stringstream ss; ss << std::hex << hash;
	std::string patt = ss.str();
	return std::string(std::max(n, patt.size()) - patt.size(), '0') + patt;
}

template<u32 p0, u32 p1, u32 p2, u32 p3, u32 p4, u32 p5>
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
template<u32 p0, u32 p1, u32 p2, u32 p3>
u64 index4t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	return index;
}
template<u32 p0, u32 p1, u32 p2, u32 p3, u32 p4, u32 p5, u32 p6, u32 p7>
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
template<u32 p0, u32 p1, u32 p2, u32 p3, u32 p4, u32 p5, u32 p6>
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
template<u32 p0, u32 p1, u32 p2, u32 p3, u32 p4>
u64 index5t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
	index += b.at(p4) << 16;
	return index;
}

u64 indexnta(const board& b, const std::vector<u32>& p) {
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

template<u32 transpose>
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

template<u32 transpose, u32 qu0, u32 qu1>
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
	index |= std::min(u32(num[9] + num[10]), 7u) << 12; // 512+1024, 3-bit
	index |= std::min(u32(num[11]), 3u) << 15; // 2048~16384, 2-bit ea.
	index |= std::min(u32(num[12]), 3u) << 17;
	index |= std::min(u32(num[13]), 3u) << 19;
	index |= std::min(u32(num[14]), 3u) << 21;
	index |= std::min(u32(num[15]), 1u) << 23; // 32768, 1-bit
	return index;
}

u64 indexnum5lt(const board& b) { // 24-bit
	auto num = b.numof();
	register u64 index = 0;
	index |= std::min(u32(num[8]),  7u) <<  0; // 256, 3-bit
	index |= std::min(u32(num[9]),  7u) <<  3; // 512, 3-bit
	index |= std::min(u32(num[10]), 7u) <<  6; // 1024, 3-bit
	index |= std::min(u32(num[11]), 7u) <<  9; // 2048, 3-bit
	index |= std::min(u32(num[12]), 7u) << 12; // 4096, 3-bit
	index |= std::min(u32(num[13]), 7u) << 15; // 8192, 3-bit
	index |= std::min(u32(num[14]), 7u) << 18; // 16384, 3-bit
	index |= std::min(u32(num[15]), 7u) << 21; // 32768, 3-bit
	return index;
}

u64 indexnum5st(const board& b) { // 24-bit
	auto num = b.numof();
	register u64 index = 0;
	index |= std::min(u32(num[0]), 7u) <<  0; // 0, 3-bit
	index |= std::min(u32(num[1]), 7u) <<  3; // 2, 3-bit
	index |= std::min(u32(num[2]), 7u) <<  6; // 4, 3-bit
	index |= std::min(u32(num[3]), 7u) <<  9; // 8, 3-bit
	index |= std::min(u32(num[4]), 7u) << 12; // 16, 3-bit
	index |= std::min(u32(num[5]), 7u) << 15; // 32, 3-bit
	index |= std::min(u32(num[6]), 7u) << 18; // 64, 3-bit
	index |= std::min(u32(num[7]), 7u) << 21; // 128, 3-bit
	return index;
}

u64 indexnuma(const board& b, const std::vector<u32>& n) {
	auto num = b.numof();
	register u64 index = 0;
	register u32 offset = 0;
	for (u32 code : n) {
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

template<u32 p0, u32 p1, u32 p2, u32 p3, u32 p4, u32 p5, u32 p6, u32 p7>
u64 indexmono(const board& b) { // 24-bit
	u32 h0 = (b.at(p0)) | (b.at(p1) << 4) | (b.at(p2) << 8) | (b.at(p3) << 12);
	u32 h1 = (b.at(p4)) | (b.at(p5) << 4) | (b.at(p6) << 8) | (b.at(p7) << 12);
	return (board::cache::load(h0).left.mono) | (board::cache::load(h1).left.mono << 12);
}

template<u32 tile, u32 isomorphic>
u64 indexmask(const board& b) { // 16-bit
	board k = b;
	k.isomorphic(isomorphic);
	return k.mask(tile);
}

template<u32 isomorphic>
u64 indexmax(const board& b) { // 16-bit
	board k = b;
	k.isomorphic(isomorphic);
	return k.mask(k.max());
}

struct indexhdr {
	typedef indexer::mapper wrapper;
	typedef std::function<u64(const board&)> handler;
	typedef moporgic::list<wrapper> wrapper_list;
	typedef moporgic::list<handler> handler_list;
	static wrapper_list& wlist() { static wrapper_list w; return w; }
	static handler_list& hlist() { static handler_list h; return h; }

	operator wrapper() const { return wlist().front(); }
	indexhdr(handler hdr) { hlist().push_back(hdr); }
	~indexhdr() { wlist().pop_front(); }

	template<u32 idx>
	static u64 adapt(const board& b) { return hlist()[idx](b); }

	template<u32 idx, u32 lim>
	static void make() { make_wrappers<idx, lim>(); }

	template<u32 idx, u32 lim>
	struct make_wrappers {
		make_wrappers() { wlist().push_back(indexhdr::adapt<idx>); }
		~make_wrappers() { make_wrappers<idx + 1, lim>(); }
	};
	template<u32 idx>
	struct make_wrappers<idx, idx> {};

	static __attribute__((constructor)) void init() {
		auto make = [&](u64 sign, indexer::mapper func) {
			if (!indexer(sign)) indexer::make(sign, func);
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
		make(0x0123458c, utils::index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x8,0xc>);
		make(0xc840d9ef, utils::index8t<0xc,0x8,0x4,0x0,0xd,0x9,0xe,0xf>);
		make(0xfedcba73, utils::index8t<0xf,0xe,0xd,0xc,0xb,0xa,0x7,0x3>);
		make(0x37bf2610, utils::index8t<0x3,0x7,0xb,0xf,0x2,0x6,0x1,0x0>);
		make(0x321076bf, utils::index8t<0x3,0x2,0x1,0x0,0x7,0x6,0xb,0xf>);
		make(0xfb73eadc, utils::index8t<0xf,0xb,0x7,0x3,0xe,0xa,0xd,0xc>);
		make(0xcdef8940, utils::index8t<0xc,0xd,0xe,0xf,0x8,0x9,0x4,0x0>);
		make(0x048c1523, utils::index8t<0x0,0x4,0x8,0xc,0x1,0x5,0x2,0x3>);
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

		indexhdr::make<0, 256>();
	}
};

std::map<std::string, std::string> aliases() {
	std::map<std::string, std::string> alias;
	alias["4x6patt/khyeh"] = "012345:012345! 456789:456789! 012456:012456! 45689a:45689a! ";
	alias["khyeh"] = alias["4x6patt/khyeh"];
	alias["5x6patt/42-33"] = "012345:012345! 456789:456789! 89abcd:89abcd! 012456:012456! 45689a:45689a! ";
	alias["2x4patt/4"] = "0123:0123! 4567:4567! ";
	alias["5x4patt/4-22"] = alias["2x4patt/4"] + "0145:0145! 1256:1256! 569a:569a! ";
	alias["2x8patt/44"] = "01234567:01234567! 456789ab:456789ab! ";
	alias["3x8patt/44-4211"] = alias["2x8patt/44"] + "0123458c:0123458c! ";
//	alias["k.matsuzaki"] = "012456:012456! 12569d:12569d! 012345:012345! 01567a:01567a! 01259a:01259a! 0159de:0159de! 01589d:01589d! 01246a:01246a! ";
	alias["4x6patt/k.matsuzaki"] = "012456:012456! 456789:456789! 012345:012345! 234569:234569! ";
	alias["5x6patt/k.matsuzaki"] = alias["4x6patt/k.matsuzaki"] + "01259a:01259a! ";
	alias["6x6patt/k.matsuzaki"] = alias["5x6patt/k.matsuzaki"] + "345678:345678! ";
	alias["7x6patt/k.matsuzaki"] = alias["6x6patt/k.matsuzaki"] + "134567:134567! ";
	alias["8x6patt/k.matsuzaki"] = alias["7x6patt/k.matsuzaki"] + "01489a:01489a! ";
	alias["k.matsuzaki"] = alias["8x6patt/k.matsuzaki"];
	alias["monotonic"] = "fd012301[^24]:fd012301,fd37bf01,fdfedc01,fdc84001,fd321001,fdfb7301,fdcdef01,fd048c01 "
	                     "fd456701[^24]:fd456701,fd26ae01,fdba9801,fdd95101,fd765401,fdea6201,fd89ab01,fd159d01 ";
	alias["quantity"] = "fe000005[^24]:fe000005 fe000015[^24]:fe000015 ";
	alias["moporgic"] = alias["4x6patt/khyeh"] + alias["monotonic"] + alias["quantity"];
	alias["4x6patt"] = alias["4x6patt/khyeh"];
	alias["5x6patt"] = alias["5x6patt/42-33"];
	alias["6x6patt"] = alias["6x6patt/k.matsuzaki"];
	alias["7x6patt"] = alias["7x6patt/k.matsuzaki"];
	alias["8x6patt"] = alias["8x6patt/k.matsuzaki"];
	alias["5x4patt"] = alias["5x4patt/4-22"];
	alias["2x4patt"] = alias["2x4patt/4"];
	alias["2x8patt"] = alias["2x8patt/44"];
	alias["3x8patt"] = alias["3x8patt/44-4211"];
	alias["default"] = alias["4x6patt"];
	return alias;
}
u32 make_network(utils::options::option opt) {
	std::string tokens = opt;
	if (tokens.empty() && feature::feats().empty())
		tokens = "default";

	const auto npos = std::string::npos;
	for (size_t i; (i = tokens.find(" norm")) != npos; tokens[i] = '/');

	auto aliases = utils::aliases();
	std::stringstream unalias(tokens); tokens.clear();
	for (std::string token; unalias >> token; tokens += (token + ' ')) {
		if (token.find(':') != npos) continue;
		std::string name = token.substr(0, token.find_first_of("&|="));
		std::string info = token != name ? token.substr(name.size()) : "";
		if (aliases.find(name) != aliases.end()) token = aliases[name];
		if (info.empty()) continue;

		std::string winfo, iinfo, buff;
		for (char set : std::string("&|=")) {
			if (info.find(set) != npos) buff = info.substr(0, info.find_first_of("&|=", info.find(set) + 1)).substr(info.find(set));
			if (buff.find(set) != npos) winfo += buff.substr(0, buff.find(':'));
			if (buff.find(':') != npos) iinfo += buff.substr(buff.find(':') + 1).insert(0, 1, set);
		}
		std::stringstream rephrase(token); token.clear();
		for (std::string wtok, itok; rephrase >> buff; token += (wtok + itok + ' ')) {
			wtok = buff.substr(0, buff.find(':')) + winfo;
			itok = buff.find(':') != npos ? buff.substr(buff.find(':')) + iinfo : "";
		}
	}

	std::stringstream uncomma(tokens); tokens.clear();
	for (std::string token; uncomma >> token; tokens += (token + ' ')) {
		if (token.find(':') == npos || token.find(',') == npos) continue;
		for (size_t i; (i = token.find(',')) != npos; token[i] = ' ');
		std::stringstream lbuf(token.substr(0, token.find(':')));
		std::stringstream rbuf(token.substr(token.find(':') + 1));
		std::vector<std::string> lvals, rvals;
		for (std::string val; lbuf >> val; lvals.push_back(val));
		for (std::string val; rbuf >> val; rvals.push_back(val));
		token.clear();
		for (auto lval : lvals) for (auto rval : rvals) {
			token += (lval + ':' + rval + ' ');
			lval = lval.substr(0, lval.find('='));
		}
	}

	std::stringstream unisomorphic(tokens); tokens.clear();
	for (std::string token; unisomorphic >> token; tokens += (token + ' ')) {
		if (token.find('!') == npos) continue;
		std::vector<std::string> lvals, rvals;
		lvals.push_back(token.substr(0, token.find(':')));
		rvals.push_back(token.find(':') != npos ? token.substr(token.find(':')) : "");
		if (lvals.back().find('!') != npos) {
			std::string lval = lvals.back(); lvals.clear();
			std::string hash = lval.substr(0, lval.find('!'));
			std::string tail = lval.substr(lval.find('!') + 1);
			for (u32 iso = 0; iso < 8; iso++) lvals.push_back(hashpatt(hashpatt(hashpatt(hash, iso)), hash.size()) + tail);
		}
		if (rvals.back().find('!') != npos) {
			std::string rval = rvals.back(); rvals.clear();
			std::string hash = rval.substr(0, rval.find('!')).substr(1);
			std::string tail = rval.substr(rval.find('!') + 1);
			for (u32 iso = 0; iso < 8; iso++) rvals.push_back(':' + hashpatt(hashpatt(hashpatt(hash, iso)), hash.size()) + tail);
		}
		lvals.resize(8, lvals.back().substr(0, lvals.back().find('=')));
		rvals.resize(8, rvals.front());
		token.clear();
		for (size_t iso = 0; iso < 8; iso++) token += (lvals[iso] + rvals[iso] + ' ');
	}

	size_t num = 0;
	std::stringstream counter(tokens);
	for (std::string token; counter >> token; num++);

	std::stringstream parser(tokens);
	std::string token;
	while (parser >> token) {
		u64 wght = 0, idxr = 0;
		std::string wtok = token.substr(0, token.find(':'));
		std::string itok = token.substr(token.find(':') + 1);

		if (wtok.size()) {
			// allocate: weight[size] weight(size)
			// initialize: ...=0 ...=10000 ...=100000+norm
			// map or remove: destination={source} id={}
			for (size_t i; (i = wtok.find_first_of("[]()")) != npos; wtok[i] = ' ');
			std::string name = wtok.substr(0, wtok.find_first_of("!&|:= "));
			std::string info = wtok.find(' ') != npos ? wtok.substr(0, wtok.find_first_of("!&|:=")).substr(wtok.find(' ') + 1) : "?";
			std::string init = wtok.find('=') != npos ? wtok.substr(wtok.find('=') + 1) : "?";
			u64 mska = wtok.find('&') != npos ? std::stoull(wtok.substr(wtok.find('&') + 1), nullptr, 16) : -1ull;
			u64 msko = wtok.find('|') != npos ? std::stoull(wtok.substr(wtok.find('|') + 1), nullptr, 16) : 0ull;
			u64 sign = (std::stoull(name, nullptr, 16) & mska) | msko;
			size_t size = 0;
			if (info.find_first_of("p^?") != npos) { // ^10 16^10 16^ p ?
				u32 base = 16, power = name.size();
				if (info.find('^') != npos) {
					base = info.front() != '^' ? std::stoul(info.substr(0, info.find('^'))) : 2;
					power = info.back() != '^' ? std::stoul(info.substr(info.find('^') + 1)) : name.size();
				}
				size = std::pow(base, power);
			} else if (info.find_first_not_of("0123456789.-x") == npos) {
				size = std::stoull(info, nullptr, 0);
			}
			if (init.find_first_of("{}") != npos && init != "{}") {
				weight src(std::stoull(init.substr(0, init.find('}')).substr(init.find('{') + 1), nullptr, 16));
				size = std::max(size, src.size());
			} else if (init == "{}") {
				size = 0;
			}
			if (weight(sign) && weight(sign).size() != size)
				weight::erase(sign);
			if (!weight(sign) && size) {
				weight dst = weight::make(sign, size);
				if (init.find_first_of("{}") != npos && init != "{}") {
					weight src(std::stoull(init.substr(0, init.find('}')).substr(init.find('{') + 1), nullptr, 16));
					for (size_t n = 0; n < dst.size(); n += src.size()) {
						std::copy_n(src.data(), src.size(), dst.data() + n);
					}
				} else if (init.find_first_of("0123456789.-") == 0) {
					numeric val = std::stod(init) * (init.find("norm") != npos ? std::pow(num, -1) : 1);
					std::fill_n(dst.data(), dst.size(), val);
				}
			}
			wght = weight(sign).sign();
		}

		if (itok.size()) {
			// indexer indexer,indexer,indexer pattern! signature&mask|mask
			std::string name = itok.substr(0, itok.find_first_of("!&|"));
			u64 mska = itok.find('&') != npos ? std::stoull(itok.substr(itok.find('&') + 1), nullptr, 16) : -1ull;
			u64 msko = itok.find('|') != npos ? std::stoull(itok.substr(itok.find('|') + 1), nullptr, 16) : 0ull;
			u64 hash = std::stoull(name, nullptr, 16);
			u64 sign = (hash & mska) | msko;
			if (!indexer(sign)) {
				indexer::mapper index = indexer(hash).index();
				if (!index) index = utils::indexhdr(std::bind(utils::indexnta, std::placeholders::_1, utils::hashpatt(name)));
				indexer::make(sign, index);
			}
			idxr = indexer(sign).sign();
		}

		if (wght && idxr && !feature(wght, idxr)) feature::make(wght, idxr);
	}

	return 0;
}
u32 load_network(utils::options::option opt) {
	for (std::string path : opt) {
		std::ifstream in;
		in.open(path, std::ios::in | std::ios::binary);
		while (in.peek() != -1) {
			auto type = in.peek();
			if (type != 0) {
				in.ignore(1);
			} else { // legacy binaries always beginning with 0, so use name suffix to determine the type
				type = path[path.find_last_of(".") + 1];
			}
			if (type == 'w')  weight::load(in);
			if (type == 'f') feature::load(in);
		}
		in.close();
	}
	return 0;
}
u32 save_network(utils::options::option opt) {
	for (std::string path : opt) {
		std::ofstream out;
		out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) continue;
		auto type = path[path.find_last_of(".") + 1];
		if (type != 'f')  weight::save(type != 'w' ? out.write("w", 1) : out);
		if (type != 'w') feature::save(type != 'f' ? out.write("f", 1) : out);
		out.flush();
		out.close();
	}
	return 0;
}

void list_mapping() {
	for (weight w : list<weight>(weight::wghts())) {
		char buf[64];
		std::string feats;
		for (feature f : feature::feats()) {
			if (f.value() == w) {
				snprintf(buf, sizeof(buf), " %08" PRIx64, f.index().sign());
				feats += buf;
			}
		}
		if (feats.size()) {
			u32 usageK = (sizeof(weight::numeric) * w.size()) >> 10;
			u32 usageM = usageK >> 10;
			u32 usageG = usageM >> 10;
			u32 usage = usageG ? usageG : (usageM ? usageM : usageK);
			char scale = usageG ? 'G' : (usageM ? 'M' : 'K');
			int n = snprintf(buf, sizeof(buf), "weight(%08" PRIx64 ")[%zu] = %d%c", w.sign(), w.size(), usage, scale);
			u32 stride = sizeof(weight::segment) / sizeof(weight::numeric);
			if (stride > 1) snprintf(buf + n, sizeof(buf) - n, " x%u", stride);
			std::cout << buf << " :" << feats << std::endl;
		} else {
			snprintf(buf, sizeof(buf), "%08" PRIx64, w.sign());
			weight::erase(w.sign());
			std::cerr << "unused weight (" << buf << ") at list_mapping, erased" << std::endl;
		}
	}
}

typedef numeric(*estimator)(const board&, clip<feature>);
typedef numeric(*optimizer)(const board&, numeric, clip<feature>);

inline numeric estimate(const board& state,
		clip<feature> range = feature::feats()) {
	register numeric esti = 0;
	for (register feature& feat : range)
		esti += feat[state];
	return esti;
}

inline numeric optimize(const board& state, numeric error,
		clip<feature> range = feature::feats()) {
	register numeric esti = 0;
	for (register feature& feat : range)
		esti += (feat[state] += error);
	return esti;
}

} // utils


struct state {
	board move;
	i32 score;
	numeric esti;
	inline state() : score(-1), esti(0) {}
	inline state(const state& s) = default;

	inline operator bool() const { return score >= 0; }
	inline operator board() const { return move; }
	declare_comparators(const state&, esti, inline);

	inline numeric value() const { return esti - score; }
	inline numeric reward() const { return score; }

	inline void assign(const board& b, u32 op = -1) {
		move = b;
		score = move.operate(op);
	}
	inline numeric estimate(
			clip<feature> range = feature::feats()) {
		if (score >= 0) {
			esti = state::reward() + utils::estimate(move, range);
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
		return esti;
	}
	inline numeric optimize(numeric exact, numeric alpha = state::alpha(),
			clip<feature> range = feature::feats()) {
		esti = state::reward() + utils::optimize(move, (exact - state::value()) * alpha, range);
		return esti;
	}

	inline static numeric& alpha() { static numeric a = numeric(0.01); return a; }
	inline static numeric& alpha(numeric a) { return (state::alpha() = a); }
};
struct select {
	state move[4];
	state *best;
	inline select() : best(move) {}
	inline select& operator ()(const board& b, clip<feature> range = feature::feats()) {
		move[0].assign(b, 0);
		move[1].assign(b, 1);
		move[2].assign(b, 2);
		move[3].assign(b, 3);
		move[0].estimate(range);
		move[1].estimate(range);
		move[2].estimate(range);
		move[3].estimate(range);
		best = std::max_element(move, move + 4);
		return *this;
	}
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(state& s) const { s = (*best); }
	inline void operator >>(board& b) const { b = best->move; }

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
		std::string res = (it != opt.end()) ? *it : (opt.empty() ? "1000" : "0");
		try {
			loop = std::stol(res);
			if (res.find('x') != npos) unit = std::stol(res.substr(res.find('x') + 1));
			if (res.find(':') != npos) winv = std::stol(res.substr(res.find(':') + 1));
		} catch (std::invalid_argument&) {}

		loop = std::stol(opt.find("loop", std::to_string(loop)));
		unit = std::stol(opt.find("unit", std::to_string(unit)));
		winv = std::stol(opt.find("win",  std::to_string(winv)));

		limit = loop * unit;
		format();

		every = {};
		total = {};
		local = {};
		local.time = moporgic::millisec();
		loop = 1;

		return limit;
	}

	class format_t : public std::array<char, 64> {
	public:
		inline void operator =(const std::string& s) { std::copy_n(s.begin(), s.size() + 1, begin()); }
		inline operator const char*() const { return data(); }
	};

	format_t indexf;
	format_t localf;
	format_t totalf;
	format_t summaf;

	void format(u32 dec = 0, const std::string& suffix = "") {
//		indexf = "%03llu/%03llu %llums %.2fops";
//		localf = "local:  avg=%llu max=%u tile=%u win=%.2f%%";
//		totalf = "total:  avg=%llu max=%u tile=%u win=%.2f%%";
//		summaf = "summary %llums %.2fops";
		if (!dec) dec = std::max(std::floor(std::log10(limit / unit)) + 1, 3.0);
		indexf = "%0" + std::to_string(dec) + PRIu64 "/%0" + std::to_string(dec) + PRIu64 " %" PRIu64 "ms %.2fops" + suffix;
		localf = "local: " + std::string(dec * 2 - 5, ' ') + "avg=%" PRIu64 " max=%u tile=%u win=%.2f%%";
		totalf = "total: " + std::string(dec * 2 - 5, ' ') + "avg=%" PRIu64 " max=%u tile=%u win=%.2f%%";
		summaf = "summary" + std::string(dec * 2 - 5, ' ') + "%" PRIu64 "ms %.2fops" + suffix;
	}

	inline u64 operator++(int) { return (++loop) - 1; }
	inline u64 operator++() { return (++loop); }
	inline operator bool() const { return loop <= limit; }
	inline bool checked() const { return (loop % unit) == 0; }

	void update(u32 score, u32 hash, u32 opers) {
		local.score += score;
		local.hash |= hash;
		local.opers += opers;
		local.win += (hash >= winv ? 1 : 0);
		local.max = std::max(local.max, score);
		every.count[math::log2(hash)] += 1;
		every.score[math::log2(hash)] += score;
		every.opers[math::log2(hash)] += opers;

		if ((loop % unit) != 0) return;

		u64 tick = moporgic::millisec();
		local.time = tick - local.time;
		total.score += local.score;
		total.win += local.win;
		total.time += local.time;
		total.opers += local.opers;
		total.hash |= local.hash;
		total.max = std::max(total.max, local.max);

		char buf[256];
		u32 size = 0;

		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size, indexf, // "%03llu/%03llu %llums %.2fops",
				loop / unit,
				limit / unit,
				local.time,
				local.opers * 1000.0 / local.time);
		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size, localf, // "local:  avg=%llu max=%u tile=%u win=%.2f%%",
				local.score / unit,
				local.max,
				math::msb32(local.hash),
				local.win * 100.0 / unit);
		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size, totalf, // "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / loop,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / loop);
		buf[size++] = '\n';
		buf[size++] = '\0';

		std::cout << buf << std::flush;

		local = {};
		local.time = tick;
	}

	void summary() const {
		char buf[1024];
		u32 size = 0;

		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size, summaf,
				total.time,
				total.opers * 1000.0 / total.time);
		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size, totalf, // "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / limit,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / limit);
		buf[size++] = '\n';
		size += snprintf(buf + size, sizeof(buf) - size,
		        "%-6s"  "%8s"    "%8s"    "%8s"   "%9s"   "%9s",
		        "tile", "count", "score", "move", "rate", "win");
		buf[size++] = '\n';
		const auto& count = every.count;
		const auto& score = every.score;
		const auto& opers = every.opers;
		auto total = std::accumulate(count.begin(), count.end(), 0);
		for (auto left = total, i = 0; left; left -= count[i++]) {
			if (count[i] == 0) continue;
			size += snprintf(buf + size, sizeof(buf) - size,
					"%-6d" "%8d" "%8d" "%8d" "%8.2f%%" "%8.2f%%",
					board::tile::itov(i), u32(count[i]),
					u32(score[i] / count[i]), u32(opers[i] / count[i]),
					count[i] * 100.0 / total, left * 100.0 / total);
			buf[size++] = '\n';
		}
		buf[size++] = '\0';

		std::cout << buf << std::flush;
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

statistic optimize(utils::options::option args, utils::options::option opts = {}) {
	std::vector<state> path;
	path.reserve(65536);
	statistic stats;
	select best;
	state last;
	board b;

	switch (to_hash(args["mode"])) {
	case to_hash("backward"):
	case to_hash("backward-best"):
		for (stats.init(args); stats; stats++) {

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
				v = path.back().optimize(v);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	default:
	case to_hash("forward"):
	case to_hash("forward-best"):
		for (stats.init(args); stats; stats++) {

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
				last.optimize(best.esti());
				score += best.score();
				opers += 1;
				best >> last;
				best >> b;
				b.next();
			}
			last.optimize(0);

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

statistic evaluate(utils::options::option args, utils::options::option opts = {}) {
	statistic stats;
	select best;
	board b;

	switch (to_hash(args["mode"])) {
	default:
	case to_hash("best"):
		for (stats.init(args); stats; stats++) {

			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best << b; b.next()) {
				score += best.score();
				opers += 1;
				best >> b;
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("random"):
		for (stats.init(args); stats; stats++) {

			u32 score = 0;
			u32 opers = 0;
			hex a;

			for (b.init(); (a = b.actions()).size(); b.next()) {
				score += b.operate(a[moporgic::rand() % a.size()]);
				opers += 1;
			}

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

utils::options parse(int argc, const char* argv[]) {
	utils::options opts;
	for (int i = 1; i < argc; i++) {
		std::string label = argv[i];
		auto next_opt = [&](const std::string& v) -> std::string {
			return (i + 1 < argc && *(argv[i + 1]) != '-') ? argv[++i] : v;
		};
		auto next_opts = [&]() -> utils::options::list {
			utils::options::list args;
			for (std::string v; (v = next_opt("")).size(); ) args.push_back(v);
			return args;
		};
		switch (to_hash(label)) {
		case to_hash("-a"):
		case to_hash("--alpha"):
			opts[""] = next_opts();
			if (opts[""].empty()) (opts[""] += "0.1") += "norm";
			opts["alpha"] = opts[""];
			break;
		case to_hash("-t"):
		case to_hash("--train"):
		case to_hash("--optimize"):
			opts[""] = opts["optimize"];
			opts["optimize"] = next_opt("1000");
			opts["optimize"] += next_opts();
			opts["optimize"] += opts[""];
			break;
		case to_hash("-e"):
		case to_hash("--test"):
		case to_hash("--evaluate"):
			opts[""] = opts["evaluate"];
			opts["evaluate"] = next_opt("1000");
			opts["evaluate"] += next_opts();
			opts["evaluate"] += opts[""];
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
			opts["seed"] = next_opt("moporgic");
			break;
		case to_hash("-io"):
		case to_hash("--input-output"):
		case to_hash("-nio"):
		case to_hash("--network-input-output"):
		case to_hash("-wio"):
		case to_hash("--weight-input-output"):
		case to_hash("-fio"):
		case to_hash("--feature-input-output"):
			opts[""] = next_opt(opts.find("make", argv[0]) + '.' + label[label.find_first_not_of('-')]);
			opts[""] += next_opts();
			opts["load"] += opts[""];
			opts["save"] += opts[""];
			break;
		case to_hash("-i"):
		case to_hash("--input"):
		case to_hash("-wi"):
		case to_hash("--weight-input"):
		case to_hash("-fi"):
		case to_hash("--feature-input"):
		case to_hash("-ni"):
		case to_hash("--network-input"):
			opts[""] = next_opt(opts.find("make", argv[0]) + '.' + label[label.find_first_not_of('-')]);
			opts[""] += next_opts();
			opts["load"] += opts[""];
			break;
		case to_hash("-o"):
		case to_hash("--output"):
		case to_hash("-wo"):
		case to_hash("--weight-output"):
		case to_hash("-fo"):
		case to_hash("--feature-output"):
		case to_hash("-no"):
		case to_hash("--network-output"):
			opts[""] = next_opt(opts.find("make", argv[0]) + '.' + label[label.find_first_not_of('-')]);
			opts[""] += next_opts();
//			opts["save"] += opts[""];
			for (auto opt : opts[""]) { // e.g. "-o 2048.x" indicates logging
				auto flag = opt[opt.find('.') + 1] != 'x' ? "save" : "logging";
				opts[flag] += opt;
			}
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
		case to_hash("-f"):
		case to_hash("--feature"):
		case to_hash("-n"):
		case to_hash("--network"):
		case to_hash("-wf"):
		case to_hash("-fw"):
			opts[""] = next_opt("default");
			opts[""] += next_opts();
			opts["make"] += opts[""];
			break;
		case to_hash("-%"):
		case to_hash("-I"):
		case to_hash("--info"):
			opts["info"] = next_opt("full");
			opts["info"] += next_opts();
			break;
		case to_hash("--option"):
		case to_hash("--options"):
			opts["options"] += next_opts();
			break;
		case to_hash("-tt"):
		case to_hash("-tm"):
		case to_hash("--train-type"):
		case to_hash("--train-mode"):
			opts["optimize"]["mode"] = next_opt("bias");
			break;
		case to_hash("-et"):
		case to_hash("-em"):
		case to_hash("--test-type"):
		case to_hash("--test-mode"):
			opts["evaluate"]["mode"] = next_opt("bias");
			break;
		case to_hash("-tc"):
		case to_hash("-tu"):
		case to_hash("--train-check"):
		case to_hash("--train-unit"):
			opts["optimize"]["unit"] = next_opt("1000");
			break;
		case to_hash("-ec"):
		case to_hash("-eu"):
		case to_hash("--test-check"):
		case to_hash("--test-unit"):
			opts["evaluate"]["unit"] = next_opt("1000");
			break;
		case to_hash("-u"):
		case to_hash("--unit"):
		case to_hash("-chk"):
		case to_hash("--check"):
			opts["optimize"]["unit"] = opts["evaluate"]["unit"] = next_opt("1000");
			break;
		case to_hash("-v"):
		case to_hash("--win"):
			opts["optimize"]["win"] = opts["evaluate"]["win"] = next_opt("2048");
			break;
		case to_hash("-c"):
		case to_hash("--comment"):
			opts["comment"] = next_opts();
			break;
		case to_hash("-x"):
		case to_hash("-log"):
		case to_hash("--logging"):
			opts["logging"] = next_opt(opts.find("make", argv[0]) + ".x");
			break;
		case to_hash("-"):
		case to_hash("-|"):
		case to_hash("--|"):
			opts = {};
			break;
		default:
			opts["options"][label.substr(label.find_first_not_of('-'))] += next_opts();
			break;
		}
	}
	return opts;
}

int main(int argc, const char* argv[]) {
	utils::options opts = parse(argc, argv);
	if (!opts("optimize")) opts["optimize"] = opts("evaluate") ? 0 : 1000;
	if (!opts("evaluate")) opts["evaluate"] = opts("optimize") ? 0 : 1000;
	if (!opts("alpha")) opts["alpha"] = 1.0, opts["alpha"] += "norm";
	if (!opts("seed")) opts["seed"] = ({std::stringstream ss; ss << std::hex << rdtsc(); ss.str();});

	utils::logging(opts["logging"]);
	std::cout << "TDL2048+ by Hung Guei" << std::endl;
	std::cout << "develop-coherence" << " build GCC " __VERSION__ << " C++" << __cplusplus;
	std::cout << " (" __DATE__ " " __TIME__ ")" << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << moporgic::millisec() << std::endl;
	std::cout << "seed = " << opts["seed"] << std::endl;
	std::cout << "alpha = " << opts["alpha"] << std::endl;
	std::cout << std::endl;

	utils::load_network(opts["load"]);
	utils::make_network(opts["make"]);
	utils::list_mapping();

	moporgic::srand(moporgic::to_hash(opts["seed"]));
	state::alpha(std::stod(opts["alpha"]));
	if (opts("alpha", "norm")) state::alpha(state::alpha() / feature::feats().size());

	if (statistic(opts["optimize"])) {
		std::cout << std::endl << "start training..." << std::endl;
		statistic stat = optimize(opts["optimize"], opts["options"]);
		if (opts["info"] == "full") stat.summary();
	}

	utils::save_network(opts["save"]);

	if (statistic(opts["evaluate"])) {
		std::cout << std::endl << "start testing..." << std::endl;
		statistic stat = evaluate(opts["evaluate"], opts["options"]);
		if (opts["info"] != "none") stat.summary();
	}

	std::cout << std::endl;
	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
