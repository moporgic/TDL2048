//============================================================================
// Name        : 2048.cpp
// Author      : Hung Guei
// Version     : alpha
// Description : 2048
//============================================================================

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "moporgic/type.h"
#include "moporgic/util.h"
#include "moporgic/math.h"
#include "moporgic/io.h"
#include "board.h"
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <cmath>
#include <ctime>
#include <numeric>
#include <string>
#include <thread>
#include <limits>

namespace moporgic {

typedef double numeric;
numeric alpha = 0.0025;

class weight {
public:
	weight() : sign(0), value(nullptr), size(0) {}
	weight(const weight& w) : sign(w.sign), value(w.value), size(w.size) {}
	~weight() {}

	u32 signature() const { return sign; }
	numeric& operator [](const u64& i) { return value.get()[i]; }
	size_t length() const { return size; }

	template<typename rxx> void write(std::ostream& out) {
		numeric *v = value.get();
		for (u64 i = 0; i < size; i++)
			out.write(rxx(v[i]).le(), sizeof(rxx));
	}
	template<typename rxx> void read(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		numeric *v = value.get();
		for (u64 i = 0; i < size; i++)
			v[i] = rxx(load(sizeof(rxx))).le();
	}

	void operator >>(std::ostream& out) {
		const int LE = moporgic::endian::le;
		const char serial = 1;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(sign).endian(LE), 4);
			out.write(r64(size).endian(LE), 8);
			for (u64 i = 0; i < size; i++)
				out.write(r32(value.get()[i]).endian(LE), 4);
			break;
		case 1:
			out.write(r32(sign).le(), 4);
			out.write(r64(size).le(), 8);
			switch (sizeof(numeric)) {
			case 4: write<r32>(out);
				break;
			case 8: write<r64>(out);
				break;
			}
			break;
		default:
			std::cerr << "unknown serial at weight::>>" << std::endl;
			break;
		}
	}
	void operator <<(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		const int LE = moporgic::endian::le;
		switch (*load(1)) {
		case 0:
			sign = r32(load(4), LE);
			size = r64(load(8), LE);
			value = std::shared_ptr<numeric>(new numeric[size]());
			for (u64 i = 0; i < size; i++)
				value.get()[i] = r32(load(4), LE);
			break;
		case 1:
			sign = r32(load(4)).le();
			size = r64(load(8)).le();
			value = std::shared_ptr<numeric>(new numeric[size]());
			switch (sizeof(numeric)) {
			case 4: read<r32>(in);
				break;
			case 8: read<r64>(in);
				break;
			}
			break;
		default:
			std::cerr << "unknown serial at weight::<<" << std::endl;
			break;
		}
	}

	static void save(std::ostream& out) {
		const int LE = moporgic::endian::le;
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(cache().size()).endian(LE), 4);
			for (auto iter = cache().begin(); iter != cache().end(); iter++)
				iter->second >> out;
			break;
		default:
			break;
		}
		out.flush();
	}

	static void load(std::istream& in) {
		const int LE = moporgic::endian::le;
		char buf[8];
		char serial;
		in.read(&serial, 1);
		switch (serial) {
		case 0:
			in.read(buf, 4);
			for (u32 size = r32(buf, LE); size; size--) {
				weights().push_back(weight());
				weights().back() << in;
				cache()[weights().back().sign] = weights().back();
			}
			break;
		default:
			break;
		}
	}

	static bool save(const std::string& path) {
		std::ofstream out;
		char buf[1 << 20];
		out.rdbuf()->pubsetbuf(buf, sizeof(buf));
		out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return false;
		weight::save(out);
		out.flush();
		out.close();
		return true;
	}
	static bool load(const std::string& path) {
		std::ifstream in;
		char buf[1 << 20];
		in.rdbuf()->pubsetbuf(buf, sizeof(buf));
		in.open(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) return false;
		weight::load(in);
		in.close();
		return true;
	}

	static weight make(const u32& sign, const u64& size) {
		weights().push_back(weight(sign, size));
		cache()[sign] = weights().back();
		return weights().back();
	}
	static weight find(const u32& sign) { return cache().at(sign); }
	static std::vector<weight>::iterator begin() { return weights().begin(); }
	static std::vector<weight>::iterator end() { return weights().end(); }
