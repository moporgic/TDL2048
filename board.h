//============================================================================
// Name        : moporgic/TDL2048+ - board.h
// Author      : Hung Guei @ moporgic
// Version     : 6.8
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
	u16 ext;
	u16 opt;
	u32 inf;

public:
	inline constexpr board(u64 raw = 0, u16 ext = 0, u32 inf = 0) : raw(raw), ext(ext), opt(), inf(inf) {}
	inline constexpr board(u64 raw, u16 ext, u16 opt, u32 inf) : raw(raw), ext(ext), opt(opt), inf(inf) {}
	inline constexpr board(const u128& x) : board(u64(x), u16(x >> 64), u16(x >> 80), u32(x >> 96)) {}
	inline constexpr board(const board& b) = default;
	template<typename type, typename = enable_if_is_base_of<board, type>>
	inline constexpr board(const type& x) : board(raw_cast<board>(x)) {}
	template<typename type, typename = enable_if_not_is_base_of<board, type>, typename = enable_if_is_convertible<type, u64>>
	inline constexpr board(const type& x) : board(u64(x)) {}

	inline constexpr operator u64&() { return raw; }
	inline constexpr operator u64() const { return raw; }
	inline constexpr explicit operator u128() const { return u128(raw) | (u128(ext) << 64) | (u128(opt) << 80) | (u128(inf) << 96); }

	inline constexpr board& operator =(u64 x) { raw = x; ext = 0; return *this; }
	inline constexpr board& operator =(const u128& x) { return operator =(board(x)); }
	inline constexpr board& operator =(const board& b) = default;
	template<typename type, typename = enable_if_is_base_of<board, type>>
	inline constexpr board& operator =(const type& x) { return operator =(raw_cast<board>(x)); }
	template<typename type, typename = enable_if_not_is_base_of<board, type>, typename = enable_if_is_convertible<type, u64>>
	inline constexpr board& operator =(const type& x) { return operator =(u64(x)); }
	declare_comparators(const board&, operator u128(), inline constexpr)

	inline constexpr void set(u64 x, u16 e = 0) { raw = x; ext = e; }
	inline constexpr void set(const u128& x) { set(u64(x), u16(x >> 64)); }
	inline constexpr void set(const board& b) { set(b.raw, b.ext); }
	template<typename type, typename = enable_if_is_base_of<board, type>>
	inline constexpr void set(const type& x) { set(raw_cast<board>(x)); }
	template<typename type, typename = enable_if_not_is_base_of<board, type>, typename = enable_if_is_convertible<type, u64>>
	inline constexpr void set(const type& x) { set(u64(x), 0); }
	inline constexpr int  cmp(const board& b) const { return (ext > b.ext) - (ext < b.ext) ?: (raw > b.raw) - (raw < b.raw); }

