//============================================================================
// Name        : moporgic/TDL2048+ - board.h
// Author      : Hung Guei @ moporgic
// Version     : 6.3
// Description : The Most Effective Bitboard Implementation for 2048
//============================================================================

#pragma once
#include "moporgic/type.h"
#include "moporgic/util.h"
#include "moporgic/math.h"

namespace moporgic {

class board {
private:
	u64 raw;
	u32 ext;
	u32 inf;

public:
	inline constexpr board(u64 raw = 0, u32 ext = 0, u32 inf = 0) : raw(raw), ext(ext), inf(inf) {}
	inline constexpr board(u64 raw, u16 ext) : board(raw, u32(ext) << 16) {}
	inline constexpr board(const board& b) = default;
	inline constexpr board& operator =(u64 x) { raw = x; ext = 0; return *this; }
	inline constexpr board& operator =(const board& b) = default;

	inline constexpr operator u64&() { return raw; }
	inline constexpr operator u64() const { return raw; }
	declare_comparators_with(const board&, raw_cast<u128>(*this), raw_cast<u128>(v), inline constexpr)

	inline constexpr void set(u64 x) { raw = x; }
	inline constexpr void set(u64 x, u16 e) { raw = x; ext = e << 16; }
	inline constexpr void set(const board& b) { raw = b.raw; ext = b.ext; }

public:
	class cache {
	public:
		static inline const cache& load(u32 i) {
			static byte block[sizeof(cache) * (1 << 20)] = {};
			return pointer_cast<cache>(block)[i];
		}
		static __attribute__((constructor)) void make() {
			if (load(0).moved) return;
			for (u32 i = 0; i < (1 << 20); i++) new (const_cast<board::cache*>(&load(i))) board::cache(i);
		}

	public:
		cache(const cache& c) = default;
		cache() : raw(0), ext(0), species(0), merge(0), moved(-1), legal(0) {}
		cache(u32 r) : raw(r & 0x0ffff), ext(r & 0xf0000), species(0), merge(0), left(r, false), right(r, true), moved(-1), legal(0) {
			for (int i = 0; i < 4; i++) {
				u32 t = ((r >> (i << 2)) & 0x0f) | ((r >> (12 + i)) & 0x10);
				species |= (1 << t);
				numof[t] += 1;
				mask[t] |= (1 << i);
			}
			for (int i = 0; i < 16; i++) {
				if ((r >> i) & 1) layout.push_back(i);
			}
			merge = left.merge | right.merge;
			moved = left.moved & right.moved;
			if (left.moved == 0)  legal |= (0x08 | 0x01);
			if (right.moved == 0) legal |= (0x02 | 0x04);
		}

	public:
		class move {
		public:
			template<int i> inline void moveh64(board& mv) const {
				mv.raw |= u64(rawh) << (i << 4);
				mv.inf += score;
			}
			template<int i> inline void moveh80(board& mv) const {
				moveh64<i>(mv);
				mv.ext |= exth << (i << 2);
			}
			template<int i> inline void movev64(board& mv) const {
				mv.raw |= rawv << (i << 2);
				mv.inf += score;
			}
			template<int i> inline void movev80(board& mv) const {
				movev64<i>(mv);
				mv.ext |= extv << i;
			}

		public:
			move(const move& op) = default;
			move() : rawh(0), exth(0), rawv(0), extv(0), score(0), moved(-1), merge(0), mono(0) {}
			move(u32 r, bool reverse) : rawh(0), exth(0), rawv(0), extv(0), score(0), moved(-1), merge(0), mono(0) {
				u32 row[] = {((r >> 0) & 0x0f) | ((r >> 12) & 0x10), ((r >> 4) & 0x0f) | ((r >> 13) & 0x10),
							((r >> 8) & 0x0f) | ((r >> 14) & 0x10), ((r >> 12) & 0x0f) | ((r >> 15) & 0x10)};
				if (reverse) std::reverse(row, row + 4);
				u32 top = 0, hold = 0;
				for (u32 i = 0; i < 4; i++) {
					u32 tile = row[i];
					if (tile == 0) continue;
					row[i] = 0;
					if (hold) {
						if (tile == hold) {
							row[top++] = ++tile;
							score += (1 << tile);
							merge++;
							hold = 0;
						} else {
							row[top++] = hold;
							hold = tile;
						}
					} else {
						hold = tile;
					}
				}
				if (hold) row[top] = hold;
				if (reverse) std::reverse(row, row + 4);

				u32 lo[] = { (row[0] & 0x0f), (row[1] & 0x0f), (row[2] & 0x0f), (row[3] & 0x0f) };
				u32 hi[] = { (row[0] & 0x10) >> 4, (row[1] & 0x10) >> 4, (row[2] & 0x10) >> 4, (row[3] & 0x10) >> 4 };
				rawh = ((lo[0] << 0) | (lo[1] << 4) | (lo[2] << 8) | (lo[3] << 12));
				exth = ((hi[0] << 0) | (hi[1] << 1) | (hi[2] << 2) | (hi[3] << 3)) << 16;
				rawv = (u64(lo[0]) << 0) | (u64(lo[1]) << 16) | (u64(lo[2]) << 32) | (u64(lo[3]) << 48);
				extv = ((hi[0] << 0) | (hi[1] << 4) | (hi[2] << 8) | (hi[3] << 12)) << 16;
				moved = ((rawh | exth) == r) ? -1 : 0;

				const u32 monores[6][2] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 }, };
				for (u32 i = 0; i < 6; i++) {
					u32 a = row[monores[i][0]], b = row[monores[i][1]];
					mono |= ((a == b ? (a ? 0b11 : 0b00) : (a > b ? 0b01 : 0b10)) << (i << 1));
				}
			}

		public:
			u32 rawh; // horizontal move (16-bit raw)
			u32 exth; // horizontal move (4-bit extra)
			u64 rawv; // vertical move (64-bit raw)
			u32 extv; // vertical move (16-bit extra)
			u32 score; // merge score (reward)
			i32 moved; // moved or not (moved: 0, otherwise -1)
			u16 merge; // number of merged tiles
			u16 mono; // monotonic decreasing value (12-bit)
		};

		template<int i> inline void moveh64(board& L, board& R) const {
			left.moveh64<i>(L);
			right.moveh64<i>(R);
		}
		template<int i> inline void moveh80(board& L, board& R) const {
			left.moveh80<i>(L);
			right.moveh80<i>(R);
		}
		template<int i> inline void movev64(board& U, board& D) const {
			left.movev64<i>(U);
			right.movev64<i>(D);
		}
		template<int i> inline void movev80(board& U, board& D) const {
			left.movev80<i>(U);
			right.movev80<i>(D);
		}