private:
	weight(const u32& sign, const u64& size)
			: sign(sign), value(new numeric[size]()), size(size) {}
	static inline std::map<u32, weight>& cache() { static std::map<u32, weight> m; return m; }
	static inline std::vector<weight>& weights() { static std::vector<weight> w; return w; }

	u32 sign;
	std::shared_ptr<numeric> value;
	u64 size;
};

class indexer {
public:
	indexer() : sign(0), map(nullptr) {}
	indexer(const indexer& i) : sign(i.sign), map(i.map) {}
	~indexer() {}

	u32 signature() const { return sign; }
	inline u64 operator ()(const board& b) const { return map(b); }

	typedef std::function<u64(const board&)> mapper;
	static indexer make(const u32& sign, mapper map) {
		indexers().push_back(indexer(sign, map));
		cache()[sign] = indexers().back();
		return indexers().back();
	}
	static indexer find(const u32& sign) { return cache().at(sign); }
	static inline std::vector<indexer>::iterator begin() { return indexers().begin(); }
	static inline std::vector<indexer>::iterator end() { return indexers().end(); }
	static inline u64 size() { return indexers().size(); }
private:
	indexer(const u32& sign, mapper map) : sign(sign), map(map) {}
	static inline std::map<u32, indexer>& cache() { static std::map<u32, indexer> c; return c; }
	static inline std::vector<indexer>& indexers() { static std::vector<indexer> i; return i; }

	u32 sign;
	mapper map;
};

class feature {
public:
	feature() {}
	feature(const feature& t) : index(t.index), value(t.value) {}
	~feature() {}

	u64 signature() const {
		return (u64(value.signature()) << 32) | index.signature();
	}
	inline numeric& operator [](const board& b) { return value[index(b)]; }
	inline numeric& operator [](const u64& idx) { return value[idx]; }
	inline u64 operator ()(const board& b) const { return index(b); }
	operator indexer() const { return index; }
	operator weight() const { return value; }

	void operator >>(std::ostream& out) {
		const int LE = moporgic::endian::le;
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(index.signature()).endian(LE), 4);
			out.write(r32(value.signature()).endian(LE), 4);
			break;
		default:
			break;
		}
	}
	void operator <<(std::istream& in) {
		char buf[8];
		auto load = [&](u32 len) -> char* {
			if (!in.read(buf, len)) {
				std::cerr << "load failure" << std::endl;
				std::exit(2);
			}
			return buf;
		};
		const int LE = moporgic::endian::le;
		switch (*load(1)) {
		case 0:
			index = indexer::find(r32(load(4), LE));
			value = weight::find(r32(load(4), LE));
			break;
		default:
			break;
		}
	}

	static void save(std::ostream& out) {
		const int LE = moporgic::endian::le;
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(feats().size()).endian(LE), 4);
			for (u32 i = 0, size = feats().size(); i < size; i++)
				feats()[i] >> out;
			break;
		default:
			break;
		}
		out.flush();
	}

	static void load(std::istream& in) {
		const int LE = moporgic::endian::le;
		char buf[4];
		char serial;
		in.read(&serial, 1);
		switch (serial) {
		case 0:
			in.read(buf, 4);
			for (u32 size = r32(buf, LE); size; size--) {
				feats().push_back(feature());
				feats().back() << in;
			}
			break;
		default:
			break;
		}
	}

	static bool save(const std::string& path) {
		std::ofstream out;
		out.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return false;
		feature::save(out);
		out.flush();
		out.close();
		return true;
	}
	static bool load(const std::string& path) {
		std::ifstream in;
		in.open(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) return false;
		feature::load(in);
		in.close();
		return true;
	}

	static feature make(const u32& idx, const u32& wgt) {
		feats().push_back(feature(indexer::find(idx), weight::find(wgt)));
		return feats().back();
	}

	static inline std::vector<feature>::iterator begin() { return feats().begin(); }
	static inline std::vector<feature>::iterator end() { return feats().end(); }
	static inline u64 size() { return feats().size(); }

private:
	feature(const indexer& index, const weight& value) : index(index), value(value) {}
	static inline std::vector<feature>& feats() { static std::vector<feature> f; return f; }

	indexer index;
	weight value;
};