public:
	class cache {
	public:
		static inline const cache& load(u32 i) {
			static byte block[sizeof(cache) * (1 << 20)] = {};
			return pointer_cast<cache>(block)[i];
		}
		static __attribute__((constructor)) void make() {
			for (u32 i = 0; i < (1 << 20); i++) new (const_cast<cache*>(&load(i))) cache(i);
		}

	public:
		cache(const cache& c) = default;
		cache(u32 src) : raw(src & 0xffffu), ext(src >> 16) {
			std::array<u32, 4> row;
			for (u32 i = 0; i < 4; i++) {
				u32 t = row[i] = ((src >> (i << 2)) & 0x0f) | ((src >> (i + 12)) & 0x10);
				species |= (1 << t);
				numof[t] += 1;
			}
			u32 cmp[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
			for (u32 i = 0; i < 6; i++) {
				u32 t0 = row[cmp[i][0]], t1 = row[cmp[i][1]];
				mono |= ((t0 | t1) ? ((t0 >= t1) | ((t1 >= t0) << 1)) : 0) << (i << 1);
			}
			auto res_l = slide(row, src, +1);
			auto res_r = slide(row, src, -1);
			mvl = std::get<0>(res_l);
			mvr = std::get<0>(res_r);
			score = std::get<1>(res_l);
			merge = std::get<2>(res_l);
			moved = (mvl.moved | mvr.moved);
			legal = (mvl.moved & 0b1001) | (mvr.moved & 0b0110);
		}

		class move {
		public:
			move() : rawh(), exth(), moved(), extv(), rawv() {}
			move(const std::array<u32, 4>& row, u32 src) {
				u32 lo[] = { (row[0] & 0x0f),      (row[1] & 0x0f),      (row[2] & 0x0f),      (row[3] & 0x0f) };
				u32 hi[] = { (row[0] & 0x10) >> 4, (row[1] & 0x10) >> 4, (row[2] & 0x10) >> 4, (row[3] & 0x10) >> 4 };
				rawh  = (     (lo[0]  <<  0)  | (    lo[1]  <<  4)  | (    lo[2]  <<  8)  | (    lo[3]  << 12) );
				exth  = (     (hi[0]  <<  0)  | (    hi[1]  <<  1)  | (    hi[2]  <<  2)  | (    hi[3]  <<  3) );
				rawv  = ( (u64(lo[0]) <<  0)  | (u64(lo[1]) << 16)  | (u64(lo[2]) << 32)  | (u64(lo[3]) << 48) );
				extv  = (     (hi[0]  <<  0)  | (    hi[1]  <<  4)  | (    hi[2]  <<  8)  | (    hi[3]  << 12) );
				moved = ( (u32(rawh) | (u32(exth) << 16)) != src) ? -1 : 0;
			}
			template<int i> inline void moveh64(board& mv) const {
				mv.raw |= u64(rawh) << (i << 4);
			}
			template<int i> inline void moveh80(board& mv) const {
				mv.raw |= u64(rawh) << (i << 4);
				mv.ext |= u16(exth) << (i << 2);
			}
			template<int i> inline void movev64(board& mv) const {
				mv.raw |= u64(rawv) << (i << 2);
			}
			template<int i> inline void movev80(board& mv) const {
				mv.raw |= u64(rawv) << (i << 2);
				mv.ext |= u16(extv) << (i);
			}
		public:
			u16 rawh;  // horizontal move (16-bit raw)
			u8  exth;  // horizontal move (4-bit extra)
			i8  moved; // moved (-1) or not (0)
			u16 extv;  // vertical move (16-bit extra)
			u64 rawv;  // vertical move (64-bit raw)
		};

	protected:
		std::tuple<move, u32, u32> slide(std::array<u32, 4> row, u32 src, int step = 1) const {
			u32 score = 0, merge = 0;
			u32 idx[4] = {0, 1, 2, 3};
			if (step == -1) std::reverse(idx, idx + 4);
			u32 top = idx[0], hold = 0;
			for (u32 i : idx) {
				u32 tile = row[i];
				if (tile == 0) continue;
				row[i] = 0;
				if (hold) {
					if (tile == hold) {
						hold = ++tile;
						score += (1 << tile);
						merge++;
						tile = 0;
					}
					row[top] = hold;
					top += step;
				}
				hold = tile;
			}
			row[top] = hold;
			return {move(row, src), score, merge};
		}

	public:
		template<u32 op, u32 i> inline void move64(board& mv) const {
			switch (op) {
			case action::up:    mvl.movev64<i>(mv); mv.inf += score; break;
			case action::right: mvr.moveh64<i>(mv); mv.inf += score; break;
			case action::down:  mvr.movev64<i>(mv); mv.inf += score; break;
			case action::left:  mvl.moveh64<i>(mv); mv.inf += score; break;
			}
		}
		template<u32 op, u32 i> inline void move80(board& mv) const {
			switch (op) {
			case action::up:    mvl.movev80<i>(mv); mv.inf += score; break;
			case action::right: mvr.moveh80<i>(mv); mv.inf += score; break;
			case action::down:  mvr.movev80<i>(mv); mv.inf += score; break;
			case action::left:  mvl.moveh80<i>(mv); mv.inf += score; break;
			}
		}
		template<int i> inline void moveh64(board& L, board& R) const {
			move64<action::left,  i>(L);
			move64<action::right, i>(R);
		}
		template<int i> inline void moveh80(board& L, board& R) const {
			move80<action::left,  i>(L);
			move80<action::right, i>(R);
		}
		template<int i> inline void movev64(board& U, board& D) const {
			move64<action::up,   i>(U);
			move64<action::down, i>(D);
		}
		template<int i> inline void movev80(board& U, board& D) const {
			move80<action::up,   i>(U);
			move80<action::down, i>(D);
		}

	public:
		u16  raw;     // 16-bit raw of this row
		u8   ext;     // 4-bit extra of this row
		u8   legal;   // legal actions (4-bit)
		u32  species; // species of this row
		hexa numof;   // number of each tile-type
		move mvl;     // LUT for moving left/up
		move mvr;     // LUT for moving right/down
		u32  score;   // merge score (reward)
		u8   merge;   // number of merged tiles
		i8   moved;   // moved (-1) or not (0)
		u16  mono;    // cell relationship (12-bit)
	};

	inline const cache& qrow(u32 i) const { return qrow16(i); }
	inline const cache& qrow16(u32 i) const { return cache::load(row16(i)); }
	inline const cache& qrow20(u32 i) const { return cache::load(row20(i)); }

	inline const cache& qcol(u32 i) const { return qcol16(i); }
	inline const cache& qcol16(u32 i) const { return cache::load(col16(i)); }
	inline const cache& qcol20(u32 i) const { return cache::load(col20(i)); }

	inline constexpr u32 row(u32 i) const { return row16(i); }
	inline constexpr u32 row16(u32 i) const {
		return u32(raw >> (i << 4)) & 0xffff;
	}
	inline constexpr u32 row20(u32 i) const {
		return row16(i) | ((u32(ext >> (i << 2)) << 16) & 0xf0000);
	}
	inline constexpr void row(u32 i, u32 r) { row16(i, r); }
	inline constexpr void row16(u32 i, u32 r) {
		raw = (raw & ~(0xffffull << (i << 4))) | (u64(r) << (i << 4));
	}
	inline constexpr void row20(u32 i, u32 r) {
		row16(i, r & 0xffff);
		ext = (ext & ~(0x000f << (i << 2))) | ((r >> 16) << (i << 2));
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
		return col16(i) | (math::pext32(ext, 0x1111u << i) << 16);
#else
		return col16(i) | ((((ext >> i) & 0x1111u) * 0x12480u) & 0xf0000u);
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
		u32 m = 0x1111u << i;
#if defined(__BMI2__) && !defined(PREFER_LEGACY_COL)
		ext = (ext & ~m) | math::pdep32(c >> 16, m);
#else
		u32 e = c >> 16;
		u32 x = e | (e << 3) | (e << 6) | (e << 9);
		ext = (ext & ~m) | ((x & 0x1111u) << i);
#endif
	}

	inline constexpr u32 at(u32 i) const { return at4(i); }
	inline constexpr u32 at4(u32 i) const {
		return (raw >> (i << 2)) & 0x0f;
	}
	inline constexpr u32 at5(u32 i) const {
		return at4(i) | ((ext >> i << 4) & 0x10);
	}
	inline constexpr void at(u32 i, u32 t) { at4(i, t); }
	inline constexpr void at4(u32 i, u32 t) {
		raw = (raw & ~(0x0full << (i << 2))) | (u64(t) << (i << 2));
	}
	inline constexpr void at5(u32 i, u32 t) {
		at4(i, t & 0x0f);
		ext = (ext & ~(1 << i)) | (t >> 4 << i);
	}

	inline constexpr u32 exact(u32 i) const { return exact4(i); }
	inline constexpr u32 exact4(u32 i) const { return (1u << at4(i)) & 0xfffffffeu; }
	inline constexpr u32 exact5(u32 i) const { return (1u << at5(i)) & 0xfffffffeu; }
	inline constexpr void exact(u32 i, u32 t) { exact4(i, t); }
	inline constexpr void exact4(u32 i, u32 t) { at4(i, math::log2(t | 1u)); }
	inline constexpr void exact5(u32 i, u32 t) { at5(i, math::log2(t | 1u)); }

	inline constexpr u32 fat(u32 i) const { return opts(style::extend) ? fat5(i) : fat4(i); }
	inline constexpr u32 fat4(u32 i) const { return opts(style::exact) ? exact4(i) : at4(i); }
	inline constexpr u32 fat5(u32 i) const { return opts(style::exact) ? exact5(i) : at5(i); }
	inline constexpr void fat(u32 i, u32 t) { return opts(style::extend) ? fat5(i, t) : fat4(i, t); }
	inline constexpr void fat4(u32 i, u32 t) { return opts(style::exact) ? exact4(i, t) : at4(i, t); }
	inline constexpr void fat5(u32 i, u32 t) { return opts(style::exact) ? exact5(i, t) : at5(i, t); }

	inline constexpr void put(u64 where, u32 t) { return put64(where, t); }
	inline constexpr void put64(u64 where, u32 t) {
		raw = (raw & ~(where * 0x0full)) | (where * t);
	}
	inline constexpr void put80(u64 where, u32 t) {
		put64(where, t & 0x0f);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_PUT)
		u32 mask = math::pext64(where, 0x1111111111111111ull);
#else
		where |= where >> 3;
		where |= where >> 6;
		where &= 0x000f000f000f000full;
		where |= where >> 12;
		where |= where >> 24;
		u32 mask = where & 0xffffu;
#endif
		ext = (ext ^ (ext & mask)) | (t & 0x10 ? mask : 0);
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
		ext = (ext ^ (ext & mask)) | (t & 0x10 ? mask : 0);
	}

	inline constexpr void mirror() { mirror64(); }
	inline constexpr void mirror64() {
		raw = ((raw & 0x000f000f000f000full) << 12) | ((raw & 0x00f000f000f000f0ull) << 4)
		    | ((raw & 0x0f000f000f000f00ull) >> 4) | ((raw & 0xf000f000f000f000ull) >> 12);
	}
	inline constexpr void mirror80() {
		mirror64();
		ext = ((ext & 0x1111) << 3) | ((ext & 0x2222) << 1)
		    | ((ext & 0x4444) >> 1) | ((ext & 0x8888) >> 3);
	}

	inline constexpr void flip() { flip64(); }
	inline constexpr void flip64() {
		u64 buf = (raw ^ math::rol64(raw, 16)) & 0x0000ffff0000ffffull;
		raw ^= (buf | math::ror64(buf, 16));
	}
	inline constexpr void flip80() {
		flip64();
		u16 buf = (ext ^ math::rol16(ext, 4)) & 0x0f0fu;
		ext ^= (buf | math::ror16(buf, 4));
	}

	inline constexpr void transpose() { transpose64(); }
	inline constexpr void transpose64() {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_TRANSPOSE)
		raw = (math::pext64(raw, 0x000f000f000f000full) <<  0) | (math::pext64(raw, 0x00f000f000f000f0ull) << 16)
		    | (math::pext64(raw, 0x0f000f000f000f00ull) << 32) | (math::pext64(raw, 0xf000f000f000f000ull) << 48);
