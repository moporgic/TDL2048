#pragma once
//============================================================================
// Name        : board.h
// Author      : Hung Guei
// Version     : 2.71
// Description : bitboard of 2048
//============================================================================
#include "moporgic/type.h"
#include "moporgic/util.h"
#include "moporgic/math.h"
#include "moporgic/io.h"
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <array>

namespace moporgic {

class board {
public:
	struct list {
		u64 tile;
		u32 size;
		list(const u64& t, const u32& s) : tile(t), size(s) {}
		list(const list& t) = default;
		list() : tile(0), size(0) {}
		~list() = default;
		inline u32 operator[] (const u32& i) const { return (tile >> (i << 2)) & 0x0f; }
		inline operator bool() const { return size > 0; }
		struct iter {
			u64 raw;
			i32 idx;
			iter(const u64& raw, const i32& idx) : raw(raw), idx(idx) {}
			inline u32 operator *() const { return (raw >> (idx << 2)) & 0x0f; }
			inline bool operator==(const iter& i) const { return idx == i.idx; }
			inline bool operator!=(const iter& i) const { return idx != i.idx; }
			inline iter& operator++() { ++idx; return *this; }
			inline iter& operator--() { --idx; return *this; }
			inline iter  operator++(int) { return iter(raw, ++idx - 1); }
			inline iter  operator--(int) { return iter(raw, --idx + 1); }
		};
		inline iter begin() const { return iter(tile, 0); }
		inline iter end() const { return iter(tile, size); }
	};
	class cache {
	friend class board;
	public:
		class operation {
		friend class board;
		friend class cache;
		public:
			const u32 rawh; // horizontal move (16-bit raw)
			const u32 exth; // horizontal move (4-bit extra)
			const u64 rawv; // vertical move (64-bit raw)
			const u32 extv; // vertical move (16-bit extra)
			const u32 score; // merge score
			const i32 moved; // moved or not (moved: 0, otherwise -1)
			const u32 mono; // monotonic decreasing value (6-bit)