	public:
		u32 raw; // base row (16-bit raw)
		u32 ext; // base row (4-bit extra)
		u32 species; // species of this row
		u32 merge; // number of merged tiles
		move left; // left operation
		move right; // right operation
		hexa numof; // number of each tile-type
		hexa mask; // mask of each tile-type
		hexa layout; // layout of board-type
		i32 moved; // moved or not
		u32 legal; // legal actions
	};

	inline const cache& qrow(u32 i) const { return qrow16(i); }
	inline const cache& qrow16(u32 i) const { return cache::load(row16(i)); }
	inline const cache& qrow20(u32 i) const { return cache::load(row20(i)); }

	inline const cache& qcol(u32 i) const { return qcol16(i); }
	inline const cache& qcol16(u32 i) const { return cache::load(col16(i)); }
	inline const cache& qcol20(u32 i) const { return cache::load(col20(i)); }

	inline constexpr u32 row(u32 i) const { return row16(i); }
	inline constexpr u32 row16(u32 i) const {
		return ((raw >> (i << 4)) & 0xffff);
	}
	inline constexpr u32 row20(u32 i) const {
		return row16(i) | ((ext >> (i << 2)) & 0xf0000);
	}
	inline constexpr void row(u32 i, u32 r) { row16(i, r); }
	inline constexpr void row16(u32 i, u32 r) {
		raw = (raw & ~(0xffffull << (i << 4))) | (u64(r) << (i << 4));
	}
	inline constexpr void row20(u32 i, u32 r) {
		row16(i, r & 0xffff);
		ext = (ext & ~(0xf0000 << (i << 2))) | ((r & 0xf0000) << (i << 2));
	}

	inline constexpr u32 col(u32 i) const { return col16(i); }
	inline constexpr u32 col16(u32 i) const {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_COL)
		return math::pext64(raw, 0x000f000f000f000full << (i << 2));
#else
		return ((raw >> (i << 2)) & 0x000f000f000f000full) * 0x0001001001001000ull >> 48;
#endif
	}
	inline constexpr u32 col20(u32 i) const {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_COL)
		return col16(i) | (math::pext32(ext, 0x11110000u << i) << 16);
#else
		return col16(i) | (((((ext >> i) & 0x11110000u) * 0x1248u) >> 12) & 0xf0000u);
#endif
	}
	inline constexpr void col(u32 i, u32 c) { col16(i, c); }
	inline constexpr void col16(u32 i, u32 c) {
		u64 m = 0x000f000f000f000full << (i << 2);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_COL)
		raw = (raw & ~m) | math::pdep64(c, m);
#else
		u64 x = u64(c) | (u64(c) << 12) | (u64(c) << 24) | (u64(c) << 36);
		raw = (raw & ~m) | ((x & 0x000f000f000f000full) << (i << 2));
#endif
	}
	inline constexpr void col20(u32 i, u32 c) {
		col16(i, c & 0xffff);
		u32 m = 0x11110000u << i;
#if defined(__BMI2__) && !defined(PREFER_LEGACY_COL)
		ext = (ext & ~m) | math::pdep32(c >> 16, m);
#else
		u32 e = c & 0xf0000u;
		u32 x = e | (e << 3) | (e << 6) | (e << 9);
		ext = (ext & ~m) | ((x & 0x11110000u) << i);
#endif
	}

	inline constexpr u32 at(u32 i) const { return at4(i); }
	inline constexpr u32 at4(u32 i) const {
		return (raw >> (i << 2)) & 0x0f;
	}
	inline constexpr u32 at5(u32 i) const {
		return at4(i) | ((ext >> (i + 12)) & 0x10);
	}
	inline constexpr void at(u32 i, u32 t) { at4(i, t); }
	inline constexpr void at4(u32 i, u32 t) {
		raw = (raw & ~(0x0full << (i << 2))) | (u64(t) << (i << 2));
	}
	inline constexpr void at5(u32 i, u32 t) {
		at4(i, t & 0x0f);
		ext = (ext & ~(1U << (i + 16))) | ((t & 0x10) << (i + 12));
	}

	inline constexpr void put(u64 where, u32 t) { return put64(where, t); }
	inline constexpr void put64(u64 where, u32 t) {
		raw = (raw & ~(where * 0x0full)) | (where * t);
	}
	inline constexpr void put80(u64 where, u32 t) {
		put64(where, t & 0x0f);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_PUT)
		u32 w = math::pext64(where, 0x1111111111111111ull) << 16;
#else
		where |= where >> 3;
		where |= where >> 6;
		where &= 0x000f000f000f000full;
		where |= where >> 12;
		where |= where >> 24;
		u32 w = (where & 0xffffu) << 16;
#endif
		ext = (ext ^ (ext & w)) | (t & 0x10 ? w : 0);
	}
	inline constexpr void put(u16 mask, u32 t) { return put64(mask, t); }
	inline constexpr void put64(u16 mask, u32 t) {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_PUT)
		u64 where = math::pdep64(mask, 0x1111111111111111ull);
#else
		u64 where = mask;
		where |= where << 24;
		where |= where << 12;
		where &= 0x000f000f000f000full;
		where |= where << 6;
		where |= where << 3;
		where &= 0x1111111111111111ull;
#endif
		return put64(where, t);
	}
	inline constexpr void put80(u16 mask, u32 t) {
		put64(mask, t & 0x0f);
		u32 w = mask << 16;
		ext = (ext ^ (ext & w)) | (t & 0x10 ? w : 0);
	}

	inline constexpr void mirror() { mirror64(); }
	inline constexpr void mirror64() {
		raw = ((raw & 0x000f000f000f000full) << 12) | ((raw & 0x00f000f000f000f0ull) << 4)
			| ((raw & 0x0f000f000f000f00ull) >> 4) | ((raw & 0xf000f000f000f000ull) >> 12);
	}
	inline constexpr void mirror80() {
		mirror64();
		ext = ((ext & 0x11110000) << 3) | ((ext & 0x22220000) << 1)
			| ((ext & 0x44440000) >> 1) | ((ext & 0x88880000) >> 3);
	}

	inline constexpr void flip() { flip64(); }
	inline constexpr void flip64() {
		u64 buf = (raw ^ math::rol64(raw, 16)) & 0x0000ffff0000ffffull;
		raw ^= (buf | math::ror64(buf, 16));
	}
	inline constexpr void flip80() {
		flip64();
		u32 buf = ((ext >> 16) ^ math::rol16(ext >> 16, 4)) & 0x0f0fu;
		ext ^= (buf | math::ror16(buf, 4)) << 16;
	}

	inline constexpr void transpose() { transpose64(); }
	inline constexpr void transpose64() {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_TRANSPOSE)
		raw = (math::pext64(raw, 0x000f000f000f000full) <<  0) | (math::pext64(raw, 0x00f000f000f000f0ull) << 16)
		    | (math::pext64(raw, 0x0f000f000f000f00ull) << 32) | (math::pext64(raw, 0xf000f000f000f000ull) << 48);
#else
		register u64 buf = 0;
		buf = (raw ^ (raw >> 12)) & 0x0000f0f00000f0f0ull;
		raw ^= buf ^ (buf << 12);
		buf = (raw ^ (raw >> 24)) & 0x00000000ff00ff00ull;
		raw ^= buf ^ (buf << 24);
