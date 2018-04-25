#pragma once
#include "moporgic/type.h"
#include "moporgic/util.h"
#include "moporgic/math.h"
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <array>

//============================================================================
// Name        : board.h
// Author      : Hung Guei
// Version     : 3.5
// Description : bitboard of 2048
//============================================================================

namespace moporgic {

class board {
public:
	class cache {
	friend class board;
	public:
		class move {
		friend class board;
		friend class cache;
		public:
			u32 rawh; // horizontal move (16-bit raw)
			u32 exth; // horizontal move (4-bit extra)
			u64 rawv; // vertical move (64-bit raw)
			u32 extv; // vertical move (16-bit extra)
			u32 score; // merge score (reward)
			i32 moved; // moved or not (moved: 0, otherwise -1)
			u16 merge; // number of merged tiles
			u16 mono; // monotonic decreasing value (12-bit)

			template<int i>
			inline void moveh64(u64& raw, u32& sc, i32& mv) const {
				raw |= u64(rawh) << (i << 4);
				sc += score;
				mv &= moved;
			}
			template<int i>
			inline void moveh80(u64& raw, u32& ext, u32& sc, i32& mv) const {
				moveh64<i>(raw, sc, mv);
				ext |= exth << (i << 2);
			}

			template<int i>
			inline void movev64(u64& raw, u32& sc, i32& mv) const {
				raw |= rawv << (i << 2);
				sc += score;
				mv &= moved;
			}
			template<int i>
			inline void movev80(u64& raw, u32& ext, u32& sc, i32& mv) const {
				movev64<i>(raw, sc, mv);
				ext |= extv << i;
			}

			move(const move& op) = default;
			~move() = default;

			static move make(u32 r, bool reverse) {
				u32 row[] = {((r >> 0) & 0x0f) | ((r >> 12) & 0x10), ((r >> 4) & 0x0f) | ((r >> 13) & 0x10),
							((r >> 8) & 0x0f) | ((r >> 14) & 0x10), ((r >> 12) & 0x0f) | ((r >> 15) & 0x10)};
				if (reverse) std::reverse(row, row + 4);

				u32 mono = 0;
				u32 monores[6][2] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 }, };
				for (u32 i = 0; i < 6; i++) {
					u32 a = row[monores[i][0]], b = row[monores[i][1]];
					mono |= ((a == b ? (a ? 0b11 : 0b00) : (a > b ? 0b01 : 0b10)) << (i << 1));
				}

				u32 merge = 0;
				u32 score = 0;
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

				u32 lo[4], hi[4];
				for (u32 i = 0; i < 4; i++) {
					hi[i] = (row[i] & 0x10) >> 4;
					lo[i] = (row[i] & 0x0f);
				}
				u32 hraw = ((lo[0] << 0) | (lo[1] << 4) | (lo[2] << 8) | (lo[3] << 12));
				u32 hext = ((hi[0] << 0) | (hi[1] << 1) | (hi[2] << 2) | (hi[3] << 3)) << 16;
				u64 vraw = (u64(lo[0]) << 0) | (u64(lo[1]) << 16) | (u64(lo[2]) << 32) | (u64(lo[3]) << 48);
				u32 vext = ((hi[0] << 0) | (hi[1] << 4) | (hi[2] << 8) | (hi[3] << 12)) << 16;
				i32 moved = ((hraw | hext) == r) ? -1 : 0;

				return move(hraw, hext, vraw, vext, score, moved, merge, mono);
			}

		private:
			move(u32 rawh, u32 exth, u64 rawv, u32 extv, u32 score, i32 moved, u16 merge, u16 mono)
				: rawh(rawh), exth(exth), rawv(rawv), extv(extv), score(score), moved(moved), merge(merge), mono(mono) {}
			move() : move(0, 0, 0, 0, 0, -1, 0, 0) {}
			move& operator =(const move& op) = default;
		};

		typedef std::array<u16, 32> info;
		u32 raw; // base row (16-bit raw)
		u32 ext; // base row (4-bit extra)
		u32 species; // species of this row
		u32 merge; // number of merged tiles
		move left; // left operation
		move right; // right operation
		info numof; // number of each tile-type
		info mask; // mask of each tile-type
		hex num; // number of 0~f tile-type
		hexa layout; // layout of board-type
		i32 moved; // moved or not
		u32 legal; // legal actions

