#pragma once
#include "moporgic/type.h"
#include "moporgic/util.h"
#include <algorithm>
#include <iostream>
#include <array>

namespace moporgic {

class board {
public:
	template<typename Tis>
	struct tiles {
		Tis tile;
		u32 size;
		tiles(const Tis& t, const u32& s) : tile(t), size(s) {}
		tiles(const tiles<Tis>& t) = default;
		tiles() : tile(0), size(0) {}
		~tiles() = default;
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
			operation(u32 rawh, u32 exth, u64 rawv, u32 extv, u32 score, i32 moved)
				: rawh(rawh), exth(exth), rawv(rawv), extv(extv), score(score), moved(moved) {}
		};
		typedef std::array<u16, 32> info;
		const u32 raw; // base row (16-bit raw)
		const u32 ext; // base row (4-bit extra)
		const u32 hash; // hash of this row
		const u32 merge; // number of merged tiles
		const operation left; // left operation
		const operation right; // right operation
		const info count; // number of each tile-type
		const info mask; // mask of each tile-type
		const tiles<u64> pos; // layout of board-type

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

			mvleft(L, score, merge);
			u32 mvL = assign(L, Ll, Lh, hraw, hext);
			std::reverse(Ll, Ll + 4); std::reverse(Lh, Lh + 4);
			moved = mvL == r ? -1 : 0;
			map(vraw, vext, Ll, Lh, 12, 8, 4, 0);
			operation left(hraw, hext, vraw, vext, score, moved);

			mvleft(R, score, merge); std::reverse(R, R + 4);
			u32 mvR = assign(R, Rl, Rh, hraw, hext);
			std::reverse(Rl, Rl + 4); std::reverse(Rh, Rh + 4);
			moved = mvR == r ? -1 : 0;
			map(vraw, vext, Rl, Rh, 12, 8, 4, 0);
			operation right(hraw, hext, vraw, vext, score, moved);

			u32 hash = 0;
			info count = {};
			info mask = {};
			for (int i = 0; i < 4; i++) {
				hash |= (1 << V[i]);
				count[V[i]]++;
				mask[V[i]] |= (1 << i);
			}

			u64 ltile = 0;
			u32 lsize = 0;
			for (int i = 0; i < 16; i++) {
				if ((r >> i) & 1) ltile |= (u64(i) << ((lsize++) << 2));
			}
			tiles<u64> pos(ltile, lsize);

			return cache(raw, ext, hash, merge, left, right, count, mask, pos);
		}
	private:
		cache(u32 raw, u32 ext, u32 hash, u32 merge, operation left, operation right, info count, info mask, tiles<u64> pos)
				: raw(raw), ext(ext), hash(hash), merge(merge),
				  left(left), right(right), count(count), mask(mask), pos(pos) {}

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
		static void mvleft(u32 row[], u32& score, u32& merge) {
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

	inline static const cache& lookup(const u32& i) {
		static cache look[1 << 20](cache::make(seq32_static()));
		return look[i];
	}

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
	inline u32 exact(const u32& i) const { return (1 << at(i)) & 0xfffffffe; }
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

	inline void mark() {
		rawc = raw;
		extc = ext;
	}
	inline void reset() {
		raw = rawc;
		ext = extc;
	}

	inline u32 hash() const {
		return query(0).hash | query(1).hash | query(2).hash | query(3).hash;
	}
	inline u32 max() const {
		return math::log2(hash());
	}

	inline u32 count(const u32& t) const {
		return query(0).count[t] + query(1).count[t] + query(2).count[t] + query(3).count[t];
	}
	inline void count(u16 num[32], const u32& min = 0, const u32& max = 32) const {
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
	inline void mask(u16 msk[32], const u32& min = 0, const u32& max = 32) const {
		const cache::info& mask0 = query(0).mask;
		const cache::info& mask1 = query(1).mask;
		const cache::info& mask2 = query(2).mask;
		const cache::info& mask3 = query(3).mask;
		for (u32 i = min; i < max; i++) {
			msk[i] = (mask0[i] << 0) | (mask1[i] << 4) | (mask2[i] << 8) | (mask3[i] << 12);
		}
	}

	inline tiles<u64> find(const u32& t) const {
		return board::lookup(mask(t)).pos;
	}

	inline const cache& query(const u32& r) const {
		return board::lookup(fetch(r));
	}

	inline void operator >>(std::ostream& out) const {
		moporgic::write(out, raw);
		moporgic::write(out, ext);
	}

	inline void operator <<(std::istream& in) {
		moporgic::read(in, raw);
		moporgic::read(in, ext);
	}

	void print(const bool& raw = true, std::ostream& out = std::cout) const {
		const char* format = raw ? "|%4d%4d%4d%4d|" : "|%6d%6d%6d%6d|";
		const char* edge = raw ? "+----------------+" : "+------------------------+";
		auto get = raw ? &board::at : &board::exact;
		static char buff[40];
		out << edge << std::endl;
		for (int i = 0; i < 16; i += 4) {
			u32 t0 = (this->*get)(i + 0);
			u32 t1 = (this->*get)(i + 1);
			u32 t2 = (this->*get)(i + 2);
			u32 t3 = (this->*get)(i + 3);
			std::snprintf(buff, sizeof(buff), format, t0, t1, t2, t3);
			out << buff << std::endl;
		}
		out << edge << std::endl;
	}
};

} // namespace moporgic