#endif
	}
	inline constexpr void transpose80() {
		transpose64();
#if defined(__BMI2__) && !defined(PREFER_LEGACY_TRANSPOSE)
		ext = (math::pext32(ext, 0x11110000u) << 16) | (math::pext32(ext, 0x22220000u) << 20)
			| (math::pext32(ext, 0x44440000u) << 24) | (math::pext32(ext, 0x88880000u) << 28);
#else
		register u32 buf = 0;
		buf = (ext ^ (ext >> 3)) & 0x0a0a0000;
		ext ^= buf ^ (buf << 3);
		buf = (ext ^ (ext >> 6)) & 0x00cc0000;
		ext ^= buf ^ (buf << 6);
#endif
	}

	inline constexpr void rotate(u32 r = 1) { rotate64(r); }
	inline constexpr void rotate64(u32 r = 1) {
		switch (r % 4) {
		case 0: break;
		case 1: flip64(); transpose64(); break;
		case 2: mirror64();    flip64(); break;
		case 3: transpose64(); flip64(); break;
		}
	}
	inline constexpr void rotate80(u32 r = 1) {
		switch (r % 4) {
		case 0: break;
		case 1: flip80(); transpose80(); break;
		case 2: mirror80();    flip80(); break;
		case 3: transpose80(); flip80(); break;
		}
	}

	inline constexpr void isom(u32 i = 0) { return isom64(i); }
	inline constexpr void isom64(u32 i = 0) {
		if (i & 4) flip64();
		rotate64(i);
	}
	inline constexpr void isom80(u32 i = 0) {
		if (i & 4) flip80();
		rotate80(i);
	}

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr void isoms(btype iso[]) const { return isoms64(iso); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr void isoms64(btype iso[]) const {
		iso[5] = *this;       iso[0] = iso[5];
		iso[5].flip64();      iso[4] = iso[5];
		iso[5].transpose64(); iso[1] = iso[5];
		iso[5].flip64();      iso[7] = iso[5];
		iso[5].transpose64(); iso[2] = iso[5];
		iso[5].flip64();      iso[6] = iso[5];
		iso[5].transpose64(); iso[3] = iso[5];
		iso[5].flip64();
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr void isoms80(btype iso[]) const {
		iso[5] = *this;       iso[0] = iso[5];
		iso[5].flip80();      iso[4] = iso[5];
		iso[5].transpose80(); iso[1] = iso[5];
		iso[5].flip80();      iso[7] = iso[5];
		iso[5].transpose80(); iso[2] = iso[5];
		iso[5].flip80();      iso[6] = iso[5];
		iso[5].transpose80(); iso[3] = iso[5];
		iso[5].flip80();
	}

	inline std::array<board, 8> isoms() const { return isoms64(); }
	inline std::array<board, 8> isoms64() const { std::array<board, 8> iso; isoms64(iso.data()); return iso; }
	inline std::array<board, 8> isoms80() const { std::array<board, 8> iso; isoms80(iso.data()); return iso; }

	inline constexpr u32 empty() const { return empty64(); }
	inline constexpr u32 empty64() const { return count64(0); }
	inline constexpr u32 empty80() const { return count80(0); }

	inline hexa spaces() const { return spaces64(); }
	inline hexa spaces64() const { return find64(0); }
	inline hexa spaces80() const { return find80(0); }

	inline void init() {
		u32 u = moporgic::rand();
		u32 k = (u & 0xffff);
		u32 i = (k) % 16;
		u32 j = (i + 1 + (k >> 4) % 15) % 16;
		u32 r = (u >> 16) % 100;
		raw = ((r >=  1 ? 1ull : 2ull) << (i << 2))
		    | ((r >= 19 ? 1ull : 2ull) << (j << 2));
		ext = 0;
	}

	inline void next() { return next64(); }
	inline void next64() {
		u64 x = where64(0);
		u32 e = math::popcnt64(x);
		u32 u = moporgic::rand();
#if defined(__BMI2__) && !defined(PREFER_LEGACY_NEXT)
		u64 t = math::nthset64(x, (u >> 16) % e);
#else
		u32 k = (u >> 16) % e;
		while (k--) x &= x - 1;
		u64 t = x & -x;
#endif
		raw |= (t << (u % 10 ? 0 : 1));
	}
	inline void next80() {
		u64 x = where80(0);
		u32 e = math::popcnt64(x);
		u32 u = moporgic::rand();
#if defined(__BMI2__) && !defined(PREFER_LEGACY_NEXT)
		u64 t = math::nthset64(x, (u >> 16) % e);
#else
		u32 k = (u >> 16) % e;
		while (k--) x &= x - 1;
		u64 t = x & -x;
#endif
		raw |= (t << (u % 10 ? 0 : 1));
	}

	inline bool popup() { return popup64(); }
	inline bool popup64() { return empty64() ? next64(), true : false; }
	inline bool popup80() { return empty80() ? next80(), true : false; }

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 popups(btype popup[]) const { return popups64(popup); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 popups64(btype popup[]) const {
		u32 i = 0;
		u64 x = where64(0);
		for (u64 t; (t = x & -x) != 0; x ^= t) {
			popup[i++] = board(raw | (t << 0), ext);
			popup[i++] = board(raw | (t << 1), ext);
		}
		return i;
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 popups80(btype popup[]) const {
		u32 i = 0;
		u64 x = where80(0);
		for (u64 t; (t = x & -x) != 0; x ^= t) {
			popup[i++] = board(raw | (t << 0), ext);
			popup[i++] = board(raw | (t << 1), ext);
		}
		return i;
	}

	inline std::vector<board> popups() const   { std::vector<board> popup(empty() * 2); popups(popup.data()); return popup; }
	inline std::vector<board> popups64() const { std::vector<board> popup(empty64() * 2); popups64(popup.data()); return popup; }
	inline std::vector<board> popups80() const { std::vector<board> popup(empty80() * 2); popups80(popup.data()); return popup; }

	inline i32 left()  { return left64(); }
	inline i32 right() { return right64(); }
	inline i32 up()    { return up64(); }
	inline i32 down()  { return down64(); }

	inline i32 left64() {
		board move;
		qrow16(0).left.moveh64<0>(move);
		qrow16(1).left.moveh64<1>(move);
		qrow16(2).left.moveh64<2>(move);
		qrow16(3).left.moveh64<3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 right64() {
		board move;
		qrow16(0).right.moveh64<0>(move);
		qrow16(1).right.moveh64<1>(move);
		qrow16(2).right.moveh64<2>(move);
		qrow16(3).right.moveh64<3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 up64() {
		board move;
		qcol16(0).left.movev64<0>(move);
		qcol16(1).left.movev64<1>(move);
		qcol16(2).left.movev64<2>(move);
		qcol16(3).left.movev64<3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 down64() {
		board move;
		qcol16(0).right.movev64<0>(move);
		qcol16(1).right.movev64<1>(move);
		qcol16(2).right.movev64<2>(move);
		qcol16(3).right.movev64<3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}

	inline i32 left80() {
		board move;
		qrow20(0).left.moveh80<0>(move);
		qrow20(1).left.moveh80<1>(move);
		qrow20(2).left.moveh80<2>(move);
		qrow20(3).left.moveh80<3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 right80() {
		board move;
		qrow20(0).right.moveh80<0>(move);
		qrow20(1).right.moveh80<1>(move);
		qrow20(2).right.moveh80<2>(move);
		qrow20(3).right.moveh80<3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 up80() {
		board move;
		qcol20(0).left.movev80<0>(move);
		qcol20(1).left.movev80<1>(move);
		qcol20(2).left.movev80<2>(move);
		qcol20(3).left.movev80<3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 down80() {
		board move;
		qcol20(0).right.movev80<0>(move);
		qcol20(1).right.movev80<1>(move);
		qcol20(2).right.movev80<2>(move);
		qcol20(3).right.movev80<3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}

	inline i32 move(u32 op)   {
		switch (op & 0xf0u) {
		default:            return move64(op);
		case action::x64:   return move64(op & 0x0fu);
		case action::x80:   return move80(op & 0x0fu);
		}
	}
	inline i32 move64(u32 op) {
		switch (op) {
		case action::up:    return up64();
		case action::right: return right64();
		case action::down:  return down64();
		case action::left:  return left64();
		default:            return -1;
		}
	}
	inline i32 move80(u32 op) {
		switch (op) {
		case action::up:    return up80();
		case action::right: return right80();
		case action::down:  return down80();
		case action::left:  return left80();
		default:            return -1;
		}
	}

	inline void moves(board& U, board& R, board& D, board& L) const {
		return moves64(U, R, D, L);
	}
	inline void moves64(board& U, board& R, board& D, board& L) const {
#if defined(__AVX2__) && !defined(PREFER_LUT_MOVES)
		__m256i dst, buf, rbf, rwd, chk;

		// use left for all 4 directions, transpose and mirror first
		u64 x = raw;
		raw_cast<board>(x).transpose64();
		dst = _mm256_set_epi64x(raw, 0, 0, x); // L, 0, 0, U
		buf = _mm256_set_epi64x(0, x, raw, 0); // 0, D, R, 0
		dst = _mm256_or_si256(dst, _mm256_slli_epi16(buf, 12));
		dst = _mm256_or_si256(dst, _mm256_slli_epi16(_mm256_and_si256(buf, _mm256_set1_epi16(0x00f0)), 4));
		dst = _mm256_or_si256(dst, _mm256_srli_epi16(_mm256_and_si256(buf, _mm256_set1_epi16(0x0f00)), 4));
		dst = _mm256_or_si256(dst, _mm256_srli_epi16(buf, 12));

		// slide to left most
		buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x0f00));
		chk = _mm256_and_si256(_mm256_cmpeq_epi16(buf, _mm256_setzero_si256()), _mm256_set1_epi16(0xff00));
		dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));
		buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x00f0));
		chk = _mm256_and_si256(_mm256_cmpeq_epi16(buf, _mm256_setzero_si256()), _mm256_set1_epi16(0xfff0));
		dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));
		buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x000f));
		chk = _mm256_cmpeq_epi16(buf, _mm256_setzero_si256());
		dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));

		// merge same tiles, slide if necessary
		buf = _mm256_srli_epi16(_mm256_add_epi8(dst, _mm256_set1_epi16(0x0010)), 4);
		rbf = _mm256_and_si256(dst, _mm256_set1_epi16(0x000f));
		chk = _mm256_and_si256(_mm256_srli_epi16(dst, 4), _mm256_set1_epi16(0x000f));
		chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
		dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
		chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_and_si256(chk, _mm256_set1_epi16(0x0001)), 0));
		rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
		rwd = _mm256_sllv_epi32(chk, rbf);

		buf = _mm256_add_epi8(_mm256_srli_epi16(dst, 4), _mm256_set1_epi16(0x0010));
		rbf = _mm256_and_si256(buf, _mm256_set1_epi16(0x000f));
		chk = _mm256_and_si256(_mm256_srli_epi16(dst, 8), _mm256_set1_epi16(0x000f));
		chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
		chk = _mm256_and_si256(chk, _mm256_set1_epi16(0xfff0));
		dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
		chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_srli_epi16(chk, 15), 0));
		rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
		rwd = _mm256_add_epi32(rwd, _mm256_sllv_epi32(chk, rbf));

		buf = _mm256_srli_epi16(_mm256_add_epi16(dst, _mm256_set1_epi16(0x1000)), 4);
		rbf = _mm256_srli_epi16(dst, 12);
		chk = _mm256_and_si256(_mm256_srli_epi16(dst, 8), _mm256_set1_epi16(0x000f));
		chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
		chk = _mm256_and_si256(chk, _mm256_set1_epi16(0xff00));
		dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
		chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_srli_epi16(chk, 15), 0));
		rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
		rwd = _mm256_add_epi32(rwd, _mm256_sllv_epi32(chk, rbf));

		// mirror and transpose back to original direction
		L = _mm256_extract_epi64(dst, 3); // L = left
		buf = _mm256_slli_epi16(dst, 12);
		buf = _mm256_or_si256(buf, _mm256_slli_epi16(_mm256_and_si256(dst, _mm256_set1_epi16(0x00f0)), 4));
		buf = _mm256_or_si256(buf, _mm256_srli_epi16(_mm256_and_si256(dst, _mm256_set1_epi16(0x0f00)), 4));
		buf = _mm256_or_si256(buf, _mm256_srli_epi16(dst, 12));
		R = _mm256_extract_epi64(buf, 1); // R = mirror left mirror

		buf = _mm256_blend_epi32(dst, buf, 0b11111100);
		rbf = _mm256_and_si256(_mm256_xor_si256(buf, _mm256_srli_epi64(buf, 12)), _mm256_set1_epi64x(0x0000f0f00000f0f0ull));
		buf = _mm256_xor_si256(buf, _mm256_xor_si256(rbf, _mm256_slli_epi64(rbf, 12)));
		rbf = _mm256_and_si256(_mm256_xor_si256(buf, _mm256_srli_epi64(buf, 24)), _mm256_set1_epi64x(0x00000000ff00ff00ull));
		buf = _mm256_xor_si256(buf, _mm256_xor_si256(rbf, _mm256_slli_epi64(rbf, 24)));
		U = _mm256_extract_epi64(buf, 0); // U = transpose left transpose
		D = _mm256_extract_epi64(buf, 2); // D = transpose mirror left mirror transpose

		// sum the final reward and check moved or not
		rwd = _mm256_add_epi64(rwd, _mm256_srli_si256(rwd, 8 /* bytes */));
		rwd = _mm256_add_epi64(rwd, _mm256_srli_epi64(rwd, 32));
		U.inf = _mm256_extract_epi32(rwd, 0);
		R.inf = _mm256_extract_epi32(rwd, 4);
		rwd = _mm256_set_epi64x(R.inf, U.inf, R.inf, U.inf);
		chk = _mm256_cmpeq_epi64(_mm256_set_epi64x(L, D, R, U), _mm256_set1_epi64x(raw));
		rwd = _mm256_or_si256(rwd, chk);
		U.inf = _mm256_extract_epi32(rwd, 0);
		R.inf = _mm256_extract_epi32(rwd, 2);
		D.inf = _mm256_extract_epi32(rwd, 4);
		L.inf = _mm256_extract_epi32(rwd, 6);