		cache(const cache& c) = default;
		~cache() = default;

		static cache make(u32 r) {
			u32 raw = r & 0x0ffff;
			u32 ext = r & 0xf0000;

			move left = move::make(r, false);
			move right = move::make(r, true);

			u32 species = 0;
			info numof = {};
			info mask = {};
			hex num;
			for (int i = 0; i < 4; i++) {
				u32 tile = ((r >> (i << 2)) & 0x0f) | ((r >> (12 + i)) & 0x10);
				species |= (1 << tile);
				numof[tile]++;
				mask[tile] |= (1 << i);
				num[tile] = num[tile] + 1;
			}

			hexa layout;
			for (int i = 0; i < 16; i++) {
				if ((r >> i) & 1) layout.push_back(i);
			}

			u32 merge = left.merge | right.merge;
			i32 moved = left.moved & right.moved;

			u32 legal = 0;
			if (left.moved == 0)  legal |= (0x08 | 0x01);
			if (right.moved == 0) legal |= (0x02 | 0x04);

			return cache(raw, ext, species, merge, left, right, numof, mask, num, layout, moved, legal);
		}

	private:
		cache(u32 raw, u32 ext, u32 species, u32 merge, move left, move right,
			  info numof, info mask, hex num, hexa layout, i32 moved, u32 legal)
		: raw(raw), ext(ext), species(species), merge(merge), left(left), right(right),
		  numof(numof), mask(mask), num(num), layout(layout), moved(moved), legal(legal) {}
		cache() : cache(0, 0, 0, 0, {}, {}, {}, {}, {}, {}, -1, 0) {
			static u32 seq = 0;
			operator =(make(seq++));
		}
		cache& operator =(const cache& c) = default;

	};
	static const cache lookup[1 << 20];

private:
	u64 raw;
	u32 ext;
	u32 inf;

