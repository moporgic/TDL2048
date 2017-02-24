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
#include <random>

namespace moporgic {

typedef float numeric;

class weight {
public:
	weight() : id(0), length(0), value(nullptr) {}
	weight(const weight& w) = default;
	~weight() {}

	inline u64 sign() const { return id; }
	inline u64 size() const { return length; }
	inline numeric& operator [](const u64& i) { return value[i]; }

	inline bool operator ==(const weight& w) const { return id == w.id; }
	inline bool operator !=(const weight& w) const { return id != w.id; }

    friend std::ostream& operator <<(std::ostream& out, const weight& w) {
    	auto& id = w.id;
    	auto& length = w.length;
    	auto& value = w.value;
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
			write_cast<numeric>(out, value, value + length);
			write_cast<u16>(out, 0);
			break;
		}
		return out;
    }
	friend std::istream& operator >>(std::istream& in, weight& w) {
    	auto& id = w.id;
    	auto& length = w.length;
    	auto& value = w.value;
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
		case 1:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			value = alloc(length);
			if (code == 0) read_cast<f32>(in, value, value + length);
			if (code == 1) read_cast<f64>(in, value, value + length);
			break;
		case 2:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			read_cast<u16>(in, code);
			value = alloc(length);
			switch (code) {
			case 4: read_cast<f32>(in, value, value + length); break;
			case 8: read_cast<f64>(in, value, value + length); break;
			}
			break;
		case 127:
			read_cast<u32>(in, id);
			read_cast<u64>(in, length);
			read_cast<u16>(in, code);
			value = alloc(length);
			switch (code) {
			case 4: read_cast<f32>(in, value, value + length); break;
			case 8: read_cast<f64>(in, value, value + length); break;
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
			value = alloc(length);
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

	static void save(std::ostream& out) {
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
				out << w;
			break;
		}
		out.flush();
	}
	static void load(std::istream& in) {
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		default:
			std::cerr << "unknown serial (" << code << ") at weight::load, ";
			std::cerr << "use default (0) instead..." << std::endl;
			// no break
		case 0:
			for (read_cast<u32>(in, code); code; code--) {
				weight w; in >> w;
				wghts().push_back(w);
			}
			break;
		}
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
	weight(const u32& sign, const size_t& size) : id(sign), length(size), value(alloc(size)) {}
	static inline std::vector<weight>& wghts() { static std::vector<weight> w; return w; }

	static inline numeric* alloc(const size_t& size) {
		return new numeric[size]();
	}
	static inline void free(numeric* v) {
		delete[] v;
	}


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

	static void save(std::ostream& out) {
		u32 code = 0;
		write_cast<u8>(out, code);
		switch (code) {
		case 0:
			write_cast<u32>(out, feats().size());
			for (feature f : feature::list())
				out << f;
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::save" << std::endl;
			break;
		}
		out.flush();
	}
	static void load(std::istream& in) {
		u32 code;
		read_cast<u8>(in, code);
		switch (code) {
		case 0:
			for (read_cast<u32>(in, code); code; code--) {
				feature f; in >> f;
				feats().push_back(f);
			}
			break;
		default:
			std::cerr << "unknown serial (" << code << ") at feature::load" << std::endl;
			break;
		}
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

class zhasher {
public:
	zhasher(const u64& seed  = moporgic::millisec(),
			const u64& seeda = 0x0000000000000000ULL, const u64& seedb = 0xffffffffffffffffULL)
	: id(seed ^ seeda ^ seedb), zhash(new u64[4][1 << 20]), ztile(new u64[16][32]), seeda(seeda), seedb(seedb) {
		std::default_random_engine engine(seed);
		std::uniform_int_distribution<u64> dist;
		for (u32 idx = 0; idx < 16; idx++) {
			for (u32 tile = 0; tile < 32; tile++) {
				ztile[idx][tile] = dist(engine);
			}
		}
		for (u32 row = 0; row < 4; row++) {
			auto rhash = zhash[row];
			auto rtile = ztile + (row * 4);
			for (u32 value = 0; value < (1 << 20); value++) {
				board tile; tile.place20(0, value);
				auto t0 = rtile[0][tile.at5(0)];
				auto t1 = rtile[1][tile.at5(1)];
				auto t2 = rtile[2][tile.at5(2)];
				auto t3 = rtile[3][tile.at5(3)];
				rhash[value] = t0 ^ t1 ^ t2 ^ t3;
			}
		}
	}
	zhasher(const zhasher& z)
	: id(z.id), zhash(new u64[4][1 << 20]), ztile(new u64[16][32]), seeda(z.seeda), seedb(z.seedb) {
		std::copy(cast<u64*>(z.zhash), cast<u64*>(z.zhash) + sizeof(u64[4][1 << 20]), cast<u64*>(zhash));
		std::copy(cast<u64*>(z.ztile), cast<u64*>(z.ztile) + sizeof(u64[16][32]),     cast<u64*>(ztile));
	}
	zhasher(zhasher&& z)
	: id(z.id), zhash(z.zhash), ztile(z.ztile), seeda(z.seeda), seedb(z.seedb) {
		z.zhash = nullptr;
		z.ztile = nullptr;
	}
	~zhasher() {
		if (zhash) delete[] zhash;
		if (ztile) delete[] ztile;
	}

	zhasher& operator =(const zhasher& z) {
		id = z.id;
		std::copy(cast<u64*>(z.zhash), cast<u64*>(z.zhash) + sizeof(u64[4][1 << 20]), cast<u64*>(zhash));
		std::copy(cast<u64*>(z.ztile), cast<u64*>(z.ztile) + sizeof(u64[16][32]),     cast<u64*>(ztile));
		seeda = z.seeda;
		seedb = z.seedb;
		return *this;
	}
	zhasher& operator =(zhasher&& z) {
		id = z.id;
		zhash = z.zhash; z.zhash = nullptr;
		ztile = z.ztile; z.ztile = nullptr;
		seeda = z.seeda;
		seedb = z.seedb;
		return *this;
	}

	inline u64 sign() const { return id; }

	inline u64 operator ()(const board& b, const bool& after = true) const {
		register u64 hash = after ? seeda : seedb;
		hash ^= zhash[0][b.fetch(0)];
		hash ^= zhash[1][b.fetch(1)];
		hash ^= zhash[2][b.fetch(2)];
		hash ^= zhash[3][b.fetch(3)];
		return hash;
	}
	inline u64 operator ()(const board::tile& t, const bool& after = true) const {
		return (after ? seeda : seedb) ^ ztile[t.i][u32(t)];
	}

    friend std::ostream& operator <<(std::ostream& out, const zhasher& z) {
    	auto& seeda = z.seeda;
    	auto& seedb = z.seedb;
    	auto& zhash = z.zhash;
    	auto& ztile = z.ztile;
    	auto& id = z.id;
		u32 code = 1;
		moporgic::write_cast<byte>(out, code);
		switch (code) {
		case 1:
			moporgic::write(out, id);
			moporgic::write(out, seeda);
			moporgic::write(out, seedb);
			moporgic::write(out, *zhash, sizeof(u64[4][1 << 20]));
			moporgic::write(out, *ztile, sizeof(u64[16][32]));
			break;
		default:
			std::cerr << "unknown serial at zhasher::>>" << std::endl;
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, zhasher& z) {
    	auto& seeda = z.seeda;
    	auto& seedb = z.seedb;
    	auto& zhash = z.zhash;
    	auto& ztile = z.ztile;
    	auto& id = z.id;
		u32 code;
		read_cast<byte>(in, code);
		switch (code) {
		case 0:
			moporgic::read(in, seeda);
			moporgic::read(in, seedb);
			moporgic::read(in, *zhash, sizeof(u64[4][1 << 20]));
			for (u32 i = 0; i < 16; i++) // build ztile
				for (u32 t = 0; t < 32; t++)
					ztile[i][t] = rand64();
			moporgic::read(in, id);
			break;
		case 1:
			moporgic::read(in, id);
			moporgic::read(in, seeda);
			moporgic::read(in, seedb);
			moporgic::read(in, *zhash, sizeof(u64[4][1 << 20]));
			moporgic::read(in, *ztile, sizeof(u64[16][32]));
			break;
		default:
			std::cerr << "unknown serial at zhasher::<<" << std::endl;
			break;
		}
		return in;
	}
private:
	typedef u64(*zhash_t)[1<<20];
	typedef u64(*ztile_t)[32];

	u64 id;
//	u64 zhash[4][1 << 20];
//	u64 ztile[16][32];
	zhash_t zhash;
	ztile_t ztile;
	u64 seeda;
	u64 seedb;
};

class segment {
public:
	class piece {
	friend class segment;
	public:
		inline piece(const piece& i) = default;
		inline piece(byte* alloc = nullptr, size_t space = 0) : alloc(alloc), space(space) {}

		template<typename pointer>
		inline operator pointer() { return cast<pointer>(alloc); }
		inline operator byte*()   { return alloc; }
		inline operator bool()    { return alloc && space; }

		inline size_t size()  const { return space; }
		inline byte*  begin() const { return alloc; }
		inline byte*  end()   const { return alloc + space; }

		inline bool operator <(const piece& i) const { return alloc < i.alloc; }
		inline bool operator >(const piece& i) const { return alloc > i.alloc; }

	private:
		byte*  alloc;
		size_t space;
	};
	typedef std::list<piece> plist;

	segment() { free.emplace_back(); }
	segment(const segment& seg) = delete;

	template<typename pointer = piece>
	pointer allocate(size_t space) {
		for (auto it = free.begin(); it != free.end(); it++) {
			if (it->space >= space) {
				byte* alloc = it->alloc;
				it->alloc += space;
				it->space -= space;
				if (it->space == 0) free.erase(it);
				return piece(alloc, space);
			}
		}
		return piece();
	}

	template<typename pointer = piece>
	pointer release(byte* alloc, size_t space) {
		piece tok(alloc, space);
		auto it = std::lower_bound(free.begin(), free.end(), tok);
		auto pt = it; pt--;
		if (it != free.begin() && pt->end() == tok.begin()) { // -=
			pt->space += tok.space;
			if (pt->end() == it->begin() && it->size()) { // -=-
				pt->space += it->space;
				tok.space += it->space;
				free.erase(it);
			}
		} else if (tok.end() == it->begin() && it->size()) { // =-
			it->alloc -= tok.space;
			it->space += tok.space;
		} else { // = -
			free.insert(it, tok);
		}
		return piece(tok.end());
	}

	inline byte* poll(size_t size) {
		return allocate(size);
	}

	inline void offer(byte* data, size_t size) {
		if (size == 0) return;
		if (free.back().alloc < data + size) {
			free.back().alloc = data + size;
		}
		release(data, size);
	}

private:
	plist  free;
};

class transposition {
public:
	class position {
	friend class transposition;
	public:
		u64 sign;
		f32 esti;
		i16 depth;
		u16 info;
		position(const position& e) = default;
		position(const u64& sign = 0) : sign(sign), esti(0), depth(-1), info(0) {}

		inline operator numeric() const { return esti; }
		inline operator bool()    const { return sign; }
		inline bool operator ==(const u64& s) const { return sign == s; }
		inline bool operator >=(const i32& d) const { return depth >= d; }

		inline bool operator < (const position& en) const { return info < en.info; }
		inline bool operator > (const position& en) const { return info > en.info; }

		inline position& save(const f32& e, const i32& d) {
			esti = e;
			depth = d;
			return *this;
		}
		inline position& operator()(const u64& s) {
			if (sign != s) {
				sign = s;
				esti = 0;
				depth = -1;
				info = 0;
			} else {
				info = std::min(info + 1, 65535);
			}
			return *this;
		}
	};

	transposition() : zhash(), cache(nullptr), mpool(), zsize(0), limit(0), zmask(-1) {}
	transposition(const transposition& tp) = default;
	~transposition() {}
	inline size_t length() const { return zsize; }
	inline size_t size() const { return zsize + limit; }

	inline position& operator[] (const board& b) {
		auto x = u64(b);
		for (auto i = 1; i < 8; i++) {
			board t = b; t.isomorphic(i);
			x = std::min(x, u64(t));
		}
		auto hash = zhash(x) & zmask;

		auto& data = cache[hash];
		if (!data) return data(x);
		if (!std::isnan(data.esti)) {
			if (data == x || !data.info) return data(x);
			auto list = mpool_alloc(2);
			if (!list) return data(x);
			list[0] = data;
			data(cast<uintptr_t>(list)).save(0.0 / 0.0, 1);
			return list[1](x);
		}

		auto& list = raw_cast<position*>(data.sign);
		auto& last = raw_cast<u16>(data.depth);
		auto  size = last + 1;
		for (auto it = list; it != list + size; it++)
			if ((*it) == x) return (*it)(x);
		if (size != (1 << math::ones16(last))) return list[++last](x);

		for (auto it = list; it != list + last; it++)
			if (*(it) < *(it + 1)) std::swap(*(it), *(it + 1));
		auto mini = list[last].info;
		auto hits = 0;
		for (auto it = list; it != list + size; it++) {
			hits += it->info;
			it->info -= mini;
		}
		if (mini <= (hits / (size * 2))) return list[last](x);
		if (size == 65536)               return list[last](x);

		auto temp = mpool_realloc(list, size);
		if (!temp) return list[last](x);
		list = temp;
		return list[++last](x);
	}

    friend std::ostream& operator <<(std::ostream& out, const transposition& tp) {
    	auto& zhash = tp.zhash;
    	auto& cache = tp.cache;
    	auto& zsize = tp.zsize;
    	auto& limit = tp.limit;
		u32 code = 1;
		write_cast<byte>(out, code);
		switch (code) {
		default:
			std::cerr << "unknown serial at transposition::>>" << std::endl;
			std::cerr << "use default serial (1) instead" << std::endl;
			// no break;
		case 1:
			out << zhash;
			write_cast<u64>(out, zsize);
			write_cast<u64>(out, limit);
			for (u64 i = 0; i < zsize; i++) {
				auto& data = cache[i];
				write_cast<u64>(out, data.sign);
				if (!data.sign) continue;
				write_cast<f32>(out, data.esti);
				write_cast<i16>(out, data.depth);
				write_cast<u16>(out, data.info);
				if (!std::isnan(data.esti)) continue;

				auto size = u16(data.depth) + 1u;
				auto list = cast<position*>(data.sign);
				std::sort(list, list + size, std::greater<position>());
				for (auto it = list; it != list + size; it++) {
					write_cast<u64>(out, it->sign);
					write_cast<f32>(out, it->esti);
					write_cast<i16>(out, it->depth);
					write_cast<u16>(out, it->info);
				}
			}
			write_cast<u16>(out, 0); // reserved fields
			break;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, transposition& tp) {
    	auto& zhash = tp.zhash;
    	auto& cache = tp.cache;
    	auto& mpool = tp.mpool;
    	auto& zsize = tp.zsize;
    	auto& limit = tp.limit;
    	auto& zmask = tp.zmask;
		u32 code = 0;
		read_cast<byte>(in, code);
		switch (code) {
		case 1:
			in >> zhash;
			read_cast<u64>(in, zsize);
			read_cast<u64>(in, limit);
			zmask = zsize - 1;
			cache = alloc(zsize + limit);
			mpool.offer(cast<byte*>(cache + zsize), sizeof(position) * limit);
			for (u64 i = 0; i < zsize; i++) {
				auto& data = cache[i];
				read_cast<u64>(in, data.sign);
				if (!data.sign) continue;
				read_cast<f32>(in, data.esti);
				read_cast<i16>(in, data.depth);
				read_cast<u16>(in, data.info);
				if (!std::isnan(data.esti)) continue;

				auto size = u16(data.depth) + 1u;
				auto list = tp.mpool_alloc((2 << math::lg(size - 1)));
				data.sign = cast<uintptr_t>(list);
				for (auto it = list; it != list + size; it++) {
					read_cast<u64>(in, it->sign);
					read_cast<f32>(in, it->esti);
					read_cast<i16>(in, it->depth);
					read_cast<u16>(in, it->info);
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

	void summary() {
		std::cout << std::endl << "summary" << std::endl;
		if (zsize <= 1) {
			std::cout << "no transposition" << std::endl;
			return;
		}
		std::vector<u64> stat = count();
		std::cout << "used" "\t" "count" << std::endl;
		for (u32 i = 0; i < stat.size(); i++)
			std::cout << i << "\t" << stat[i] << std::endl;
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

	static transposition& make(size_t len, size_t lim = 0) {
		return instance().shape(std::max(len, size_t(1)), lim);
	}
	static inline transposition& instance() { static transposition tp(1, 0); return tp; }
	static inline position& find(const board& b) { return instance()[b]; }
	static inline position& remove(const board& b) { return find(b)(0); }

private:
	transposition(const size_t& len, const size_t& lim) : zsize(len), limit(lim), zmask(len - 1) {
		cache = zsize ? alloc(zsize) : nullptr;
		mpool.offer(cast<byte*>(limit ? alloc(limit) : nullptr), sizeof(position) * limit);
	}
	static inline position* alloc(size_t len) { return new position[len](); }
	static inline void free(position* alloc) { delete[] alloc; }

	inline position* mpool_alloc(size_t num) {
		return mpool.allocate<position*>(sizeof(position) * num);
	}
	inline position* mpool_free(position* pos, size_t now) {
		return mpool.release(cast<byte*>(pos), sizeof(position) * now);
	}
	inline position* mpool_realloc(position* pos, size_t now) {
		auto alloc = mpool_alloc(now + now);
		if (!alloc) return nullptr;
		std::copy(pos, pos + now, alloc);
		mpool_free(pos, now);
		return alloc;
	}

	transposition& shape(size_t len, size_t lim = 0) {
		if (math::ones64(len) != 1) {
			std::cerr << "unsupported transposition size: " << len << ", ";
			len = 1ull << (math::lg64(len));
			std::cerr << "fit to " << len << std::endl;
		}

		if (cache && zsize > 1) {
			if (zsize > len) std::cerr << "unsupported operation: shrink z-size" << std::endl;
			if (zsize < len) std::cerr << "unsupported operation: enlarge z-size" << std::endl;
			if (limit > lim) std::cerr << "unsupported operation: shrink m-pool" << std::endl;
			if (limit < lim) {
				mpool_offer(lim - limit);
				limit = lim;
			}
		} else {
			zsize = len;
			limit = lim;
			zmask = len - 1;
			if (cache) free(cache);
			cache = alloc(zsize);
			mpool_offer(limit);
		}
		return *this;
	}

	void mpool_offer(size_t total) {
		try {
			mpool.offer(cast<byte*>(alloc(total)), sizeof(position) * total);
		} catch (std::bad_alloc&) {	// try allocate block size 1G
			for (auto block = 0ull; total; total -= block) {
				block = std::min(size_t(1ull << 30) / sizeof(position), total);
				mpool.offer(cast<byte*>(alloc(block)), sizeof(position) * block);
			}
		}
	}

	std::vector<u64> count() const {
		std::vector<u64> stat(65537);
		for (u64 i = 0; i < zsize; i++) {
			auto& h = cache[i];
			if (std::isnan(h.esti)) {
				stat[u16(h.depth) + 1]++;
			} else {
				stat[h.sign ? 1 : 0]++;
			}
		}
		while (stat.back() == 0)
			stat.pop_back();
		return stat;
	}

private:
	zhasher zhash;
	position* cache;
	segment mpool;
	size_t zsize;
	size_t limit;
	size_t zmask;
};

namespace utils {

class options {
public:
	options() {}
	options(const options& opts) : opts(opts.opts), extra(opts.extra) {}

	std::string& operator [](const std::string& opt) {
		if (opts.find(opt) == opts.end()) {
			auto it = std::find(extra.begin(), extra.end(), opt);
			if (it != extra.end()) extra.erase(it);
			opts[opt] = "";
		}
		return opts[opt];
	}

	bool operator ()(const std::string& opt) const {
		if (opts.find(opt) != opts.end()) return true;
		if (std::find(extra.begin(), extra.end(), opt) != extra.end()) return true;
		return false;
	}

	bool operator +=(const std::string& opt) {
		if (operator ()(opt)) return false;
		extra.push_back(opt);
		return true;
	}
	bool operator -=(const std::string& opt) {
		if (operator ()(opt) == false) return false;
		if (opts.find(opt) != opts.end()) opts.erase(opts.find(opt));
		auto it = std::find(extra.begin(), extra.end(), opt);
		if (it != extra.end()) extra.erase(it);
		return true;
	}

	std::string find(const std::string& opt, const std::string& def = "") const {
		return operator()(opt) ? const_cast<options&>(*this)[opt] : def;
	}

	std::string find(const std::string& opt, const std::vector<std::string>& ext = {}, const std::string& def = "") const {
		if (!operator()(opt)) return def;
		std::stringstream ss(const_cast<options&>(*this)[opt]);
		for (std::string token; ss >> token; ) {
			if (std::find(ext.begin(), ext.end(), token.substr(0, token.find('='))) != ext.end()) {
				return token.substr(token.find('=') + 1);
			}
		}
		return def;
	}

	operator std::string() const {
		std::string res;
		for (auto v : opts) {
			res.append(v.first).append("=").append(v.second).append(" ");
		}
		for (auto s : extra) {
			res.append(s).append(" ");
		}
		return res;
	}

private:
	std::map<std::string, std::string> opts;
	std::list<std::string> extra;
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

u64 indexmerge0(const board& b) { // 16-tpbit
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
u64 indexmerge1(const board& b) { // 8-tpbit
	register u32 merge = 0;
	board k = b; if (transpose) k.transpose();
	merge |= k.query(0).merge << 0;
	merge |= k.query(1).merge << 2;
	merge |= k.query(2).merge << 4;
	merge |= k.query(3).merge << 6;
	return merge;
}

u64 indexnum0(const board& b) { // 10-tpbit
	// 2k ~ 32k, 2-tpbit ea.
	auto num = b.numof();
	register u64 index = 0;
	index += (num[11] & 0x03) << 0;
	index += (num[12] & 0x03) << 2;
	index += (num[13] & 0x03) << 4;
	index += (num[14] & 0x03) << 6;
	index += (num[15] & 0x03) << 8;
	return index;
}

u64 indexnum1(const board& b) { // 25-tpbit
	auto num = b.numof();
	register u64 index = 0;
	index += ((num[5] + num[6]) & 0x0f) << 0; // 32 & 64, 4-bit
	index += (num[7] & 0x07) << 4; // 128, 3-bit
	index += (num[8] & 0x07) << 7; // 256, 3-bit
	index += (num[9] & 0x07) << 10; // 512, 3-tpbit
	index += (num[10] & 0x03) << 13; // 1k ~ 32k, 2-tpbit ea.
	index += (num[11] & 0x03) << 15;
	index += (num[12] & 0x03) << 17;
	index += (num[13] & 0x03) << 19;
	index += (num[14] & 0x03) << 21;
	index += (num[15] & 0x03) << 23;
	return index;
}

u64 indexnum2(const board& b) { // 25-tpbit
	auto num = b.numof();
	register u64 index = 0;
	index += ((num[1] + num[2]) & 0x07) << 0; // 2 & 4, 3-tpbit
	index += ((num[3] + num[4]) & 0x07) << 3; // 8 & 16, 3-tpbit
	index += ((num[5] + num[6]) & 0x07) << 6; // 32 & 64, 3-tpbit
	index += ((num[7] + num[8]) & 0x07) << 9; // 126 & 256, 3-tpbit
	index += ((num[9] + num[10]) & 0x07) << 12; // 512 & 1k, 3-tpbit
	index += ((num[11]) & 0x03) << 15; // 2k ~ 32k, 2-tpbit ea.
	index += ((num[12]) & 0x03) << 17;
	index += ((num[13]) & 0x03) << 19;
	index += ((num[14]) & 0x03) << 21;
	index += ((num[15]) & 0x03) << 23;

	return index;
}

template<int transpose, int qu0, int qu1>
u64 indexnum2x(const board& b) { // 25-tpbit
	board o = b;
	if (transpose) o.transpose();
	auto& m = o.query(qu0).numof;
	auto& n = o.query(qu1).numof;

	register u64 index = 0;
	index += ((m[1] + n[1] + m[2] + n[2]) & 0x07) << 0; // 2 & 4, 3-tpbit
	index += ((m[3] + n[3] + m[4] + n[4]) & 0x07) << 3; // 8 & 16, 3-tpbit
	index += ((m[5] + n[5] + m[6] + n[6]) & 0x07) << 6; // 32 & 64, 3-tpbit
	index += ((m[7] + n[7] + m[8] + n[8]) & 0x07) << 9; // 126 & 256, 3-tpbit
	index += ((m[9] + n[9] + m[10] + n[10]) & 0x07) << 12; // 512 & 1k, 3-tpbit
	index += ((m[11] + n[11]) & 0x03) << 15; // 2k ~ 32k, 2-tpbit ea.
	index += ((m[12] + n[12]) & 0x03) << 17;
	index += ((m[13] + n[13]) & 0x03) << 19;
	index += ((m[14] + n[14]) & 0x03) << 21;
	index += ((m[15] + n[15]) & 0x03) << 23;

	return index;
}

u64 indexnum3(const board& b) { // 28-tpbit
	auto num = b.numof();
	register u64 index = 0;
	index += ((num[0] + num[1] + num[2]) & 0x0f) << 0; // 0 & 2 & 4, 4-tpbit
	index += ((num[3] + num[4]) & 0x07) << 4; // 8 & 16, 3-tpbit
	index += ((num[5] + num[6]) & 0x07) << 7; // 32 & 64, 3-bit
	index += (num[7] & 0x03) << 10; // 128, 2-bit
	index += (num[8] & 0x03) << 12; // 256, 2-bit
	index += (num[9] & 0x03) << 14; // 512, 2-tpbit
	index += (num[10] & 0x03) << 16; // 1k ~ 32k, 2-tpbit ea.
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
u64 indexmask(const board& b) { // 16-tpbit
	board k = b;
	k.rotate(isomorphic);
	if (isomorphic >= 4) k.mirror();
	return k.mask(tile);
}

template<int isomorphic>
u64 indexmax(const board& b) { // 16-tpbit
	board k = b;
	k.rotate(isomorphic);
	if (isomorphic >= 4) k.mirror();
	return k.mask(k.max());
}

u32 make_indexers(const std::string& res = "") {
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

u32 save_weights(const std::string& res) {
	u32 succ = 0;
	std::string path(res);
	for (auto last = path.find('|'); (last = path.find('|')) != std::string::npos; path = path.substr(last + 1)) {
		std::ofstream out;
		char buf[1 << 20];
		out.rdbuf()->pubsetbuf(buf, sizeof(buf));
		out.open(path.substr(0, last), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) continue;
		weight::save(out);
		out.flush();
		out.close();
		succ++;
	}
	return succ;
}
u32 load_weights(const std::string& res) {
	u32 succ = 0;
	std::string path(res);
	for (auto last = path.find('|'); (last = path.find('|')) != std::string::npos; path = path.substr(last + 1)) {
		std::ifstream in;
		char buf[1 << 20];
		in.rdbuf()->pubsetbuf(buf, sizeof(buf));
		in.open(path.substr(0, last), std::ios::in | std::ios::binary);
		if (!in.is_open()) continue;
		weight::load(in);
		in.close();
		succ++;
	}
	return succ;
}
u32 make_weights(const std::string& res = "") {
	u32 succ = 0;
	auto wmake = [&](u32 sign, u64 size) {
		if (weight::find(sign) != weight::end()) return;
		weight::make(sign, size); succ++;
	};

	// weight:size weight(size) weight[size] weight:patt weight:? weight:^bit
	std::string in(res);
	if (in.empty() && weight::list().empty())
		in = "default";
	std::map<std::string, std::string> predefined;
	predefined["khyeh"] = "012345:patt 456789:patt 012456:patt 45689a:patt ";
	predefined["patt/42-33"] = "012345:patt 456789:patt 89abcd:patt 012456:patt 45689a:patt ";
	predefined["patt/4-22"] = "0123:patt 4567:patt 0145:patt 1256:patt 569a:patt ";
	predefined["k.matsuzaki"] = "012456:? 12569d:? 012345:? 01567a:? 01259a:? 0159de:? 01589d:? 01246a:? ";
	predefined["monotonic"] = "fd012301:^24 fd456701:^24 ";
	predefined["default"] = predefined["khyeh"] + predefined["monotonic"] + "fe000005:^24 fe000015:^24 ";
	predefined["4x6patt"] = predefined["khyeh"];
	predefined["5x6patt"] = predefined["patt/42-33"];
	predefined["8x6patt"] = predefined["k.matsuzaki"];
	predefined["5x4patt"] = predefined["patt/4-22"];
	for (auto predef : predefined) {
		if (in.find(predef.first) != std::string::npos) { // insert predefined weights
			in.insert(in.find(predef.first), predef.second);
			in.replace(in.find(predef.first), predef.first.size(), "");
		}
	}

	while (in.find_first_of(":|()[],") != std::string::npos)
		in[in.find_first_of(":|()[],")] = ' ';
	std::stringstream wghtin(in);
	std::string signs, sizes;
	while (wghtin >> signs && wghtin >> sizes) {
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

u32 save_features(const std::string& res) {
	u32 succ = 0;
	std::string path(res);
	for (auto last = path.find('|'); (last = path.find('|')) != std::string::npos; path = path.substr(last + 1)) {
		std::ofstream out;
		out.open(path.substr(0, last), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) continue;
		feature::save(out);
		out.flush();
		out.close();
		succ++;
	}
	return succ;
}
u32 load_features(const std::string& res) {
	u32 succ = 0;
	std::string path(res);
	for (auto last = path.find('|'); (last = path.find('|')) != std::string::npos; path = path.substr(last + 1)) {
		std::ifstream in;
		in.open(path.substr(0, last), std::ios::in | std::ios::binary);
		if (!in.is_open()) continue;
		feature::load(in);
		in.close();
		succ++;
	}
	return succ;
}
u32 make_features(const std::string& res = "") {
	u32 succ = 0;
	auto fmake = [&](u32 wght, u32 idxr) {
		if (feature::find(wght, idxr) != feature::end()) return;
		feature::make(wght, idxr); succ++;
	};

	// weight:indexer weight(indexer) weight[indexer]
	std::string in(res);
	if (in.empty() && feature::list().empty())
		in = "default";
	std::map<std::string, std::string> predefined;
	predefined["khyeh"] = "012345[012345!] 456789[456789!] 012456[012456!] 45689a[45689a!] ";
	predefined["patt/42-33"] = "012345[012345!] 456789[456789!] 89abcd[89abcd!] 012456[012456!] 45689a[45689a!] ";
	predefined["patt/4-22"] = "0123[0123!] 4567[4567!] 0145[0145!] 1256[1256!] 569a[569a!] ";
	predefined["monotonic"] = "fd012301[fd012301] fd012301[fd37bf01] fd012301[fdfedc01] fd012301[fdc84001] "
							  "fd012301[fd321001] fd012301[fdfb7301] fd012301[fdcdef01] fd012301[fd048c01] "
							  "fd456701[fd456701] fd456701[fd26ae01] fd456701[fdba9801] fd456701[fdd95101] "
							  "fd456701[fd765401] fd456701[fdea6201] fd456701[fd89ab01] fd456701[fd159d01] ";
	predefined["k.matsuzaki"] = "012456:012456! 12569d:12569d! 012345:012345! 01567a:01567a! "
								"01259a:01259a! 0159de:0159de! 01589d:01589d! 01246a:01246a! ";
	predefined["default"] = predefined["khyeh"] + predefined["monotonic"] + "fe000005[fe000005] fe000015[fe000015] ";
	predefined["4x6patt"] = predefined["khyeh"];
	predefined["5x6patt"] = predefined["patt/42-33"];
	predefined["8x6patt"] = predefined["k.matsuzaki"];
	predefined["5x4patt"] = predefined["patt/4-22"];
	for (auto predef : predefined) {
		if (in.find(predef.first) != std::string::npos) { // insert predefined features
			in.insert(in.find(predef.first), predef.second);
			in.replace(in.find(predef.first), predef.first.size(), "");
		}
	}

	while (in.find_first_of(":|()[],") != std::string::npos)
		in[in.find_first_of(":|()[],")] = ' ';
	std::stringstream featin(in);
	std::string wghts, idxrs;
	while (featin >> wghts && featin >> idxrs) {
		u32 wght = 0, idxr = 0;

		std::stringstream(wghts) >> std::hex >> wght;
		if (weight::find(wght) == weight::end()) {
			std::cerr << "unknown weight (" << wghts << ") at make_features, ";
			std::cerr << "assume as pattern descriptor..." << std::endl;
			weight::make(wght, std::pow(16ull, wghts.size()));
		}

		std::vector<int> isomorphic = { 0 };
		for (; !std::isxdigit(idxrs.back()); idxrs.pop_back()) {
			switch (idxrs.back()) {
			case '!': isomorphic = { 0, 1, 2, 3, 4, 5, 6, 7 }; break;
			}
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
			snprintf(buf, sizeof(buf), "weight(%08llx)[%llu] = %d%c", w.sign(), w.size(), usage, scale);
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

inline numeric update(const board& state, const numeric& updv,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
	register numeric esti = 0;
	for (auto f = begin; f != end; f++)
		esti += ((*f)[state] += updv);
	return esti;
}

void make_transposition(const std::string& res = "") {
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
bool load_transposition(const std::string& path) {
	std::ifstream in;
	char buf[1 << 20];
	in.rdbuf()->pubsetbuf(buf, sizeof(buf));
	in.open(path, std::ios::in | std::ios::binary);
	if (!in.is_open()) return false;
	transposition::load(in);
	in.close();
	return true;
}
bool save_transposition(const std::string& path) {
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

numeric search_expt(const board& after, const i32& depth,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end());
numeric search_max(const board& before, const i32& depth,
		const feature::iter begin = feature::begin(), const feature::iter end = feature::end());

numeric search_expt(const board& after, const i32& depth,
		const feature::iter begin, const feature::iter end) {
	if (depth <= 0) return utils::estimate(after, begin, end);
	auto& t = transposition::find(after);
	if (t >= depth) return t;
	const auto spaces = after.spaces();
	numeric expt = 0;
	board before = after;
	for (u32 i = 0; i < spaces.size; i++) {
		const u32 pos = spaces[i];
		before.set(pos, 1);
		expt += 9 * search_max(before, depth - 1, begin, end);
		before.set(pos, 2);
		expt += 1 * search_max(before, depth - 1, begin, end);
		before = after;
	}
	numeric esti = expt / (10 * spaces.size);
	return t.save(esti, depth);
}

numeric search_max(const board& before, const i32& depth,
		const feature::iter begin, const feature::iter end) {
	numeric expt = 0;
	board after = before;
	register i32 reward;
	if ((reward = after.up()) != -1) {
		expt = std::max(expt, reward + search_expt(after, depth - 1, begin, end));
		after = before;
	}
	if ((reward = after.right()) != -1) {
		expt = std::max(expt, reward + search_expt(after, depth - 1, begin, end));
		after = before;
	}
	if ((reward = after.down()) != -1) {
		expt = std::max(expt, reward + search_expt(after, depth - 1, begin, end));
		after = before;
	}
	if ((reward = after.left()) != -1) {
		expt = std::max(expt, reward + search_expt(after, depth - 1, begin, end));
		after = before;
	}
	return expt;
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
		esti = state::reward() + utils::update(move, alpha * (accu - state::value()), begin, end);
		return esti;
	}
	inline numeric search(const i32& depth,
			const feature::iter begin = feature::begin(), const feature::iter end = feature::end()) {
		if (score >= 0) {
			esti = state::reward() + utils::search_expt(move, depth, begin, end);
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
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
	inline operator board() const { return move; }
	inline operator bool() const { return score >= 0; }

	void operator >>(std::ostream& out) const {
		move >> out;
		moporgic::write(out, score);
	}
	void operator <<(std::istream& in) {
		move << in;
		moporgic::read(in, score);
	}

	inline static numeric& alpha() {
		static numeric a = numeric(0.0025);
		return a;
	}
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
	inline numeric estimate() const { return best->estimate(); }
};
struct search : select {
	u32 policy[16];
	search(const u32* depth = search::depth()) : select() { std::copy(depth, depth + 16, policy); }

	inline select& operator ()(const board& b) {
		return operator ()(b, feature::begin(), feature::end());
	}
	inline select& operator ()(const board& b, const feature::iter begin, const feature::iter end) {
		u32 depth = policy[b.spaces().size] - 1;
		move[0].assign(b);
		move[1].assign(b);
		move[2].assign(b);
		move[3].assign(b);
		move[0].search(depth, begin, end);
		move[1].search(depth, begin, end);
		move[2].search(depth, begin, end);
		move[3].search(depth, begin, end);
		return update();
	}
	inline select& operator <<(const board& b) { return operator ()(b); }

	static u32* depth() {
		static u32 dd[16] = { 7, 7, 7, 7, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3 };
		return dd;
	}
};
struct statistic {
	u64 limit;
	u64 loop;
	u64 check;
	u32 winv;

	std::string indexf;
	std::string localf;
	std::string totalf;

	struct control {
		u64 num;
		u64 chk;
		u32 win;
		control(u64 num = 1000, u64 chk = 1000, u32 win = 2048) : num(num), chk(chk), win(win) {}
		operator bool() const { return num; }
	};

	struct record {
		u64 score;
		u64 win;
		u64 time;
		u64 opers;
		u32 max;
		u32 hash;
	} total, local;

	struct each {
		std::array<u64, 32> score;
		std::array<u64, 32> opers;
		std::array<u64, 32> count;
	} every;


	void init(const control& ctrl = control()) {
		limit = ctrl.num * ctrl.chk;
		loop = 1;
		check = ctrl.chk;
		winv = ctrl.win;

//		indexf = "%03llu/%03llu %llums %.2fops";
//		localf = "local:  avg=%llu max=%u tile=%u win=%.2f%%";
//		totalf = "total:  avg=%llu max=%u tile=%u win=%.2f%%";
		u32 dec = std::max(std::floor(std::log10(ctrl.num)) + 1, 3.0);
		indexf = "%0" + std::to_string(dec) + "llu/%0" + std::to_string(dec) + "llu %llums %.2fops";
		localf = "local:" + std::string((dec << 1) - 4, ' ') + "avg=%llu max=%u tile=%u win=%.2f%%";
		totalf = "total:" + std::string((dec << 1) - 4, ' ') + "avg=%llu max=%u tile=%u win=%.2f%%";

		every = {};
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
		if (hash >= winv) local.win += 1;
		local.max = std::max(local.max, score);
		every.count[std::log2(hash)] += 1;
		every.score[std::log2(hash)] += score;
		every.opers[std::log2(hash)] += opers;

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
		snprintf(buf, sizeof(buf), indexf.c_str(), // "%03llu/%03llu %llums %.2fops",
				loop / check,
				limit / check,
				elapsedtime,
				local.opers * 1000.0 / elapsedtime);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), localf.c_str(), // "local:  avg=%llu max=%u tile=%u win=%.2f%%",
				local.score / check,
				local.max,
				math::msb32(local.hash),
				local.win * 100.0 / check);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), totalf.c_str(), // "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / loop,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / loop);
		std::cout << buf << std::endl;

		local = {};
		local.time = currtimept;
	}

	void summary() const {
		std::cout << std::endl << "summary" << std::endl;
		char buf[80];
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

	void merge(const statistic& stat) {
		total.score += stat.total.score;
		total.win += stat.total.win;
		total.time += stat.total.time;
		total.opers += stat.total.opers;
		total.hash |= stat.total.hash;
		total.max = std::max(total.max, stat.total.max);
		std::transform(every.count.begin(), every.count.end(), stat.every.count.begin(), every.count.begin(), std::plus<u64>());
		std::transform(every.score.begin(), every.score.end(), stat.every.score.begin(), every.score.begin(), std::plus<u64>());
		std::transform(every.opers.begin(), every.opers.end(), stat.every.opers.begin(), every.opers.begin(), std::plus<u64>());
	}
};

inline utils::options parse(int argc, const char* argv[]) {
	utils::options opts;
	auto find_opt = [&](int& i, const char* defval) -> std::string {
		if (i + 1 < argc && *(argv[i + 1]) != '-') return argv[++i];
		if (defval != nullptr) return defval;
		std::cerr << "invalid: " << argv[i] << std::endl;
		std::exit(1); return "";
	};
	auto find_opts = [&](int& i, const char& split) -> std::string {
		std::string vu;
		for (std::string v; (v = find_opt(i, "")).size(); ) vu += (v += split);
		return vu;
	};
	for (int i = 1; i < argc; i++) {
		switch (to_hash(argv[i])) {
		case to_hash("-a"):
		case to_hash("--alpha"):
			opts["alpha"] = find_opt(i, nullptr);
			break;
		case to_hash("-t"):
		case to_hash("--train"):
			opts["train"] = find_opt(i, nullptr);
			break;
		case to_hash("-T"):
		case to_hash("-e"):
		case to_hash("--test"):
			opts["test"] = find_opt(i, nullptr);
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
		case to_hash("--srand"):
			opts["seed"] = find_opt(i, nullptr);
			break;
		case to_hash("-wio"):
		case to_hash("--weight-input-output"):
			opts["weight-input"] = opts["weight-output"] = find_opts(i, '|');
			break;
		case to_hash("-wi"):
		case to_hash("--weight-input"):
			opts["weight-input"] = find_opts(i, '|');
			break;
		case to_hash("-wo"):
		case to_hash("--weight-output"):
			opts["weight-output"] = find_opts(i, '|');
			break;
		case to_hash("-fio"):
		case to_hash("--feature-input-output"):
			opts["feature-input"] = opts["feature-output"] = find_opts(i, '|');
			break;
		case to_hash("-fi"):
		case to_hash("--feature-input"):
			opts["feature-input"] = find_opts(i, '|');
			break;
		case to_hash("-fo"):
		case to_hash("--feature-output"):
			opts["feature-output"] = find_opts(i, '|');
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
		case to_hash("--weight-value"):
			opts["weight-value"] = find_opts(i, ',');
			break;
		case to_hash("-f"):
		case to_hash("--feature"):
		case to_hash("--feature-value"):
			opts["feature-value"] = find_opts(i, ',');
			break;
		case to_hash("-wf"):
		case to_hash("-fw"):
			opts["feature-value"] = opts["weight-value"] = find_opts(i, ',');
			break;
		case to_hash("-o"):
		case to_hash("--option"):
		case to_hash("--options"):
		case to_hash("--extra"):
			opts["options"] = find_opts(i, ' ');
			break;
		case to_hash("-tt"):
		case to_hash("-tm"):
		case to_hash("--train-mode"):
			opts["train-mode"] = find_opt(i, "");
			break;
		case to_hash("-Tt"):
		case to_hash("-et"):
		case to_hash("-em"):
		case to_hash("--test-mode"):
			opts["test-mode"] = find_opt(i, "");
			break;
		case to_hash("-tc"):
		case to_hash("-tu"):
		case to_hash("--train-check"):
		case to_hash("--train-check-interval"):
		case to_hash("--train-unit"):
			opts["train-unit"] = find_opt(i, "1000");
			break;
		case to_hash("-Tc"):
		case to_hash("-ec"):
		case to_hash("-eu"):
		case to_hash("--test-check"):
		case to_hash("--test-check-interval"):
		case to_hash("--test-unit"):
			opts["test-unit"] = find_opt(i, "1000");
			break;
		case to_hash("-c"):
		case to_hash("--comment"):
			opts["comment"] = find_opts(i, ' ');
			break;
		case to_hash("-d"):
		case to_hash("--depth"):
		case to_hash("-dd"):
		case to_hash("--depth-dynamic"):
			opts["depth"] = find_opts(i, ',');
			break;
		case to_hash("-tp"):
		case to_hash("--cache"):
		case to_hash("--cache-value"):
		case to_hash("--transposition"):
		case to_hash("--transposition-value"):
			opts["cache-value"] = find_opt(i, "");
			break;
		case to_hash("-tpio"):
		case to_hash("--cache-input-output"):
		case to_hash("--transposition-input-output"):
			opts["cache-output"] = opts["cache-input"] = find_opt(i, "tdl2048.cache");
			break;
		case to_hash("-tpi"):
		case to_hash("--cache-input"):
		case to_hash("--transposition-input"):
			opts["cache-input"] = find_opt(i, "tdl2048.cache");
			break;
		case to_hash("-tpo"):
		case to_hash("--cache-output"):
		case to_hash("--transposition-output"):
			opts["cache-output"] = find_opt(i, "tdl2048.cache");
			break;
		default:
			std::cerr << "unknown: " << argv[i] << std::endl;
			std::exit(1);
			break;
		}
	}
	return opts;
}

statistic train(statistic::control trainctl, utils::options opts = {}) {
	board b;
	state last;
	search xbest;
	statistic stats;
	std::vector<state> path;
	path.reserve(65536);
	u32 score;
	u32 opers;

	switch (to_hash(opts["train-mode"])) {
	case to_hash("backward-xbest"):
		for (stats.init(trainctl); stats; stats++) {

			score = 0;
			opers = 0;

			for (b.init(); xbest << b; b.next()) {
				score += xbest.score();
				opers += 1;
				xbest >> path;
				xbest >> b;
			}

			for (numeric v = 0; path.size(); path.pop_back()) {
				transposition::remove(path.back());
				path.back().estimate();
				v = path.back().update(v);
			}

			stats.update(score, b.hash(), opers);
		}
		break;

	default:
	case to_hash("forward-xbest"):
		for (stats.init(trainctl); stats; stats++) {

			score = 0;
			opers = 0;

			b.init();
			xbest << b;
			score += xbest.score();
			opers += 1;
			xbest >> last;
			xbest >> b;
			b.next();
			while (xbest << b) {
				last += xbest.estimate();
				transposition::remove(last);
				score += xbest.score();
				opers += 1;
				xbest >> last;
				xbest >> b;
				b.next();
			}
			last += 0;
			transposition::remove(last);

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

statistic test(statistic::control testctl, utils::options opts = {}) {
	board b;
	select best;
	search xbest;
	statistic stats;
	u32 score;
	u32 opers;

	switch (to_hash(opts["test-mode"])) {
	case to_hash("best"):
		for (stats.init(testctl); stats; stats++) {

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
	default:
	case to_hash("xbest"):
		for (stats.init(testctl); stats; stats++) {

			u32 score = 0;
			u32 opers = 0;

			for (b.init(); xbest << b; b.next()) {
				score += xbest.score();
				opers += 1;
				xbest >> b;
			}

			stats.update(score, b.hash(), opers);
		}
		break;
	}

	return stats;
}

int main(int argc, const char* argv[]) {
	statistic::control trainctl(1000, 1000);
	statistic::control testctl(1000, 1);
	u32 timestamp = std::time(nullptr);
	u32 seed = timestamp;
	numeric& alpha = state::alpha();
	auto& depth = search::depth();

	utils::options opts = parse(argc, argv);
	if (opts("alpha")) alpha = std::stod(opts["alpha"]);
	if (opts("train")) trainctl.num = std::stol(opts["train"]);
	if (opts("test")) testctl.num = std::stol(opts["test"]);
	if (opts("train-unit")) trainctl.chk = std::stol(opts["train-unit"]);
	if (opts("test-unit")) testctl.chk = std::stol(opts["test-unit"]);
	if (opts("seed")) seed = std::stol(opts["seed"]);
	if (opts("depth")) {
		std::string dyndepth(opts["depth"]);
		for (u32 d = 0, e = 0; e < 16; depth[e++] = d) {
			if (dyndepth.empty()) continue;
			auto it = dyndepth.find_first_of(", ");
			d = std::stol(dyndepth.substr(0, it));
			dyndepth = dyndepth.substr(it + 1);
		}
	}

	std::srand(seed);
	std::cout << "TDL2048+ LOG" << std::endl;
	std::cout << "develop-search" << " build C++" << __cplusplus;
	std::cout << " " << __DATE__ << " " << __TIME__ << std::endl;
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl;
	std::cout << "time = " << timestamp << std::endl;
	std::cout << "seed = " << seed << std::endl;
	std::cout << "alpha = " << alpha << std::endl;
	std::cout << "depth = ";
	std::copy(depth, depth + 16, std::ostream_iterator<u32>(std::cout, " "));
	std::cout << std::endl;
//	printf("board::look[%d] = %lluM", (1 << 20), ((sizeof(board::cache) * (1 << 20)) >> 20));
	std::cout << std::endl;


	utils::make_indexers();

	utils::load_weights(opts["weight-input"]);
	utils::make_weights(opts["weight-value"]);

	utils::load_features(opts["feature-input"]);
	utils::make_features(opts["feature-value"]);

	utils::list_mapping();


	if (trainctl) {
		std::cout << std::endl << "start training..." << std::endl;
		train(trainctl, opts);
	}

	utils::save_weights(opts["weight-output"]);
	utils::save_features(opts["feature-output"]);


	utils::load_transposition(opts["cache-input"]);
	utils::make_transposition(opts["cache-value"]);

	if (testctl) {
		std::cout << std::endl << "start testing..." << std::endl;
		test(testctl, opts).summary();
		transposition::instance().summary();
	}

	utils::save_transposition(opts["cache-output"]);

	std::cout << std::endl;

	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