#else // if AVX2 is unavailable or disabled
		U = R = D = L = board();

		qrow16(0).moveh64<0>(L, R);
		qrow16(1).moveh64<1>(L, R);
		qrow16(2).moveh64<2>(L, R);
		qrow16(3).moveh64<3>(L, R);
		L.inf |= (L.raw ^ raw) ? 0 : -1;
		R.inf |= (R.raw ^ raw) ? 0 : -1;

		qcol16(0).movev64<0>(U, D);
		qcol16(1).movev64<1>(U, D);
		qcol16(2).movev64<2>(U, D);
		qcol16(3).movev64<3>(U, D);
		U.inf |= (U.raw ^ raw) ? 0 : -1;
		D.inf |= (D.raw ^ raw) ? 0 : -1;
#endif
	}
	inline void moves80(board& U, board& R, board& D, board& L) const {
		U = R = D = L = board();

		qrow20(0).moveh80<0>(L, R);
		qrow20(1).moveh80<1>(L, R);
		qrow20(2).moveh80<2>(L, R);
		qrow20(3).moveh80<3>(L, R);
		L.inf |= (L.raw ^ raw) | (L.ext ^ ext) ? 0 : -1;
		R.inf |= (R.raw ^ raw) | (R.ext ^ ext) ? 0 : -1;

		qcol20(0).movev80<0>(U, D);
		qcol20(1).movev80<1>(U, D);
		qcol20(2).movev80<2>(U, D);
		qcol20(3).movev80<3>(U, D);
		U.inf |= (U.raw ^ raw) | (U.ext ^ ext) ? 0 : -1;
		D.inf |= (D.raw ^ raw) | (D.ext ^ ext) ? 0 : -1;
	}

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline void moves(btype move[]) const   { moves(move[0], move[1], move[2], move[3]); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline void moves64(btype move[]) const { moves64(move[0], move[1], move[2], move[3]); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline void moves80(btype move[]) const { moves80(move[0], move[1], move[2], move[3]); }

	inline std::array<board, 4> moves() const   { std::array<board, 4> move; moves(move.data()); return move; }
	inline std::array<board, 4> moves64() const { std::array<board, 4> move; moves64(move.data()); return move; }
	inline std::array<board, 4> moves80() const { std::array<board, 4> move; moves80(move.data()); return move; }

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 movals(btype move[]) const { return movals64(move); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 movals64(btype move[]) const {
		moves64(move);
		u32 i = 0;
		move[i] = move[0]; i += ~move[0].inf >> 31;
		move[i] = move[1]; i += ~move[1].inf >> 31;
		move[i] = move[2]; i += ~move[2].inf >> 31;
		move[i] = move[3]; i += ~move[3].inf >> 31;
		return i;
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline u32 movals80(btype move[]) const {
		moves80(move);
		u32 i = 0;
		move[i] = move[0]; i += ~move[0].inf >> 31;
		move[i] = move[1]; i += ~move[1].inf >> 31;
		move[i] = move[2]; i += ~move[2].inf >> 31;
		move[i] = move[3]; i += ~move[3].inf >> 31;
		return i;
	}

	inline std::vector<board> movals() const   { std::vector<board> move(4); move.resize(movals(move.data())); return move; }
	inline std::vector<board> movals64() const { std::vector<board> move(4); move.resize(movals64(move.data())); return move; }
	inline std::vector<board> movals80() const { std::vector<board> move(4); move.resize(movals80(move.data())); return move; }

	class action {
	public:
		action() = delete;
		enum opcode : u32 {
			up    = 0x00u,
			right = 0x01u,
			down  = 0x02u,
			left  = 0x03u,
			next  = 0x0eu,
			init  = 0x0cu,
			nop   = 0x0fu,

			x64   = 0x40u,
			x80   = 0x80u,
			ext   = x80,
		};
	};

	inline i32 operate(u32 op) {
		switch (op & 0xf0u) {
		default:           return operate64(op);
		case action::x64:  return operate64(op & 0x0fu);
		case action::x80:  return operate80(op & 0x0fu);
		}
	}
	inline i32 operate64(u32 op) {
		switch (op) {
		default:           return move64(op);
		case action::next: return popup64() ? 0 : -1;
		case action::init: return init(), 0;
		}
	}
	inline i32 operate80(u32 op) {
		switch (op) {
		default:           return move80(op);
		case action::next: return popup80() ? 0 : -1;
		case action::init: return init(), 0;
		}
	}

	inline constexpr u32 shift(u32 n = 1, u32 u = 0) { return shift64(n, u); }
	inline constexpr u32 shift64(u32 n = 1, u32 u = 0) {
		u32 rank = scale64();
		u32 mask = (math::msb(rank) - 1) & ~((2 << u) - 1);
		u32 hole = ~rank & mask;
		u32 hcnt = std::max(math::popcnt(hole), n);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_SHIFT)
		hole &= ~(math::nthset(hole, hcnt - n) - 1);
#else
		while (hcnt-- > n) hole &= (hole - 1);
#endif
		rank &= ~(math::lsb(hole) - 1);
		for (u32 last = 0, tile = 0, step = 0; rank; rank ^= last) {
			last = math::lsb(rank);
			tile = math::tzcnt(last);
			step = math::popcnt(hole & (last - 1));
			put64(where64(tile), tile - step);
		}
		return hole;
	}
	inline constexpr u32 shift80(u32 n = 1, u32 u = 0) {
		u32 rank = scale80();
		u32 mask = (math::msb(rank) - 1) & ~((2 << u) - 1);
		u32 hole = ~rank & mask;
		u32 hcnt = std::max(math::popcnt(hole), n);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_SHIFT)
		hole &= ~(math::nthset(hole, hcnt - n) - 1);
#else
		while (hcnt-- > n) hole &= (hole - 1);
#endif
		rank &= ~(math::lsb(hole) - 1);
		for (u32 last = 0, tile = 0, step = 0; rank; rank ^= last) {
			last = math::lsb(rank);
			tile = math::tzcnt(last);
			step = math::popcnt(hole & (last - 1));
			put80(where80(tile), tile - step);
		}
		return hole;
	}

	inline constexpr u32 advance(u32 n = 1, u32 u = 0) { return advance64(n, u); }
	inline constexpr u32 advance64(u32 n = 1, u32 u = 0) {
		u32 rank = scale64();
		u32 mask = (math::msb(rank) - 1) & ~((2 << u) - 1);
		u32 hole = ~rank & mask;
		u32 hmax = math::msb(hole);
		hole = ((hmax << n) - 1) & ~(hmax - 1);
		rank &= ~(math::lsb(hole) - 1);
		for (u32 last = 0, tile = 0; rank; rank ^= last) {
			last = math::msb(rank);
			tile = math::tzcnt(last);
			put64(where64(tile), (tile + n) & 0x0fu);
		}
		return hole;
	}
	inline constexpr u32 advance80(u32 n = 1, u32 u = 0) {
		u32 rank = scale80();
		u32 mask = (math::msb(rank) - 1) & ~((2 << u) - 1);
		u32 hole = ~rank & mask;
		u32 hmax = math::msb(hole);
		hole = ((hmax << n) - 1) & ~(hmax - 1);
		rank &= ~(math::lsb(hole) - 1);
		for (u32 last = 0, tile = 0; rank; rank ^= last) {
			last = math::msb(rank);
			tile = math::tzcnt(last);
			put80(where80(tile), (tile + n) & 0x1fu);
		}
		return hole;
	}

	inline constexpr u32 scale(u32 scale, u32 thres = 0) { return scale64(scale, thres); }
	inline constexpr u32 scale64(u32 scale, u32 thres = 0) {
		u32 rank = scale64();
		i32 dist = math::lg(rank) - math::lg(scale);
		while (dist-- > 0 && shift64()) rank = scale64();
		u32 mask = ~(math::msb(thres | 1) - 1);
		u32 diff = (scale ^ rank) & mask;
		for (u32 u = 0; diff != 0; diff ^= u) {
			u = math::msb(diff);
			if (scale & u) { // if u is expected but not present
				u32 t = math::msb(rank & (u - 1));
				u64 w = t ? where64(math::tzcnt(t)) : 0;
				u32 n = math::popcnt(w);
				for (u32 k = rank & ~(u - 1); !n && k; k ^= t) {
					t = math::lsb(k);
					w = where64(math::tzcnt(t));
					n = math::popcnt(w);
					n = n > 1 ? n : 0;
				}
				if (n == 0) continue;
				put64(math::lsb(w), math::tzcnt(u));
				diff ^= (n == 1) ? (scale ^ diff) & t : 0;
				rank = (rank ^ (n > 1 ? 0 : t)) | u;
			} else { // if u is unexpected but present
				u64 w = where64(math::tzcnt(u));
				u32 t = math::msb((scale | ~mask) & (u - 1)) ?:
				       (math::lsb(rank ^ u) ?: 1);
				put64(w, math::tzcnt(t));
				rank = (rank ^ u) | t;
			}
		}
		return rank;
	}
	inline constexpr u32 scale80(u32 scale, u32 thres = 0) {
		scale &= ~(math::msb(thres | 1) - 1);
		u32 rank = scale80();
		while (rank >= 65536 && shift80()) rank = scale80();
		i32 dist = math::popcnt(scale & -2u) - math::popcnt(rank);
		while (dist-- > 0) rank |= math::msb(~rank & 0xffff);
		rank |= (scale & 1);
		rank = scale64(rank, math::lsb(rank));
		for (u32 s = 0, h = 0; scale; scale ^= s, rank ^= s ^ h) {
			s = math::msb(scale);
			h = math::msb(rank & (s ^ (s - 1)));
			put80(where80(math::tzcnt(h)), math::tzcnt(s));
		}
		return rank;
	}

	inline constexpr void isomax() { return isomax64(); }
	inline constexpr void isomax64() {
		u64 x = raw;
		flip64();      x = std::max(x, raw);
		transpose64(); x = std::max(x, raw);
		flip64();      x = std::max(x, raw);
		transpose64(); x = std::max(x, raw);
		flip64();      x = std::max(x, raw);
		transpose64(); x = std::max(x, raw);
		flip64();      x = std::max(x, raw);
		raw = x;
	}
	inline constexpr void isomax80() {
		board x(*this);
		flip80();      x = std::max(x, *this);
		transpose80(); x = std::max(x, *this);
		flip80();      x = std::max(x, *this);
		transpose80(); x = std::max(x, *this);
		flip80();      x = std::max(x, *this);
		transpose80(); x = std::max(x, *this);
		flip80();      x = std::max(x, *this);
		operator =(x);
	}
	inline constexpr void isomin() { return isomin64(); }
	inline constexpr void isomin64() {
		u64 x = raw;
		flip64();      x = std::min(x, raw);
		transpose64(); x = std::min(x, raw);
		flip64();      x = std::min(x, raw);
		transpose64(); x = std::min(x, raw);
		flip64();      x = std::min(x, raw);
		transpose64(); x = std::min(x, raw);
		flip64();      x = std::min(x, raw);
		raw = x;
	}
	inline constexpr void isomin80() {
		board x(*this);
		flip80();      x = std::min(x, *this);
		transpose80(); x = std::min(x, *this);
		flip80();      x = std::min(x, *this);
		transpose80(); x = std::min(x, *this);
		flip80();      x = std::min(x, *this);
		transpose80(); x = std::min(x, *this);
		flip80();      x = std::min(x, *this);
		operator =(x);
	}

	inline u32 species() const { return species64(); }
	inline u32 species64() const {
		return qrow16(0).species | qrow16(1).species | qrow16(2).species | qrow16(3).species;
	}
	inline u32 species80() const {
		return qrow20(0).species | qrow20(1).species | qrow20(2).species | qrow20(3).species;
	}

	inline constexpr u32 scale() const   { return scale64(); }
	inline constexpr u32 scale64() const {
		register u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at4(i));
		return h;
	}
	inline constexpr u32 scale80() const {
		register u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at5(i));
		return h;
	}

	inline constexpr u64 hash() const { return hash64(); }
	inline constexpr u64 hash64() const { return math::fmix64(raw); }
	inline constexpr u64 hash80() const {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_HASH)
		return hash64() ^ math::fmix64(math::pdep64(ext >> 16, 0x1111111111111111ull));