			inline void moveh(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const { moveh64(raw, ext, sc, mv, i); }
			inline void movev(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const { movev64(raw, ext, sc, mv, i); }

			inline void moveh64(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				raw |= u64(rawh) << (i << 4);
				sc += score;
				mv &= moved;
			}
			inline void moveh80(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				moveh64(raw, ext, sc, mv, i);
				ext |= exth << (i << 2);
			}
			inline void movev64(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				raw |= rawv << (i << 2);
				sc += score;
				mv &= moved;
			}
			inline void movev80(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				movev64(raw, ext, sc, mv, i);
				ext |= extv << i;
			}
			operation(const operation& op) = default;
			operation() = delete;
			~operation() = default;
		private:
			operation(u32 rawh, u32 exth, u64 rawv, u32 extv, u32 score, i32 moved, u32 mono)
				: rawh(rawh), exth(exth), rawv(rawv), extv(extv), score(score), moved(moved), mono(mono) {}
		};
		typedef std::array<u16, 32> info;
		const u32 raw; // base row (16-bit raw)
		const u32 ext; // base row (4-bit extra)
		const u32 scale; // hash of this row
		const u32 merge; // number of merged tiles
		const operation left; // left operation
		const operation right; // right operation
		const info count; // number of each tile-type
		const info mask; // mask of each tile-type
		const list pos; // layout of board-type
		const i32 moved; // moved or not
		const u32 legal; // legal actions

		cache(const cache& c) = default;
		cache() = delete;
		~cache() = default;

		static cache make(const u32& r) {
			// HIGH [null][N0~N3 high 1-bit (totally 4-bit)][N0~N3 low 4-bit (totally 16-bit)] LOW

			u32 V[4] = {((r >> 0) & 0x0f) | ((r >> 12) & 0x10), ((r >> 4) & 0x0f) | ((r >> 13) & 0x10),
						((r >> 8) & 0x0f) | ((r >> 14) & 0x10), ((r >> 12) & 0x0f) | ((r >> 15) & 0x10)};
			u32 L[4] = { V[0], V[1], V[2], V[3] }, Ll[4], Lh[4];
			u32 R[4] = { V[3], V[2], V[1], V[0] }, Rl[4], Rh[4]; // mirrored

			u32 raw, ext;
			assign(L, Ll, Lh, raw, ext);

			u32 merge;

			u32 hraw;
			u32 hext;
			u64 vraw;
			u32 vext;
			u32 score;
			i32 moved;
			u32 mono;

			mvleft(L, score, merge, mono);
			u32 mvL = assign(L, Ll, Lh, hraw, hext);
			std::reverse(Ll, Ll + 4); std::reverse(Lh, Lh + 4);
			moved = mvL == r ? -1 : 0;
			map(vraw, vext, Ll, Lh, 12, 8, 4, 0);
			operation left(hraw, hext, vraw, vext, score, moved, mono);

			mvleft(R, score, merge, mono); std::reverse(R, R + 4);
			u32 mvR = assign(R, Rl, Rh, hraw, hext);
			std::reverse(Rl, Rl + 4); std::reverse(Rh, Rh + 4);
			moved = mvR == r ? -1 : 0;
			map(vraw, vext, Rl, Rh, 12, 8, 4, 0);
			operation right(hraw, hext, vraw, vext, score, moved, mono);

			u32 scale = 0;
			info count = {};
			info mask = {};
			for (int i = 0; i < 4; i++) {
				scale |= (1 << V[i]);
				count[V[i]]++;
				mask[V[i]] |= (1 << i);
			}

			u64 ltile = 0;
			u32 lsize = 0;
			for (int i = 0; i < 16; i++) {
				if ((r >> i) & 1) ltile |= (u64(i) << ((lsize++) << 2));
			}
			list pos(ltile, lsize);
			moved = left.moved & right.moved;
			u32 legal = 0;
			if (mvL != r) legal |= (0x08 | 0x01);
			if (mvR != r) legal |= (0x02 | 0x04);

			return cache(raw, ext, scale, merge, left, right, count, mask, pos, moved, legal);
		}
	private:
		cache(u32 raw, u32 ext, u32 scale, u32 merge, operation left, operation right,
			info count, info mask, list pos, i32 moved, u32 legal)
				: raw(raw), ext(ext), scale(scale), merge(merge), left(left), right(right),
				  count(count), mask(mask), pos(pos), moved(moved), legal(legal) {}

		static u32 assign(u32 src[], u32 lo[], u32 hi[], u32& raw, u32& ext) {
			for (u32 i = 0; i < 4; i++) {
				hi[i] = (src[i] & 0x10) >> 4;
				lo[i] = (src[i] & 0x0f);
			}
			raw = ((lo[0] << 0) | (lo[1] << 4) | (lo[2] << 8) | (lo[3] << 12));
			ext = ((hi[0] << 0) | (hi[1] << 1) | (hi[2] << 2) | (hi[3] << 3)) << 16;
			return raw | ext;
		}
		static void map(u64& raw, u32& ext, u32 lo[], u32 hi[], int s0, int s1, int s2, int s3) {
			raw = (u64(lo[0]) << (s0 << 2)) | (u64(lo[1]) << (s1 << 2))
				| (u64(lo[2]) << (s2 << 2)) | (u64(lo[3]) << (s3 << 2));
			ext = ((hi[0] << s0) | (hi[1] << s1) | (hi[2] << s2) | (hi[3] << s3)) << 16;
		}
		static void mvleft(u32 row[], u32& score, u32& merge, u32& mono) {
			u32 top = 0;
			u32 tmp = 0;
			score = merge = mono = 0;
			if (row[0] >= row[1]) mono |= 0x01;
			if (row[0] >= row[2]) mono |= 0x02;
			if (row[0] >= row[3]) mono |= 0x04;
			if (row[1] >= row[2]) mono |= 0x08;
			if (row[1] >= row[3]) mono |= 0x10;
			if (row[2] >= row[3]) mono |= 0x20;

			for (u32 i = 0; i < 4; i++) {
				u32 tile = row[i];
				if (tile == 0) continue;
				row[i] = 0;
				if (tmp != 0) {
					if (tile == tmp) {
						tile = tile + 1;
						row[top++] = tile;
						score += (1 << tile);
						merge++;
						tmp = 0;
					} else {
						row[top++] = tmp;
						tmp = tile;
					}
				} else {
					tmp = tile;
				}
			}
			if (tmp != 0) row[top] = tmp;
		}
	};

	inline static const cache& lookup(const u32& i) {
		static cache look[1 << 20](cache::make(seq32_static()));
		return look[i];
	}