struct state {
	board move;
	i32 (board::*oper)();
	i32 score;
	numeric esti;
	state() : state(nullptr) {}
	state(i32 (board::*oper)()) : oper(oper), score(-1), esti(0) {}
	state(const state& s)
		: move(s.move), oper(s.oper), score(s.score), esti(s.esti) {}
	void operator <<(const board& b) {
		move = b;
		score = (move.*oper)();
		if (score >= 0) {
			esti = score;
			for (auto f = feature::begin(); f != feature::end(); f++)
				esti += (*f)[move];
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
	}
	numeric operator +=(const numeric& v) {
		const numeric update = alpha * (v - (esti - score));
		esti = score;
		for (auto f = feature::begin(); f != feature::end(); f++)
			esti += ((*f)[move] += update);
		return esti;
	}
	void operator >>(board& b) const { b = move; }
	bool operator >(const state& s) const { return esti > s.esti; }
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
	inline select& operator <<(state move[]) {
		best = move;
		if (move[1] > *best) best = move + 1;
		if (move[2] > *best) best = move + 2;
		if (move[3] > *best) best = move + 3;
		return *this;
	}
	inline select& operator <<(const board& b) {
		move[0] << b;
		move[1] << b;
		move[2] << b;
		move[3] << b;
		return operator <<(move);
	}
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(board& b) const { *best >> b; }
	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }
};
struct statistic {
	u64 maxloop;
	u64 loop;
	u64 check;

	u64 totalscore;
	u64 totalwin;
	u64 localscore;
	u64 localwin;
	u32 maxscore;
	u32 maxtile;
	u64 timestart;
	u64 operations;

	void init(const u64& max, const u64& chk = 1000) {
		maxloop = max * chk;
		loop = 1;
		check = chk;
		totalscore = localscore = 0;
		totalwin = localwin = 0;
		maxscore = maxtile = 0;
		timestart = moporgic::millisec();
		operations = 0;
	}
	u64 operator++(int) { return (++loop) - 1; }
	operator bool() { return loop <= maxloop; }
	void update(const u32& score, const u32& max, const u32& opers) {
		operations += opers;
		localscore += score;
		if ((1 << max) >= 2048) localwin++;
		maxscore = std::max(maxscore, score);
		maxtile = std::max(maxtile, max);
		if ((loop % check) == 0) {
			u64 timenow = moporgic::millisec();
			totalscore += localscore;
			totalwin += localwin;
			std::cout << std::endl;
			printf("%03llu/%03llu %llums %.2fops", loop / check,
					maxloop / check, timenow - timestart,
					operations * 1000.0 / (timenow - timestart));
			std::cout << std::endl;
			printf("local:  avg=%llu win=%.2f%%", localscore / check,
					localwin * 100.0 / check);
			std::cout << std::endl;
			printf("total:  avg=%llu max=%u tile=%u win=%.2f%%",
					totalscore / loop, maxscore, (1 << maxtile),
					totalwin * 100.0 / loop);
			std::cout << std::endl;
			localscore = 0;
			localwin = 0;
			timestart = timenow;
			operations = 0;
		}
	}
};