#else
		u64 e = ext >> 16;
		e |= (e << 24);
		e |= (e << 12);
		e &= 0x000f000f000f000full;
		e |= (e << 6);
		e |= (e << 3);
		e &= 0x1111111111111111ull;
		return hash64() ^ math::fmix64(e);
#endif
	}

	inline constexpr u32 max()   const { return max64(); }
	inline constexpr u32 max64() const { return math::log2(scale64()); }
	inline constexpr u32 max80() const { return math::log2(scale80()); }

	inline constexpr u32 min()   const { return min64(); }
	inline constexpr u32 min64() const { return math::tzcnt32(scale64()); }
	inline constexpr u32 min80() const { return math::tzcnt32(scale80()); }

	inline hex numof() const {
		return qrow(0).numof + qrow(1).numof + qrow(2).numof + qrow(3).numof;
	}
	inline u32 numof(u32 t) const { return numof64(t); }
	inline u32 numof64(u32 t) const {
		return qrow16(0).numof[t] + qrow16(1).numof[t] + qrow16(2).numof[t] + qrow16(3).numof[t];
	}
	inline u32 numof80(u32 t) const {
		return qrow20(0).numof[t] + qrow20(1).numof[t] + qrow20(2).numof[t] + qrow20(3).numof[t];
	}
	inline void numof(u32 num[], u32 min, u32 max) const { return numof64(num, min, max); }
	inline void numof64(u32 num[], u32 min, u32 max) const {
		hexa numof0 = qrow16(0).numof;
		hexa numof1 = qrow16(1).numof;
		hexa numof2 = qrow16(2).numof;
		hexa numof3 = qrow16(3).numof;
		for (u32 i = min; i < max; i++) num[i] = numof0[i] + numof1[i] + numof2[i] + numof3[i];
	}
	inline void numof80(u32 num[], u32 min, u32 max) const {
		hexa numof0 = qrow20(0).numof;
		hexa numof1 = qrow20(1).numof;
		hexa numof2 = qrow20(2).numof;
		hexa numof3 = qrow20(3).numof;
		for (u32 i = min; i < max; i++) num[i] = numof0[i] + numof1[i] + numof2[i] + numof3[i];
	}

	inline constexpr hex count() const {
		return ((count64(0)  <<  0) | (count64(1)  <<  4) | (count64(2)  <<  8) | (count64(3)  << 12)
		      | (count64(4)  << 16) | (count64(5)  << 20) | (count64(6)  << 24) | (count64(7)  << 28))
		 | (u64((count64(8)  <<  0) | (count64(9)  <<  4) | (count64(10) <<  8) | (count64(11) << 12)
		      | (count64(12) << 16) | (count64(13) << 20) | (count64(14) << 24) | (count64(15) << 28)) << 32);
	}
	inline constexpr u32 count(u32 t) const { return count64(t); }
	inline constexpr u32 count64(u32 t) const { return math::popcnt(where64(t)); }
	inline constexpr u32 count80(u32 t) const { return math::popcnt(where80(t)); }
	inline constexpr void count(u32 num[], u32 min, u32 max) const { return count64(num, min, max); }
	inline constexpr void count64(u32 num[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) num[i] = count64(i);
	}
	inline constexpr void count80(u32 num[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) num[i] = count80(i);
	}

	inline constexpr u64 where(u32 t) const { return where64(t); }
	inline constexpr u64 where64(u32 t) const {
		u64 x = raw ^ (t * 0x1111111111111111ull);
		x |= (x >> 2);
		x |= (x >> 1);
		return ~x & 0x1111111111111111ull;
	}
	inline constexpr u64 where80(u32 t) const {
		u64 x = raw ^ ((t & 0x0f) * 0x1111111111111111ull);
		x |= (x >> 2);
		x |= (x >> 1);
		u32 e = ext ^ (i32((t & 0x10) << 27) >> 15 /*(t & 0x10) ? 0xffff0000 : 0x00000000*/);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_WHERE)
		return ~x & math::pdep64(~e >> 16, 0x1111111111111111ull);