#else
		u64 buf = 0;
		buf = (raw ^ (raw >> 12)) & 0x0000f0f00000f0f0ull;
		raw ^= buf ^ (buf << 12);
		buf = (raw ^ (raw >> 24)) & 0x00000000ff00ff00ull;
		raw ^= buf ^ (buf << 24);
#endif
	}
	inline constexpr void transpose80() {
		transpose64();
#if defined(__BMI2__) && !defined(PREFER_LEGACY_TRANSPOSE)
		ext = (math::pext32(ext, 0x1111u) <<  0) | (math::pext32(ext, 0x2222u) <<  4)
		    | (math::pext32(ext, 0x4444u) <<  8) | (math::pext32(ext, 0x8888u) << 12);
#else
		u16 buf = 0;
		buf = (ext ^ (ext >> 3)) & 0x0a0a;
		ext ^= buf ^ (buf << 3);
		buf = (ext ^ (ext >> 6)) & 0x00cc;
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
	inline constexpr void isoms(btype iso[]) const { isoms64(iso); }
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

	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::array<btype, 8> isoms() const { return isoms64(); }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::array<btype, 8> isoms64() const { std::array<btype, 8> iso; isoms64(iso.data()); return iso; }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::array<btype, 8> isoms80() const { std::array<btype, 8> iso; isoms80(iso.data()); return iso; }

	inline constexpr u32 empty() const { return empty64(); }
	inline constexpr u32 empty64() const { return count64(0); }
	inline constexpr u32 empty80() const { return count80(0); }

	inline constexpr nthit spaces() const { return spaces64(); }
	inline constexpr nthit spaces64() const { return find64(0); }
	inline constexpr nthit spaces80() const { return find80(0); }

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
		u64 t = math::lsb64(x, (u >> 16) % e);
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
		u64 t = math::lsb64(x, (u >> 16) % e);
#else
		u32 k = (u >> 16) % e;
		while (k--) x &= x - 1;
		u64 t = x & -x;
#endif
		raw |= (t << (u % 10 ? 0 : 1));
	}

	inline i32 popup() { return popup64(); }
	inline i32 popup64() { return empty64() ? next64(), 0 : -1; }
	inline i32 popup80() { return empty80() ? next80(), 0 : -1; }

	inline i32 popup(u32 op) { return popup64(op); }
	inline i32 popup64(u32 op) { action::pop p(op); return p.type() && !at4(p.pos()) ? at4(p.pos(), p.type()), 0 : -1; }
	inline i32 popup80(u32 op) { action::pop p(op); return p.type() && !at5(p.pos()) ? at5(p.pos(), p.type()), 0 : -1; }

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr xthit popups(btype popup[]) const { return popups64(popup); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr xthit popups64(btype popup[]) const {
		u64 ops = 0, x = where64(0);
		u32 n = math::popcnt(x);
		u32 f1 = ({f32 f = 0.9 / n; raw_cast<u32>(f);});
		u32 f2 = ({f32 f = 0.1 / n; raw_cast<u32>(f);});
		btype *pop1 = popup, *pop2 = popup + n;
		for (u64 t : u64it(x)) {
			u32 p = math::tzcnt(t) >> 2;
			*(pop1++) = board(raw | (t << 0), ext, f1); ops |= (1ull << action::pop(1, p));
			*(pop2++) = board(raw | (t << 1), ext, f2); ops |= (1ull << action::pop(2, p));
		}
		return ops;
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr xthit popups80(btype popup[]) const {
		u64 ops = 0, x = where80(0);
		u32 n = math::popcnt(x);
		u32 f1 = ({f32 f = 0.9 / n; raw_cast<u32>(f);});
		u32 f2 = ({f32 f = 0.1 / n; raw_cast<u32>(f);});
		btype *pop1 = popup, *pop2 = popup + n;
		for (u64 t : u64it(x)) {
			u32 p = math::tzcnt(t) >> 2;
			*(pop1++) = board(raw | (t << 0), ext, f1); ops |= (1ull << action::pop(1, p));
			*(pop2++) = board(raw | (t << 1), ext, f2); ops |= (1ull << action::pop(2, p));
		}
		return ops;
	}

	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::vector<btype> popups() const   { return popups64(); }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::vector<btype> popups64() const { btype popup[32]; return {popup, popup + popups64(popup).size()}; }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline constexpr std::vector<btype> popups80() const { btype popup[32]; return {popup, popup + popups80(popup).size()}; }

	inline i32 left()  { return left64(); }
	inline i32 right() { return right64(); }
	inline i32 up()    { return up64(); }
	inline i32 down()  { return down64(); }

	inline i32 left64() {
		board move(0, ext, opt, 0);
		qrow16(0).move64<action::left, 0>(move);
		qrow16(1).move64<action::left, 1>(move);
		qrow16(2).move64<action::left, 2>(move);
		qrow16(3).move64<action::left, 3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 right64() {
		board move(0, ext, opt, 0);
		qrow16(0).move64<action::right, 0>(move);
		qrow16(1).move64<action::right, 1>(move);
		qrow16(2).move64<action::right, 2>(move);
		qrow16(3).move64<action::right, 3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 up64() {
		board move(0, ext, opt, 0);
		qcol16(0).move64<action::up, 0>(move);
		qcol16(1).move64<action::up, 1>(move);
		qcol16(2).move64<action::up, 2>(move);
		qcol16(3).move64<action::up, 3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 down64() {
		board move(0, ext, opt, 0);
		qcol16(0).move64<action::down, 0>(move);
		qcol16(1).move64<action::down, 1>(move);
		qcol16(2).move64<action::down, 2>(move);
		qcol16(3).move64<action::down, 3>(move);
		move.inf |= (move.raw ^ raw) ? 0 : -1;
		return operator =(move).inf;
	}

	inline i32 left80() {
		board move(0, 0, opt, 0);
		qrow20(0).move80<action::left, 0>(move);
		qrow20(1).move80<action::left, 1>(move);
		qrow20(2).move80<action::left, 2>(move);
		qrow20(3).move80<action::left, 3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 right80() {
		board move(0, 0, opt, 0);
		qrow20(0).move80<action::right, 0>(move);
		qrow20(1).move80<action::right, 1>(move);
		qrow20(2).move80<action::right, 2>(move);
		qrow20(3).move80<action::right, 3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 up80() {
		board move(0, 0, opt, 0);
		qcol20(0).move80<action::up, 0>(move);
		qcol20(1).move80<action::up, 1>(move);
		qcol20(2).move80<action::up, 2>(move);
		qcol20(3).move80<action::up, 3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}
	inline i32 down80() {
		board move(0, 0, opt, 0);
		qcol20(0).move80<action::down, 0>(move);
		qcol20(1).move80<action::down, 1>(move);
		qcol20(2).move80<action::down, 2>(move);
		qcol20(3).move80<action::down, 3>(move);
		move.inf |= (move.raw ^ raw) | (move.ext ^ ext) ? 0 : -1;
		return operator =(move).inf;
	}

	inline i32 move(u32 op) { return move64(op); }
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

	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline nthit moves(btype move[], bool compact) const   { return moves64(move, compact); }
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline nthit moves64(btype move[], bool compact) const {
		moves64(move[0], move[1], move[2], move[3]);
		nthit ops = actions(move), o = ops;
		if (compact) {
			move[0] = move[(*o++) & 0b11];
			move[1] = move[(*o++) & 0b11];
			move[2] = move[(*o++) & 0b11];
			move[3] = move[(*o++) & 0b11];
		}
		return ops;
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline nthit moves80(btype move[], bool compact) const {
		moves80(move[0], move[1], move[2], move[3]);
		nthit ops = actions(move), o = ops;
		if (compact) {
			move[0] = move[(*o++) & 0b11];
			move[1] = move[(*o++) & 0b11];
			move[2] = move[(*o++) & 0b11];
			move[3] = move[(*o++) & 0b11];
		}
		return ops;
	}

	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::array<btype, 4> moves() const   { std::array<btype, 4> move; moves(move.data()); return move; }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::array<btype, 4> moves64() const { std::array<btype, 4> move; moves64(move.data()); return move; }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::array<btype, 4> moves80() const { std::array<btype, 4> move; moves80(move.data()); return move; }

	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::vector<btype> moves(bool compact) const   { return moves64(compact); }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::vector<btype> moves64(bool compact) const { btype move[4]; return {move, move + moves64(move, compact).size()}; }
	template<typename btype = board, typename = enable_if_is_base_of<board, btype>>
	inline std::vector<btype> moves80(bool compact) const { btype move[4]; return {move, move + moves80(move, compact).size()}; }

	class action {
	public:
		action() = delete;
		enum opcode : u32 {
			up    = 0x00u, // move up
			right = 0x01u, // move right
			down  = 0x02u, // move down
			left  = 0x03u, // move left
			next  = 0x04u, // add next tile randomly
			clear = 0x08u, // clear the board
			init  = 0x0cu, // reset the board (with tiles)
		};
		class pop { // add new tile: 4-bit type, 4-bit position
		public:
			inline constexpr pop(u32 t, u32 p) : pop((t << 4) | p) {}
			inline constexpr pop(u32 code) : code(code) {}
			inline constexpr operator u32() const { return code; }
			inline constexpr u32 type() const { return code >> 4; }
			inline constexpr u32 pos() const { return code & 0xf; }
		private:
			const u32 code;
		};
	};

	inline i32 operate(u32 op) { return operate64(op); }
	inline i32 operate64(u32 op) {
		switch (op) {
		case action::up:    return up64();
		case action::right: return right64();
		case action::down:  return down64();
		case action::left:  return left64();
		case action::next:  return popup64();
		case action::clear: return set(0ull, 0), 0;
		case action::init:  return init(), 0;
		default: /* pop */  return popup64(op);
		}
	}
	inline i32 operate80(u32 op) {
		switch (op) {
		case action::up:    return up80();
		case action::right: return right80();
		case action::down:  return down80();
		case action::left:  return left80();
		case action::next:  return popup80();
		case action::clear: return set(0ull, 0), 0;
		case action::init:  return init(), 0;
		default: /* pop */  return popup80(op);
		}
	}

	inline constexpr u32 shift(u32 n = 1, u32 u = 0) { return shift64(n, u); }
	inline constexpr u32 shift64(u32 n = 1, u32 u = 0) {
		u32 rank = scale64();
		u32 mask = (math::msb(rank) - 1) & ~((2 << u) - 1);
		u32 hole = ~rank & mask;
		u32 hcnt = std::max(math::popcnt(hole), n);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_SHIFT)
		hole &= ~(math::lsb(hole, hcnt - n) - 1);
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
		hole &= ~(math::lsb(hole, hcnt - n) - 1);
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
				u32 t = math::msb((scale | ~mask) & (u - 1)) ?: (math::lsb(rank ^ u) ?: 1);
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
		set(x.raw, x.ext);
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
		set(x.raw, x.ext);
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
		u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at4(i));
		return h;
	}
	inline constexpr u32 scale80() const {
		u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at5(i));
		return h;
	}

	inline constexpr u64 hash() const { return hash64(); }
	inline constexpr u64 hash64() const { return math::fmix64(raw); }
	inline constexpr u64 hash80() const {
#if defined(__BMI2__) && !defined(PREFER_LEGACY_HASH)
		return hash64() ^ math::fmix64(math::pdep64(ext, 0x1111111111111111ull));
#else
		u64 e = ext;
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
		u32 e = ext ^ ((t & 0x10) ? 0xffff : 0x0000);
#if defined(__BMI2__) && !defined(PREFER_LEGACY_WHERE)
		return ~x & math::pdep64(~e, 0x1111111111111111ull);
#else
		u64 w = ~e;
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
		return mask64(t & 0x0f) & ~(ext ^ ((t & 0x10) ? 0xffff : 0x0000));
	}
	inline constexpr void mask(u32 msk[], u32 min, u32 max) const { return mask64(msk, min, max); }
	inline constexpr void mask64(u32 msk[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) msk[i] = mask64(i);
	}
	inline constexpr void mask80(u32 msk[], u32 min, u32 max) const {
		for (u32 i = min; i < max; i++) msk[i] = mask80(i);
	}

	inline constexpr nthit find(u32 t) const { return find64(t); }
	inline constexpr nthit find64(u32 t) const { return nthit(mask64(t)); }
	inline constexpr nthit find80(u32 t) const { return nthit(mask80(t)); }

	inline u64 monorow() const { return monorow64(); }
	inline u64 monorow64() const {
		return (u64(qrow16(0).mono) <<  0) | (u64(qrow16(1).mono) << 12) |
		       (u64(qrow16(2).mono) << 24) | (u64(qrow16(3).mono) << 36);
	}
	inline u64 monorow80() const {
		return (u64(qrow20(0).mono) <<  0) | (u64(qrow20(1).mono) << 12) |
		       (u64(qrow20(2).mono) << 24) | (u64(qrow20(3).mono) << 36);
	}

	inline u64 monocol() const { return monocol64(); }
	inline u64 monocol64() const {
		return (u64(qcol16(0).mono) <<  0) | (u64(qcol16(1).mono) << 12) |
		       (u64(qcol16(2).mono) << 24) | (u64(qcol16(3).mono) << 36);
	}
	inline u64 monocol80() const {
		return (u64(qcol20(0).mono) <<  0) | (u64(qcol20(1).mono) << 12) |
		       (u64(qcol20(2).mono) << 24) | (u64(qcol20(3).mono) << 36);
	}

	inline nthit actions() const { return actions64(); }
	inline nthit actions64() const {
		u32 hori = qrow16(0).legal | qrow16(1).legal | qrow16(2).legal | qrow16(3).legal;
		u32 vert = qcol16(0).legal | qcol16(1).legal | qcol16(2).legal | qcol16(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}
	inline nthit actions80() const {
		u32 hori = qrow20(0).legal | qrow20(1).legal | qrow20(2).legal | qrow20(3).legal;
		u32 vert = qcol20(0).legal | qcol20(1).legal | qcol20(2).legal | qcol20(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}

	inline constexpr nthit actions(const board& U, const board& R, const board& D, const board& L) const {
		return ~((U.inf & 0x10000000u) | (R.inf & 0x20000000u) |
		         (D.inf & 0x40000000u) | (L.inf & 0x80000000u)) >> 28;
	}
	template<typename btype, typename = enable_if_is_base_of<board, btype>>
	inline constexpr nthit actions(const btype move[]) const {
		return actions(move[0], move[1], move[2], move[3]);
	}

	inline bool movable() const   { return movable64(); }
	inline bool movable64() const {
		return empty64() > 0 || (qrow16(0).moved | qrow16(1).moved | qrow16(2).moved | qrow16(3).moved)
		                     || (qcol16(0).moved | qcol16(1).moved | qcol16(2).moved | qcol16(3).moved);
	}
	inline bool movable80() const {
		return empty80() > 0 || (qrow20(0).moved | qrow20(1).moved | qrow20(2).moved | qrow20(3).moved)
		                     || (qcol20(0).moved | qcol20(1).moved | qcol20(2).moved | qcol20(3).moved);
	}

	class tile {
	friend class board;
	public:
		inline static constexpr u32 itov(u32 i) { return (1u << i) & 0xfffffffeu; }
		inline static constexpr u32 vtoi(u32 v) { return math::log2(v | 1u); }
	public:
		inline constexpr tile(const tile& t) = default;
		inline constexpr tile() = delete;
		inline constexpr const board& source() const { return b; }
		inline constexpr board& source() { return b; }
		inline constexpr u32 index() const { return i; }
	public:
		inline constexpr operator u32() const { return b.fat(i); }
		inline constexpr tile& operator =(u32 t) { b.fat(i, t); return *this; }
		friend std::ostream& operator <<(std::ostream& out, const tile& t) {
			return (out << u32(t));
		}
		friend std::istream& operator >>(std::istream& in, tile& t) {
			u32 v;
			if (in >> v) t = v;
			return in;
		}
	protected:
		inline constexpr tile(const board& b, u32 i) : b(const_cast<board&>(b)), i(i) {}
		board& b;
		u32 i;
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
	inline constexpr const tile operator [](u32 i) const { return tile(*this, i); }
	inline constexpr const iter begin() const { return iter(*this, 0); }
	inline constexpr const iter end() const { return iter(*this, 16); }
	inline constexpr tile operator [](u32 i) { return tile(*this, i); }
	inline constexpr iter begin() { return iter(*this, 0); }
	inline constexpr iter end() { return iter(*this, 16); }

	class style {
	public:
		style() = delete;
		enum fmtcode : u32 {
			index  = 0x0000u, /* print (or write) tile indexes, this is the default option */
			exact  = 0x1000u, /* print tile values (string); write with board info (binary) */
			alter  = 0x2000u, /* use alternative: print board as hex string (string); also write the options (binary) */
			binary = 0x4000u, /* switch between string and binary mode */
			extend = 0x8000u, /* print (or write) with 16-bit extension */
			full   = 0xf000u, /* enable all flags: will write the whole data structure (128-bit) */

			at     = index,
			ext    = extend,
			lite   = alter,
			raw    = binary,
		};
	};
	inline constexpr board& format(u32 fmt = style::index) { opts(style::full, fmt); return *this; }

	inline constexpr u32 info() const { return inf; }
	inline constexpr u32 info(u32 i) { u32 ix = inf; inf = i; return ix; }

	inline constexpr u16 opts(u16 ns = -1) const { return opt & ns; }
	template<typename flag, typename = enable_if_is_integral<flag>>
	inline constexpr u16 opts(u16 ns, flag o) { u16 ox = opts(ns); opt = (opt & ~ns) | (u16(o) & ns); return ox; }
	inline constexpr u16 opts(u16 ns, bool o) { return opts(ns, o ? -1u : 0); }

	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		if (b.opts(style::binary)) {
			moporgic::write<u64>(out, b.raw);
			if (b.opts(style::extend)) moporgic::write<u16>(out, b.ext);
			if (b.opts(style::alter))  moporgic::write<u16>(out, b.opt);
			if (b.opts(style::exact))  moporgic::write<u32>(out, b.inf);
		} else if (b.opts(style::alter)) {
			char buf[24];
			std::snprintf(buf, sizeof(buf), "[%016" PRIx64 "]", b.raw);
			if (b.opts(style::extend)) std::snprintf(buf + 17, sizeof(buf) - 17, "|%04x]", b.ext);
			out << buf;
		} else {
			char buf[180];
			u32 n = 0, w = (b.opts(style::exact)) ? 6 : 4;
			n += snprintf(buf + n, sizeof(buf) - n, "+%.*s+" "\n", (w * 4), "------------------------");
			for (u32 i = 0; i < 16; i += 4) {
				u32 t[4] = { b.fat(i + 0), b.fat(i + 1), b.fat(i + 2), b.fat(i + 3) };
				n += snprintf(buf + n, sizeof(buf) - n, "|%*u%*u%*u%*u|" "\n", w, t[0], w, t[1], w, t[2], w, t[3]);
			}
			n += snprintf(buf + n, sizeof(buf) - n, "+%.*s+" "\n", (w * 4), "------------------------");
			out << buf;
		}
		return out;
	}

	friend std::istream& operator >>(std::istream& in, board& b) {
		if (b.opts(style::binary)) {
			moporgic::read<u64>(in, b.raw);
			if (b.opts(style::extend)) moporgic::read<u16>(in, b.ext);
			if (b.opts(style::alter))  moporgic::read<u16>(in, b.opt);
			if (b.opts(style::exact))  moporgic::read<u32>(in, b.inf);
		} else if (b.opts(style::alter)) {
			bool nobox(in >> std::hex >> b.raw);
			if (!nobox) (in.clear(), in.ignore(1)) >> std::hex >> b.raw;
			if (b.opts(style::extend)) in.ignore(1) >> std::hex >> b.ext;
			if (!nobox) in.ignore(1);
		} else {
			for (u32 k, i = 0; i < 16; i++) {
				for (k = 0; !(in >> k) && !in.eof(); in.clear(), in.ignore(1));
				b.fat(i, k);
			}
		}
		return in;
	}

};

} // namespace moporgic
