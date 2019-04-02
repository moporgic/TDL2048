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
	inline weight() : id(0), length(0), raw(nullptr) {}
	inline weight(const weight& w) = default;
	inline ~weight() {}

	typedef moporgic::numeric numeric;
	typedef weight::numeric segment;

	inline u64 sign() const { return id; }
	inline size_t size() const { return length; }
	inline segment& operator [](u64 i) { return raw[i]; }
	inline segment* data(u64 i = 0) { return raw + i; }
	inline clip<numeric> value() const { return { raw, raw + length }; }
	inline operator bool() const { return raw; }
	declare_comparators(const weight&, sign(), inline);

	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		u32 code = 4;
		write_cast<u8>(out, code);
		switch (code) {
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
		u32 code = 4;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
		case 1:
		case 2:
			read_cast<u32>(in, w.id);
			read_cast<u64>(in, w.length);
			w.raw = weight::alloc(w.length);
			switch ((code == 2) ? read<u16>(in) : (code == 1 ? 8 : 4)) {
			case 4: read_cast<f32>(in, w.value().begin(), w.value().end()); break;
			case 8: read_cast<f64>(in, w.value().begin(), w.value().end()); break;
			}
			break;
		default:
		case 4:
			read_cast<u32>(in, w.id);
			read_cast<u32>(in, code);
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
		weight& make(u64 sign, size_t size) { return list<weight>::as(*this).emplace_back(weight(sign, size)); }
		weight erase(u64 sign) { auto it = find(sign); auto w = *it; free(it->data()); list<weight>::as(*this).erase(it); return w; }
		weight* find(u64 sign) const { return std::find_if(begin(), end(), [=](const weight& w) { return w.sign() == sign; }); }
		weight& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("weight::at"); }
		weight& operator[](u64 sign) const { return (*find(sign)); }
		weight operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : ({ weight w; w.id = sign; w; }); }
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
		indexer erase(u64 sign) { auto it = find(sign); auto x = *it; list<indexer>::as(*this).erase(it); return x; }
		indexer* find(u64 sign) const { return std::find_if(begin(), end(), [=](const indexer& i) { return i.sign() == sign; }); }
		indexer& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("indexer::at"); }
		indexer& operator[](u64 sign) const { return (*find(sign)); }
		indexer operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : ({ indexer x; x.id = sign; x; }); }
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
	inline feature() : id(0), raw(), map() {}
	inline feature(const feature& t) = default;
	inline ~feature() {}

	inline u64 sign() const { return id; }
	inline weight::segment& operator [](const board& b) { return raw[map(b)]; }
	inline weight::segment& operator [](u64 idx) { return raw[idx]; }
	inline u64 operator ()(const board& b) const { return map(b); }

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
			write_cast<u32>(out, u32(f.sign()));
			write_cast<u32>(out, u32(f.sign() >> 32));
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
			read_cast<u32>(in, code);
			f.map = indexer(code);
			f.id = u64(code);
			read_cast<u32>(in, code);
			f.raw = weight(code);
			f.id |= u64(code) << 32;
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
		feature erase(u64 wgt, u64 idx) { return erase((wgt << 32) | idx); }
		feature erase(u64 sign) { auto it = find(sign); auto f = *it; list<feature>::as(*this).erase(it); return f; }
		feature* find(u64 wgt, u64 idx) const { return find((wgt << 32) | idx); }
		feature* find(u64 sign) const { return std::find_if(begin(), end(), [=](const feature& f) { return f.sign() == sign; }); }
		feature& at(u64 wgt, u64 idx) const { return at((wgt << 32) | idx); }
		feature& at(u64 sign) const { auto it = find(sign); if (it != end()) return *it; throw std::out_of_range("feature::at"); }
		feature& operator[](u64 sign) const { return (*find(sign)); }
		feature operator()(u64 wgt, u64 idx) const { return operator()((wgt << 32) | idx); }
		feature operator()(u64 sign) const { auto it = find(sign); return it != end() ? *it : ({ feature f; f.id = sign; f; }); }
	};

	static inline feature::container& feats() { static container f; return f; }
	static inline feature& make(u64 wgt, u64 idx, container& src = feats()) { return src.make(wgt, idx); }
	static inline size_t erase(u64 wgt, u64 idx, container& src = feats()) { return src.erase(wgt, idx); }
	inline feature(u64 wgt, u64 idx, const container& src = feats()) : feature(src(wgt, idx)) {}

private:
	inline feature(const weight& value, const indexer& index) : id((value.sign() << 32) | index.sign()), raw(value), map(index) {}

	u64 id;
	weight raw;
	indexer map;
};