#else
		u64 w = ~e >> 16;
		w |= (w << 24);
		w |= (w << 12);
		w &= 0x000f000f000f000full;
		w |= (w << 6);
		w |= (w << 3);
		w &= 0x1111111111111111ull;
		return ~x & w;
#endif
	}

	inline constexpr u32 mask(u32 t) const { return mask64(t); }
	inline constexpr u32 mask64(u32 t) const {
		u64 x = raw ^ (t * 0x1111111111111111ull);
		x |= (x >> 2);
		x |= (x >> 1);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_MASK)
		return math::pext64(~x, 0x1111111111111111ull);
#else
		x = ~x & 0x1111111111111111ull;
		x |= (x >> 3);
		x |= (x >> 6);
		x &= 0x000f000f000f000full;
		x |= (x >> 12);
		x |= (x >> 24);
		return x & 0xffffu;
#endif
	}
	inline constexpr u32 mask80(u32 t) const {
		return mask64(t & 0x0f) & (~(ext ^ (i32((t & 0x10) << 27) >> 15)) >> 16);
	}
	inline constexpr void mask(u32 msk[], u32 min, u32 max) const { return mask64(msk, min, max); }
	inline constexpr void mask64(u32 msk[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) msk[i] = mask64(i);
	}
	inline constexpr void mask80(u32 msk[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) msk[i] = mask80(i);
	}

	inline hexa find(u32 t) const { return find64(t); }
	inline hexa find64(u32 t) const { return cache::load(mask64(t)).layout; }
	inline hexa find80(u32 t) const { return cache::load(mask80(t)).layout; }

	inline u64 mono(bool left = true) const { return mono64(left); }
	inline u64 mono64(bool left = true) const {
		register u64 mono = 0;
		if (left) {
			mono |= u64(qrow16(0).left.mono) <<  0;
			mono |= u64(qrow16(1).left.mono) << 12;
			mono |= u64(qrow16(2).left.mono) << 24;
			mono |= u64(qrow16(3).left.mono) << 36;
		} else {
			mono |= u64(qrow16(0).right.mono) <<  0;
			mono |= u64(qrow16(1).right.mono) << 12;
			mono |= u64(qrow16(2).right.mono) << 24;
			mono |= u64(qrow16(3).right.mono) << 36;
		}
		return mono;
	}
	inline u64 mono80(bool left = true) const {
		register u64 mono = 0;
		if (left) {
			mono |= u64(qrow20(0).left.mono) <<  0;
			mono |= u64(qrow20(1).left.mono) << 12;
			mono |= u64(qrow20(2).left.mono) << 24;
			mono |= u64(qrow20(3).left.mono) << 36;
		} else {
			mono |= u64(qrow20(0).right.mono) <<  0;
			mono |= u64(qrow20(1).right.mono) << 12;
			mono |= u64(qrow20(2).right.mono) << 24;
			mono |= u64(qrow20(3).right.mono) << 36;
		}
		return mono;
	}

	inline u32 legal() const { return legal64(); }
	inline u32 legal64() const {
		u32 hori = qrow16(0).legal | qrow16(1).legal | qrow16(2).legal | qrow16(3).legal;
		u32 vert = qcol16(0).legal | qcol16(1).legal | qcol16(2).legal | qcol16(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}
	inline u32 legal80() const {
		u32 hori = qrow20(0).legal | qrow20(1).legal | qrow20(2).legal | qrow20(3).legal;
		u32 vert = qcol20(0).legal | qcol20(1).legal | qcol20(2).legal | qcol20(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}

	inline hex actions() const { return actions64(); }
	inline hex actions64() const {
		u32 o = legal64();
		u32 n = math::popcnt(o);
		u32 k = 0;
		k |= (math::popcnt((o & -o) - 1) <<  0) & 0x000fu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) <<  4) & 0x00ffu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) <<  8) & 0x0fffu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) << 12) & 0xffffu; o &= o - 1;
		return k | (u64(n) << 60);
	}
	inline hex actions80() const {
		u32 o = legal80();
		u32 n = math::popcnt(o);
		u32 k = 0;
		k |= (math::popcnt((o & -o) - 1) <<  0) & 0x000fu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) <<  4) & 0x00ffu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) <<  8) & 0x0fffu; o &= o - 1;
		k |= (math::popcnt((o & -o) - 1) << 12) & 0xffffu; o &= o - 1;
		return k | (u64(n) << 60);
	}

	inline bool movable() const   { return movable64(); }
	inline bool movable64() const {
		return empty64() > 0 ||
			(qrow16(0).moved & qrow16(1).moved & qrow16(2).moved & qrow16(3).moved) == 0 ||
			(qcol16(0).moved & qcol16(1).moved & qcol16(2).moved & qcol16(3).moved) == 0;
	}
	inline bool movable80() const {
		return empty80() > 0 ||
			(qrow20(0).moved & qrow20(1).moved & qrow20(2).moved & qrow20(3).moved) == 0 ||
			(qcol20(0).moved & qcol20(1).moved & qcol20(2).moved & qcol20(3).moved) == 0;
	}

	class tile {
	friend class board;
	public:
		inline constexpr tile(const tile& t) = default;
		inline constexpr tile() = delete;
		inline constexpr board& whole() const { return b; }
		inline constexpr u32 where() const { return i; }
		inline constexpr operator u32() const { return at(is(style::extend), is(style::exact)); }
		inline constexpr tile& operator =(u32 k) { at(k, is(style::extend), is(style::exact)); return *this; }
		inline constexpr tile& operator =(const tile& t) { return operator =(u32(t)); }
	public:
		inline static constexpr u32 itov(u32 i) { return (1u << i) & 0xfffffffeu; }
		inline static constexpr u32 vtoi(u32 v) { return math::log2(v); }
	public:
		friend std::ostream& operator <<(std::ostream& out, const tile& t) {
			u32 v = t.at(t.is(style::extend), !t.is(style::binary) && t.is(style::exact));
			return t.is(style::binary) ? moporgic::write_cast<byte>(out, v) : (out << v);
		}
		friend std::istream& operator >>(std::istream& in, const tile& t) {
			u32 v;
			if (t.is(style::binary) ? moporgic::read_cast<byte>(in, v) : (in >> v))
				t.at(v, t.is(style::extend), !t.is(style::binary) && t.is(style::exact));
			return in;
		}
	protected:
		inline constexpr tile(const board& b, u32 i) : b(const_cast<board&>(b)), i(i) {}
		board& b;
		u32 i;
		inline constexpr bool is(u32 item) const { return b.inf & item; }
		inline constexpr u32 at(bool extend, bool exact) const {
			u32 v = extend ? b.at5(i) : b.at4(i);
			return exact ? tile::itov(v) : v;
		}
		inline constexpr void at(u32 k, bool extend, bool exact) const {
			u32 v = exact ? tile::itov(k) : k;
			if (extend) b.at5(i, v); else b.at4(i, v);
		}
	};
	class iter : tile {
	friend class board;
	public:
		typedef u32 value_type;
		typedef i32 difference_type;
		typedef tile& reference;
		typedef iter pointer;
		typedef std::forward_iterator_tag iterator_category;
		inline constexpr iter(const iter& t) = default;
		inline constexpr iter() = delete;
		inline constexpr tile& operator *() { return *this; }
		inline constexpr const tile& operator *() const { return *this; }
		inline constexpr tile  operator ->() const { return *this; }
		inline constexpr bool  operator ==(const iter& t) const { return ((b == t.b) & (i == t.i)); }
		inline constexpr bool  operator !=(const iter& t) const { return ((b != t.b) | (i != t.i)); }
		inline constexpr bool  operator < (const iter& t) const { return ((b == t.b) & (i < t.i)) | (b < t.b); }
		inline constexpr i32   operator - (const iter& t) const { return i32(i) - i32(t.i); }
		inline constexpr iter  operator + (u32 n) const { return iter(b, i + n); }
		inline constexpr iter  operator - (u32 n) const { return iter(b, i - n); }
		inline constexpr iter& operator +=(u32 n) { i += n; return *this; }
		inline constexpr iter& operator -=(u32 n) { i -= n; return *this; }
		inline constexpr iter& operator ++() { i += 1; return *this; }
		inline constexpr iter& operator --() { i -= 1; return *this; }
		inline constexpr iter  operator ++(int) { return iter(b, ++i - 1); }
		inline constexpr iter  operator --(int) { return iter(b, --i + 1); }
	protected:
		inline constexpr iter(const board& b, u32 i) : tile(b, i) {};
	};
	inline constexpr tile operator [](u32 i) const { return tile(*this, i); }
	inline constexpr iter begin() const { return iter(*this, 0); }
	inline constexpr iter end() const { return iter(*this, 16); }

	class style {
	public:
		style() = delete;
		enum fmtcode : u32 {
			index  = 0x00000000u, /* print (or write) tile indexes, this is the default option */
			exact  = 0x10000000u, /* print tile values (string); write with board info (binary) */
			alter  = 0x20000000u, /* use alternative: print board as hex string (string); write with extension placeholder (binary) */
			binary = 0x40000000u, /* switch between string and binary mode */
			extend = 0x80000000u, /* print (or write) with 16-bit extension */
			full   = 0xf0000000u, /* enable all flags: will write the whole data structure (128-bit) */

			at     = index,
			ext    = extend,
			lite   = alter,
			raw    = binary,
		};
	};
	inline constexpr board& format(u32 i = style::index) { info((i & style::full) | (inf & ~style::full)); return *this; }

	inline constexpr u32 info() const { return inf; }
	inline constexpr void info(u32 i) { inf = i; }

	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		if (b.inf & style::binary) {
			moporgic::write<u64>(out, b.raw);
			if (b.inf & style::extend) moporgic::write_cast<u16>(out, b.ext >> 16);
			if (b.inf & style::alter)  moporgic::write_cast<u16>(out, b.ext & 0xffff);
			if (b.inf & style::exact)  moporgic::write<u32>(out, b.inf);
		} else if (b.inf & style::alter) {
			char buf[32];
			std::snprintf(buf, sizeof(buf), "[%016" PRIx64 "]", b.raw);
			if (b.inf & style::extend) std::snprintf(buf + 17, sizeof(buf) - 17, "|%04x]", b.ext >> 16);
			out << buf;
		} else {
			char buf[64];
			u32 w = (b.inf & style::exact) ? 6 : 4;
			std::snprintf(buf, sizeof(buf), "+%.*s+", (w * 4), "------------------------");
			out << buf << std::endl;
			for (u32 i = 0; i < 16; i += 4) {
				u32 t[4] = { b[i + 0], b[i + 1], b[i + 2], b[i + 3] };
				std::snprintf(buf, sizeof(buf), "|%*u%*u%*u%*u|", w, t[0], w, t[1], w, t[2], w, t[3]);
				out << buf << std::endl;
			}
			std::snprintf(buf, sizeof(buf), "+%.*s+", (w * 4), "------------------------");
			out << buf << std::endl;
		}
		return out;
	}

	friend std::istream& operator >>(std::istream& in, board& b) {
		if (b.inf & style::binary) {
			moporgic::read<u64>(in, b.raw);
			if (b.inf & style::extend) moporgic::read_cast<u16>(in, raw_cast<u16, 1>(b.ext));
			if (b.inf & style::alter)  moporgic::read_cast<u16>(in, raw_cast<u16, 0>(b.ext));
			if (b.inf & style::exact)  moporgic::read<u32>(in, b.inf);
		} else if (b.inf & style::alter) {
			bool nobox(in >> std::hex >> b.raw);
			if (!nobox) (in.clear(), in.ignore(1)) >> std::hex >> b.raw;
			if (b.inf & style::extend) in.ignore(1) >> std::hex >> raw_cast<u16, 1>(b.ext);
			if (!nobox) in.ignore(1);
		} else {
			for (u32 k, i = 0; i < 16; i++) {
				for (k = 0; !(in >> k) && !in.eof(); in.clear(), in.ignore(1));
				b[i] = k;
			}
		}
		return in;
	}

};

} // namespace moporgic
