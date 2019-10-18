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
#include <iomanip>
#include <list>
#include <random>

namespace moporgic {

typedef float numeric;

class weight {
public:
	inline weight() : name(), length(0), raw(nullptr) {}
	inline weight(const weight& w) = default;
	inline ~weight() {}

	typedef std::string sign_t;
	typedef moporgic::numeric numeric;
	typedef weight::numeric segment;

	inline sign_t sign() const { return name; }
	inline size_t size() const { return length; }
	constexpr inline segment& operator [](u64 i) { return raw[i]; }
	constexpr inline segment* data(u64 i = 0) { return raw + i; }
	constexpr inline clip<numeric> value() const { return { raw, raw + length }; }
	inline operator bool() const { return raw; }
	declare_comparators(const weight&, sign(), inline);

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		u32 code = 4;
		write_cast<u8>(out, code);
		switch (code) {
		default:
		case 4:
			try {
				write_cast<u32>(out, std::stoul(w.sign(), nullptr, 16));
				write_cast<u16>(out, w.sign().size());
				write_cast<u16>(out, 0);
			} catch (std::invalid_argument&) {
				out.write(w.sign().append(8, ' ').c_str(), 8);
			}
			write_cast<u16>(out, sizeof(weight::numeric));
			write_cast<u64>(out, w.size());
			write_cast<numeric>(out, w.value().begin(), w.value().end());
			write_cast<u16>(out, 0);
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, weight& w) {
		u32 code = 4;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
		case 1:
		case 2:
			w.name = ({
				std::stringstream ss;
				ss << std::setw(8) << std::setfill('0') << std::hex << read<u32>(in);
				ss.str();
			});
			read_cast<u64>(in, w.length);
			if (w.length == (1ull << math::lg64(w.length)))
				w.name = w.name.substr(8 - (math::lg64(w.length) >> 2));
			w.raw = weight::alloc(w.length);
			switch ((code == 2) ? read<u16>(in) : (code == 1 ? 8 : 4)) {
			case 4: read_cast<f32>(in, w.value().begin(), w.value().end()); break;
			case 8: read_cast<f64>(in, w.value().begin(), w.value().end()); break;
			}
			break;
		default:
		case 4:
			in.read(const_cast<char*>(w.name.assign(8, ' ').data()), 8);
			w.name = raw_cast<u16>(w.name[6]) == 0 ? ({
				std::stringstream ss;
				ss << std::setw(raw_cast<u16>(w.name[4]) ? raw_cast<u16>(w.name[4]) : ({
					read_cast<u64>(in.ignore(2), w.length).seekg(-10, std::ios::cur);
					(w.length == (1ull << math::lg64(w.length))) ? (math::lg64(w.length) >> 2) : 8;
				})) << std::setfill('0') << std::hex << raw_cast<u32>(w.name[0]);
				ss.str();
			}) : w.name.substr(0, w.name.find(' '));
			read_cast<u16>(in, code);
			read_cast<u64>(in, w.length);
			w.raw = weight::alloc(w.length);
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
		weight& make(sign_t sign, size_t size) { return list<weight>::as(*this).emplace_back(weight(sign, size)); }
		weight erase(sign_t sign) { auto it = find(sign); auto w = *it; free(it->data()); list<weight>::as(*this).erase(it); return w; }
		weight* find(sign_t sign) const { return std::find_if(begin(), end(), [=](const weight& w) { return w.sign() == sign; }); }
		weight& at(sign_t sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("weight::at"); }
		weight& operator[](sign_t sign) const { return (*find(sign)); }
		weight operator()(sign_t sign) const { auto it = find(sign); return it != end() ? *it : ({ weight w; w.name = sign; w; }); }
	};

	static inline weight::container& wghts() { static container w; return w; }
	static inline weight& make(sign_t sign, size_t size, container& src = wghts()) { return src.make(sign, size); }
	static inline size_t erase(sign_t sign, container& src = wghts()) { return src.erase(sign); }
	inline weight(sign_t sign, const container& src = wghts()) : weight(src(sign)) {}

private:
	inline weight(sign_t sign, size_t size) : name(sign), length(size), raw(alloc(size)) {}

	static inline segment* alloc(size_t size) { return new segment[size](); }
	static inline void free(segment* v) { delete[] v; }

	sign_t name;
	size_t length;
	segment* raw;
};

class indexer {
public:
	inline indexer() : name(), map(nullptr) {}
	inline indexer(const indexer& i) = default;
	inline ~indexer() {}

	typedef std::string sign_t;
	typedef u64(*mapper)(const board&);

	inline sign_t sign() const { return name; }
	constexpr inline mapper index() const { return map; }
	constexpr inline u64 operator ()(const board& b) const { return (*map)(b); }
	inline operator bool() const { return map; }
	declare_comparators(const indexer&, sign(), inline);

	class container : public clip<indexer> {
	public:
		constexpr container() noexcept : clip<indexer>() {}
		constexpr container(const clip<indexer>& i) : clip<indexer>(i) {}
	public:
		indexer& make(sign_t sign, mapper map) { return list<indexer>::as(*this).emplace_back(indexer(sign, map)); }
		indexer erase(sign_t sign) { auto it = find(sign); auto x = *it; list<indexer>::as(*this).erase(it); return x; }
		indexer* find(sign_t sign) const { return std::find_if(begin(), end(), [=](const indexer& i) { return i.sign() == sign; }); }
		indexer& at(sign_t sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("indexer::at"); }
		indexer& operator[](sign_t sign) const { return (*find(sign)); }
		indexer operator()(sign_t sign) const { auto it = find(sign); return it != end() ? *it : ({ indexer x; x.name = sign; x; }); }
	};

	static inline indexer::container& idxrs() { static container i; return i; }
	static inline indexer& make(sign_t sign, mapper map, container& src = idxrs()) { return src.make(sign, map); }
	static inline size_t erase(sign_t sign, container& src = idxrs()) { return src.erase(sign); }
	inline indexer(sign_t sign, const container& src = idxrs()) : indexer(src(sign)) {}

private:
	inline indexer(sign_t sign, mapper map) : name(sign), map(map) {}

	sign_t name;
	mapper map;
};

class feature {
public:
	inline feature() : name(), raw(), map() {}
	inline feature(const feature& t) = default;
	inline ~feature() {}

	typedef std::string sign_t;

	inline sign_t sign() const { return name; }
	constexpr inline weight::segment& operator [](const board& b) { return raw[map(b)]; }
	constexpr inline weight::segment& operator [](u64 idx) { return raw[idx]; }
	constexpr inline u64 operator ()(const board& b) const { return map(b); }

	inline indexer index() const { return map; }
	inline weight  value() const { return raw; }
	inline operator bool() const { return map && raw; }
	declare_comparators(const feature&, sign(), inline);

	friend std::ostream& operator <<(std::ostream& out, const feature& f) {
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		default:
		case 0:
			write_cast<u32>(out, std::stoul(f.index().sign(), nullptr, 16));
			write_cast<u32>(out, std::stoul(f.value().sign(), nullptr, 16));
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, feature& f) {
		u32 code = 0;
		read_cast<u8>(in, code);
		switch (code) {
		default:
		case 0:
			f.map = indexer(({
				std::stringstream ss;
				ss << std::setw(8) << std::setfill('0') << std::hex << read<u32>(in);
				ss.str();
			}));
			f.raw = weight(({
				std::stringstream ss;
				ss << std::setw(8) << std::setfill('0') << std::hex << read<u32>(in);
				ss.str();
			}));
			f.name = f.raw.sign() + ':' + f.map.sign();
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
		feature& make(sign_t wgt, sign_t idx) { return list<feature>::as(*this).emplace_back(feature(weight(wgt), indexer(idx))); }
		feature& make(sign_t sign) { return make(sign.substr(0, sign.find(':')), sign.substr(sign.find(':') + 1)); }
		feature erase(sign_t wgt, sign_t idx) { return erase(wgt + ':' + idx); }
		feature erase(sign_t sign) { auto it = find(sign); auto f = *it; list<feature>::as(*this).erase(it); return f; }
		feature* find(sign_t wgt, sign_t idx) const { return find(wgt + ':' + idx); }
		feature* find(sign_t sign) const { return std::find_if(begin(), end(), [=](const feature& f) { return f.sign() == sign; }); }
		feature& at(sign_t wgt, sign_t idx) const { return at(wgt + ':' + idx); }
		feature& at(sign_t sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("feature::at"); }
		feature& operator[](sign_t sign) const { return (*find(sign)); }
		feature operator()(sign_t wgt, sign_t idx) const { return operator()(wgt + ':' + idx); }
		feature operator()(sign_t sign) const { auto it = find(sign); return it != end() ? *it : ({ feature f; f.name = sign; f; }); }
	};

	static inline feature::container& feats() { static container f; return f; }
	static inline feature& make(sign_t wgt, sign_t idx, container& src = feats()) { return src.make(wgt, idx); }
	static inline size_t erase(sign_t wgt, sign_t idx, container& src = feats()) { return src.erase(wgt, idx); }
	inline feature(sign_t wgt, sign_t idx, const container& src = feats()) : feature(src(wgt, idx)) {}

private:
	inline feature(const weight& value, const indexer& index) : name(value.sign() + ':' + index.sign()), raw(value), map(index) {}

	sign_t name;
	weight raw;
	indexer map;
};

class cache {
public:
	class block {
	public:
		class access {
		public:
			constexpr access(u64 sign, u32 hold, block& blk) : sign(sign), hold(hold), safe(-1u), blk(blk) {}
			constexpr access(u64 sign, u32 hold) : access(sign, hold, raw_cast<block>(*this)) {}
			constexpr access(access&& acc) : access(acc.sign, acc.hold, &(acc.blk) != &(raw_cast<block>(acc)) ? acc.blk : raw_cast<block>(*this)) {}
			constexpr access(const access&) = delete;
			constexpr access& operator =(const access&) = delete;

			constexpr operator bool() const {
				return blk.sign() == sign && blk.hold() >= hold;
			}
			constexpr numeric fetch() const {
				u64 info = blk.info;
				f32 esti = raw_cast<f32, 0>(info);
				raw_cast<u16, 3>(info) = std::min(blk.hits() + 1, 65535);
				u64 hash = sign ^ info;
				blk.hash = hash;
				blk.info = info;
				return esti;
			}
			constexpr numeric store(numeric esti) const {
				u64 info = 0;
				raw_cast<f32, 0>(info) = esti;
				raw_cast<u16, 2>(info) = hold;
				raw_cast<u16, 3>(info) = blk.sign() == sign ? blk.hits() : 0;
				u64 hash = sign ^ info;
				blk.hash = hash;
				blk.info = info;
				return esti;
			}
		private:
			u64 sign;
			u32 hold;
			u32 safe;
			block& blk;
		};

		constexpr block(const block& e) = default;
		constexpr block(u64 sign = 0, u64 info = 0) : hash(sign ^ info), info(info) {}
		constexpr access operator()(u64 x, u32 n) { return access(x, n, *this); }
		constexpr u64 sign() const { return hash ^ info; }
		constexpr f32 esti() const { return raw_cast<f32, 0>(info); }
		constexpr u16 hold() const { return raw_cast<u16, 2>(info); }
		constexpr u16 hits() const { return raw_cast<u16, 3>(info); }
		u64 sign(u64 sign) { return std::exchange(hash, sign ^ info) ^ info; }
		f32 esti(f32 esti) { u64 sign = hash ^ info; esti = std::exchange(raw_cast<f32, 0>(info), esti); hash = sign ^ info; return esti; }
		u16 hold(u16 hold) { u64 sign = hash ^ info; hold = std::exchange(raw_cast<u16, 2>(info), hold); hash = sign ^ info; return hold; }
		u16 hits(u16 hits) { u64 sign = hash ^ info; hits = std::exchange(raw_cast<u16, 3>(info), hits); hash = sign ^ info; return hits; }

	    friend std::ostream& operator <<(std::ostream& out, const block& blk) {
	    	write_cast<u64>(out, blk.hash);
	    	write_cast<u64>(out, blk.info);
	    	return out;
	    }
		friend std::istream& operator >>(std::istream& in, block& blk) {
			read_cast<u64>(in, blk.hash);
			read_cast<u64>(in, blk.info);
			return in;
		}
	private:
		u64 hash;
		u64 info; // f32 esti; u16 hold; u16 hits;
	};

