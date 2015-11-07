#pragma once
#include "moporgic/type.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <cstdio>

namespace moporgic {

class board {
public:
	template<typename Tis>
	struct tiles {
		Tis tile;
		u32 size;
		tiles(const Tis& t, const u32& s) : tile(t), size(s) {}
		tiles(const tiles<Tis>& t) : tile(t.tile), size(t.size) {}
		tiles() : tile(0), size(0) {}
	};
	class cache {
	friend class board;
	public:
		struct oper {
			u32 moveHraw; // horizontal move (16-bit raw)
			u32 moveHext; // horizontal move (4-bit extra)
			u64 moveVraw; // vertical move (64-bit raw)
			u32 moveVext; // vertical move (16-bit extra)
			u32 score; // merge score
			i32 moved; // moved or not (moved: 0, otherwise -1)

			inline void moveH(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) { moveH64(raw, ext, sc, mv, i); }
			inline void moveV(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) { moveV64(raw, ext, sc, mv, i); }

			inline void moveH64(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				raw |= u64(moveHraw) << (i << 4);
				sc += score;
				mv &= moved;
			}
			inline void moveH80(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				moveH64(raw, ext, sc, mv, i);
				ext |= moveHext << (i << 2);
			}
			inline void moveV64(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				raw |= moveVraw << (i << 2);
				sc += score;
				mv &= moved;
			}
			inline void moveV80(u64& raw, u32& ext, u32& sc, i32& mv, const int& i) const {
				moveV64(raw, ext, sc, mv, i);
				ext |= moveVext << i;
			}
		};
		u32 rowraw; // base row (16-bit raw)
		u32 rowext; // base row (4-bit extra)
		u32 maxtile; // max tile
		u32 merged; // number of merged tiles
		oper left; // left operation
		oper right; // right operation
		u16 count[32]; // number of each tile-type
		u16 mask[32]; // mask of each tile-type
		tiles<u64> layout; // layout of board-type
	private:
		~cache() {}
		cache() : rowraw(0), rowext(0), maxtile(0), merged(0), count(), mask() {}
		cache(const u32& r) : count(), mask() {
			// HIGH [null][N0~N3 high 1-bit (totally 4-bit)][N0~N3 low 4-bit (totally 16-bit)] LOW

			u32 V[4] = {((r >> 0) & 0x0f) | ((r >> 12) & 0x10), ((r >> 4) & 0x0f) | ((r >> 13) & 0x10),
						((r >> 8) & 0x0f) | ((r >> 14) & 0x10), ((r >> 12) & 0x0f) | ((r >> 15) & 0x10)};
			u32 L[4] = { V[0], V[1], V[2], V[3] }, Ll[4], Lh[4];
			u32 R[4] = { V[3], V[2], V[1], V[0] }, Rl[4], Rh[4]; // mirrored

			assign(L, Ll, Lh, rowraw, rowext);
			maxtile = (1 << *std::max_element(V, V + 4)) & 0xfffffffeU;
			for (int i = 0; i < 4; i++) {
				count[V[i]]++;
				mask[V[i]] |= (1 << i);
			}
			for (int i = 0; i < 16; i++) {
				if ((r >> i) & 1) layout.tile |= (u64(i) << ((layout.size++) << 2));
			}

			mvleft(L, left.score, merged);
			u32 mvL = assign(L, Ll, Lh, left.moveHraw, left.moveHext);
			std::reverse(Ll, Ll + 4); std::reverse(Lh, Lh + 4);
			left.moved = mvL == r ? -1 : 0;
			map(left.moveVraw, left.moveVext, Ll, Lh, 12, 8, 4, 0);

			mvleft(R, right.score, merged); std::reverse(R, R + 4);
			u32 mvR = assign(R, Rl, Rh, right.moveHraw, right.moveHext);
			std::reverse(Rl, Rl + 4); std::reverse(Rh, Rh + 4);
			right.moved = mvR == r ? -1 : 0;
			map(right.moveVraw, right.moveVext, Rl, Rh, 12, 8, 4, 0);
		}