	u64 raw;
	u32 ext;
	u32 nul;

public:
	inline board(const u64& raw = 0) : raw(raw), ext(0), nul(0) {}
	inline board(const u64& raw, const u32& ext) : raw(raw), ext(ext), nul(0) {}
	inline board(const u64& raw, const u16& ext) : board(raw, u32(ext) << 16) {}
	inline board(const board& b) = default;
	inline ~board() = default;
	inline board& operator =(const u64& raw) { this->raw = raw; return *this; }
	inline board& operator =(const board& b) { raw = b.raw, ext = b.ext; return *this; }
	inline bool operator==(const board& b) const { return raw == b.raw && ext == b.ext; }
	inline bool operator!=(const board& b) const { return raw != b.raw || ext != b.ext; }
	inline bool operator==(const u64& raw) const { return this->raw == raw && this->ext == 0; }
	inline bool operator!=(const u64& raw) const { return this->raw != raw || this->ext != 0; }

	inline u32 fetch(const u32& i) const { return fetch16(i); }
	inline void place(const u32& i, const u32& r) { place16(i, r); }
	inline u32 at(const u32& i) const { return at4(i); }
	inline u32 exact(const u32& i) const { return (1 << at(i)) & 0xfffffffe; }
	inline void set(const u32& i, const u32& t) { set4(i, t); }
	inline void mirror() { mirror64(); }
	inline void flip() { flip64(); }
	inline void reflect(const bool& hori = true) { if (hori) mirror(); else flip(); }
	inline void transpose() { transpose64(); }

	inline u32 fetch16(const u32& i) const {
		return ((raw >> (i << 4)) & 0xffff);
	}
	inline u32 fetch20(const u32& i) const {
		return fetch16(i) | ((ext >> (i << 2)) & 0xf0000);
	}

	inline void place16(const u32& i, const u32& r) {
		raw = (raw & ~(0xffffULL << (i << 4))) | (u64(r & 0xffff) << (i << 4));
	}
	inline void place20(const u32& i, const u32& r) {
		place16(i, r & 0xffff);
		ext = (ext & ~(0xf0000 << (i << 2))) | ((r & 0xf0000) << (i << 2));
	}

	inline u32 at4(const u32& i) const {
		return (raw >> (i << 2)) & 0x0f;
	}
	inline u32 at5(const u32& i) const {
		return at4(i) | ((ext >> (i + 12)) & 0x10);
	}

	inline void set4(const u32& i, const u32& t) {
		raw = (raw & ~(0x0fULL << (i << 2))) | (u64(t & 0x0f) << (i << 2));
	}
	inline void set5(const u32& i, const u32& t) {
		set4(i, t);
		ext = (ext & ~(1U << (i + 16))) | ((t & 0x10) << (i + 12));
	}

	inline void mirror64() {
		raw = ((raw & 0x000f000f000f000fULL) << 12) | ((raw & 0x00f000f000f000f0ULL) << 4)
			| ((raw & 0x0f000f000f000f00ULL) >> 4) | ((raw & 0xf000f000f000f000ULL) >> 12);
	}
	inline void mirror80() {
		mirror64();
		ext = ((ext & 0x11110000) << 3) | ((ext & 0x22220000) << 1)
			| ((ext & 0x44440000) >> 1) | ((ext & 0x88880000) >> 3);
	}

	inline void flip64() {
		raw = ((raw & 0x000000000000ffffULL) << 48) | ((raw & 0x00000000ffff0000ULL) << 16)
			| ((raw & 0x0000ffff00000000ULL) >> 16) | ((raw & 0xffff000000000000ULL) >> 48);
	}
	inline void flip80() {
		flip64();
		ext = ((ext & 0x000f0000) << 12) | ((ext & 0x00f00000) << 4)
			| ((ext & 0x0f000000) >> 4) | ((ext & 0xf0000000) >> 12);
	}

	inline void transpose64() {
		raw = (raw & 0xf0f00f0ff0f00f0fULL) | ((raw & 0x0000f0f00000f0f0ULL) << 12) | ((raw & 0x0f0f00000f0f0000ULL) >> 12);
		raw = (raw & 0xff00ff0000ff00ffULL) | ((raw & 0x00000000ff00ff00ULL) << 24) | ((raw & 0x00ff00ff00000000ULL) >> 24);
	}
	inline void transpose80() {
		transpose64();
		ext = (ext & 0xa5a50000) | ((ext & 0x0a0a0000) << 3) | ((ext & 0x50500000) >> 3);
		ext = (ext & 0xcc330000) | ((ext & 0x00cc0000) << 6) | ((ext & 0x33000000) >> 6);
	}