	constexpr cache() : cached(&initial), length(1), mask(0), nmap{} {}
	cache(const cache& tp) = default;
	~cache() {}
	inline size_t size() const { return length; }
	inline block& operator[] (size_t i) { return cached[i]; }
	inline const block& operator[] (size_t i) const { return cached[i]; }
	inline block::access operator() (const board& b, u32 n) {
		u64 x = min_isomorphic(b);
		size_t i = (math::fmix64(x) ^ nmap[n >> 1]) & mask;
		block& blk = operator[](i);
		return blk(x, n);
	}

    friend std::ostream& operator <<(std::ostream& out, const cache& c) {
		u32 code = 5;
		write_cast<byte>(out, code);
		switch (code) {
		default:
		case 4:
			write_cast<u16>(out, 0); // reserved for header
			write_cast<u64>(out, 0);
			write_cast<u16>(out, sizeof(cache::block));
			write_cast<u64>(out, c.size());
			for (size_t i = 0; i < c.size(); i++)
				out << c[i];
			write_cast<u16>(out, 0); // reserved for fields
			break;
		case 5:
			write_cast<u16>(out, 0); // reserved for header
			write_cast<u64>(out, 0);
			write_cast<u16>(out, sizeof(cache::block));
			write_cast<u64>(out, c.size());
			for (size_t i = 0; i < c.size(); i++)
				out << c[i];
			write_cast<u16>(out, sizeof(size_t));
			write_cast<u64>(out, c.nmap.size());
			write_cast<u64>(out, c.nmap.begin(), c.nmap.end());
			write_cast<u16>(out, 0); // reserved for fields
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, cache& c) {
		u32 code = 0;
		read_cast<byte>(in, code);
		switch (code) {
		default:
		case 4:
			in.ignore(read<u16>(in) * read<u64>(in));
			c.shape(read<u64>(in.ignore(2)));
			for (size_t i = 0; i < c.size(); i++)
				in >> c[i];
			while ((code = read<u16>(in)) != 0)
				in.ignore(code * read<u64>(in));
			break;
		case 5:
			in.ignore(read<u16>(in) * read<u64>(in));
			c.shape(read<u64>(in.ignore(2)));
			for (size_t i = 0; i < c.size(); i++)
				in >> c[i];
			in.ignore(2); // sizeof(size_t)
			read_cast<u64>(in, c.nmap.begin(),
				c.nmap.begin() + std::min(c.nmap.size(), read<u64>(in)));
			while ((code = read<u16>(in)) != 0)
				in.ignore(code * read<u64>(in));
			break;
		}
		return in;
	}

	static void save(std::ostream& out) {
		u32 code = 0;
		write_cast<byte>(out, code);
		switch (code) {
		default:
		case 0:
			out << instance();
			break;
		}
		out.flush();
	}
	static void load(std::istream& in) {
		u32 code;
		read_cast<byte>(in, code);
		switch (code) {
		default:
		case 0:
			in >> instance();
			break;
		}
	}

	static inline block::access find(const board& b, u32 n) { return instance()(b, n); }
	static inline cache& make(size_t len, bool peek = false) { return instance().shape(std::max(len, size_t(1)), peek); }
	static inline cache& instance() { static cache tp; return tp; }

private:
	static inline block* alloc(size_t len) { return new block[len](); }
	static inline void free(block* alloc) { delete[] alloc; }

	cache& shape(size_t len, bool peek = false) {
		length = (1ull << (math::lg64(len)));
		mask = length - 1;
		if (cached != &initial) free(cached);
		cached = alloc(length);
		for (size_t i = 0; i < nmap.size(); i++)
			nmap[i] = math::fmix64(i);
		if (peek) nmap.fill(0);
		return *this;
	}

	constexpr inline u64 min_isomorphic(board t) const {
		u64 x = u64(t);
		t.mirror64();    x = std::min(x, u64(t));
		t.transpose64(); x = std::min(x, u64(t));
		t.mirror64();    x = std::min(x, u64(t));
		t.transpose64(); x = std::min(x, u64(t));
		t.mirror64();    x = std::min(x, u64(t));
		t.transpose64(); x = std::min(x, u64(t));
		t.mirror64();    x = std::min(x, u64(t));
		return x;
	}

private:
	block* cached;
	block initial;
	size_t length;
	size_t mask;
	std::array<size_t, 16> nmap;
};

namespace index {

template<u32 p0, u32 p1, u32 p2, u32 p3>
u64 index4t(const board& b) {
	register u64 index = 0;
	index += b.at(p0) <<  0;
	index += b.at(p1) <<  4;
	index += b.at(p2) <<  8;
	index += b.at(p3) << 12;
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

template<> u64 index4t<0x0,0x1,0x2,0x3>(const board& b) { return (u32(u64(b)) & 0xffff); }
template<> u64 index4t<0x4,0x5,0x6,0x7>(const board& b) { return (u32(u64(b) >> 16) & 0xffff); }
template<> u64 index4t<0x0,0x1,0x4,0x5>(const board& b) { return (u32(u64(b)) & 0x00ff) | (u32(u64(b) >> 8) & 0xff00); }
template<> u64 index4t<0x1,0x2,0x5,0x6>(const board& b) { return (u32(u64(b) >> 4) & 0x00ff) | (u32(u64(b) >> 12) & 0xff00); }
template<> u64 index4t<0x5,0x6,0x9,0xa>(const board& b) { return (u32(u64(b) >> 20) & 0x00ff) | (u32(u64(b) >> 28) & 0xff00); }
template<> u64 index4t<0x0,0x1,0x2,0x4>(const board& b) { return (u32(u64(b)) & 0x0fff) | (u32(u64(b) >> 4) & 0xf000); }
template<> u64 index4t<0x1,0x2,0x3,0x5>(const board& b) { return (u32(u64(b) >> 4) & 0x0fff) | (u32(u64(b) >> 8) & 0xf000); }
template<> u64 index4t<0x4,0x5,0x6,0x8>(const board& b) { return (u32(u64(b) >> 16) & 0x0fff) | (u32(u64(b) >> 20) & 0xf000); }
template<> u64 index4t<0x5,0x6,0x7,0x9>(const board& b) { return (u32(u64(b) >> 20) & 0x0fff) | (u32(u64(b) >> 24) & 0xf000); }
template<> u64 index4t<0x0,0x1,0x2,0x5>(const board& b) { return (u32(u64(b)) & 0x0fff) | (u32(u64(b) >> 8) & 0xf000); }
template<> u64 index4t<0x4,0x5,0x6,0x9>(const board& b) { return (u32(u64(b) >> 16) & 0x0fff) | (u32(u64(b) >> 24) & 0xf000); }
template<> u64 index5t<0x0,0x1,0x2,0x3,0x4>(const board& b) { return (u32(u64(b)) & 0xfffff); }
template<> u64 index5t<0x4,0x5,0x6,0x7,0x8>(const board& b) { return (u32(u64(b) >> 16) & 0xfffff); }
template<> u64 index5t<0x0,0x1,0x2,0x4,0x5>(const board& b) { return (u32(u64(b)) & 0x00fff) | ((u32(u64(b)) >> 4) & 0xff000); }
template<> u64 index5t<0x4,0x5,0x6,0x8,0x9>(const board& b) { return (u32(u64(b) >> 16) & 0x00fff) | (u32(u64(b) >> 20) & 0xff000); }
template<> u64 index5t<0x0,0x1,0x2,0x3,0x5>(const board& b) { return (u32(u64(b)) & 0x0ffff) | (u32(u64(b) >> 4) & 0xf0000); }
template<> u64 index5t<0x4,0x5,0x6,0x7,0x9>(const board& b) { return (u32(u64(b) >> 16) & 0x0ffff) | (u32(u64(b) >> 20) & 0xf0000); }
template<> u64 index6t<0x0,0x1,0x2,0x3,0x4,0x5>(const board& b) { return (u32(u64(b)) & 0xffffff); }
template<> u64 index6t<0x4,0x5,0x6,0x7,0x8,0x9>(const board& b) { return (u32(u64(b) >> 16) & 0xffffff); }
template<> u64 index6t<0x8,0x9,0xa,0xb,0xc,0xd>(const board& b) { return (u32(u64(b) >> 32) & 0xffffff); }
template<> u64 index6t<0x0,0x1,0x2,0x4,0x5,0x6>(const board& b) { return (u32(u64(b)) & 0x000fff) | ((u32(u64(b)) >> 4) & 0xfff000); }
template<> u64 index6t<0x4,0x5,0x6,0x8,0x9,0xa>(const board& b) { return (u32(u64(b) >> 16) & 0x000fff) | (u32(u64(b) >> 20) & 0xfff000); }
template<> u64 index6t<0x2,0x3,0x4,0x5,0x6,0x9>(const board& b) { return (u32(u64(b) >> 8) & 0x0fffff) | (u32(u64(b) >> 16) & 0xf00000); }
template<> u64 index6t<0x0,0x1,0x2,0x5,0x9,0xa>(const board& b) { return (u32(u64(b)) & 0x000fff) | (u32(u64(b) >> 8) & 0x00f000) | (u32(u64(b) >> 20) & 0xff0000); }
template<> u64 index6t<0x3,0x4,0x5,0x6,0x7,0x8>(const board& b) { return (u32(u64(b) >> 12) & 0xffffff); }
template<> u64 index6t<0x1,0x3,0x4,0x5,0x6,0x7>(const board& b) { return (u32(u64(b) >> 4) & 0x00000f) | (u32(u64(b) >> 8) & 0xfffff0); }
template<> u64 index6t<0x0,0x1,0x4,0x8,0x9,0xa>(const board& b) { return (u32(u64(b)) & 0x0000ff) | (u32(u64(b) >> 8) & 0x000f00) | (u32(u64(b) >> 20) & 0xfff000); }
template<> u64 index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x6>(const board& b) { return (u32(u64(b)) & 0xfffffff); }
template<> u64 index7t<0x4,0x5,0x6,0x7,0x8,0x9,0xa>(const board& b) { return (u32(u64(b) >> 16) & 0xfffffff); }
template<> u64 index7t<0x0,0x1,0x2,0x3,0x4,0x8,0xc>(const board& b) { return (u32(u64(b)) & 0x00fffff) | (u32(u64(b) >> 12) & 0x0f00000) | (u32(u64(b) >> 24) & 0xf000000); }
template<> u64 index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>(const board& b) { return (u32(u64(b))); }
template<> u64 index8t<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>(const board& b) { return (u32(u64(b) >> 16)); }

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

struct indexapt {
	static inline auto& wlist() { static moporgic::list<indexer::mapper> w; return w; }
	static inline auto& hlist() { static moporgic::list<std::function<u64(const board&)>> h; return h; }

	inline operator indexer::mapper() const { return wlist().front(); }
	indexapt(std::function<u64(const board&)> hdr) { hlist().push_back(hdr); }
	~indexapt() { wlist().pop_front(); }

	template<u32 idx>
	static u64 adapt(const board& b) { return hlist()[idx](b); }

	template<u32 idx, u32 lim>
	static void make() { make_wrappers<idx, lim>(); }

	template<u32 idx, u32 lim>
	struct make_wrappers {
		make_wrappers() { wlist().push_back(indexapt::adapt<idx>); }
		~make_wrappers() { make_wrappers<idx + 1, lim>(); }
	};
	template<u32 idx>
	struct make_wrappers<idx, idx> {};
};

__attribute__((constructor)) void init() {
	auto make = [&](indexer::sign_t sign, indexer::mapper func) {
		if (!indexer(sign)) indexer::make(sign, func);
		else if (indexer(sign).index() != func) std::exit(127);
	};
	make("012345", index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	make("37bf26", index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	make("fedcba", index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	make("c840d9", index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	make("321076", index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	make("048c15", index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	make("cdef89", index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	make("fb73ea", index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	make("456789", index6t<0x4,0x5,0x6,0x7,0x8,0x9>);
	make("26ae15", index6t<0x2,0x6,0xa,0xe,0x1,0x5>);
	make("ba9876", index6t<0xb,0xa,0x9,0x8,0x7,0x6>);
	make("d951ea", index6t<0xd,0x9,0x5,0x1,0xe,0xa>);
	make("7654ba", index6t<0x7,0x6,0x5,0x4,0xb,0xa>);
	make("159d26", index6t<0x1,0x5,0x9,0xd,0x2,0x6>);
	make("89ab45", index6t<0x8,0x9,0xa,0xb,0x4,0x5>);
	make("ea62d9", index6t<0xe,0xa,0x6,0x2,0xd,0x9>);
	make("012456", index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	make("37b26a", index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	make("fedba9", index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	make("c84d95", index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	make("321765", index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	make("048159", index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	make("cde89a", index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	make("fb7ea6", index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	make("45689a", index6t<0x4,0x5,0x6,0x8,0x9,0xa>);
	make("26a159", index6t<0x2,0x6,0xa,0x1,0x5,0x9>);
	make("ba9765", index6t<0xb,0xa,0x9,0x7,0x6,0x5>);
	make("d95ea6", index6t<0xd,0x9,0x5,0xe,0xa,0x6>);
	make("765ba9", index6t<0x7,0x6,0x5,0xb,0xa,0x9>);
	make("15926a", index6t<0x1,0x5,0x9,0x2,0x6,0xa>);
	make("89a456", index6t<0x8,0x9,0xa,0x4,0x5,0x6>);
	make("ea6d95", index6t<0xe,0xa,0x6,0xd,0x9,0x5>);
	make("89abcd", index6t<0x8,0x9,0xa,0xb,0xc,0xd>);
	make("159d04", index6t<0x1,0x5,0x9,0xd,0x0,0x4>);
	make("765432", index6t<0x7,0x6,0x5,0x4,0x3,0x2>);
	make("ea62fb", index6t<0xe,0xa,0x6,0x2,0xf,0xb>);
	make("ba98fe", index6t<0xb,0xa,0x9,0x8,0xf,0xe>);
	make("26ae37", index6t<0x2,0x6,0xa,0xe,0x3,0x7>);
	make("456701", index6t<0x4,0x5,0x6,0x7,0x0,0x1>);
	make("d951c8", index6t<0xd,0x9,0x5,0x1,0xc,0x8>);
	make("012458", index6t<0x0,0x1,0x2,0x4,0x5,0x8>);
	make("c84d9e", index6t<0xc,0x8,0x4,0xd,0x9,0xe>);
	make("fedba7", index6t<0xf,0xe,0xd,0xb,0xa,0x7>);
	make("37b261", index6t<0x3,0x7,0xb,0x2,0x6,0x1>);
	make("32176b", index6t<0x3,0x2,0x1,0x7,0x6,0xb>);
	make("fb7ead", index6t<0xf,0xb,0x7,0xe,0xa,0xd>);
	make("cde894", index6t<0xc,0xd,0xe,0x8,0x9,0x4>);
	make("048152", index6t<0x0,0x4,0x8,0x1,0x5,0x2>);
	make("123569", index6t<0x1,0x2,0x3,0x5,0x6,0x9>);
	make("84095a", index6t<0x8,0x4,0x0,0x9,0x5,0xa>);
	make("edca96", index6t<0xe,0xd,0xc,0xa,0x9,0x6>);
	make("7bf6a5", index6t<0x7,0xb,0xf,0x6,0xa,0x5>);
	make("21065a", index6t<0x2,0x1,0x0,0x6,0x5,0xa>);
	make("b73a69", index6t<0xb,0x7,0x3,0xa,0x6,0x9>);
	make("def9a5", index6t<0xd,0xe,0xf,0x9,0xa,0x5>);
	make("48c596", index6t<0x4,0x8,0xc,0x5,0x9,0x6>);
	make("45689c", index6t<0x4,0x5,0x6,0x8,0x9,0xc>);
	make("d95eaf", index6t<0xd,0x9,0x5,0xe,0xa,0xf>);
	make("ba9763", index6t<0xb,0xa,0x9,0x7,0x6,0x3>);
	make("26a150", index6t<0x2,0x6,0xa,0x1,0x5,0x0>);
	make("765baf", index6t<0x7,0x6,0x5,0xb,0xa,0xf>);
	make("ea6d9c", index6t<0xe,0xa,0x6,0xd,0x9,0xc>);
	make("89a450", index6t<0x8,0x9,0xa,0x4,0x5,0x0>);
	make("159263", index6t<0x1,0x5,0x9,0x2,0x6,0x3>);
	make("5679ad", index6t<0x5,0x6,0x7,0x9,0xa,0xd>);
	make("951a6b", index6t<0x9,0x5,0x1,0xa,0x6,0xb>);
	make("a98652", index6t<0xa,0x9,0x8,0x6,0x5,0x2>);
	make("6ae594", index6t<0x6,0xa,0xe,0x5,0x9,0x4>);
	make("654a9e", index6t<0x6,0x5,0x4,0xa,0x9,0xe>);
	make("a62958", index6t<0xa,0x6,0x2,0x9,0x5,0x8>);
	make("9ab561", index6t<0x9,0xa,0xb,0x5,0x6,0x1>);
	make("59d6a7", index6t<0x5,0x9,0xd,0x6,0xa,0x7>);
	make("012348", index6t<0x0,0x1,0x2,0x3,0x4,0x8>);
	make("c840de", index6t<0xc,0x8,0x4,0x0,0xd,0xe>);
	make("fedcb7", index6t<0xf,0xe,0xd,0xc,0xb,0x7>);
	make("37bf21", index6t<0x3,0x7,0xb,0xf,0x2,0x1>);
	make("32107b", index6t<0x3,0x2,0x1,0x0,0x7,0xb>);
	make("fb73ed", index6t<0xf,0xb,0x7,0x3,0xe,0xd>);
	make("cdef84", index6t<0xc,0xd,0xe,0xf,0x8,0x4>);
	make("048c12", index6t<0x0,0x4,0x8,0xc,0x1,0x2>);
	make("45678c", index6t<0x4,0x5,0x6,0x7,0x8,0xc>);
	make("d951ef", index6t<0xd,0x9,0x5,0x1,0xe,0xf>);
	make("ba9873", index6t<0xb,0xa,0x9,0x8,0x7,0x3>);
	make("26ae10", index6t<0x2,0x6,0xa,0xe,0x1,0x0>);
	make("7654bf", index6t<0x7,0x6,0x5,0x4,0xb,0xf>);
	make("ea62dc", index6t<0xe,0xa,0x6,0x2,0xd,0xc>);
	make("89ab40", index6t<0x8,0x9,0xa,0xb,0x4,0x0>);
	make("159d23", index6t<0x1,0x5,0x9,0xd,0x2,0x3>);

	// k.matsuzaki
	make("012456", index6t<0x0,0x1,0x2,0x4,0x5,0x6>);
	make("37b26a", index6t<0x3,0x7,0xb,0x2,0x6,0xa>);
	make("fedba9", index6t<0xf,0xe,0xd,0xb,0xa,0x9>);
	make("c84d95", index6t<0xc,0x8,0x4,0xd,0x9,0x5>);
	make("321765", index6t<0x3,0x2,0x1,0x7,0x6,0x5>);
	make("048159", index6t<0x0,0x4,0x8,0x1,0x5,0x9>);
	make("cde89a", index6t<0xc,0xd,0xe,0x8,0x9,0xa>);
	make("fb7ea6", index6t<0xf,0xb,0x7,0xe,0xa,0x6>);
	make("456789", index6t<0x4,0x5,0x6,0x7,0x8,0x9>);
	make("26ae15", index6t<0x2,0x6,0xa,0xe,0x1,0x5>);
	make("ba9876", index6t<0xb,0xa,0x9,0x8,0x7,0x6>);
	make("d951ea", index6t<0xd,0x9,0x5,0x1,0xe,0xa>);
	make("7654ba", index6t<0x7,0x6,0x5,0x4,0xb,0xa>);
	make("159d26", index6t<0x1,0x5,0x9,0xd,0x2,0x6>);
	make("89ab45", index6t<0x8,0x9,0xa,0xb,0x4,0x5>);
	make("ea62d9", index6t<0xe,0xa,0x6,0x2,0xd,0x9>);
	make("012345", index6t<0x0,0x1,0x2,0x3,0x4,0x5>);
	make("37bf26", index6t<0x3,0x7,0xb,0xf,0x2,0x6>);
	make("fedcba", index6t<0xf,0xe,0xd,0xc,0xb,0xa>);
	make("c840d9", index6t<0xc,0x8,0x4,0x0,0xd,0x9>);
	make("321076", index6t<0x3,0x2,0x1,0x0,0x7,0x6>);
	make("048c15", index6t<0x0,0x4,0x8,0xc,0x1,0x5>);
	make("cdef89", index6t<0xc,0xd,0xe,0xf,0x8,0x9>);
	make("fb73ea", index6t<0xf,0xb,0x7,0x3,0xe,0xa>);
	make("234569", index6t<0x2,0x3,0x4,0x5,0x6,0x9>);
	make("bf26a5", index6t<0xb,0xf,0x2,0x6,0xa,0x5>);
	make("dcba96", index6t<0xd,0xc,0xb,0xa,0x9,0x6>);
	make("40d95a", index6t<0x4,0x0,0xd,0x9,0x5,0xa>);
	make("10765a", index6t<0x1,0x0,0x7,0x6,0x5,0xa>);
	make("8c1596", index6t<0x8,0xc,0x1,0x5,0x9,0x6>);
	make("ef89a5", index6t<0xe,0xf,0x8,0x9,0xa,0x5>);
	make("73ea69", index6t<0x7,0x3,0xe,0xa,0x6,0x9>);
	make("01259a", index6t<0x0,0x1,0x2,0x5,0x9,0xa>);
	make("37b659", index6t<0x3,0x7,0xb,0x6,0x5,0x9>);
	make("feda65", index6t<0xf,0xe,0xd,0xa,0x6,0x5>);
	make("c849a6", index6t<0xc,0x8,0x4,0x9,0xa,0x6>);
	make("3216a9", index6t<0x3,0x2,0x1,0x6,0xa,0x9>);
	make("04856a", index6t<0x0,0x4,0x8,0x5,0x6,0xa>);
	make("cde956", index6t<0xc,0xd,0xe,0x9,0x5,0x6>);
	make("fb7a95", index6t<0xf,0xb,0x7,0xa,0x9,0x5>);
	make("345678", index6t<0x3,0x4,0x5,0x6,0x7,0x8>);
	make("f26ae1", index6t<0xf,0x2,0x6,0xa,0xe,0x1>);
	make("cba987", index6t<0xc,0xb,0xa,0x9,0x8,0x7>);
	make("0d951e", index6t<0x0,0xd,0x9,0x5,0x1,0xe>);
	make("07654b", index6t<0x0,0x7,0x6,0x5,0x4,0xb>);
	make("c159d2", index6t<0xc,0x1,0x5,0x9,0xd,0x2>);
	make("f89ab4", index6t<0xf,0x8,0x9,0xa,0xb,0x4>);
	make("3ea62d", index6t<0x3,0xe,0xa,0x6,0x2,0xd>);
	make("134567", index6t<0x1,0x3,0x4,0x5,0x6,0x7>);
	make("7f26ae", index6t<0x7,0xf,0x2,0x6,0xa,0xe>);
	make("ecba98", index6t<0xe,0xc,0xb,0xa,0x9,0x8>);
	make("80d951", index6t<0x8,0x0,0xd,0x9,0x5,0x1>);
	make("207654", index6t<0x2,0x0,0x7,0x6,0x5,0x4>);
	make("4c159d", index6t<0x4,0xc,0x1,0x5,0x9,0xd>);
	make("df89ab", index6t<0xd,0xf,0x8,0x9,0xa,0xb>);
	make("b3ea62", index6t<0xb,0x3,0xe,0xa,0x6,0x2>);
	make("01489a", index6t<0x0,0x1,0x4,0x8,0x9,0xa>);
	make("372159", index6t<0x3,0x7,0x2,0x1,0x5,0x9>);
	make("feb765", index6t<0xf,0xe,0xb,0x7,0x6,0x5>);
	make("c8dea6", index6t<0xc,0x8,0xd,0xe,0xa,0x6>);
	make("327ba9", index6t<0x3,0x2,0x7,0xb,0xa,0x9>);
	make("04126a", index6t<0x0,0x4,0x1,0x2,0x6,0xa>);
	make("cd8456", index6t<0xc,0xd,0x8,0x4,0x5,0x6>);
	make("fbed95", index6t<0xf,0xb,0xe,0xd,0x9,0x5>);

	make("01234", index5t<0x0,0x1,0x2,0x3,0x4>);
	make("c840d", index5t<0xc,0x8,0x4,0x0,0xd>);
	make("fedcb", index5t<0xf,0xe,0xd,0xc,0xb>);
	make("37bf2", index5t<0x3,0x7,0xb,0xf,0x2>);
	make("32107", index5t<0x3,0x2,0x1,0x0,0x7>);
	make("fb73e", index5t<0xf,0xb,0x7,0x3,0xe>);
	make("cdef8", index5t<0xc,0xd,0xe,0xf,0x8>);
	make("048c1", index5t<0x0,0x4,0x8,0xc,0x1>);
	make("45678", index5t<0x4,0x5,0x6,0x7,0x8>);
	make("d951e", index5t<0xd,0x9,0x5,0x1,0xe>);
	make("ba987", index5t<0xb,0xa,0x9,0x8,0x7>);
	make("26ae1", index5t<0x2,0x6,0xa,0xe,0x1>);
	make("7654b", index5t<0x7,0x6,0x5,0x4,0xb>);
	make("ea62d", index5t<0xe,0xa,0x6,0x2,0xd>);
	make("89ab4", index5t<0x8,0x9,0xa,0xb,0x4>);
	make("159d2", index5t<0x1,0x5,0x9,0xd,0x2>);
	make("89abc", index5t<0x8,0x9,0xa,0xb,0xc>);
	make("ea62f", index5t<0xe,0xa,0x6,0x2,0xf>);
	make("76543", index5t<0x7,0x6,0x5,0x4,0x3>);
	make("159d0", index5t<0x1,0x5,0x9,0xd,0x0>);
	make("ba98f", index5t<0xb,0xa,0x9,0x8,0xf>);
	make("d951c", index5t<0xd,0x9,0x5,0x1,0xc>);
	make("45670", index5t<0x4,0x5,0x6,0x7,0x0>);
	make("26ae3", index5t<0x2,0x6,0xa,0xe,0x3>);
	make("01235", index5t<0x0,0x1,0x2,0x3,0x5>);
	make("c8409", index5t<0xc,0x8,0x4,0x0,0x9>);
	make("fedca", index5t<0xf,0xe,0xd,0xc,0xa>);
	make("37bf6", index5t<0x3,0x7,0xb,0xf,0x6>);
	make("32106", index5t<0x3,0x2,0x1,0x0,0x6>);
	make("fb73a", index5t<0xf,0xb,0x7,0x3,0xa>);
	make("cdef9", index5t<0xc,0xd,0xe,0xf,0x9>);
	make("048c5", index5t<0x0,0x4,0x8,0xc,0x5>);
	make("45679", index5t<0x4,0x5,0x6,0x7,0x9>);
	make("d951a", index5t<0xd,0x9,0x5,0x1,0xa>);
	make("ba986", index5t<0xb,0xa,0x9,0x8,0x6>);
	make("26ae5", index5t<0x2,0x6,0xa,0xe,0x5>);
	make("7654a", index5t<0x7,0x6,0x5,0x4,0xa>);
	make("ea629", index5t<0xe,0xa,0x6,0x2,0x9>);
	make("89ab5", index5t<0x8,0x9,0xa,0xb,0x5>);
	make("159d6", index5t<0x1,0x5,0x9,0xd,0x6>);
	make("89abd", index5t<0x8,0x9,0xa,0xb,0xd>);
	make("ea62b", index5t<0xe,0xa,0x6,0x2,0xb>);
	make("76542", index5t<0x7,0x6,0x5,0x4,0x2>);
	make("159d4", index5t<0x1,0x5,0x9,0xd,0x4>);
	make("ba98e", index5t<0xb,0xa,0x9,0x8,0xe>);
	make("d9518", index5t<0xd,0x9,0x5,0x1,0x8>);
	make("45671", index5t<0x4,0x5,0x6,0x7,0x1>);
	make("26ae7", index5t<0x2,0x6,0xa,0xe,0x7>);
	make("01245", index5t<0x0,0x1,0x2,0x4,0x5>);
	make("c84d9", index5t<0xc,0x8,0x4,0xd,0x9>);
	make("fedba", index5t<0xf,0xe,0xd,0xb,0xa>);
	make("37b26", index5t<0x3,0x7,0xb,0x2,0x6>);
	make("32176", index5t<0x3,0x2,0x1,0x7,0x6>);
	make("fb7ea", index5t<0xf,0xb,0x7,0xe,0xa>);
	make("cde89", index5t<0xc,0xd,0xe,0x8,0x9>);
	make("04815", index5t<0x0,0x4,0x8,0x1,0x5>);
	make("12356", index5t<0x1,0x2,0x3,0x5,0x6>);
	make("84095", index5t<0x8,0x4,0x0,0x9,0x5>);
	make("edca9", index5t<0xe,0xd,0xc,0xa,0x9>);
	make("7bf6a", index5t<0x7,0xb,0xf,0x6,0xa>);
	make("21065", index5t<0x2,0x1,0x0,0x6,0x5>);
	make("b73a6", index5t<0xb,0x7,0x3,0xa,0x6>);
	make("def9a", index5t<0xd,0xe,0xf,0x9,0xa>);
	make("48c59", index5t<0x4,0x8,0xc,0x5,0x9>);
	make("45689", index5t<0x4,0x5,0x6,0x8,0x9>);
	make("d95ea", index5t<0xd,0x9,0x5,0xe,0xa>);
	make("ba976", index5t<0xb,0xa,0x9,0x7,0x6>);
	make("26a15", index5t<0x2,0x6,0xa,0x1,0x5>);
	make("765ba", index5t<0x7,0x6,0x5,0xb,0xa>);
	make("ea6d9", index5t<0xe,0xa,0x6,0xd,0x9>);
	make("89a45", index5t<0x8,0x9,0xa,0x4,0x5>);
	make("15926", index5t<0x1,0x5,0x9,0x2,0x6>);
	make("5679a", index5t<0x5,0x6,0x7,0x9,0xa>);
	make("951a6", index5t<0x9,0x5,0x1,0xa,0x6>);
	make("a9865", index5t<0xa,0x9,0x8,0x6,0x5>);
	make("6ae59", index5t<0x6,0xa,0xe,0x5,0x9>);
	make("654a9", index5t<0x6,0x5,0x4,0xa,0x9>);
	make("a6295", index5t<0xa,0x6,0x2,0x9,0x5>);
	make("9ab56", index5t<0x9,0xa,0xb,0x5,0x6>);
	make("59d6a", index5t<0x5,0x9,0xd,0x6,0xa>);
	make("89acd", index5t<0x8,0x9,0xa,0xc,0xd>);
	make("ea6fb", index5t<0xe,0xa,0x6,0xf,0xb>);
	make("76532", index5t<0x7,0x6,0x5,0x3,0x2>);
	make("15904", index5t<0x1,0x5,0x9,0x0,0x4>);
	make("ba9fe", index5t<0xb,0xa,0x9,0xf,0xe>);
	make("d95c8", index5t<0xd,0x9,0x5,0xc,0x8>);
	make("45601", index5t<0x4,0x5,0x6,0x0,0x1>);
	make("26a37", index5t<0x2,0x6,0xa,0x3,0x7>);
	make("9abde", index5t<0x9,0xa,0xb,0xd,0xe>);
	make("a62b7", index5t<0xa,0x6,0x2,0xb,0x7>);
	make("65421", index5t<0x6,0x5,0x4,0x2,0x1>);
	make("59d48", index5t<0x5,0x9,0xd,0x4,0x8>);
	make("a98ed", index5t<0xa,0x9,0x8,0xe,0xd>);
	make("95184", index5t<0x9,0x5,0x1,0x8,0x4>);
	make("56712", index5t<0x5,0x6,0x7,0x1,0x2>);
	make("6ae7b", index5t<0x6,0xa,0xe,0x7,0xb>);
	make("01248", index5t<0x0,0x1,0x2,0x4,0x8>);
	make("c84de", index5t<0xc,0x8,0x4,0xd,0xe>);
	make("fedb7", index5t<0xf,0xe,0xd,0xb,0x7>);
	make("37b21", index5t<0x3,0x7,0xb,0x2,0x1>);
	make("3217b", index5t<0x3,0x2,0x1,0x7,0xb>);
	make("fb7ed", index5t<0xf,0xb,0x7,0xe,0xd>);
	make("cde84", index5t<0xc,0xd,0xe,0x8,0x4>);
	make("04812", index5t<0x0,0x4,0x8,0x1,0x2>);
	make("12359", index5t<0x1,0x2,0x3,0x5,0x9>);
	make("8409a", index5t<0x8,0x4,0x0,0x9,0xa>);
	make("edca6", index5t<0xe,0xd,0xc,0xa,0x6>);
	make("7bf65", index5t<0x7,0xb,0xf,0x6,0x5>);
	make("2106a", index5t<0x2,0x1,0x0,0x6,0xa>);
	make("b73a9", index5t<0xb,0x7,0x3,0xa,0x9>);
	make("def95", index5t<0xd,0xe,0xf,0x9,0x5>);
	make("48c56", index5t<0x4,0x8,0xc,0x5,0x6>);
	make("4568c", index5t<0x4,0x5,0x6,0x8,0xc>);
	make("d95ef", index5t<0xd,0x9,0x5,0xe,0xf>);
	make("ba973", index5t<0xb,0xa,0x9,0x7,0x3>);
	make("26a10", index5t<0x2,0x6,0xa,0x1,0x0>);
	make("765bf", index5t<0x7,0x6,0x5,0xb,0xf>);
	make("ea6dc", index5t<0xe,0xa,0x6,0xd,0xc>);
	make("89a40", index5t<0x8,0x9,0xa,0x4,0x0>);
	make("15923", index5t<0x1,0x5,0x9,0x2,0x3>);
	make("5679d", index5t<0x5,0x6,0x7,0x9,0xd>);
	make("951ab", index5t<0x9,0x5,0x1,0xa,0xb>);
	make("a9862", index5t<0xa,0x9,0x8,0x6,0x2>);
	make("6ae54", index5t<0x6,0xa,0xe,0x5,0x4>);
	make("654ae", index5t<0x6,0x5,0x4,0xa,0xe>);
	make("a6298", index5t<0xa,0x6,0x2,0x9,0x8>);
	make("9ab51", index5t<0x9,0xa,0xb,0x5,0x1>);
	make("59d67", index5t<0x5,0x9,0xd,0x6,0x7>);

	make("0123", index4t<0x0,0x1,0x2,0x3>);
	make("c840", index4t<0xc,0x8,0x4,0x0>);
	make("fedc", index4t<0xf,0xe,0xd,0xc>);
	make("37bf", index4t<0x3,0x7,0xb,0xf>);
	make("3210", index4t<0x3,0x2,0x1,0x0>);
	make("fb73", index4t<0xf,0xb,0x7,0x3>);
	make("cdef", index4t<0xc,0xd,0xe,0xf>);
	make("048c", index4t<0x0,0x4,0x8,0xc>);
	make("4567", index4t<0x4,0x5,0x6,0x7>);
	make("d951", index4t<0xd,0x9,0x5,0x1>);
	make("ba98", index4t<0xb,0xa,0x9,0x8>);
	make("26ae", index4t<0x2,0x6,0xa,0xe>);
	make("7654", index4t<0x7,0x6,0x5,0x4>);
	make("ea62", index4t<0xe,0xa,0x6,0x2>);
	make("89ab", index4t<0x8,0x9,0xa,0xb>);
	make("159d", index4t<0x1,0x5,0x9,0xd>);
	make("89ab", index4t<0x8,0x9,0xa,0xb>); // legacy non-isomorphic
	make("cdef", index4t<0xc,0xd,0xe,0xf>); // legacy non-isomorphic
	make("048c", index4t<0x0,0x4,0x8,0xc>); // legacy non-isomorphic
	make("159d", index4t<0x1,0x5,0x9,0xd>); // legacy non-isomorphic
	make("26ae", index4t<0x2,0x6,0xa,0xe>); // legacy non-isomorphic
	make("37bf", index4t<0x3,0x7,0xb,0xf>); // legacy non-isomorphic
	make("0124", index4t<0x0,0x1,0x2,0x4>);
	make("c84d", index4t<0xc,0x8,0x4,0xd>);
	make("fedb", index4t<0xf,0xe,0xd,0xb>);
	make("37b2", index4t<0x3,0x7,0xb,0x2>);
	make("3217", index4t<0x3,0x2,0x1,0x7>);
	make("fb7e", index4t<0xf,0xb,0x7,0xe>);
	make("cde8", index4t<0xc,0xd,0xe,0x8>);
	make("0481", index4t<0x0,0x4,0x8,0x1>);
	make("1235", index4t<0x1,0x2,0x3,0x5>);
	make("8409", index4t<0x8,0x4,0x0,0x9>);
	make("edca", index4t<0xe,0xd,0xc,0xa>);
	make("7bf6", index4t<0x7,0xb,0xf,0x6>);
	make("2106", index4t<0x2,0x1,0x0,0x6>);
	make("b73a", index4t<0xb,0x7,0x3,0xa>);
	make("def9", index4t<0xd,0xe,0xf,0x9>);
	make("48c5", index4t<0x4,0x8,0xc,0x5>);
	make("4568", index4t<0x4,0x5,0x6,0x8>);
	make("d95e", index4t<0xd,0x9,0x5,0xe>);
	make("ba97", index4t<0xb,0xa,0x9,0x7>);
	make("26a1", index4t<0x2,0x6,0xa,0x1>);
	make("765b", index4t<0x7,0x6,0x5,0xb>);
	make("ea6d", index4t<0xe,0xa,0x6,0xd>);
	make("89a4", index4t<0x8,0x9,0xa,0x4>);
	make("1592", index4t<0x1,0x5,0x9,0x2>);
	make("5679", index4t<0x5,0x6,0x7,0x9>);
	make("951a", index4t<0x9,0x5,0x1,0xa>);
	make("a986", index4t<0xa,0x9,0x8,0x6>);
	make("6ae5", index4t<0x6,0xa,0xe,0x5>);
	make("654a", index4t<0x6,0x5,0x4,0xa>);
	make("a629", index4t<0xa,0x6,0x2,0x9>);
	make("9ab5", index4t<0x9,0xa,0xb,0x5>);
	make("59d6", index4t<0x5,0x9,0xd,0x6>);
	make("89ac", index4t<0x8,0x9,0xa,0xc>);
	make("ea6f", index4t<0xe,0xa,0x6,0xf>);
	make("7653", index4t<0x7,0x6,0x5,0x3>);
	make("1590", index4t<0x1,0x5,0x9,0x0>);
	make("ba9f", index4t<0xb,0xa,0x9,0xf>);
	make("d95c", index4t<0xd,0x9,0x5,0xc>);
	make("4560", index4t<0x4,0x5,0x6,0x0>);
	make("26a3", index4t<0x2,0x6,0xa,0x3>);
	make("9abd", index4t<0x9,0xa,0xb,0xd>);
	make("a62b", index4t<0xa,0x6,0x2,0xb>);
	make("6542", index4t<0x6,0x5,0x4,0x2>);
	make("59d4", index4t<0x5,0x9,0xd,0x4>);
	make("a98e", index4t<0xa,0x9,0x8,0xe>);
	make("9518", index4t<0x9,0x5,0x1,0x8>);
	make("5671", index4t<0x5,0x6,0x7,0x1>);
	make("6ae7", index4t<0x6,0xa,0xe,0x7>);
	make("0125", index4t<0x0,0x1,0x2,0x5>);
	make("c849", index4t<0xc,0x8,0x4,0x9>);
	make("feda", index4t<0xf,0xe,0xd,0xa>);
	make("37b6", index4t<0x3,0x7,0xb,0x6>);
	make("3216", index4t<0x3,0x2,0x1,0x6>);
	make("fb7a", index4t<0xf,0xb,0x7,0xa>);
	make("cde9", index4t<0xc,0xd,0xe,0x9>);
	make("0485", index4t<0x0,0x4,0x8,0x5>);
	make("1236", index4t<0x1,0x2,0x3,0x6>);
	make("8405", index4t<0x8,0x4,0x0,0x5>);
	make("edc9", index4t<0xe,0xd,0xc,0x9>);
	make("7bfa", index4t<0x7,0xb,0xf,0xa>);
	make("2105", index4t<0x2,0x1,0x0,0x5>);
	make("b736", index4t<0xb,0x7,0x3,0x6>);
	make("defa", index4t<0xd,0xe,0xf,0xa>);
	make("48c9", index4t<0x4,0x8,0xc,0x9>);
	make("4569", index4t<0x4,0x5,0x6,0x9>);
	make("d95a", index4t<0xd,0x9,0x5,0xa>);
	make("ba96", index4t<0xb,0xa,0x9,0x6>);
	make("26a5", index4t<0x2,0x6,0xa,0x5>);
	make("765a", index4t<0x7,0x6,0x5,0xa>);
	make("ea69", index4t<0xe,0xa,0x6,0x9>);
	make("89a5", index4t<0x8,0x9,0xa,0x5>);
	make("1596", index4t<0x1,0x5,0x9,0x6>);
	make("567a", index4t<0x5,0x6,0x7,0xa>);
	make("9516", index4t<0x9,0x5,0x1,0x6>);
	make("a985", index4t<0xa,0x9,0x8,0x5>);
	make("6ae9", index4t<0x6,0xa,0xe,0x9>);
	make("6549", index4t<0x6,0x5,0x4,0x9>);
	make("a625", index4t<0xa,0x6,0x2,0x5>);
	make("9ab6", index4t<0x9,0xa,0xb,0x6>);
	make("59da", index4t<0x5,0x9,0xd,0xa>);
	make("89ad", index4t<0x8,0x9,0xa,0xd>);
	make("ea6b", index4t<0xe,0xa,0x6,0xb>);
	make("7652", index4t<0x7,0x6,0x5,0x2>);
	make("1594", index4t<0x1,0x5,0x9,0x4>);
	make("ba9e", index4t<0xb,0xa,0x9,0xe>);
	make("d958", index4t<0xd,0x9,0x5,0x8>);
	make("4561", index4t<0x4,0x5,0x6,0x1>);
	make("26a7", index4t<0x2,0x6,0xa,0x7>);
	make("9abe", index4t<0x9,0xa,0xb,0xe>);
	make("a627", index4t<0xa,0x6,0x2,0x7>);
	make("6541", index4t<0x6,0x5,0x4,0x1>);
	make("59d8", index4t<0x5,0x9,0xd,0x8>);
	make("a98d", index4t<0xa,0x9,0x8,0xd>);
	make("9514", index4t<0x9,0x5,0x1,0x4>);
	make("5672", index4t<0x5,0x6,0x7,0x2>);
	make("6aeb", index4t<0x6,0xa,0xe,0xb>);
	make("0145", index4t<0x0,0x1,0x4,0x5>);
	make("c8d9", index4t<0xc,0x8,0xd,0x9>);
	make("feba", index4t<0xf,0xe,0xb,0xa>);
	make("3726", index4t<0x3,0x7,0x2,0x6>);
	make("3276", index4t<0x3,0x2,0x7,0x6>);
	make("fbea", index4t<0xf,0xb,0xe,0xa>);
	make("cd89", index4t<0xc,0xd,0x8,0x9>);
	make("0415", index4t<0x0,0x4,0x1,0x5>);
	make("1256", index4t<0x1,0x2,0x5,0x6>);
	make("8495", index4t<0x8,0x4,0x9,0x5>);
	make("eda9", index4t<0xe,0xd,0xa,0x9>);
	make("7b6a", index4t<0x7,0xb,0x6,0xa>);
	make("2165", index4t<0x2,0x1,0x6,0x5>);
	make("b7a6", index4t<0xb,0x7,0xa,0x6>);
	make("de9a", index4t<0xd,0xe,0x9,0xa>);
	make("4859", index4t<0x4,0x8,0x5,0x9>);
	make("569a", index4t<0x5,0x6,0x9,0xa>);
	make("95a6", index4t<0x9,0x5,0xa,0x6>);
	make("a965", index4t<0xa,0x9,0x6,0x5>);
	make("6a59", index4t<0x6,0xa,0x5,0x9>);
	make("65a9", index4t<0x6,0x5,0xa,0x9>);
	make("a695", index4t<0xa,0x6,0x9,0x5>);
	make("9a56", index4t<0x9,0xa,0x5,0x6>);
	make("596a", index4t<0x5,0x9,0x6,0xa>);
	make("2367", index4t<0x2,0x3,0x6,0x7>); // legacy non-isomorphic
	make("4589", index4t<0x4,0x5,0x8,0x9>); // legacy non-isomorphic
	make("67ab", index4t<0x6,0x7,0xa,0xb>); // legacy non-isomorphic
	make("89cd", index4t<0x8,0x9,0xc,0xd>); // legacy non-isomorphic
	make("9ade", index4t<0x9,0xa,0xd,0xe>); // legacy non-isomorphic
	make("abef", index4t<0xa,0xb,0xe,0xf>); // legacy non-isomorphic

	make("01234567", index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	make("c840d951", index8t<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	make("fedcba98", index8t<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	make("37bf26ae", index8t<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	make("32107654", index8t<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	make("fb73ea62", index8t<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	make("cdef89ab", index8t<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	make("048c159d", index8t<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	make("456789ab", index8t<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	make("d951ea62", index8t<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	make("ba987654", index8t<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	make("26ae159d", index8t<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	make("7654ba98", index8t<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	make("ea62d951", index8t<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	make("89ab4567", index8t<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	make("159d26ae", index8t<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	make("01234589", index8t<0x0,0x1,0x2,0x3,0x4,0x5,0x8,0x9>);
	make("c840d9ea", index8t<0xc,0x8,0x4,0x0,0xd,0x9,0xe,0xa>);
	make("fedcba76", index8t<0xf,0xe,0xd,0xc,0xb,0xa,0x7,0x6>);
	make("37bf2615", index8t<0x3,0x7,0xb,0xf,0x2,0x6,0x1,0x5>);
	make("321076ba", index8t<0x3,0x2,0x1,0x0,0x7,0x6,0xb,0xa>);
	make("fb73ead9", index8t<0xf,0xb,0x7,0x3,0xe,0xa,0xd,0x9>);
	make("cdef8945", index8t<0xc,0xd,0xe,0xf,0x8,0x9,0x4,0x5>);
	make("048c1526", index8t<0x0,0x4,0x8,0xc,0x1,0x5,0x2,0x6>);
	make("01245689", index8t<0x0,0x1,0x2,0x4,0x5,0x6,0x8,0x9>);
	make("c84d95ea", index8t<0xc,0x8,0x4,0xd,0x9,0x5,0xe,0xa>);
	make("fedba976", index8t<0xf,0xe,0xd,0xb,0xa,0x9,0x7,0x6>);
	make("37b26a15", index8t<0x3,0x7,0xb,0x2,0x6,0xa,0x1,0x5>);
	make("321765ba", index8t<0x3,0x2,0x1,0x7,0x6,0x5,0xb,0xa>);
	make("fb7ea6d9", index8t<0xf,0xb,0x7,0xe,0xa,0x6,0xd,0x9>);
	make("cde89a45", index8t<0xc,0xd,0xe,0x8,0x9,0xa,0x4,0x5>);
	make("04815926", index8t<0x0,0x4,0x8,0x1,0x5,0x9,0x2,0x6>);

	make("0123456", index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x6>);
	make("c840d95", index7t<0xc,0x8,0x4,0x0,0xd,0x9,0x5>);
	make("fedcba9", index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x9>);
	make("37bf26a", index7t<0x3,0x7,0xb,0xf,0x2,0x6,0xa>);
	make("3210765", index7t<0x3,0x2,0x1,0x0,0x7,0x6,0x5>);
	make("fb73ea6", index7t<0xf,0xb,0x7,0x3,0xe,0xa,0x6>);
	make("cdef89a", index7t<0xc,0xd,0xe,0xf,0x8,0x9,0xa>);
	make("048c159", index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x9>);
	make("456789a", index7t<0x4,0x5,0x6,0x7,0x8,0x9,0xa>);
	make("d951ea6", index7t<0xd,0x9,0x5,0x1,0xe,0xa,0x6>);
	make("ba98765", index7t<0xb,0xa,0x9,0x8,0x7,0x6,0x5>);
	make("26ae159", index7t<0x2,0x6,0xa,0xe,0x1,0x5,0x9>);
	make("7654ba9", index7t<0x7,0x6,0x5,0x4,0xb,0xa,0x9>);
	make("ea62d95", index7t<0xe,0xa,0x6,0x2,0xd,0x9,0x5>);
	make("89ab456", index7t<0x8,0x9,0xa,0xb,0x4,0x5,0x6>);
	make("159d26a", index7t<0x1,0x5,0x9,0xd,0x2,0x6,0xa>);
	make("89abcde", index7t<0x8,0x9,0xa,0xb,0xc,0xd,0xe>);
	make("ea62fb7", index7t<0xe,0xa,0x6,0x2,0xf,0xb,0x7>);
	make("7654321", index7t<0x7,0x6,0x5,0x4,0x3,0x2,0x1>);
	make("159d048", index7t<0x1,0x5,0x9,0xd,0x0,0x4,0x8>);
	make("ba98fed", index7t<0xb,0xa,0x9,0x8,0xf,0xe,0xd>);
	make("d951c84", index7t<0xd,0x9,0x5,0x1,0xc,0x8,0x4>);
	make("4567012", index7t<0x4,0x5,0x6,0x7,0x0,0x1,0x2>);
	make("26ae37b", index7t<0x2,0x6,0xa,0xe,0x3,0x7,0xb>);
	make("0123458", index7t<0x0,0x1,0x2,0x3,0x4,0x5,0x8>);
	make("c840d9e", index7t<0xc,0x8,0x4,0x0,0xd,0x9,0xe>);
	make("fedcba7", index7t<0xf,0xe,0xd,0xc,0xb,0xa,0x7>);
	make("37bf261", index7t<0x3,0x7,0xb,0xf,0x2,0x6,0x1>);
	make("321076b", index7t<0x3,0x2,0x1,0x0,0x7,0x6,0xb>);
	make("fb73ead", index7t<0xf,0xb,0x7,0x3,0xe,0xa,0xd>);
	make("cdef894", index7t<0xc,0xd,0xe,0xf,0x8,0x9,0x4>);
	make("048c152", index7t<0x0,0x4,0x8,0xc,0x1,0x5,0x2>);
	make("456789c", index7t<0x4,0x5,0x6,0x7,0x8,0x9,0xc>);
	make("d951eaf", index7t<0xd,0x9,0x5,0x1,0xe,0xa,0xf>);
	make("ba98763", index7t<0xb,0xa,0x9,0x8,0x7,0x6,0x3>);
	make("26ae150", index7t<0x2,0x6,0xa,0xe,0x1,0x5,0x0>);
	make("7654baf", index7t<0x7,0x6,0x5,0x4,0xb,0xa,0xf>);
	make("ea62d9c", index7t<0xe,0xa,0x6,0x2,0xd,0x9,0xc>);
	make("89ab450", index7t<0x8,0x9,0xa,0xb,0x4,0x5,0x0>);
	make("159d263", index7t<0x1,0x5,0x9,0xd,0x2,0x6,0x3>);

	make("ff000000", indexmerge0);
	make("ff000001", indexmerge1<0>);
	make("ff000011", indexmerge1<1>);
	make("fe000000", indexnum0);
	make("fe000001", indexnum1);
	make("fe000002", indexnum2);
	make("fe000082", indexnum2x<0, 0, 1>);
	make("fe000092", indexnum2x<0, 2, 3>);
	make("fe0000c2", indexnum2x<1, 0, 1>);
	make("fe0000d2", indexnum2x<1, 2, 3>);
	make("fe000003", indexnum3);
	make("fe000004", indexnum4);
	make("fe000005", indexnum5lt);
	make("fe000015", indexnum5st);
	make("fd012301", indexmono<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>);
	make("fd37bf01", indexmono<0x3,0x7,0xb,0xf,0x2,0x6,0xa,0xe>);
	make("fdfedc01", indexmono<0xf,0xe,0xd,0xc,0xb,0xa,0x9,0x8>);
	make("fdc84001", indexmono<0xc,0x8,0x4,0x0,0xd,0x9,0x5,0x1>);
	make("fd321001", indexmono<0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4>);
	make("fdfb7301", indexmono<0xf,0xb,0x7,0x3,0xe,0xa,0x6,0x2>);
	make("fdcdef01", indexmono<0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb>);
	make("fd048c01", indexmono<0x0,0x4,0x8,0xc,0x1,0x5,0x9,0xd>);
	make("fd456701", indexmono<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>);
	make("fd26ae01", indexmono<0x2,0x6,0xa,0xe,0x1,0x5,0x9,0xd>);
	make("fdba9801", indexmono<0xb,0xa,0x9,0x8,0x7,0x6,0x5,0x4>);
	make("fdd95101", indexmono<0xd,0x9,0x5,0x1,0xe,0xa,0x6,0x2>);
	make("fd765401", indexmono<0x7,0x6,0x5,0x4,0xb,0xa,0x9,0x8>);
	make("fdea6201", indexmono<0xe,0xa,0x6,0x2,0xd,0x9,0x5,0x1>);
	make("fd89ab01", indexmono<0x8,0x9,0xa,0xb,0x4,0x5,0x6,0x7>);
	make("fd159d01", indexmono<0x1,0x5,0x9,0xd,0x2,0x6,0xa,0xe>);
	make("fc000000", indexmax<0>);
	make("fc000010", indexmax<1>);
	make("fc000020", indexmax<2>);
	make("fc000030", indexmax<3>);
	make("fc000040", indexmax<4>);
	make("fc000050", indexmax<5>);
	make("fc000060", indexmax<6>);
	make("fc000070", indexmax<7>);

	indexapt::make<0, 256>();
}

} // namespace utils

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
		std::string value(const std::string& def) const { auto val = value(); return val.size() ? val : def; }
		numeric value(numeric def) const { try { return numeric(*this); } catch (std::invalid_argument&) {} return def; }
		std::string operator +(const std::string& val) { return value() + val; }
		friend std::string operator +(const std::string& val, const opinion& opi) { return val + opi.value(); }
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
		std::string value(const std::string& def) const { auto val = value(); return val.size() ? val : def; }
		numeric value(numeric def) const { try { return numeric(*this); } catch (std::invalid_argument&) {} return def; }
		std::string operator +(const std::string& val) { return value() + val; }
		friend std::string operator +(const std::string& val, const option& opt) { return val + opt.value(); }
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

void init_logging(utils::options::option opt) {
	static std::ofstream logout;
	for (std::string path : opt) {
		char type = path[path.find_last_of(".") + 1];
		if (logout.is_open() || (type != 'x' && type != 'l')) continue; // .x and .log are suffix for log files
		logout.open(path, std::ios::out | std::ios::app);
	}
	if (!logout.is_open()) return;
	static moporgic::teestream tee(std::cout, logout);
	static moporgic::redirector redirect(tee, std::cout);
}

std::map<std::string, std::string> aliases() {
	std::map<std::string, std::string> alias;
	alias["4x6patt/khyeh"] = "012345:012345! 456789:456789! 012456:012456! 45689a:45689a! ";
	alias["khyeh"] = alias["4x6patt/khyeh"];
	alias["5x6patt/42-33"] = "012345:012345! 456789:456789! 89abcd:89abcd! 012456:012456! 45689a:45689a! ";
	alias["2x4patt/4"] = "0123:0123! 4567:4567! ";
	alias["5x4patt/4-22"] = alias["2x4patt/4"] + "0145:0145! 1256:1256! 569a:569a! ";
	alias["2x8patt/44"] = "01234567:01234567! 456789ab:456789ab! ";
	alias["3x8patt/44-4211"] = alias["2x8patt/44"] + "0123458c:0123458c! ";
	alias["4x6patt/k.matsuzaki"] = "012456:012456! 456789:456789! 012345:012345! 234569:234569! ";
	alias["5x6patt/k.matsuzaki"] = alias["4x6patt/k.matsuzaki"] + "01259a:01259a! ";
	alias["6x6patt/k.matsuzaki"] = alias["5x6patt/k.matsuzaki"] + "345678:345678! ";
	alias["7x6patt/k.matsuzaki"] = alias["6x6patt/k.matsuzaki"] + "134567:134567! ";
	alias["8x6patt/k.matsuzaki"] = alias["7x6patt/k.matsuzaki"] + "01489a:01489a! ";
	alias["4x6patt/redundant"] = alias["4x6patt/khyeh"] + "01234:01234! 45678:45678! 01235:01235! 45679:45679! 01245:01245! 45689:45689! "
	                             "0124:0124! 1235:1235! 0125:0125! 4568:4568! 5679:5679! 4569:4569! " + alias["5x4patt/4-22"];
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
	alias["8x4patt"] = "0123 4567 89ab cdef 048c 159d 26ae 37bf ";
	alias["5x4patt"] = alias["5x4patt/4-22"];
	alias["2x4patt"] = alias["2x4patt/4"];
	alias["2x8patt"] = alias["2x8patt/44"];
	alias["3x8patt"] = alias["3x8patt/44-4211"];
	alias["default"] = alias["4x6patt"];
	return alias;
}
void make_network(utils::options::option opt) {
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

	auto stov = [](std::string hash, u32 iso = 0) -> std::vector<u32> {
		std::vector<u32> patt;
		board x(0xfedcba9876543210ull); x.isomorphic(-iso);
		for (char tile : hash) patt.push_back(x.at((tile & 15) + (tile & 64 ? 9 : 0)));
		return patt;
	};
	auto vtos = [](std::vector<u32> patt) -> std::string {
		std::string hash;
		for (u32 tile : patt) hash.push_back(tile <= 9 ? tile + '0' : tile - 10 + 'a');
		return hash;
	};

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
			for (u32 iso = 0; iso < 8; iso++) lvals.push_back(vtos(stov(hash, iso)) + tail);
		}
		if (rvals.back().find('!') != npos) {
			std::string rval = rvals.back(); rvals.clear();
			std::string hash = rval.substr(0, rval.find('!')).substr(1);
			std::string tail = rval.substr(rval.find('!') + 1);
			for (u32 iso = 0; iso < 8; iso++) rvals.push_back(':' + vtos(stov(hash, iso)) + tail);
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
		std::string wght, idxr;
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
			std::string sign = wtok.find('&') == npos && wtok.find('|') == npos ? name : ({
				std::stringstream ss;
				u64 mska = wtok.find('&') != npos ? std::stoull(wtok.substr(wtok.find('&') + 1), nullptr, 16) : -1ull;
				u64 msko = wtok.find('|') != npos ? std::stoull(wtok.substr(wtok.find('|') + 1), nullptr, 16) : 0ull;
				ss << std::hex << std::setfill('0') << std::setw(8) << ((std::stoull(name, nullptr, 16) & mska) | msko);
				ss.str();
			});
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
				weight src(init.substr(0, init.find('}')).substr(init.find('{') + 1));
				size = std::max(size, src.size());
			} else if (init == "{}") {
				size = 0;
			}
			if (weight(sign) && weight(sign).size() != size)
				weight::erase(sign);
			if (!weight(sign) && size) { // weight id matching: 012345 <--> 00012345
				weight test(std::string(std::max(8 - sign.size(), size_t(8)), '0') + sign);
				while (!test && (test.sign() + ' ')[0] == '0') test = weight(test.sign().substr(1));
				if (test.size() == size) raw_cast<std::string>(weight::wghts().at(test.sign())) = sign; // unsafe!
			}
			if (!weight(sign) && size) { // create new weight table
				weight dst = weight::make(sign, size);
				if (init.find_first_of("{}") != npos && init != "{}") {
					weight src(init.substr(0, init.find('}')).substr(init.find('{') + 1));
					for (size_t n = 0; n < dst.size(); n += src.size()) {
						std::copy_n(src.data(), src.size(), dst.data() + n);
					}
				} else if (init.find_first_of("0123456789.+-") == 0) {
					numeric val = std::stod(init) * (init.find("norm") != npos ? std::pow(num, -1) : 1);
					std::fill_n(dst.data(), dst.size(), val);
				}
			} else if (weight(sign) && size) { // table already exists
				weight dst = weight(sign);
				if (init.find_first_of("+-") == 0) {
					numeric off = std::stod(init) * (init.find("norm") != npos ? std::pow(num, -1) : 1);
					std::transform(dst.data(), dst.data() + dst.size(), dst.data(), [=](numeric val) { return val += off; });
				}
			}
			wght = weight(sign).sign();
		}

		if (itok.size()) {
			// indexer indexer,indexer,indexer pattern! signature&mask|mask
			std::string name = itok.substr(0, itok.find_first_of("!&|"));
			std::string sign = itok.find('&') == npos && itok.find('|') == npos ? name : ({
				std::stringstream ss;
				u64 mska = itok.find('&') != npos ? std::stoull(itok.substr(itok.find('&') + 1), nullptr, 16) : -1ull;
				u64 msko = itok.find('|') != npos ? std::stoull(itok.substr(itok.find('|') + 1), nullptr, 16) : 0ull;
				ss << std::hex << std::setfill('0') << std::setw(8) << ((std::stoull(name, nullptr, 16) & mska) | msko);
				ss.str();
			});
			if (!indexer(sign)) {
				indexer::mapper index = indexer(name).index();
				if (!index) index = index::indexapt(std::bind(index::indexnta, std::placeholders::_1, stov(name)));
				indexer::make(sign, index);
			}
			idxr = indexer(sign).sign();
		}

		if (wght.size() && idxr.size() && !feature(wght, idxr)) feature::make(wght, idxr);
	}
}
void load_network(utils::options::option opt) {
	for (std::string path : opt) {
		std::ifstream in;
		in.open(path, std::ios::in | std::ios::binary);
		while (in.peek() != -1) {
			char type = in.peek();
			if (type != 0) { // new binaries already store its type, so use it for the later loading
				in.ignore(1);
			} else { // legacy binaries always beginning with 0, so use name suffix to determine the type
				type = path[path.find_last_of(".") + 1];
			}
			if (type == 'w')  weight::load(in);
			if (type == 'f') feature::load(in);
			if (type == 'c')   cache::load(in);
		}
		in.close();
	}
	for (feature f : list<feature>(std::move(feature::feats()))) {
		feature::make(f.value().sign(), f.index().sign());
	}
}
void save_network(utils::options::option opt) {
	char buf[1 << 20];
	for (std::string path : opt) {
		char type = path[path.find_last_of(".") + 1];
		if (type == 'x' || type == 'l') continue; // .x and .log are suffix for log files
		std::ofstream out;
		out.rdbuf()->pubsetbuf(buf, sizeof(buf));
		out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) continue;
		// for upward compatibility, we still write legacy binaries for traditional suffixes
		if (type != 'c') {
			if (type != 'f')  weight::save(type != 'w' ? out.write("w", 1) : out);
			if (type != 'w') feature::save(type != 'f' ? out.write("f", 1) : out);
		} else { // .c is reserved for cache binary
			cache::save(type != 'c' ? out.write("c", 1) : out);
		}
		out.flush();
		out.close();
	}
}

void list_network() {
	for (weight w : weight::wghts()) {
		std::stringstream buf;

		buf << w.sign();
		buf << "[";
		if (w.size() >> 30)
			buf << (w.size() >> 30) << "G";
		else if (w.size() >> 20)
			buf << (w.size() >> 20) << "M";
		else if (w.size() >> 10)
			buf << (w.size() >> 10) << "k";
		else
			buf << (w.size());
		buf << "]";

		buf << " : (unused)";
		buf.seekp(-9, std::ios::end);
		for (feature f : feature::feats())
			if (f.value() == w) buf << " " << f.index().sign();

		std::cout << buf.rdbuf() << std::endl;
	}
	std::cout << std::endl;
}

void init_cache(std::string res = "") {
	std::string in(res);
	if (in == "default") in = "2G";

	if (in.size()) {
		size_t unit = 0, size = std::stoull(in, &unit);
		if (unit < in.size())
			switch (std::toupper(in[unit])) {
			case 'K': size *= ((1ULL << 10) / sizeof(cache::block)); break;
			case 'M': size *= ((1ULL << 20) / sizeof(cache::block)); break;
			case 'G': size *= ((1ULL << 30) / sizeof(cache::block)); break;
			}
		bool peek = (in.find("+peek") != std::string::npos);
		cache::make(size, peek);
	}
}

} // utils


struct method {
	typedef numeric(*estimator)(const board&, clip<feature>);
	typedef numeric(*optimizer)(const board&, numeric, clip<feature>);

	estimator estim;
	optimizer optim;
	constexpr inline method(estimator estim = estimate, optimizer optim = optimize) : estim(estim), optim(optim) {}
	constexpr inline operator estimator() const { return estim; }
	constexpr inline operator optimizer() const { return optim; }

	constexpr static inline numeric estimate(const board& state, clip<feature> range = feature::feats()) {
		register numeric esti = 0;
		for (register feature& feat : range)
			esti += feat[state];
		return esti;
	}
	constexpr static inline numeric optimize(const board& state, numeric error, clip<feature> range = feature::feats()) {
		register numeric esti = 0;
		for (register feature& feat : range)
			esti += (feat[state] += error);
		return esti;
	}
	constexpr static inline numeric illegal(const board& state, clip<feature> range = feature::feats()) {
		return -std::numeric_limits<numeric>::max();
	}

	template<indexer::mapper... indexes>
	struct isomorphism {
		constexpr static std::array<indexer::mapper, sizeof...(indexes)> index = { indexes... };
		constexpr operator method() { return { isomorphism<indexes...>::estimate, isomorphism<indexes...>::optimize }; }

		template<indexer::mapper index, indexer::mapper... follow> constexpr static
		inline_always typename std::enable_if<(sizeof...(follow) != 0), numeric>::type invoke(const board& iso, clip<feature> f) {
			return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3][index(iso)]) + invoke<follow...>(iso, f);
		}
		template<indexer::mapper index, indexer::mapper... follow> constexpr static
		inline_always typename std::enable_if<(sizeof...(follow) == 0), numeric>::type invoke(const board& iso, clip<feature> f) {
			return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3][index(iso)]);
		}

		template<indexer::mapper index, indexer::mapper... follow> constexpr static
		inline_always typename std::enable_if<(sizeof...(follow) != 0), numeric>::type invoke(const board& iso, numeric updv, clip<feature> f) {
			return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3][index(iso)] += updv) + invoke<follow...>(iso, updv, f);
		}
		template<indexer::mapper index, indexer::mapper... follow> constexpr static
		inline_always typename std::enable_if<(sizeof...(follow) == 0), numeric>::type invoke(const board& iso, numeric updv, clip<feature> f) {
			return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3][index(iso)] += updv);
		}

		constexpr static inline numeric estimate(const board& state, clip<feature> range = feature::feats()) {
			register numeric esti = 0;
			register board iso;
			esti += invoke<indexes...>(({ iso = state;     iso; }), range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), range);
			return esti;
		}
		constexpr static inline numeric optimize(const board& state, numeric updv, clip<feature> range = feature::feats()) {
			register numeric esti = 0;
			register board iso;
			esti += invoke<indexes...>(({ iso = state;     iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.transpose(); iso; }), updv, range);
			esti += invoke<indexes...>(({ iso.mirror();    iso; }), updv, range);
			return esti;
		}
	};

	template<typename source = method>
	struct expectimax {
		constexpr expectimax(utils::options::option depth) {
			u32 n = expectimax<source>::depth(depth);
			std::stringstream in(({
				std::string limit = depth.find("limit", depth.value());
				while (limit.find(',') != std::string::npos) limit[limit.find(',')] = ' ';
				limit;
			}));
			for (u32& lim : expectimax<source>::limit()) {
				in >> n;
				lim = n & -2u;
			}
		}
		constexpr operator method() { return { expectimax<source>::estimate, expectimax<source>::optimize }; }

		inline static numeric search_expt(const board& after, u32 depth, clip<feature> range = feature::feats()) {
			numeric expt = 0;
			hexa spaces;
			if (depth) depth = std::min(depth, limit((spaces = after.spaces()).size()));
			cache::block::access lookup = cache::find(after, depth);
			if (lookup) return lookup.fetch();
			if (!depth) return source::estimate(after, range);
			for (u32 pos : spaces) {
				board before = after;
				expt += 0.9 * search_best(({ before.set(pos, 1); before; }), depth - 1, range);
				expt += 0.1 * search_best(({ before.set(pos, 2); before; }), depth - 1, range);
			}
			expt = lookup.store(expt / spaces.size());
			return expt;
		}

		inline static numeric search_best(const board& before, u32 depth, clip<feature> range = feature::feats()) {
			numeric best = 0;
			for (const board& after : before.afters()) {
				auto search = after.info() != -1u ? search_expt : search_illegal;
				best = std::max(best, after.info() + search(after, depth - 1, range));
			}
			return best;
		}

		inline static numeric search_illegal(const board& after, u32 depth, clip<feature> range = feature::feats()) {
			return -std::numeric_limits<numeric>::max();
		}

		inline static numeric estimate(const board& after, clip<feature> range = feature::feats()) {
			return search_expt(after, depth() - 1, range);
		}

		inline static numeric optimize(const board& state, numeric updv, clip<feature> range = feature::feats()) {
			return source::optimize(state, updv, range);
		}

		inline static u32& depth() { static u32 depth = 1; return depth; }
		inline static u32& depth(u32 n) { return (expectimax<source>::depth() = n); }
		inline static std::array<u32, 17>& limit() { static std::array<u32, 17> limit = {}; return limit; }
		inline static u32& limit(u32 e) { return limit()[e]; }
	};

	typedef isomorphism<
			index::index6t<0x0,0x1,0x2,0x3,0x4,0x5>,
			index::index6t<0x4,0x5,0x6,0x7,0x8,0x9>,
			index::index6t<0x0,0x1,0x2,0x4,0x5,0x6>,
			index::index6t<0x4,0x5,0x6,0x8,0x9,0xa>> isomorphic4x6patt;
	typedef isomorphism<
			index::index6t<0x0,0x1,0x2,0x3,0x4,0x5>,
			index::index6t<0x4,0x5,0x6,0x7,0x8,0x9>,
			index::index6t<0x8,0x9,0xa,0xb,0xc,0xd>,
			index::index6t<0x0,0x1,0x2,0x4,0x5,0x6>,
			index::index6t<0x4,0x5,0x6,0x8,0x9,0xa>> isomorphic5x6patt;
	typedef isomorphism<
			index::index6t<0x0,0x1,0x2,0x4,0x5,0x6>,
			index::index6t<0x4,0x5,0x6,0x7,0x8,0x9>,
			index::index6t<0x0,0x1,0x2,0x3,0x4,0x5>,
			index::index6t<0x2,0x3,0x4,0x5,0x6,0x9>,
			index::index6t<0x0,0x1,0x2,0x5,0x9,0xa>,
			index::index6t<0x3,0x4,0x5,0x6,0x7,0x8>> isomorphic6x6patt;
	typedef isomorphism<
			index::index6t<0x0,0x1,0x2,0x4,0x5,0x6>,
			index::index6t<0x4,0x5,0x6,0x7,0x8,0x9>,
			index::index6t<0x0,0x1,0x2,0x3,0x4,0x5>,
			index::index6t<0x2,0x3,0x4,0x5,0x6,0x9>,
			index::index6t<0x0,0x1,0x2,0x5,0x9,0xa>,
			index::index6t<0x3,0x4,0x5,0x6,0x7,0x8>,
			index::index6t<0x1,0x3,0x4,0x5,0x6,0x7>> isomorphic7x6patt;
	typedef isomorphism<
			index::index6t<0x0,0x1,0x2,0x4,0x5,0x6>,
			index::index6t<0x4,0x5,0x6,0x7,0x8,0x9>,
			index::index6t<0x0,0x1,0x2,0x3,0x4,0x5>,
			index::index6t<0x2,0x3,0x4,0x5,0x6,0x9>,
			index::index6t<0x0,0x1,0x2,0x5,0x9,0xa>,
			index::index6t<0x3,0x4,0x5,0x6,0x7,0x8>,
			index::index6t<0x1,0x3,0x4,0x5,0x6,0x7>,
			index::index6t<0x0,0x1,0x4,0x8,0x9,0xa>> isomorphic8x6patt;

	static method parse(utils::options opts, std::string type) {
		std::string spec = opts["options"].find("spec", "auto");
		if (spec == "auto" || spec == "default" || spec == "on") {
			spec = opts["make"].value("4x6patt");
			spec = spec.substr(0, spec.find_first_of("&|="));
		}

		if (opts["depth"].value(1) > 1) {
			switch (to_hash(spec)) {
			default: return method::expectimax<method>(opts["depth"]);
			case to_hash("4x6patt"): return method::expectimax<method::isomorphic4x6patt>(opts["depth"]);
			case to_hash("5x6patt"): return method::expectimax<method::isomorphic5x6patt>(opts["depth"]);
			case to_hash("6x6patt"): return method::expectimax<method::isomorphic6x6patt>(opts["depth"]);
			case to_hash("7x6patt"): return method::expectimax<method::isomorphic7x6patt>(opts["depth"]);
			case to_hash("8x6patt"): return method::expectimax<method::isomorphic8x6patt>(opts["depth"]);
			}
		}

		switch (to_hash(spec)) {
		default: return method();
		case to_hash("4x6patt"): return method::isomorphic4x6patt();
		case to_hash("5x6patt"): return method::isomorphic5x6patt();
		case to_hash("6x6patt"): return method::isomorphic6x6patt();
		case to_hash("7x6patt"): return method::isomorphic7x6patt();
		case to_hash("8x6patt"): return method::isomorphic8x6patt();
		}
	}

	inline static numeric& alpha() { static numeric a = numeric(0.0025); return a; }
	inline static numeric& alpha(numeric a) { return (method::alpha() = a); }
	inline static numeric& lambda() { static numeric l = 0.5; return l; }
	inline static numeric& lambda(numeric l) { return (method::lambda() = l); }
	inline static u32& step() { static u32 n = 5; return n; }
	inline static u32& step(u32 n) { return (method::step() = n); }
};

struct state : board {
	numeric esti;
	inline state() : board(0ull, 0u, -1u), esti(0) {}
	inline state(const state& s) = default;

	inline operator bool() const { return info() != -1u; }
	declare_comparators(const state&, esti, inline);

	inline numeric value() const { return esti - info(); }
	inline i32 reward() const { return info(); }

	inline void assign(const board& b, u32 op = -1) {
		set(b);
		info(move(op));
	}
	inline numeric estimate(
			clip<feature> range = feature::feats(),
			method::estimator estim = method::estimate) {
		estim = info() != -1u ? estim : method::illegal;
		esti = state::reward() + estim(*this, range);
		return esti;
	}
	inline numeric optimize(numeric exact, numeric alpha = method::alpha(),
			clip<feature> range = feature::feats(),
			method::optimizer optim = method::optimize) {
		numeric update = (exact - state::value()) * alpha;
		esti = state::reward() + optim(*this, update, range);
		return esti;
	}
};
struct select {
	state move[4];
	state *best;
	inline select() : best(move) {}
	inline select& operator ()(const board& b, clip<feature> range = feature::feats(), method::estimator estim = method::estimate) {
		b.moves(move[0], move[1], move[2], move[3]);
		move[0].estimate(range, estim);
		move[1].estimate(range, estim);
		move[2].estimate(range, estim);
		move[3].estimate(range, estim);
		best = std::max_element(move, move + 4);
		return *this;
	}
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(state& s) const { s = (*best); }
	inline void operator >>(board& b) const { b.set(*best); }

	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->info(); }
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
	statistic(const statistic&) = default;

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
		buf[size++] = '\n';
		buf[size++] = '\0';

		std::cout << buf << std::flush;

		local = {};
		local.time = tick;
	}

	void summary() const {
		char buf[1024];
		u32 size = 0;

		size += snprintf(buf + size, sizeof(buf) - size, summaf, // "summary %llums %.2fops",
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
		buf[size++] = '\n';
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

statistic run(utils::options opts, std::string type) {
	std::vector<state> path;
	statistic stats;
	select best;
	state last;

	method spec = method::parse(opts, type);
	clip<feature> feats = feature::feats();
	numeric alpha = method::alpha(opts["alpha"] / (opts("alpha", "norm") ? opts["alpha"]["norm"].value(feats.size()) : 1));
	numeric lambda = method::lambda(opts["lambda"]);
	u32 step = method::step(opts["step"]);

	switch (to_hash(opts[type]["mode"].value(type))) {
	case to_hash("optimize"):
	case to_hash("optimize:forward"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			b.init();
			best(b, feats, spec);
			score += best.score();
			opers += 1;
			best >> last;
			best >> b;
			b.next();
			while (best(b, feats, spec)) {
				last.optimize(best.esti(), alpha, feats, spec);
				score += best.score();
				opers += 1;
				best >> last;
				best >> b;
				b.next();
			}
			last.optimize(0, alpha, feats, spec);

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("optimize:backward"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, spec); b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			for (numeric esti = 0; path.size(); path.pop_back()) {
				path.back().estimate(feats, spec);
				esti = path.back().optimize(esti, alpha, feats, spec);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("optimize:forward-lambda"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			b.init();
			for (u32 i = 0; i < step && best(b, feats, spec); i++) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
				b.next();
			}
			while (best(b, feats, spec)) {
				numeric z = best.esti();
				numeric retain = 1 - lambda;
				for (u32 k = 1; k < step; k++) {
					state& source = path[opers - k];
					source.estimate(feats, spec);
					numeric r = source.reward();
					numeric v = source.value();
					z = r + (lambda * z + retain * v);
				}
				state& update = path[opers - step];
				update.estimate(feats, spec);
				update.optimize(z, alpha, feats, spec);
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
				b.next();
			}
			for (u32 tail = std::min(step, opers), i = 0; i < tail; i++) {
				numeric z = 0;
				numeric retain = 1 - lambda;
				for (u32 k = i + 1; k < tail; k++) {
					state& source = path[opers + i - k];
					source.estimate(feats, spec);
					numeric r = source.reward();
					numeric v = source.value();
					z = r + (lambda * z + retain * v);
				}
				state& update = path[opers + i - tail];
				update.estimate(feats, spec);
				update.optimize(z, alpha, feats, spec);
			}
			path.clear();

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("optimize:lambda"):
	case to_hash("optimize:backward-lambda"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, spec); b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			numeric z = 0;
			numeric r = path.back().reward();
			numeric v = path.back().optimize(0, alpha, feats, spec) - r;
			numeric retain = 1 - lambda;
			for (path.pop_back(); path.size(); path.pop_back()) {
				path.back().estimate(feats, spec);
				z = r + (lambda * z + retain * v);
				r = path.back().reward();
				v = path.back().optimize(z, alpha, feats, spec) - r;
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("evaluate"):
	case to_hash("evaluate:best"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, spec); b.next()) {
				score += best.score();
				opers += 1;
				best >> b;
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("evaluate:random"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
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

	case to_hash("evaluate:greedy"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			struct state : board {
				declare_comparators_with(const state&, int(info()), int(v.info()), inline constexpr)
			} moves[4];
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); ({ b.moves(moves); (b = *std::max_element(moves, moves + 4)).info() != -1u; }); b.next()) {
				score += b.info();
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
		case to_hash("-a"): case to_hash("--alpha"):
			opts[""] = next_opts();
			if (opts[""].empty()) (opts[""] += "0.1") += "norm";
			opts["alpha"] = opts[""];
			break;
		case to_hash("-l"): case to_hash("--lambda"):
			opts["lambda"] = next_opt("0.5");
			opts["step"] = next_opt(numeric(opts["lambda"]) ? "5" : "1");
			break;
		case to_hash("-s"): case to_hash("--seed"):
			opts["seed"] = next_opt("moporgic");
			break;
		case to_hash("-r"): case to_hash("--recipe"):
			opts["recipe"] = next_opt("");
			if (opts["recipe"] == "") break;
			// no break: optimize and evaluate are also handled by the same recipe logic
		case to_hash("-t"): case to_hash("--train"): case to_hash("--optimize"):
		case to_hash("-e"): case to_hash("--test"):  case to_hash("--evaluate"):
			opts["recipe"] = label.substr(label.find_first_not_of('-'));
			if (opts["recipe"] == "t" || opts["recipe"] == "train") opts["recipe"] = "optimize";
			if (opts["recipe"] == "e" || opts["recipe"] == "test")  opts["recipe"] = "evaluate";
			opts[opts["recipe"]] += next_opts();
			opts["recipes"] += opts["recipe"];
			break;
		case to_hash("--recipes"):
			opts["recipes"] = next_opts();
			break;
		case to_hash("-io"):  case to_hash("--input-output"):
		case to_hash("-nio"): case to_hash("--network-input-output"):
		case to_hash("-wio"): case to_hash("--weight-input-output"):
		case to_hash("-fio"): case to_hash("--feature-input-output"):
		case to_hash("-i"):   case to_hash("--input"):
		case to_hash("-wi"):  case to_hash("--weight-input"):
		case to_hash("-fi"):  case to_hash("--feature-input"):
		case to_hash("-ni"):  case to_hash("--network-input"):
		case to_hash("-o"):   case to_hash("--output"):
		case to_hash("-wo"):  case to_hash("--weight-output"):
		case to_hash("-fo"):  case to_hash("--feature-output"):
		case to_hash("-no"):  case to_hash("--network-output"):
			opts[""] = next_opt(opts.find("make", argv[0]) + '.' + label[label.find_first_not_of('-')]);
			opts[""] += next_opts();
			if (label.find(label[1] != '-' ? "i" : "input")  != std::string::npos) opts["load"] += opts[""];
			if (label.find(label[1] != '-' ? "o" : "output") != std::string::npos) opts["save"] += opts[""];
			break;
		case to_hash("-w"):  case to_hash("--weight"):
		case to_hash("-f"):  case to_hash("--feature"):
		case to_hash("-n"):  case to_hash("--network"):
		case to_hash("-wf"): case to_hash("--weight-feature"):
		case to_hash("-fw"): case to_hash("--feature-weight"):
			opts[""] = next_opt("default");
			opts[""] += next_opts();
			opts["make"] += opts[""];
			break;
		case to_hash("-m"): case to_hash("--mode"):
			opts["mode"] = next_opt("bias");
			break;
		case to_hash("-tt"): case to_hash("--train-type"): case to_hash("--optimize-type"):
		case to_hash("-tm"): case to_hash("--train-mode"): case to_hash("--optimize-mode"):
			opts["optimize"]["mode"] = next_opt("optimize");
			break;
		case to_hash("-et"): case to_hash("--test-type"): case to_hash("--evaluate-type"):
		case to_hash("-em"): case to_hash("--test-mode"): case to_hash("--evaluate-mode"):
			opts["evaluate"]["mode"] = next_opt("evaluate");
			break;
		case to_hash("-u"): case to_hash("--unit"):
			opts["unit"] = next_opt("1000");
			break;
		case to_hash("-v"): case to_hash("--win"):
			opts["win"] = next_opt("2048");
			break;
		case to_hash("-%"): case to_hash("-I"): case to_hash("--info"):
			opts["info"] = next_opt("full");
			break;
		case to_hash("-x"): case to_hash("-opt"): case to_hash("--options"):
			opts["options"] += next_opts();
			break;
		case to_hash("-d"): case to_hash("--depth"):
			opts["depth"] = next_opts();
			break;
		case to_hash("-c"): case to_hash("--cache"):
			opts["cache"] = next_opt("default");
			break;
		case to_hash("-#"): case to_hash("--comment"):
			opts["comment"] = next_opts();
			break;
		case to_hash("-|"):
			opts = {};
			break;
		default:
			opts["options"][label.substr(label.find_first_not_of('-'))] += next_opts();
			break;
		}
	}
	for (auto recipe : opts["recipes"]) {
		for (auto item : {"mode", "unit", "win", "info"})
			if (!opts(recipe, item) && opts(item))
				opts[recipe][item] = opts[item];
		for (auto mode : {"optimize", "evaluate"})
			if (recipe == mode && opts[recipe].find("mode", recipe).find(recipe) != 0)
				opts[recipe]["mode"] = recipe + ":" + opts[recipe]["mode"];
		if (!opts(recipe, "info") && opts[recipe].find("mode", recipe).find("evaluate") == 0)
			opts[recipe]["info"] = "auto";
		for (auto item : {"info=none", "info=off"})
			opts[recipe].remove(item);
	}
	return opts;
}

int main(int argc, const char* argv[]) {
	utils::options opts = parse(argc, argv);
	if (!opts["recipes"].size()) opts["recipes"] += "optimize", opts["optimize"] = 1000;
	if (!opts("alpha")) opts["alpha"] = 0.1, opts["alpha"] += "norm";
	if (!opts("seed")) opts["seed"] = ({std::stringstream ss; ss << std::hex << rdtsc(); ss.str();});
	if (!opts("lambda")) opts["lambda"] = 0;
	if (!opts("step")) opts["step"] = numeric(opts["lambda"]) ? 5 : 1;
	if (!opts("depth")) opts["depth"] = 1;

	utils::init_logging(opts["save"]);
	std::cout << "TDL2048+ by Hung Guei" << std::endl;
	std::cout << "Develop" << " Build GCC " __VERSION__ << " C++" << __cplusplus;
	std::cout << " (" << __DATE_ISO__ << " " << __TIME__ ")" << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << millisec() << " (" << moporgic::put_time(millisec()) << ")" << std::endl;
	std::cout << "seed = " << opts["seed"] << std::endl;
	std::cout << "alpha = " << opts["alpha"] << std::endl;
	std::cout << "lambda = " << opts["lambda"] << ", step = " << opts["step"] << std::endl;
	std::cout << "depth = " << opts["depth"] << ", cache = " << opts["cache"].value("none") << std::endl;
	std::cout << std::endl;

	moporgic::srand(to_hash(opts["seed"]));
	utils::init_cache(opts["cache"]);

	utils::load_network(opts["load"]);
	utils::make_network(opts["make"]);
	utils::list_network();

	for (std::string recipe : opts["recipes"]) {
		std::cout << recipe << ": " << opts[recipe] << std::endl << std::endl;
		statistic stat = run(opts, recipe);
		if (opts[recipe]("info")) stat.summary();
	}

	utils::save_network(opts["save"]);

	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