public:
	inline board(u64 raw = 0) : raw(raw), ext(0), inf(0) {}
	inline board(u64 raw, u32 ext) : raw(raw), ext(ext), inf(0) {}
	inline board(u64 raw, u16 ext) : board(raw, u32(ext) << 16) {}
	inline board(const board& b) = default;
	inline ~board() = default;
	inline board& operator =(u64 raw) { this->raw = raw; return *this; }
	inline board& operator =(const board& b) = default;

	inline operator u64() const { return raw; }
	inline operator bool() const { return raw | ext; }
	declare_comparators_with(const board&, raw_cast<u128>(*this), raw_cast<u128>(v))

	inline const cache& query(u32 r) const { return query16(r); }
	inline const cache& query16(u32 r) const { return board::lookup[fetch16(r)]; }
	inline const cache& query20(u32 r) const { return board::lookup[fetch20(r)]; }

	inline u32 fetch(u32 i) const { return fetch16(i); }
	inline u32 fetch16(u32 i) const {
		return ((raw >> (i << 4)) & 0xffff);
	}
	inline u32 fetch20(u32 i) const {
		return fetch16(i) | ((ext >> (i << 2)) & 0xf0000);
	}

	inline void place(u32 i, u32 r) { place16(i, r); }
	inline void place16(u32 i, u32 r) {
		raw = (raw & ~(0xffffULL << (i << 4))) | (u64(r & 0xffff) << (i << 4));
	}
	inline void place20(u32 i, u32 r) {
		place16(i, r & 0xffff);
		ext = (ext & ~(0xf0000 << (i << 2))) | ((r & 0xf0000) << (i << 2));
	}

	inline u32 at(u32 i) const { return at4(i); }
	inline u32 at4(u32 i) const {
		return (raw >> (i << 2)) & 0x0f;
	}
	inline u32 at5(u32 i) const {
		return at4(i) | ((ext >> (i + 12)) & 0x10);
	}

	inline void set(u32 i, u32 t) { set4(i, t); }
	inline void set4(u32 i, u32 t) {
		raw = (raw & ~(0x0fULL << (i << 2))) | (u64(t & 0x0f) << (i << 2));
	}
	inline void set5(u32 i, u32 t) {
		set4(i, t);
		ext = (ext & ~(1U << (i + 16))) | ((t & 0x10) << (i + 12));
	}

	inline void mirror() { mirror64(); }
	inline void mirror64() {
		raw = ((raw & 0x000f000f000f000fULL) << 12) | ((raw & 0x00f000f000f000f0ULL) << 4)
			| ((raw & 0x0f000f000f000f00ULL) >> 4) | ((raw & 0xf000f000f000f000ULL) >> 12);
	}
	inline void mirror80() {
		mirror64();
		ext = ((ext & 0x11110000) << 3) | ((ext & 0x22220000) << 1)
			| ((ext & 0x44440000) >> 1) | ((ext & 0x88880000) >> 3);
	}

	inline void flip() { flip64(); }
	inline void flip64() {
		raw = ((raw & 0x000000000000ffffULL) << 48) | ((raw & 0x00000000ffff0000ULL) << 16)
			| ((raw & 0x0000ffff00000000ULL) >> 16) | ((raw & 0xffff000000000000ULL) >> 48);
	}
	inline void flip80() {
		flip64();
		ext = ((ext & 0x000f0000) << 12) | ((ext & 0x00f00000) << 4)
			| ((ext & 0x0f000000) >> 4) | ((ext & 0xf0000000) >> 12);
	}

	inline void reverse() { reverse64(); }
	inline void reverse64() { mirror64(); flip64(); }
	inline void reverse80() { mirror80(); flip80(); }

	inline void transpose() { transpose64(); }
	inline void transpose64() {
		raw = (raw & 0xf0f00f0ff0f00f0fULL) | ((raw & 0x0000f0f00000f0f0ULL) << 12) | ((raw & 0x0f0f00000f0f0000ULL) >> 12);
		raw = (raw & 0xff00ff0000ff00ffULL) | ((raw & 0x00000000ff00ff00ULL) << 24) | ((raw & 0x00ff00ff00000000ULL) >> 24);
	}
	inline void transpose80() {
		transpose64();
		ext = (ext & 0xa5a50000) | ((ext & 0x0a0a0000) << 3) | ((ext & 0x50500000) >> 3);
		ext = (ext & 0xcc330000) | ((ext & 0x00cc0000) << 6) | ((ext & 0x33000000) >> 6);
	}

	inline u32 empty() const { return empty64(); }
	inline u32 empty64() const {
		register u64 x = raw;
		x |= (x >> 2);
		x |= (x >> 1);
		x = ~x & 0x1111111111111111ULL;
		x += x >> 32;
		x += x >> 16;
		x += x >> 8;
		x += x >> 4;
		return x & 0xf;
	}
	inline u32 empty80() const {
		register u64 x = raw;
		x |= (x >> 2);
		x |= (x >> 1);
		x = ~x & 0x1111111111111111ULL;
		register u64 k = ext >> 16;
		k = ((k & 0xff00ULL) << 24) | (k & 0x00ffULL);
		k = ((k & 0xf0000000f0ULL) << 12) | (k & 0x0f0000000fULL);
		k = ((k & 0x000c000c000c000cULL) << 6) | (k & 0x0003000300030003ULL);
		k = ((k & 0x0202020202020202ULL) << 3) | (k & 0x0101010101010101ULL);
		x &= ~k & 0x1111111111111111ULL;
		x += x >> 32;
		x += x >> 16;
		x += x >> 8;
		x += x >> 4;
		return x & 0xf;
	}

	inline hexa spaces() const { return spaces64(); }
	inline hexa spaces64() const { return find64(0); }
	inline hexa spaces80() const { return find80(0); }

	inline void init() {
		u32 k = std::rand();
		u32 i = (k) % 16;
		u32 j = (i + 1 + (k >> 4) % 15) % 16;
		u32 r = std::rand() % 100;
		raw =  (r >=  1 ? 1ULL : 2ULL) << (i << 2);
		raw |= (r >= 19 ? 1ULL : 2ULL) << (j << 2);
		ext = 0;
	}

	inline void next() { return next64(); }
	inline void next64() {
		hexa empty = spaces64();
		u32 p = empty[std::rand() % empty.size()];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
	}
	inline void next80() {
		hexa empty = spaces80();
		u32 p = empty[std::rand() % empty.size()];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
	}

	inline bool popup() { return popup64(); }
	inline bool popup64() {
		hexa empty = spaces64();
		if (empty.size() == 0) return false;
		u32 p = empty[std::rand() % empty.size()];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
		return true;
	}
	inline bool popup80() {
		hexa empty = spaces80();
		if (empty.size() == 0) return false;
		u32 p = empty[std::rand() % empty.size()];
		raw |= (std::rand() % 10 ? 1ULL : 2ULL) << (p << 2);
		return true;
	}

	inline void clear() {
		raw = 0;
		ext = 0;
	}

	inline void rotright()   { rotright64(); }
	inline void rotright64() { transpose64(); mirror64(); }
	inline void rotright80() { transpose80(); mirror80(); }

	inline void rotleft()   { rotleft64(); }
	inline void rotleft64() { transpose64(); flip64(); }
	inline void rotleft80() { transpose80(); flip80(); }

	inline void rotate(int r = 1) {
		rotate64(r);
	}
	inline void rotate64(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotright64(); break;
		case 2: reverse64(); break;
		case 3: rotleft64(); break;
		}
	}
	inline void rotate80(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotright80(); break;
		case 2: reverse80(); break;
		case 3: rotleft80(); break;
		}
	}

	inline void isomorphic(int i = 0) {
		return isomorphic64(i);
	}
	inline void isomorphic64(int i = 0) {
		if ((i % 8) / 4) mirror64();
		rotate64(i);
	}
	inline void isomorphic80(int i = 0) {
		if ((i % 8) / 4) mirror80();
		rotate80(i);
	}

	inline i32 left()  { return left64(); }
	inline i32 right() { return right64(); }
	inline i32 up()    { return up64(); }
	inline i32 down()  { return down64(); }

	inline i32 left64() {
		register u64 rawn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query16(0).left.moveh64<0>(rawn, score, moved);
		query16(1).left.moveh64<1>(rawn, score, moved);
		query16(2).left.moveh64<2>(rawn, score, moved);
		query16(3).left.moveh64<3>(rawn, score, moved);
		raw = rawn;
		return score | moved;
	}
	inline i32 right64() {
		register u64 rawn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query16(0).right.moveh64<0>(rawn, score, moved);
		query16(1).right.moveh64<1>(rawn, score, moved);
		query16(2).right.moveh64<2>(rawn, score, moved);
		query16(3).right.moveh64<3>(rawn, score, moved);
		raw = rawn;
		return score | moved;
	}
	inline i32 up64() {
		register u64 rawn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		transpose64();
		query16(0).left.movev64<0>(rawn, score, moved);
		query16(1).left.movev64<1>(rawn, score, moved);
		query16(2).left.movev64<2>(rawn, score, moved);
		query16(3).left.movev64<3>(rawn, score, moved);
		raw = rawn;
		return score | moved;
	}
	inline i32 down64() {
		register u64 rawn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		transpose64();
		query16(0).right.movev64<0>(rawn, score, moved);
		query16(1).right.movev64<1>(rawn, score, moved);
		query16(2).right.movev64<2>(rawn, score, moved);
		query16(3).right.movev64<3>(rawn, score, moved);
		raw = rawn;
		return score | moved;
	}

	inline i32 left80() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query20(0).left.moveh80<0>(rawn, extn, score, moved);
		query20(1).left.moveh80<1>(rawn, extn, score, moved);
		query20(2).left.moveh80<2>(rawn, extn, score, moved);
		query20(3).left.moveh80<3>(rawn, extn, score, moved);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 right80() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		query20(0).right.moveh80<0>(rawn, extn, score, moved);
		query20(1).right.moveh80<1>(rawn, extn, score, moved);
		query20(2).right.moveh80<2>(rawn, extn, score, moved);
		query20(3).right.moveh80<3>(rawn, extn, score, moved);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 up80() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		transpose80();
		query20(0).left.movev80<0>(rawn, extn, score, moved);
		query20(1).left.movev80<1>(rawn, extn, score, moved);
		query20(2).left.movev80<2>(rawn, extn, score, moved);
		query20(3).left.movev80<3>(rawn, extn, score, moved);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 down80() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		transpose80();
		query20(0).right.movev80<0>(rawn, extn, score, moved);
		query20(1).right.movev80<1>(rawn, extn, score, moved);
		query20(2).right.movev80<2>(rawn, extn, score, moved);
		query20(3).right.movev80<3>(rawn, extn, score, moved);
		raw = rawn;
		ext = extn;
		return score | moved;
	}

	class action {
	public:
		action() = delete;
		enum opcode : u32 {
			up    = 0x00u,
			right = 0x01u,
			down  = 0x02u,
			left  = 0x03u,
			next  = 0x0eu,
			init  = 0x0fu,

			x64   = 0x40u,
			x80   = 0x80u,
			ext   = x80,

			up64    = up | x64,
			right64 = right | x64,
			down64  = down | x64,
			left64  = left | x64,
			next64  = next | x64,
			init64  = init | x64,

			up80    = up | x80,
			right80 = right | x80,
			down80  = down | x80,
			left80  = left | x80,
			next80  = next | x80,
			init80  = init | x80,
		};
	};

	inline i32 operate(u32 op) {
		if (op & action::x64) return operate64(op);
		if (op & action::x80) return operate80(op);
        return operate64(op);
	}
	inline i32 operate64(u32 op) {
		switch (op & 0x0fu) {
		case action::up:    return up64();
		case action::right: return right64();
		case action::down:  return down64();
		case action::left:  return left64();
		case action::next:  return popup64() ? 0 : -1;
		case action::init:  return init(), 0;
		default:            return -1;
		}
	}
	inline i32 operate80(u32 op) {
		switch (op & 0x0fu) {
		case action::up:    return up80();
		case action::right: return right80();
		case action::down:  return down80();
		case action::left:  return left80();
		case action::next:  return popup80() ? 0 : -1;
		case action::init:  return init(), 0;
		default:            return -1;
		}
	}

	inline i32 move(u32 op)   { return operate(op); }
	inline i32 move64(u32 op) { return operate64(op); }
	inline i32 move80(u32 op) { return operate80(op); }

	inline u32 shift(u32 k = 0, u32 u = 0) { return shift64(k, u); }
	inline u32 shift64(u32 k = 0, u32 u = 0) {
		u32 hash = hash64();
		u32 tile = math::msb16(hash);
		u32 mask = (((k ? (1 << k) : tile) << 1) - 1) & ~((2 << u) - 1);
		u32 hole = ~hash & mask;
		if (hole == 0 || hole > tile) return 0;
		u32 h = math::lg16(hole);
		for (u32 i = 0; i < 16; i++) {
			u32 t = at4(i);
			set4(i, t > h ? t - 1 : t);
		}
		return h;
	}
	inline u32 shift80(u32 k = 0, u32 u = 0) {
		u32 hash = hash80();
		u32 tile = math::msb16(hash);
		u32 mask = (((k ? (1 << k) : tile) << 1) - 1) & ~((2 << u) - 1);
		u32 hole = ~hash & mask;
		if (hole == 0 || hole > tile) return 0;
		u32 h = math::lg16(hole);
		for (u32 i = 0; i < 16; i++) {
			u32 t = at5(i);
			set5(i, t > h ? t - 1 : t);
		}
		return h;
	}

	inline u32 species() const { return species64(); }
	inline u32 species64() const {
		return query16(0).species | query16(1).species | query16(2).species | query16(3).species;
	}
	inline u32 species80() const {
		return query20(0).species | query20(1).species | query20(2).species | query20(3).species;
	}

	inline u32 scale() const   { return scale64(); }
	inline u32 scale64() const { return species64(); }
	inline u32 scale80() const { return species80(); }

	inline u32 hash() const { return hash64(); }
	inline u32 hash64() const {
		register u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at4(i));
		return h;
	}
	inline u32 hash80() const {
		register u32 h = 0;
		for (u32 i = 0; i < 16; i++) h |= (1 << at5(i));
		return h;
	}

	inline u32 max()   const { return max64(); }
	inline u32 max64() const { return math::log2(scale64()); }
	inline u32 max80() const { return math::log2(scale80()); }

	inline hex numof() const {
		return query(0).num + query(1).num + query(2).num + query(3).num;
	}

	inline u32 numof(u32 t) const { return numof64(t); }
	inline u32 numof64(u32 t) const {
		return query16(0).numof[t] + query16(1).numof[t] + query16(2).numof[t] + query16(3).numof[t];
	}
	inline u32 numof80(u32 t) const {
		return query20(0).numof[t] + query20(1).numof[t] + query20(2).numof[t] + query20(3).numof[t];
	}

	inline void numof(u32 num[], u32 min, u32 max) const { return numof64(num, min, max); }
	inline void numof64(u32 num[], u32 min, u32 max) const {
		const cache::info& numof0 = query16(0).numof;
		const cache::info& numof1 = query16(1).numof;
		const cache::info& numof2 = query16(2).numof;
		const cache::info& numof3 = query16(3).numof;
		for (u32 i = min; i < max; i++) {
			num[i] = numof0[i] + numof1[i] + numof2[i] + numof3[i];
		}
	}
	inline void numof80(u32 num[], u32 min, u32 max) const {
		const cache::info& numof0 = query20(0).numof;
		const cache::info& numof1 = query20(1).numof;
		const cache::info& numof2 = query20(2).numof;
		const cache::info& numof3 = query20(3).numof;
		for (u32 i = min; i < max; i++) {
			num[i] = numof0[i] + numof1[i] + numof2[i] + numof3[i];
		}
	}

	inline u32 count(u32 t) const { return count64(t); }
	inline u32 count64(u32 t) const {
		register u32 num = 0;
		for (u32 i = 0; i < 16; i++)
			if (at4(i) == t) num++;
		return num;
	}
	inline u32 count80(u32 t) const {
		register u32 num = 0;
		for (u32 i = 0; i < 16; i++)
			if (at5(i) == t) num++;
		return num;
	}

	inline void count(u32 num[], u32 min, u32 max) const { return count64(num, min, max); }
	inline void count64(u32 num[], u32 min, u32 max) const {
		std::fill(num + min, num + max, 0);
		for (u32 i = 0; i < 16; i++)
			num[at4(i)]++;
	}
	inline void count80(u32 num[], u32 min, u32 max) const {
		std::fill(num + min, num + max, 0);
		for (u32 i = 0; i < 16; i++)
			num[at5(i)]++;
	}

	inline u32 mask(u32 t) const { return mask64(t); }
	inline u32 mask64(u32 t) const {
		return (query16(0).mask[t] << 0) | (query16(1).mask[t] << 4) | (query16(2).mask[t] << 8) | (query16(3).mask[t] << 12);
	}
	inline u32 mask80(u32 t) const {
		return (query20(0).mask[t] << 0) | (query20(1).mask[t] << 4) | (query20(2).mask[t] << 8) | (query20(3).mask[t] << 12);
	}

	inline void mask(u32 msk[], u32 min, u32 max) const { return mask64(msk, min, max); }
	inline void mask64(u32 msk[], u32 min, u32 max) const {
		const cache::info& mask0 = query16(0).mask;
		const cache::info& mask1 = query16(1).mask;
		const cache::info& mask2 = query16(2).mask;
		const cache::info& mask3 = query16(3).mask;
		for (u32 i = min; i < max; i++) {
			msk[i] = (mask0[i] << 0) | (mask1[i] << 4) | (mask2[i] << 8) | (mask3[i] << 12);
		}
	}
	inline void mask80(u32 msk[], u32 min, u32 max) const {
		const cache::info& mask0 = query20(0).mask;
		const cache::info& mask1 = query20(1).mask;
		const cache::info& mask2 = query20(2).mask;
		const cache::info& mask3 = query20(3).mask;
		for (u32 i = min; i < max; i++) {
			msk[i] = (mask0[i] << 0) | (mask1[i] << 4) | (mask2[i] << 8) | (mask3[i] << 12);
		}
	}

	inline hexa find(u32 t) const { return find64(t); }
	inline hexa find64(u32 t) const { return board::lookup[mask64(t)].layout; }
	inline hexa find80(u32 t) const { return board::lookup[mask80(t)].layout; }

	inline u64 monoleft() const { return monoleft64(); }
	inline u64 monoleft64() const {
		register u64 mono = 0;
		mono |= u64(query16(0).left.mono) <<  0;
		mono |= u64(query16(1).left.mono) << 12;
		mono |= u64(query16(2).left.mono) << 24;
		mono |= u64(query16(3).left.mono) << 36;
		return mono;
	}
	inline u64 monoleft80() const {
		register u64 mono = 0;
		mono |= u64(query20(0).left.mono) <<  0;
		mono |= u64(query20(1).left.mono) << 12;
		mono |= u64(query20(2).left.mono) << 24;
		mono |= u64(query20(3).left.mono) << 36;
		return mono;
	}

	inline u64 monoright() const { return monoright64(); }
	inline u64 monoright64() const {
		register u64 mono = 0;
		mono |= u64(query16(0).right.mono) <<  0;
		mono |= u64(query16(1).right.mono) << 12;
		mono |= u64(query16(2).right.mono) << 24;
		mono |= u64(query16(3).right.mono) << 36;
		return mono;
	}
	inline u64 monoright80() const {
		register u64 mono = 0;
		mono |= u64(query20(0).right.mono) <<  0;
		mono |= u64(query20(1).right.mono) << 12;
		mono |= u64(query20(2).right.mono) << 24;
		mono |= u64(query20(3).right.mono) << 36;
		return mono;
	}

	inline u64 monotonic(bool left = true) const   { return left ? monoleft() : monoright(); }
	inline u64 monotonic64(bool left = true) const { return left ? monoleft64() : monoright64(); }
	inline u64 monotonic80(bool left = true) const { return left ? monoleft80() : monoright80(); }

	inline u32 operations() const { return operations64(); }
	inline u32 operations64() const {
		board trans(*this); trans.transpose64();
		u32 hori = this->query16(0).legal | this->query16(1).legal | this->query16(2).legal | this->query16(3).legal;
		u32 vert = trans.query16(0).legal | trans.query16(1).legal | trans.query16(2).legal | trans.query16(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}
	inline u32 operations80() const {
		board trans(*this); trans.transpose80();
		u32 hori = this->query20(0).legal | this->query20(1).legal | this->query20(2).legal | this->query20(3).legal;
		u32 vert = trans.query20(0).legal | trans.query20(1).legal | trans.query20(2).legal | trans.query20(3).legal;
		return (hori & 0x0a) | (vert & 0x05);
	}

	inline hexa actions() const { return actions64(); }
	inline hexa actions64() const {
		u32 o = operations64();
		u32 k = 0, x = 0;
		u32 u = o & 1;
		k |= (u ? 0 : 0) << x;
		x += (u << 2);
		u32 r = o & 2;
		k |= (r ? 1 : 0) << x;
		x += (r << 1);
		u32 d = o & 4;
		k |= (d ? 2 : 0) << x;
		x += (d >> 0);
		u32 l = o & 8;
		k |= (l ? 3 : 0) << x;
		x += (l >> 1);
		return { k, x >> 2 };
	}
	inline hexa actions80() const {
		u32 o = operations80();
		u32 k = 0, x = 0;
		u32 u = o & 1;
		k |= (u ? 0 : 0) << x;
		x += (u << 2);
		u32 r = o & 2;
		k |= (r ? 1 : 0) << x;
		x += (r << 1);
		u32 d = o & 4;
		k |= (d ? 2 : 0) << x;
		x += (d >> 0);
		u32 l = o & 8;
		k |= (l ? 3 : 0) << x;
		x += (l >> 1);
		return { k, x >> 2 };
	}

	inline bool operable() const { return operable64(); }
	inline bool operable64() const {
		if (this->query16(0).moved == 0) return true;
		if (this->query16(1).moved == 0) return true;
		if (this->query16(2).moved == 0) return true;
		if (this->query16(3).moved == 0) return true;
		board trans(*this); trans.transpose64();
		if (trans.query16(0).moved == 0) return true;
		if (trans.query16(1).moved == 0) return true;
		if (trans.query16(2).moved == 0) return true;
		if (trans.query16(3).moved == 0) return true;
		return false;
	}
	inline bool operable80() const {
		if (this->query20(0).moved == 0) return true;
		if (this->query20(1).moved == 0) return true;
		if (this->query20(2).moved == 0) return true;
		if (this->query20(3).moved == 0) return true;
		board trans(*this); trans.transpose80();
		if (trans.query20(0).moved == 0) return true;
		if (trans.query20(1).moved == 0) return true;
		if (trans.query20(2).moved == 0) return true;
		if (trans.query20(3).moved == 0) return true;
		return false;
	}

	inline bool movable() const   { return operable(); }
	inline bool movable64() const { return operable64(); }
	inline bool movable80() const { return operable80(); }

	class tile {
	friend class board;
	private:
		inline tile(const board& b, u32 i) : b(const_cast<board&>(b)), i(i) {}
	public:
		inline tile(const tile& t) = default;
		inline tile() = delete;
		inline board& whole() const { return b; }
		inline u32 where() const { return i; }

		inline operator u32() const { return at(is(style::extend), is(style::exact)); }
		inline tile& operator =(u32 k) { set(k, is(style::extend), is(style::exact)); return *this; }
		declare_comparators_with(u32, operator u32(), v);
	public:
		friend std::ostream& operator <<(std::ostream& out, const tile& t) {
			u32 v = t.at(t.is(style::extend), !t.is(style::binary) && t.is(style::exact));
			return t.is(style::binary) ? moporgic::write_cast<byte>(out, v) : (out << v);
		}
		friend std::istream& operator >>(std::istream& in, const tile& t) {
			u32 v;
			if (t.is(style::binary) ? moporgic::read_cast<byte>(in, v) : (in >> v))
				t.set(v, t.is(style::extend), !t.is(style::binary) && t.is(style::exact));
			return in;
		}
	public:
		typedef std::ptrdiff_t difference_type;
		typedef u32 value_type;
		typedef tile& reference;
		typedef tile pointer;
		typedef std::forward_iterator_tag iterator_category;
		tile& operator *() { return *this; }
		const tile& operator *() const { return *this; }
		tile  operator->() const { return *this; }
		bool  operator==(const tile& t) const { return ((b == t.b) & (i == t.i)); }
		bool  operator!=(const tile& t) const { return ((b != t.b) | (i != t.i)); }
		bool  operator< (const tile& t) const { return ((b == t.b) & (i < t.i)) | (b < t.b); }
		tile& operator++() { ++i; return *this; }
		tile& operator--() { --i; return *this; }
		tile  operator++(int) { return tile(b, ++i - 1); }
		tile  operator--(int) { return tile(b, --i + 1); }
	private:
		board& b;
		u32 i;

		bool is(u32 item) const { return b.inf & item; }

		u32 at(bool extend, bool exact) const {
			u32 v = extend ? b.at5(i) : b.at4(i);
			return exact ? (1 << v) & -2u : v;
		}
		void set(u32 k, bool extend, bool exact) const {
			u32 v = exact ? math::lg(k) : k;
			if (extend) b.set5(i, v); else b.set4(i, v);
		}
	};
	inline tile operator [](u32 i) const { return tile(*this, i); }
	inline tile begin() const { return operator [](0); }
	inline tile end() const { return operator [](16); }

	class style {
	public:
		style() = delete;
		enum fmtcode : u32 {
			index  = 0x00000000u,
			exact  = 0x10000000u,
			alter  = 0x20000000u,
			binary = 0x40000000u,
			extend = 0x80000000u,
			full   = 0xf0000000u,

			at     = index,
			at4    = index,
			at5    = index | extend,
			ext    = extend,
			exact4 = index | exact,
			exact5 = index | exact | extend,
			actual = index | exact | extend,
			lite   = alter,
			lite64 = alter,
			lite80 = alter | extend,
			raw    = binary,
			raw64  = binary,
			raw80  = binary | extend,
		};
	};
	inline board& format(u32 i = style::index) { info((i & style::full) | (inf & ~style::full)); return *this; }

	inline u32 info(u32 i) { return std::exchange(inf, i); }
	inline u32 info() const { return inf; }

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
const board::cache board::lookup[1 << 20];

} // namespace moporgic