	inline void reverse() {
		mirror();
		flip();
	}

	inline void init() {
		u32 k = std::rand();
		u32 i = (k) & 0x0f;
		u32 j = (i + (k >> 4) + 1) & 0x0f;
//		raw = (1ULL << (i << 2)) | (1ULL << (j << 2));
		u32 r = std::rand() % 100;
		raw =  (r >=  1 ? 1ULL : 2ULL) << (i << 2);
		raw |= (r >= 18 ? 1ULL : 2ULL) << (j << 2);
		ext = 0;
	}
	inline void next() {
		list empty = spaces();
		u32 p = empty[std::rand() % empty.size];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
	}
	inline list spaces() const {
		return find(0);
	}

	inline bool popup() {
		list empty = spaces();
		if (empty.size == 0) return false;
		u32 p = empty[std::rand() % empty.size];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
		return true;
	}
	inline void clear() {
		raw = 0;
		ext = 0;
	}

	inline void rotate(const int& r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotright(); break;
		case 2: reverse(); break;
		case 3: rotleft(); break;
		}
	}
	inline void rotright() {
		transpose();
		mirror();
	}
	inline void rotleft() {
		transpose();
		flip();
	}
	inline void isomorphic(const int& i = 0) {
		if ((i % 8) / 4) mirror();
		rotate(i);
	}