class transposition {
public:
	class position {
	friend class transposition;
	public:
		u64 sign;
		f32 esti;
		i16 depth;
		u16 hits;
		constexpr position(const position& e) = default;
		constexpr position(u64 sign = 0) : sign(sign), esti(0), depth(-1), hits(0) {}

		constexpr operator numeric() const { return esti; }
		constexpr operator bool()    const { return sign; }
		constexpr bool operator ==(u64 s) const { return sign == s; }
		constexpr bool operator >=(i32 d) const { return depth >= d; }
		declare_comparators(position, hits, constexpr);

		struct result {
			position& where;
			u64 sign;

			constexpr result& save(f32 esti, i32 depth) {
				if (where.sign != sign) {
					where.sign = sign;
					where.esti = esti;
					where.depth = depth;
					where.hits = 0;
				} else {
					where.hits = std::min(where.hits + 1, 65535);
				}
				return *this;
			}

			constexpr operator numeric() const { return where; }
			constexpr operator bool() const { return where == sign; }
			constexpr bool operator >=(i32 d) const { return where >= d; }
		};
		constexpr result operator()(u64 s) {
			return { *this, s };
		}
	};

	constexpr transposition() : cache(&initial), zsize(1), limit(0), zmask(0) {}
	transposition(const transposition& tp) = default;
	~transposition() {}
	inline constexpr size_t length() const { return zsize; }
	inline constexpr size_t size() const { return zsize + limit; }