int main(int argc, const char* argv[]) {
	moporgic::board::initialize();
//	board bb; bb.init();
//	for (int i = 0; i < 16; i++) bb.set(i, rand() % 22);
//	time_t start = moporgic::millisec();
//	for (int i = 0; i < 10000000; i++) {
////		bb.rotate(rand() % 4);
////		if (!bb.next()) bb.init();
//		bb.set(rand() % 16, rand() % 22);
//	}
//	std::cout << (moporgic::millisec() - start) ;

//	weight::make(0, 10);

	u32 train = 100;
	u32 test = 10;
	time_t seed = std::time(nullptr);
	std::string weightin;
	std::string weightout;
	std::string featurein;
	std::string featureout;

	auto valueof = [&](int& i, const char* def) -> const char* {
		if (i + 1 < argc && *(argv[i + 1]) != '-') return argv[++i];
		if (def != nullptr) return def;
		std::cerr << "invalid: " << argv[i] << std::endl;
		std::exit(1);
	};
	for (int i = 1; i < argc; i++) {
		switch (to_hash(argv[i])) {
		case to_hash("-a"):
		case to_hash("--alpha"):
			alpha = std::stod(valueof(i, nullptr));
			break;
		case to_hash("-a/"):
		case to_hash("--alpha-divide"):
			alpha /= std::stod(valueof(i, nullptr));
			break;
		case to_hash("-l"):
		case to_hash("--loop"):
			train = u32(std::stod(valueof(i, "100")));
			test = train / 4; train = train * 3 / 4;
			break;
		case to_hash("-t"):
		case to_hash("--train"):
			train = u32(std::stod(valueof(i, "100")));
			break;
		case to_hash("-e"):
		case to_hash("--test"):
			test = u32(std::stod(valueof(i, "100")));
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
			seed = time_t(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-i"):
		case to_hash("--input"):
			for (std::string opt; (opt = valueof(i, "")).size(); ) {
				if (opt.find(".weight") != std::string::npos) {
					weightin = opt;
				} else if (opt.find(".feature") != std::string::npos) {
					featurein = opt;
				} else {
					weightin = featurein = opt;
					weightin.append(".weight");
					featurein.append(".feature");
				}
			}
			break;
		case to_hash("-o"):
		case to_hash("--output"):
			for (std::string opt; (opt = valueof(i, "")).size(); ) {
				if (opt.find(".weight") != std::string::npos) {
					weightout = opt;
				} else if (opt.find(".feature") != std::string::npos) {
					featureout = opt;
				} else {
					weightout = featureout = opt;
					weightout.append(".weight");
					featureout.append(".feature");
				}
			}
			break;
		case to_hash("--weight-input-output"):
			weightin = weightout = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--weight-input"):
			weightin = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--weight-output"):
			weightout = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--feature-input-output"):
			featurein = featureout = valueof(i, "tdl2048.feature");
			break;
		case to_hash("--feature-input"):
			featurein = valueof(i, "tdl2048.feature");
			break;
		case to_hash("--feature-output"):
			featureout = valueof(i, "tdl2048.feature");
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
			for (std::string wd; (wd = valueof(i, "")).size(); ) {}
			break;
		case to_hash("--weight-type"):
			break;
		case to_hash("-f"):
		case to_hash("--feature"):
			break;
		default:
			std::cerr << "unknown: " << argv[i] << std::endl;
			std::exit(1);
			break;
		}
	}

	std::srand(seed);
	std::cout << "TDL 2048" << std::endl;
	std::cout << "seed = " << seed << std::endl;
	std::cout << "alpha = " << alpha << std::endl;
	printf("board::look[%d] = %lluM", (1 << 20), (sizeof(moporgic::board::look) >> 20));
	std::cout << std::endl;

	auto rotfx = [](int& i) {i = (3 - (i >> 2)) + ((i % 4) << 2);};
	auto mirfx = [](int& s) {s = ((s >> 2) << 2) + (3 - (s % 4));};
	std::vector<std::function<void(int&)>> mapfx = { rotfx, rotfx, rotfx, mirfx, rotfx, rotfx, rotfx, mirfx };
	std::vector<std::vector<int>> patt6t =
		{ { 0, 1, 2, 3, 6, 7 }, { 4, 5, 6, 7, 10, 11 }, { 0, 1, 2, 4, 5, 6 }, { 4, 5, 6, 8, 9, 10 }, };
	auto hash =
			[](std::vector<int>& p) -> u32 {u32 h = 0; for (int t : p) h = (h << 4) | t; return h;};

	const u32 base = 16;
	auto index6t = [=](const board& b,
			const int& p0, const int& p1, const int& p2,
			const int& p3, const int& p4, const int& p5) -> u64 { // pow(base, 6)-bit
		register u32 index = 0;
//		index += b.at(p0) * moporgic::math::pow(base, 0);
//		index += b.at(p1) * moporgic::math::pow(base, 1);
//		index += b.at(p2) * moporgic::math::pow(base, 2);
//		index += b.at(p3) * moporgic::math::pow(base, 3);
//		index += b.at(p4) * moporgic::math::pow(base, 4);
//		index += b.at(p5) * moporgic::math::pow(base, 5);
		index += b.at(p0) <<  0;
		index += b.at(p1) <<  4;
		index += b.at(p2) <<  8;
		index += b.at(p3) << 12;
		index += b.at(p4) << 16;
		index += b.at(p5) << 20;
		return index;
	};
	auto indexmerge = [](const board& b) -> u64 { // 16-bit
		board d = b; d.rotright();
		u32 hori = 0, vert = 0;
		hori += board::look[b.fetch(0)].merged << 0;
		hori += board::look[b.fetch(1)].merged << 2;
		hori += board::look[b.fetch(2)].merged << 4;
		hori += board::look[b.fetch(3)].merged << 6;
		vert += board::look[d.fetch(0)].merged << 0;
		vert += board::look[d.fetch(1)].merged << 2;
		vert += board::look[d.fetch(2)].merged << 4;
		vert += board::look[d.fetch(3)].merged << 6;
		return hori | (vert << 8);
	};
	auto indexnum = [](const board& b) -> u64 { // 25-bit
		static u16 num[32];
		u64 index = 0;
		b.numof(num);
		index += (std::accumulate(num, num + 8, 0) & 0x0f);// tile 0 ~ 7: 4-bit total
		index += (std::accumulate(num + 8, num + 12, 0) & 0x07) << 4;// tile 8 ~ 11: 3-bit total
		index += (std::accumulate(num + 12, num + 14, 0) & 0x07) << 7;// tile 12 ~ 13: 3-bit total
		index += (std::accumulate(num + 14, num + 16, 0) & 0x07) << 10;// tile 14 ~ 15: 3-bit total
		index += (num[16] & 0x03) << 13;// tile 16 ~ 21: 2-bit each (12-bit total)
		index += (num[17] & 0x03) << 15;
		index += (num[18] & 0x03) << 17;
		index += (num[19] & 0x03) << 19;
		index += (num[20] & 0x03) << 21;
		index += (num[21] & 0x03) << 23;
		return index;
	};

//	for (int hash = 0; hash < 0x00ffffff; hash++) {
//		int p[6];
//		for (int i = 0; i < 6; i++) p[i] = (hash >> ((5 - i) << 2)) & 0x0f;
//		indexer::make(hash,
//			std::bind(index6t, std::placeholders::_1, p[0], p[1], p[2], p[3], p[4], p[5]));
//	}
	for (auto& p : patt6t) {
		for (auto fx : mapfx) {
			indexer::make(hash(p),
				std::bind(index6t, std::placeholders::_1, p[0], p[1], p[2], p[3], p[4], p[5]));
			std::for_each(p.begin(), p.end(), fx);
		}
	}
	indexer::make(0xfe000000, indexnum);
	indexer::make(0xff000000, indexmerge);

	if (weight::load(weightin) == false) {
		for (auto& p : patt6t) {
			weight::make(hash(p), std::pow(base, 6));
		}
//		weight::make(0xfe000000, 1 << 25);
		weight::make(0xff000000, 1 << 16);
	}

	if (feature::load(featurein) == false) {
		for (auto& p : patt6t) {
			const u32 wsign = hash(p);
			for (auto fx : mapfx) {
				feature::make(hash(p), wsign); // FIXME
				std::for_each(p.begin(), p.end(), fx);
			}
		}
	//	feature::make(0xfe000000, 0xfe000000);
		feature::make(0xff000000, 0xff000000);
	}

	for (auto it = weight::begin(); it != weight::end(); it++) {
		u32 usageK = ((sizeof(numeric) * it->length()) >> 10);
		u32 usageM = usageK >> 10;
		printf("weight(%08x)[%llu] = %d%c", it->signature(), it->length(),
				usageM ? usageM : usageK, usageM ? 'M' : 'K');
		std::vector<u32> feats;
		for (auto ft = feature::begin(); ft != feature::end(); ft++) {
			if (weight(*ft).signature() == it->signature())
				feats.push_back(feature(*ft).signature());
		}
		if (feats.size()) {
			std::cout << " :";
			for (auto f : feats) printf(" %08x", f);
		}
		std::cout << std::endl;
	}

	std::cout << "start training..." << std::endl;

	board b;
	select best;
	statistic stats;
	std::vector<state> path;
	path.reserve(5000);

	for (stats.init(train); stats; stats++) {

		u32 score = 0;
		u32 opers = 0;

		for (b.init(); best << b; b.next()) {
			score += best.score();
			opers += 1;
			best >> path;
			best >> b;
		}

		for (numeric v = 0; path.size(); path.pop_back()) {
			v = (path.back() += v);
		}

		stats.update(score, b.max(), opers);
	}

	weight::save(weightout);
	feature::save(featureout);

	std::cout << std::endl;
	std::cout << "start testing..." << std::endl;
	for (stats.init(test); stats; stats++) {

		u32 score = 0;
		u32 opers = 0;

		for (b.init(); best << b; b.next()) {
			score += best.score();
			opers += 1;
			best >> b;
		}

		stats.update(score, b.max(), opers);
	}

	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