	inline i32 left() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query(0).left.moveh(rawn, extn, score, moved, 0);
		query(1).left.moveh(rawn, extn, score, moved, 1);
		query(2).left.moveh(rawn, extn, score, moved, 2);
		query(3).left.moveh(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 right() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query(0).right.moveh(rawn, extn, score, moved, 0);
		query(1).right.moveh(rawn, extn, score, moved, 1);
		query(2).right.moveh(rawn, extn, score, moved, 2);
		query(3).right.moveh(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 up() {
		transpose();
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query(0).left.movev(rawn, extn, score, moved, 0);
		query(1).left.movev(rawn, extn, score, moved, 1);
		query(2).left.movev(rawn, extn, score, moved, 2);
		query(3).left.movev(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 down() {
		transpose();
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query(0).right.movev(rawn, extn, score, moved, 0);
		query(1).right.movev(rawn, extn, score, moved, 1);
		query(2).right.movev(rawn, extn, score, moved, 2);
		query(3).right.movev(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}

	class optype {
	public:
		optype() = delete;
		typedef i32 oper;
		static constexpr oper up = 0;
		static constexpr oper right = 1;
		static constexpr oper down = 2;
		static constexpr oper left = 3;
		static constexpr oper illegal = -1;
	};
	inline i32 operate(const optype::oper& op) {
		switch (op) {
		case optype::up:    return up();
		case optype::right: return right();
		case optype::down:  return down();
		case optype::left:  return left();
		default:            return -1;
		}
	}
	inline i32 move(const optype::oper& op) { return operate(op); }

	inline u32 scale() const {
		return query(0).scale | query(1).scale | query(2).scale | query(3).scale;
	}
	inline u32 hash() const { return scale(); }
	inline u32 max() const {
		return math::log2(scale());
	}

	inline u32 count(const u32& t) const {
		return query(0).count[t] + query(1).count[t] + query(2).count[t] + query(3).count[t];
	}
	template<typename numa>
	inline void count(numa num, const u32& min, const u32& max) const {
		const cache::info& count0 = query(0).count;
		const cache::info& count1 = query(1).count;
		const cache::info& count2 = query(2).count;
		const cache::info& count3 = query(3).count;
		for (u32 i = min; i < max; i++) {
			num[i] = count0[i] + count1[i] + count2[i] + count3[i];
		}
	}

	inline u32 mask(const u32& t) const {
		return (query(0).mask[t] << 0) | (query(1).mask[t] << 4)
			 | (query(2).mask[t] << 8) | (query(3).mask[t] << 12);
	}
	template<typename numa>
	inline void mask(numa msk, const u32& min, const u32& max) const {
		const cache::info& mask0 = query(0).mask;
		const cache::info& mask1 = query(1).mask;
		const cache::info& mask2 = query(2).mask;
		const cache::info& mask3 = query(3).mask;
		for (u32 i = min; i < max; i++) {
			msk[i] = (mask0[i] << 0) | (mask1[i] << 4) | (mask2[i] << 8) | (mask3[i] << 12);
		}
	}

	inline u32 mono(const bool& left = true) const {
		if (left) {
			return (query(0).left.mono << 0)  | (query(1).left.mono << 6)
				 | (query(2).left.mono << 12) | (query(3).left.mono << 18);
		} else {
			return (query(0).right.mono << 0)  | (query(1).right.mono << 6)
				 | (query(2).right.mono << 12) | (query(3).right.mono << 18);
		}
	}

	inline list find(const u32& t) const {
		return board::lookup(mask(t)).pos;
	}

	inline const cache& query(const u32& r) const {
		return board::lookup(fetch(r));
	}

	inline u32 operations() const {
		board trans(*this); trans.transpose();
		u32 hori = this->query(0).legal | this->query(1).legal
				 | this->query(2).legal | this->query(3).legal;
		u32 vert = trans.query(0).legal | trans.query(1).legal
				 | trans.query(2).legal | trans.query(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}
	inline list actions() const {
		u32 o = operations();
		using moporgic::math::ones32;
		using moporgic::math::msb32;
		using moporgic::math::log2;
		u32 x = ones32(o);
		u32 a = msb32(o);
		u32 b = msb32(o & ~a);
		u32 c = msb32(o & ~a & ~b);
		u32 d = msb32(o & ~a & ~b & ~c);
		u32 k = (log2(a) << 4*(x-1)) | (log2(b) << 4*(x-2)) | (log2(c) << 4*(x-3)) | (log2(d) << 4*(x-4));
		return list(k, x);
	}

	inline bool operable() const {
		if (this->query(0).moved == 0) return true;
		if (this->query(1).moved == 0) return true;
		if (this->query(2).moved == 0) return true;
		if (this->query(3).moved == 0) return true;
		board trans(*this); trans.transpose();
		if (trans.query(0).moved == 0) return true;
		if (trans.query(1).moved == 0) return true;
		if (trans.query(2).moved == 0) return true;
		if (trans.query(3).moved == 0) return true;
		return false;
	}
	inline bool movable() const { return operable(); }

	class tile {
	friend class board;
	public:
		board& b;
		const u32 i;
	private:
		inline tile(const board& b, const u32& i) : b(const_cast<board&>(b)), i(i) {}
	public:
		inline tile(const tile& t) = default;
		tile() = delete;
		inline operator u32() const { return b.at(i); }
		inline tile& operator =(const u32& k) { b.set(i, k); return *this; }
		inline tile& operator =(const tile& t) { return operator =(u32(t)); }
		inline bool operator ==(const u32& k) const { return b.at(i) == k; }
		inline bool operator !=(const u32& k) const { return b.at(i) != k; }
	};
	inline tile operator [](const u32& i) const { return tile(*this, i); }

	inline void operator >>(std::ostream& out) const { write(out); }
	inline void operator <<(std::istream& in) { read(in); }

	inline void write(std::ostream& out) const { write64(out); }
	inline void read(std::istream& in) { read64(in); }

	inline void write64(std::ostream& out) const {
		moporgic::write(out, raw);
	}
	inline void write80(std::ostream& out) const {
		write64(out);
		moporgic::write(out, raw_cast<u16>(ext, moporgic::endian::be));
	}

	inline void read64(std::istream& in) {
		moporgic::read(in, raw);
	}
	inline void read80(std::istream& in) {
		read64(in);
		moporgic::read(in, raw_cast<u16>(ext, moporgic::endian::be));
	}

	inline void print(const bool& raw = true, std::ostream& out = std::cout) const {
		u32 delim = raw ? 4 : 6;
		char edge[32], line[32], buff[32];
		snprintf(edge, sizeof(edge), "+%.*s+", delim * 4, "------------------------");
		snprintf(line, sizeof(line), "|%%%uu%%%uu%%%uu%%%uu|", delim, delim, delim, delim);
		out << edge << std::endl;
		for (u32 i = 0, t[4]; i < 16; i += 4) {
			for (u32 j = 0; j < 4; j++) t[j] = raw ? at(i+j) : exact(i+j);
			snprintf(buff, sizeof(buff), line, t[0], t[1], t[2], t[3]);
			out << buff << std::endl;
		}
		out << edge << std::endl;
	}

};

} // namespace moporgic