	inline u64 min_isomorphic(board t) const {
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

	inline position::result operator[] (const board& b) {
		u64 x = min_isomorphic(b);
		u64 hash = math::fmix64(x) & zmask;

		position& data = cache[hash];
		if (!data) return data(x);
		if (!std::isnan(data.esti)) {
			if (data == x || !data.hits) return data(x);
			position* list = mpool.allocate(2);
			if (!list) return data(x);
			list[0] = data;
			data(cast<uintptr_t>(list)).save(0.0 / 0.0, 1);
			return list[1](x);
		}

		position*& list = raw_cast<position*>(data.sign);
		u16& lim = raw_cast<u16>(data.depth);
		u32 size = lim + 1;

		for (u32 i = 0; i < lim; i++) {
			if (list[i] < list[i + 1]) std::swap(list[i], list[i + 1]);
			if (list[i] == x) return list[i](x);
		}
		if (list[lim] == x) return list[lim](x);
		if (size != (1u << math::ones16(lim))) return list[++lim](x);

		u32 hits = 0, min = list[lim].hits;
		for (u32 i = 0; i < size; i++) {
			hits += list[i].hits;
			list[i].hits -= min;
		}
		u32 thres = hits / (size * 2);
		if (min <= thres || size == 65536) return list[lim](x);

		position* temp = mpool.allocate(size + size);
		if (!temp) return list[lim](x);
		std::copy(list, list + size, temp);
		mpool.deallocate(list, size);
		return (list = temp)[++lim](x);
	}

    friend std::ostream& operator <<(std::ostream& out, const transposition& tp) {
    	auto& cache = tp.cache;
    	auto& zsize = tp.zsize;
    	auto& limit = tp.limit;
		u32 code = 2;
		write_cast<byte>(out, code);
		switch (code) {
		default:
			std::cerr << "unknown serial at transposition::>>" << std::endl;
			std::cerr << "use default serial (" << code << ") instead" << std::endl;
			// no break;
		case 2:
			write_cast<u64>(out, zsize);
			write_cast<u64>(out, limit);
			for (u64 i = 0; i < zsize; i++) {
				auto& data = cache[i];
				write_cast<u64>(out, data.sign);
				if (!data.sign) continue;
				write_cast<f32>(out, data.esti);
				write_cast<i16>(out, data.depth);
				write_cast<u16>(out, data.hits);
				if (!std::isnan(data.esti)) continue;

				auto size = u16(data.depth) + 1u;
				auto list = cast<position*>(data.sign);
				std::sort(list, list + size, std::greater<position>());
				for (auto it = list; it != list + size; it++) {
					write_cast<u64>(out, it->sign);
					write_cast<f32>(out, it->esti);
					write_cast<i16>(out, it->depth);
					write_cast<u16>(out, it->hits);
				}
			}
			write_cast<u16>(out, 0); // reserved fields
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, transposition& tp) {
    	auto& cache = tp.cache;
    	auto& mpool = tp.mpool;
    	auto& zsize = tp.zsize;
    	auto& limit = tp.limit;
    	auto& zmask = tp.zmask;
		u32 code = 0;
		read_cast<byte>(in, code);
		switch (code) {
		case 2:
			read_cast<u64>(in, zsize);
			read_cast<u64>(in, limit);
			zmask = zsize - 1;
			cache = alloc(zsize + limit);
			mpool.deallocate(cache + zsize, limit);
			for (u64 i = 0; i < zsize; i++) {
				auto& data = cache[i];
				read_cast<u64>(in, data.sign);
				if (!data.sign) continue;
				read_cast<f32>(in, data.esti);
				read_cast<i16>(in, data.depth);
				read_cast<u16>(in, data.hits);
				if (!std::isnan(data.esti)) continue;

				auto size = u16(data.depth) + 1u;
				auto list = tp.mpool.allocate((2 << math::lg(size - 1)));
				data.sign = cast<uintptr_t>(list);
				for (auto it = list; it != list + size; it++) {
					read_cast<u64>(in, it->sign);
					read_cast<f32>(in, it->esti);
					read_cast<i16>(in, it->depth);
					read_cast<u16>(in, it->hits);
				}
			}
			while (read_cast<u16>(in, code) && code) {
				u64 skip; read_cast<u64>(in, skip);
				in.ignore(code * skip);
			}
			break;
		default:
			std::cerr << "unknown serial at transposition::<<" << std::endl;
			break;
		}
		return in;
	}

	static void save(std::ostream& out) {
		u32 code = 0;
		write_cast<byte>(out, code);
		switch (code) {
		default:
			std::cerr << "unknown serial at transposition::save" << std::endl;
			// no break
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
			std::cerr << "unknown serial at transposition::load" << std::endl;
			// no break
		case 0:
			in >> instance();
			break;
		}
	}

	void summary() {
		std::cout << std::endl << "summary" << std::endl;
		if (zsize > 1) {
			std::vector<u64> stat(65537);
			for (size_t i = 0; i < zsize; i++) {
				auto& h = cache[i];
				if (std::isnan(h.esti)) {
					stat[u16(h.depth) + 1]++;
				} else {
					stat[h.sign ? 1 : 0]++;
				}
			}
			while (stat.back() == 0) stat.pop_back();

			std::cout << "block" "\t" "count" << std::endl;
			for (size_t i = 0; i < stat.size(); i++) {
				std::cout << i << "\t" << stat[i] << std::endl;
			}
		} else {
			std::cout << "no transposition" << std::endl;
		}
	}

	static transposition& make(size_t len, size_t lim = 0) {
		return instance().shape(std::max(len, size_t(1)), lim);
	}
	static inline transposition& instance() { static transposition tp; return tp; }
	static inline position::result find(const board& b) { return instance()[b]; }
//	static inline position& remove(const board& b) { return find(b)(0); } // TODO

private:
	static inline position* alloc(size_t len) { return new position[len](); }
	static inline void free(position* alloc) { delete[] alloc; }

	transposition& shape(size_t len, size_t lim = 0) {
		if (math::ones64(len) != 1) {
			std::cerr << "unsupported transposition size: " << len << ", ";
			len = 1ull << (math::lg64(len));
			std::cerr << "fit to " << len << std::endl;
		}

		std::list<size_t> blocks;
		if (cache && zsize > 1) {
			if (zsize > len) std::cerr << "unsupported operation: shrink z-size" << std::endl;
			if (zsize < len) std::cerr << "unsupported operation: enlarge z-size" << std::endl;
			if (limit > lim) std::cerr << "unsupported operation: shrink m-pool" << std::endl;
			if (limit < lim) {
				blocks.push_back(lim - limit);
				limit = lim;
			}
		} else {
			zsize = len;
			limit = lim;
			zmask = len - 1;
			if (cache != &initial) free(cache);
			cache = alloc(zsize);
			blocks.push_back(limit);
		}

		while (blocks.size()) {
			auto block = blocks.front();
			blocks.pop_front();
			try {
				mpool.deallocate(alloc(block), block);
			} catch (std::bad_alloc&) {
				if (sizeof(position) * block >= (64ull << 20) /* 64 MB */ ) {
					blocks.push_back(block >> 1); // try allocate smaller block size
					blocks.push_back(block - blocks.back());
				} else {
					std::cerr << "insufficient memory for transposition block " << block << std::endl;
				}
			}
		}
		return *this;
	}

private:
	position* cache;
	position initial;
	segment<position> mpool;
	size_t zsize;
	size_t limit;
	size_t zmask;
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
template<>
u64 index6t<0x0,0x1,0x2,0x3,0x4,0x5>(const board& b) {
	return (u32(u64(b)) & 0xffffff);
}
template<>
u64 index6t<0x4,0x5,0x6,0x7,0x8,0x9>(const board& b) {
	return (u32(u64(b) >> 16) & 0xffffff);
}
template<>
u64 index6t<0x8,0x9,0xa,0xb,0xc,0xd>(const board& b) {
	return (u32(u64(b) >> 32) & 0xffffff);
}
template<>
u64 index6t<0x0,0x1,0x2,0x4,0x5,0x6>(const board& b) {
	return (u32(u64(b)) & 0x000fff) | ((u32(u64(b)) >> 4) & 0xfff000);
}
template<>
u64 index6t<0x4,0x5,0x6,0x8,0x9,0xa>(const board& b) {
	return (u32(u64(b) >> 16) & 0x000fff) | (u32(u64(b) >> 20) & 0xfff000);
}
template<>
u64 index6t<0x2,0x3,0x4,0x5,0x6,0x9>(const board& b) {
	return (u32(u64(b) >> 8) & 0x0fffff) | (u32(u64(b) >> 16) & 0xf00000);
}
template<>
u64 index6t<0x0,0x1,0x2,0x5,0x9,0xa>(const board& b) {
	return (u32(u64(b)) & 0x000fff) | (u32(u64(b) >> 8) & 0x00f000) | (u32(u64(b) >> 20) & 0xff0000);
}
template<>
u64 index6t<0x3,0x4,0x5,0x6,0x7,0x8>(const board& b) {
	return (u32(u64(b) >> 12) & 0xffffff);
}
template<>
u64 index6t<0x1,0x3,0x4,0x5,0x6,0x7>(const board& b) {
	return (u32(u64(b) >> 4) & 0x00000f) | (u32(u64(b) >> 8) & 0xfffff0);
}
template<>
u64 index6t<0x0,0x1,0x4,0x8,0x9,0xa>(const board& b) {
	return (u32(u64(b)) & 0x0000ff) | (u32(u64(b) >> 8) & 0x000f00) | (u32(u64(b) >> 20) & 0xfff000);
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
		}
		in.close();
	}
	for (feature f : list<feature>(std::move(feature::feats())))
		feature::make(f.value().sign(), f.index().sign());
}
void save_network(utils::options::option opt) {
	for (std::string path : opt) {
		char type = path[path.find_last_of(".") + 1];
		if (type == 'x' || type == 'l') continue; // .x and .log are suffix for log files
		std::ofstream out;
		out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) continue;
		// for upward compatibility, we still store legacy binaries if suffix are traditional (.f or .w)
		if (type != 'f')  weight::save(type != 'w' ? out.write("w", 1) : out);
		if (type != 'w') feature::save(type != 'f' ? out.write("f", 1) : out);
		out.flush();
		out.close();
	}
}