		u32 assign(u32 src[], u32 lo[], u32 hi[], u32& raw, u32& ext) {
			for (u32 i = 0; i < 4; i++) {
				hi[i] = (src[i] & 0x10) >> 4;
				lo[i] = (src[i] & 0x0f);
			}
			raw = ((lo[0] << 0) | (lo[1] << 4) | (lo[2] << 8) | (lo[3] << 12));
			ext = ((hi[0] << 0) | (hi[1] << 1) | (hi[2] << 2) | (hi[3] << 3)) << 16;
			return raw | ext;
		}
		void map(u64& raw, u32& ext, u32 lo[], u32 hi[], int s0, int s1, int s2, int s3) {
			raw = (u64(lo[0]) << (s0 << 2)) | (u64(lo[1]) << (s1 << 2))
				| (u64(lo[2]) << (s2 << 2)) | (u64(lo[3]) << (s3 << 2));
			ext = ((hi[0] << s0) | (hi[1] << s1) | (hi[2] << s2) | (hi[3] << s3)) << 16;
		}
		void mvleft(u32 row[], u32& score, u32& merge) {
			u32 top = 0;
			u32 tmp = 0;
			score = merge = 0;
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
	static cache look[1 << 20];

	u64 raw;
	u64 rawc;
	u32 ext;
	u32 extc;

public:
	board(const u64& raw = 0) : raw(raw), rawc(0), ext(0), extc(0) {}
	board(const board& b) : raw(b.raw), rawc(b.rawc), ext(b.ext), extc(b.extc) {}
	inline board& operator =(const u64& raw) { this->raw = raw; return *this; }
	inline board& operator =(const board& b) { raw = b.raw, ext = b.ext; rawc = b.rawc, extc = b.extc; return *this; }
	inline bool operator==(const board& b) const { return raw == b.raw && ext == b.ext; }
	inline bool operator!=(const board& b) const { return raw != b.raw || ext != b.ext; }
	inline bool operator==(const u64& raw) const { return this->raw == raw && this->ext == 0; }
	inline bool operator!=(const u64& raw) const { return this->raw != raw || this->ext != 0; }

	inline u32 fetch(const u32& i) const { return fetch16(i); }
	inline u32 at(const u32& i) const { return at4(i); }
	inline void set(const u32& i, const u32& t) { set4(i, t); }
	inline void mirror() { mirror64(); }
	inline void flip() { flip64(); }
	inline void transpose() { transpose64(); }

	inline u32 fetch16(const u32& i) const {
		return ((raw >> (i << 4)) & 0xffff);
	}
	inline u32 fetch20(const u32& i) const {
		return fetch16(i) | ((ext >> (i << 2)) & 0xf0000);
	}

	inline u32 at4(const u32& i) const {
		return (fetch16(i >> 2) >> ((i % 4) << 2)) & 0x0f;
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
		const u32 r = rand();
		const u32 i = (r) & 0x0f;
		const u32 j = (i + 1 + (r >> 4) % 15) & 0x0f;
		raw = (1ULL << (i << 2)) | (1ULL << (j << 2));
		ext = 0;
	}
	inline tiles<u64> spaces() const {
		return find(0);
	}
	inline bool next() {
		tiles<u64> empty = spaces();
		if (empty.size == 0) return false;
		u32 p = ((empty.tile >> ((rand() % empty.size) << 2)) & 0x0f) << 2;
		raw |= u64(rand() % 10 ? 1 : 2) << p;
		return true;
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

	inline i32 left() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		lookup(0).left.moveH(rawn, extn, score, moved, 0);
		lookup(1).left.moveH(rawn, extn, score, moved, 1);
		lookup(2).left.moveH(rawn, extn, score, moved, 2);
		lookup(3).left.moveH(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}
	inline i32 right() {
		register u64 rawn = 0;
		register u32 extn = 0;
		register u32 score = 0;
		register i32 moved = -1;
		lookup(0).right.moveH(rawn, extn, score, moved, 0);
		lookup(1).right.moveH(rawn, extn, score, moved, 1);
		lookup(2).right.moveH(rawn, extn, score, moved, 2);
		lookup(3).right.moveH(rawn, extn, score, moved, 3);
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
		lookup(0).left.moveV(rawn, extn, score, moved, 0);
		lookup(1).left.moveV(rawn, extn, score, moved, 1);
		lookup(2).left.moveV(rawn, extn, score, moved, 2);
		lookup(3).left.moveV(rawn, extn, score, moved, 3);
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
		lookup(0).right.moveV(rawn, extn, score, moved, 0);
		lookup(1).right.moveV(rawn, extn, score, moved, 1);
		lookup(2).right.moveV(rawn, extn, score, moved, 2);
		lookup(3).right.moveV(rawn, extn, score, moved, 3);
		raw = rawn;
		ext = extn;
		return score | moved;
	}

	inline void mark() {
		rawc = raw;
		extc = ext;
	}
	inline void reset() {
		raw = rawc;
		ext = extc;
	}

	inline u32 max() const {
		return math::log2(lookup(0).maxtile | lookup(1).maxtile
						| lookup(2).maxtile | lookup(3).maxtile);
	}

	inline u32 count(const u32& t) const {
		return lookup(0).count[t] + lookup(1).count[t]
			 + lookup(2).count[t] + lookup(3).count[t];
	}
	inline void count(u16 num[32], const u32& min = 0, const u32& max = 32) const {
		const u16* count0 = lookup(0).count;
		const u16* count1 = lookup(1).count;
		const u16* count2 = lookup(2).count;
		const u16* count3 = lookup(3).count;
		for (u32 i = min; i < max; i++) {
			num[i] = count0[i] + count1[i] + count2[i] + count3[i];
		}
	}

	inline tiles<u64> find(const u32& t) const {
		u32 mask = (lookup(0).mask[t] << 0) | (lookup(1).mask[t] << 4)
				 | (lookup(2).mask[t] << 8) | (lookup(3).mask[t] << 12);
		return look[mask].layout;
	}

	inline cache& lookup(const u32& r) const {
		return board::look[fetch(r)];
	}

	static void initialize() {
		for (u32 i = 0; i < (1 << 20); i++)
			board::look[i] = cache(i);
	}
	static void print(const board& b, const bool& fib = false) {
		static u32 T[16];
		bool width = false;
		for (int i = 0; i < 16; i++) {
			u32 t = b.at(i);
			T[i] = fib ? ((1 << t) & 0xfffffffeUL) : t;
			width |= T[i] >= 10000;
		}
		const char* format = (width) ? "|%8d%8d%8d%8d|" : "|%4d%4d%4d%4d|";
		const char* edge = (width) ? "+--------------------------------+" : "+----------------+";
		static char buff[40];
		std::cout << edge << std::endl;
		for (u32 *t = T; t < T + 16; t += 4) {
			std::snprintf(buff, sizeof(buff), format, t[0], t[1], t[2], t[3]);
			std::cout << buff << std::endl;
		}
		std::cout << edge << std::endl;
	}
	static std::vector<std::function<i32(void)>> moves(board& b) {
		static i32 (board::*oper[])() = {&board::up, &board::right, &board::down, &board::left};
		std::vector<std::function<i32(void)>> opers = {
				std::bind(oper[0], &b), std::bind(oper[1], &b),
				std::bind(oper[2], &b), std::bind(oper[3], &b)};
		return opers;
	}
};
board::cache board::look[1 << 20];

} // namespace moporgic