void list_network() {
	for (weight w : weight::wghts()) {
		std::stringstream buf;
		buf << std::setfill('0');

		buf << std::hex << std::setw(8) << w.sign();
		buf << "[" << std::dec;
		if (w.size() >> 30)
			buf << (w.size() >> 30) << "G";
		else if (w.size() >> 20)
			buf << (w.size() >> 20) << "M";
		else if (w.size() >> 10)
			buf << (w.size() >> 10) << "k";
		else
			buf << (w.size());
		buf << "]";

		buf << " = (unused)" << std::hex;
		buf.seekp(-9, std::ios::end);
		for (feature f : feature::feats()) if (f.value() == w)
			buf << " " << std::setw(8) << f.index().sign();

		std::cout << buf.rdbuf() << std::endl;
	}
	std::cout << std::endl;
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

inline constexpr numeric illegal(const board& state,
		clip<feature> range = feature::feats()) {
	return -std::numeric_limits<numeric>::max();
}

#define invoke_4x6patt(esti, f, iso, ...)\
esti += (VA_PASS(f[0 << 3][index6t<0x0,0x1,0x2,0x3,0x4,0x5>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[1 << 3][index6t<0x4,0x5,0x6,0x7,0x8,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[2 << 3][index6t<0x0,0x1,0x2,0x4,0x5,0x6>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[3 << 3][index6t<0x4,0x5,0x6,0x8,0x9,0xa>(iso)] __VA_ARGS__));\

#define invoke_5x6patt(esti, f, iso, ...)\
esti += (VA_PASS(f[0 << 3][index6t<0x0,0x1,0x2,0x3,0x4,0x5>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[1 << 3][index6t<0x4,0x5,0x6,0x7,0x8,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[2 << 3][index6t<0x8,0x9,0xa,0xb,0xc,0xd>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[3 << 3][index6t<0x0,0x1,0x2,0x4,0x5,0x6>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[4 << 3][index6t<0x4,0x5,0x6,0x8,0x9,0xa>(iso)] __VA_ARGS__));\

#define invoke_6x6patt(esti, f, iso, ...)\
esti += (VA_PASS(f[0 << 3][index6t<0x0,0x1,0x2,0x4,0x5,0x6>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[1 << 3][index6t<0x4,0x5,0x6,0x7,0x8,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[2 << 3][index6t<0x0,0x1,0x2,0x3,0x4,0x5>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[3 << 3][index6t<0x2,0x3,0x4,0x5,0x6,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[4 << 3][index6t<0x0,0x1,0x2,0x5,0x9,0xa>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[5 << 3][index6t<0x3,0x4,0x5,0x6,0x7,0x8>(iso)] __VA_ARGS__));\

#define invoke_7x6patt(esti, f, iso, ...)\
esti += (VA_PASS(f[0 << 3][index6t<0x0,0x1,0x2,0x4,0x5,0x6>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[1 << 3][index6t<0x4,0x5,0x6,0x7,0x8,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[2 << 3][index6t<0x0,0x1,0x2,0x3,0x4,0x5>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[3 << 3][index6t<0x2,0x3,0x4,0x5,0x6,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[4 << 3][index6t<0x0,0x1,0x2,0x5,0x9,0xa>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[5 << 3][index6t<0x3,0x4,0x5,0x6,0x7,0x8>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[6 << 3][index6t<0x1,0x3,0x4,0x5,0x6,0x7>(iso)] __VA_ARGS__));\

#define invoke_8x6patt(esti, f, iso, ...)\
esti += (VA_PASS(f[0 << 3][index6t<0x0,0x1,0x2,0x4,0x5,0x6>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[1 << 3][index6t<0x4,0x5,0x6,0x7,0x8,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[2 << 3][index6t<0x0,0x1,0x2,0x3,0x4,0x5>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[3 << 3][index6t<0x2,0x3,0x4,0x5,0x6,0x9>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[4 << 3][index6t<0x0,0x1,0x2,0x5,0x9,0xa>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[5 << 3][index6t<0x3,0x4,0x5,0x6,0x7,0x8>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[6 << 3][index6t<0x1,0x3,0x4,0x5,0x6,0x7>(iso)] __VA_ARGS__));\
esti += (VA_PASS(f[7 << 3][index6t<0x0,0x1,0x4,0x8,0x9,0xa>(iso)] __VA_ARGS__));\

#define invoke_specialized_features(name, state, range, ...)({\
register numeric esti = 0;\
register board iso = state;\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.mirror();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.transpose();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.mirror();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.transpose();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.mirror();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.transpose();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
iso.mirror();\
invoke_##name(esti, range, iso, ##__VA_ARGS__);\
esti;})\

#define declare_specialization(name)\
inline numeric estimate_##name(const board& state, clip<feature> range = feature::feats()) {\
	return invoke_specialized_features(name, state, range); }\
inline numeric optimize_##name(const board& state, numeric updv, clip<feature> range = feature::feats()) {\
	return invoke_specialized_features(name, state, range, += updv); }\

declare_specialization(4x6patt);
declare_specialization(5x6patt);
declare_specialization(6x6patt);
declare_specialization(7x6patt);
declare_specialization(8x6patt);

void make_transposition(std::string res = "") {
	std::string in(res);
	if (in.empty() && transposition::instance().size() == 0) in = "default";
	if (in == "default") in = "2G+2G";
	if (in == "no" || in == "null") in = "0";

	if (in.size()) {
		u64 len = 0;
		u64 lim = 0;
		auto rebase = [](u64 v, char c) -> u64 {
			switch (c) {
			case 'K': v *= (1ULL << 10); break;
			case 'M': v *= (1ULL << 20); break;
			case 'G': v *= (1ULL << 30); break;
			}
			return v;
		};
		if (in.find('+') != std::string::npos) {
			std::string il = in.substr(in.find('+') + 1);
			if (!std::isdigit(il.back())) {
				lim = std::stoll(il.substr(0, il.size() - 1));
				lim = rebase(lim, il.back()) / sizeof(transposition::position);
			} else {
				lim = std::stoll(il);
			}
		}
		if (!std::isdigit(in.back())) {
			len = std::stoll(in.substr(0, in.size() - 1));
			len = rebase(len, in.back()) / sizeof(transposition::position);
		} else {
			len = std::stoll(in);
		}
		transposition::make(len, lim);
	}
}
bool load_transposition(std::string path) {
	std::ifstream in;
	char buf[1 << 20];
	in.rdbuf()->pubsetbuf(buf, sizeof(buf));
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return false;
	transposition::load(in);
	in.close();
	return true;
}
bool save_transposition(std::string path) {
	std::ofstream out;
	char buf[1 << 20];
	out.rdbuf()->pubsetbuf(buf, sizeof(buf));
	out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return false;
	transposition::save(out);
	out.flush();
	out.close();
	return true;
}

struct expectimax {
	static std::array<u32, 16>& depth() {
		static std::array<u32, 16> res = { 7, 7, 7, 7, 5, 5, 5, 5, 3, 3, 3, 3, 1, 1, 1, 1 };
		return res;
	}
	static std::array<u32, 16>& depth(const std::array<u32, 16>& res) { return (depth() = res); }

	static std::array<u32, 16>& depth(const std::string& res) {
		std::array<u32, 16> depthres;
		std::string dyndepth(res);
		for (u32 d = 0, e = 0; e < 16; depthres[e++] = d) {
			if (dyndepth.empty()) continue;
			auto it = dyndepth.find_first_of(", ");
			d = std::stol(dyndepth.substr(0, it));
			dyndepth = dyndepth.substr(it + 1);
		}
		return depth(depthres);
	}

	template<utils::estimator estim = utils::estimate>
	static numeric search_expt(const board& after, i32 depth,
			clip<feature> range = feature::feats()) {
		auto t = transposition::find(after); // TODO: load TP policy
		if (t && t >= depth) return t;

		if (depth <= 0) return estim(after, range);
		numeric expt = 0;
		hexa spaces = after.spaces();
		for (size_t i = 0; i < spaces.size(); i++) {
			board before = after;
			u32 pos = spaces[i];
			before.set(pos, 1);
			expt += 0.9 * search_max<estim>(before, depth - 1, range);
			before.set(pos, 2);
			expt += 0.1 * search_max<estim>(before, depth - 1, range);
		}
		expt /= spaces.size();
		return t.save(expt, depth); // TODO: store TP policy
	}

	template<utils::estimator estim = utils::estimate>
	static numeric search_max(const board& before, i32 depth,
			clip<feature> range = feature::feats()) { // TODO: depth policy?
		numeric best = 0;
		board after;
		i32 reward;
		if ((reward = (after = before).operate(0)) != -1)
			best = std::max(best, reward + search_expt<estim>(after, depth - 1, range));
		if ((reward = (after = before).operate(1)) != -1)
			best = std::max(best, reward + search_expt<estim>(after, depth - 1, range));
		if ((reward = (after = before).operate(2)) != -1)
			best = std::max(best, reward + search_expt<estim>(after, depth - 1, range));
		if ((reward = (after = before).operate(3)) != -1)
			best = std::max(best, reward + search_expt<estim>(after, depth - 1, range));
		return best;
	}

	template<utils::estimator estim = utils::estimate>
	static numeric estimate(const board& after,
			clip<feature> range = feature::feats()) {
		i32 depth = expectimax::depth()[after.empty()]; // TODO: depth policy
		return search_expt<estim>(after, depth - 1, range);
	}
};

struct specialize {
	specialize(utils::options& opts, const std::string& type) : estim(utils::estimate), optim(utils::optimize) {
		std::string spec = opts["options"].find("spec", "auto");
		if (spec == "auto" || spec == "on") {
			spec = opts["make"].value();
			spec = spec.size() ? spec.substr(0, spec.find_first_of("&|=")) : "4x6patt";
		}
		switch (to_hash(spec)) {
		case to_hash("4x6patt"): estim = utils::estimate_4x6patt; optim = utils::optimize_4x6patt; break;
		case to_hash("5x6patt"): estim = utils::estimate_5x6patt; optim = utils::optimize_5x6patt; break;
		case to_hash("6x6patt"): estim = utils::estimate_6x6patt; optim = utils::optimize_6x6patt; break;
		case to_hash("7x6patt"): estim = utils::estimate_7x6patt; optim = utils::optimize_7x6patt; break;
		case to_hash("8x6patt"): estim = utils::estimate_8x6patt; optim = utils::optimize_8x6patt; break;
		}
		if (opts[type]["mode"]("search") || opts[type]["mode"]("expectimax")) {
			std::string mode = opts[type]["mode"];
			auto it = std::min(mode.find("search"), mode.find("expectimax"));
			if (it > 0 && mode[it - 1] == '-') it--;
			opts[type]["mode"] = mode.substr(0, it);
			switch (to_hash(spec)) {
			default:                 estim = utils::expectimax::estimate<utils::estimate>; break;
			case to_hash("4x6patt"): estim = utils::expectimax::estimate<utils::estimate_4x6patt>; break;
			case to_hash("5x6patt"): estim = utils::expectimax::estimate<utils::estimate_5x6patt>; break;
			case to_hash("6x6patt"): estim = utils::expectimax::estimate<utils::estimate_6x6patt>; break;
			case to_hash("7x6patt"): estim = utils::expectimax::estimate<utils::estimate_7x6patt>; break;
			case to_hash("8x6patt"): estim = utils::expectimax::estimate<utils::estimate_8x6patt>; break;
			}
		}
	}
	constexpr specialize(utils::estimator estim, utils::optimizer optim) : estim(estim), optim(optim) {}
	constexpr operator utils::estimator() const { return estim; }
	constexpr operator utils::optimizer() const { return optim; }
	utils::estimator estim;
	utils::optimizer optim;
};

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
			clip<feature> range = feature::feats(),
			utils::estimator estim = utils::estimate) {
		estim = score >= 0 ? estim : utils::illegal;
		esti = state::reward() + estim(move, range);
		return esti;
	}
	inline numeric optimize(numeric exact, numeric alpha = state::alpha(),
			clip<feature> range = feature::feats(),
			utils::optimizer optim = utils::optimize) {
		numeric update = (exact - state::value()) * alpha;
		esti = state::reward() + optim(move, update, range);
		return esti;
	}

	inline static numeric& alpha() { static numeric a = numeric(0.0025); return a; }
	inline static numeric& alpha(numeric a) { return (state::alpha() = a); }
	inline static numeric& lambda() { static numeric l = 0.5; return l; }
	inline static numeric& lambda(numeric l) { return (state::lambda() = l); }
	inline static u32& step() { static u32 n = 5; return n; }
	inline static u32& step(u32 n) { return (state::step() = n); }
};
struct select {
	state move[4];
	state *best;
	inline select() : best(move) {}
	inline select& operator ()(const board& b, clip<feature> range = feature::feats(), utils::estimator estim = utils::estimate) {
		move[0].assign(b, 0);
		move[1].assign(b, 1);
		move[2].assign(b, 2);
		move[3].assign(b, 3);
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
	inline void operator >>(board& b) const { b = best->move; }

	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }
	inline numeric estimate() const { return best->estimate(); }
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

statistic optimize(utils::options opts, const std::string& type) {
	std::vector<state> path;
	statistic stats;
	select best;
	state last;

	utils::estimator estim = utils::specialize(opts, type);
	utils::optimizer optim = utils::specialize(opts, type);
	clip<feature> feats = feature::feats();
	numeric alpha = state::alpha();
	numeric lambda = state::lambda();
	u32 step = state::step();

	switch (to_hash(opts[type]["mode"])) {
	default:
	case to_hash("forward"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			b.init();
			best(b, feats, estim);
			score += best.score();
			opers += 1;
			best >> last;
			best >> b;
			b.next();
			while (best(b, feats, estim)) {
				last.optimize(best.esti(), alpha, feats, optim);
				score += best.score();
				opers += 1;
				best >> last;
				best >> b;
				b.next();
			}
			last.optimize(0, alpha, feats, optim);

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("backward"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, estim); b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			for (numeric esti = 0; path.size(); path.pop_back()) {
				path.back().estimate(feats, estim);
				esti = path.back().optimize(esti, alpha, feats, optim);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("forward-lambda"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			b.init();
			for (u32 i = 0; i < step && best(b, feats, estim); i++) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
				b.next();
			}
			while (best(b, feats, estim)) {
				numeric z = best.esti();
				numeric retain = 1 - lambda;
				for (u32 k = 1; k < step; k++) {
					state& source = path[opers - k];
					source.estimate(feats, estim);
					numeric r = source.reward();
					numeric v = source.value();
					z = r + (lambda * z + retain * v);
				}
				state& update = path[opers - step];
				update.estimate(feats, estim);
				update.optimize(z, alpha, feats, optim);
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
					source.estimate(feats, estim);
					numeric r = source.reward();
					numeric v = source.value();
					z = r + (lambda * z + retain * v);
				}
				state& update = path[opers + i - tail];
				update.estimate(feats, estim);
				update.optimize(z, alpha, feats, optim);
			}
			path.clear();

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("lambda"):
	case to_hash("backward-lambda"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, estim); b.next()) {
				score += best.score();
				opers += 1;
				best >> path;
				best >> b;
			}

			numeric z = 0;
			numeric r = path.back().reward();
			numeric v = path.back().optimize(0, alpha, feats, optim) - r;
			numeric retain = 1 - lambda;
			for (path.pop_back(); path.size(); path.pop_back()) {
				path.back().estimate(feats, estim);
				z = r + (lambda * z + retain * v);
				r = path.back().reward();
				v = path.back().optimize(z, alpha, feats, optim) - r;
			}

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

statistic evaluate(utils::options opts, const std::string& type) {
	statistic stats;
	select best;

	utils::estimator estim = utils::specialize(opts, type);
	clip<feature> feats = feature::feats();

	switch (to_hash(opts[type]["mode"])) {
	default:
	case to_hash("best"):
		for (stats.init(opts[type]); stats; stats++) {
			board b;
			u32 score = 0;
			u32 opers = 0;

			for (b.init(); best(b, feats, estim); b.next()) {
				score += best.score();
				opers += 1;
				best >> b;
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	case to_hash("random"):
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
		case to_hash("-l"):
		case to_hash("--lambda"):
			opts["lambda"] = next_opt("0.5");
			opts["step"] = next_opt(numeric(opts["lambda"]) ? "5" : "1");
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
			opts["save"] += opts[""];
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
		case to_hash("-x"):
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
		case to_hash("-d"):
		case to_hash("--depth"):
			opts["depth"] = next_opts();
			break;
		case to_hash("-tp"):
		case to_hash("--cache"):
		case to_hash("--cache-make"):
		case to_hash("--transposition"):
		case to_hash("--transposition-make"):
			opts["cache-make"] = next_opt("");
			break;
		case to_hash("-tpio"):
		case to_hash("--cache-input-output"):
		case to_hash("--transposition-input-output"):
			opts["cache-save"] = opts["cache-load"] = next_opt("2048.c");
			break;
		case to_hash("-tpi"):
		case to_hash("--cache-input"):
		case to_hash("--transposition-input"):
			opts["cache-load"] = next_opt("2048.c");
			break;
		case to_hash("-tpo"):
		case to_hash("--cache-output"):
		case to_hash("--transposition-output"):
			opts["cache-save"] = next_opt("2048.c");
			break;
		case to_hash("-c"):
		case to_hash("--comment"):
			opts["comment"] = next_opts();
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
	if (!opts("alpha")) opts["alpha"] = 0.1, opts["alpha"] += "norm";
	if (!opts("seed")) opts["seed"] = ({std::stringstream ss; ss << std::hex << rdtsc(); ss.str();});
	if (!opts("lambda")) opts["lambda"] = 0;
	if (!opts("step")) opts["step"] = numeric(opts["lambda"]) ? 5 : 1;
	if (!opts("depth")) opts["depth"] = "7 7 7 7 5 5 5 5 3 3 3 3 1 1 1 1";

	utils::init_logging(opts["save"]);
	std::cout << "TDL2048+ by Hung Guei" << std::endl;
	std::cout << "Develop-Search" << " Build GCC " __VERSION__ << " C++" << __cplusplus;
	std::cout << " (" __DATE__ " " __TIME__ ")" << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << moporgic::millisec() << std::endl;
	std::cout << "seed = " << opts["seed"] << std::endl;
	std::cout << "alpha = " << opts["alpha"] << std::endl;
	std::cout << "lambda = " << opts["lambda"] << ", step = " << opts["step"] << std::endl;
	std::cout << "depth = " << opts["depth"] << std::endl; // TODO: search = ?
	std::cout << std::endl;

	utils::load_network(opts["load"]);
	utils::make_network(opts["make"]);
	utils::list_network();

	moporgic::srand(moporgic::to_hash(opts["seed"]));
	state::alpha(std::stod(opts["alpha"]));
	if (opts("alpha", "norm")) state::alpha(state::alpha() / feature::feats().size());
	state::lambda(opts["lambda"]);
	if (numeric(opts["lambda"]) && !opts("optimize", "mode")) opts["optimize"]["mode"] = "lambda";
	state::step(opts["step"]);
	utils::expectimax::depth(opts["depth"]);

	if (statistic(opts["optimize"])) {
		std::cout << "optimization: " << opts["optimize"] << std::endl << std::endl;
		statistic stat = optimize(opts, "optimize");
		if (opts["info"] == "full") stat.summary();
	}

	utils::save_network(opts["save"]);

	utils::load_transposition(opts["cache-load"]);
	utils::make_transposition(opts["cache-make"]);

	if (statistic(opts["evaluate"])) {
		std::cout << "verification: " << opts["evaluate"] << std::endl << std::endl;
		statistic stat = evaluate(opts, "evaluate");
		if (opts["info"] != "none") stat.summary();
		if (opts["info"] == "full") transposition::instance().summary();
	}

	utils::save_transposition(opts["cache-save"]);

	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
