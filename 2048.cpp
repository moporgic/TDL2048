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
#include "moporgic/io.h"
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
#include <string>
#include <thread>
#include <limits>
#include <cctype>

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
	bool operator ==(const u32& s) const { return sign == s; }
	size_t length() const { return size; }

	template<typename rxx> void write(std::ostream& out) const {
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

	void operator >>(std::ostream& out) const {
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
			out.write(r32(u32(weights().size())).endian(LE), 4);
			for (weight w : weights())
				w >> out;
			break;
		default:
			std::cerr << "unknown serial at weight::save" << std::endl;
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
			}
			break;
		default:
			std::cerr << "unknown serial at weight::load" << std::endl;
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
		return weights().back();
	}
	static inline weight at(const u32& sign) {
		for (weight w : weights())
			if (w.signature() == sign) return w;
		return weights().at(-1);
	}
	typedef std::vector<weight>::iterator iter;
	static inline iter begin() { return weights().begin(); }
	static inline iter end() { return weights().end(); }
	static inline iter find(const u32& sign) {
		return std::find(begin(), end(), sign);
	}
private:
	weight(const u32& sign, const u64& size) : sign(sign), value(new numeric[size]()), size(size) {}
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
	bool operator ==(const u32& s) const { return sign == s; }

	typedef std::function<u64(const board&)> mapper;
	static indexer make(const u32& sign, mapper map) {
		indexers().push_back(indexer(sign, map));
		return indexers().back();
	}
	static inline indexer at(const u32& sign) {
		for (indexer i : indexers())
			if (i.signature() == sign) return i;
		return indexers().at(-1);
	}
	typedef std::vector<indexer>::iterator iter;
	static inline iter begin() { return indexers().begin(); }
	static inline iter end() { return indexers().end(); }
	static inline iter find(const u32& sign) {
		return std::find(begin(), end(), sign);
	}
private:
	indexer(const u32& sign, mapper map) : sign(sign), map(map) {}
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
	bool operator ==(const u64& s) const { return signature() == s; }
	operator indexer() const { return index; }
	operator weight() const { return value; }

	void operator >>(std::ostream& out) const {
		const int LE = moporgic::endian::le;
		const char serial = 0;
		out.write(&serial, 1);
		switch (serial) {
		case 0:
			out.write(r32(index.signature()).endian(LE), 4);
			out.write(r32(value.signature()).endian(LE), 4);
			break;
		default:
			std::cerr << "unknown serial at feature::>>" << std::endl;
			break;
		}
	}
	void operator <<(std::istream& in) {
		char buf[8];
		auto load = moporgic::make_load(in, buf);
		const int LE = moporgic::endian::le;
		switch (*load(1)) {
		case 0:
			index = indexer::at(r32(load(4), LE));
			value = weight::at(r32(load(4), LE));
			break;
		default:
			std::cerr << "unknown serial at feature::<<" << std::endl;
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
			std::cerr << "unknown serial at feature::save" << std::endl;
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
			std::cerr << "unknown serial at feature::load" << std::endl;
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

	static feature make(const u32& wgt, const u32& idx) {
		feats().push_back(feature(weight::at(wgt), indexer::at(idx)));
		return feats().back();
	}
	static feature at(const u32& wgt, const u32& idx) {
		for (auto feat : feats()) {
			if (weight(feat).signature() == wgt && indexer(feat).signature() == idx)
				return feat;
		}
		return feats().at(-1);
	}
	typedef std::vector<feature>::iterator iter;
	static inline iter begin() { return feats().begin(); }
	static inline iter end() { return feats().end(); }
	static inline iter find(const u32& wgt, const u32& idx) {
		return std::find(begin(), end(), (u64(wgt) << 32) | idx);
	}

private:
	feature(const weight& value, const indexer& index) : index(index), value(value) {}
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
	state(const state& s) = default;

	inline void assign(const board& b) {
		move = b;
		score = (move.*oper)();
	}
	inline void estimate(const feature::iter begin, const feature::iter end) {
		if (score >= 0) {
			esti = score;
			for (auto f = begin; f != end; f++)
				esti += (*f)[move];
		} else {
			esti = -std::numeric_limits<numeric>::max();
		}
	}
	inline numeric update(const numeric& v, const feature::iter begin, const feature::iter end) {
		const numeric upd = alpha * (v - (esti - score));
		esti = score;
		for (auto f = begin; f != end; f++)
			esti += ((*f)[move] += upd);
		return esti;
	}
	inline numeric update(const numeric& v, const numeric& alpha, const feature::iter begin, const feature::iter end) {
		const numeric upd = alpha * (v - (esti - score));
		esti = score;
		for (auto f = begin; f != end; f++)
			esti += ((*f)[move] += upd);
		return esti;
	}

	inline void operator <<(const board& b) {
		assign(b);
		estimate(feature::begin(), feature::end());
	}
	inline numeric operator +=(const numeric& v) {
		estimate(feature::begin(), feature::end());
		return update(v, feature::begin(), feature::end());
	}

	inline void operator >>(board& b) const { b = move; }
	inline bool operator >(const state& s) const { return esti > s.esti; }

	void operator >>(std::ostream& out) const {
		move >> out;
		moporgic::write(out, score);
	}
	void operator <<(std::istream& in) {
		move << in;
		moporgic::read(in, score);
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
		best = move;
		if (move[1] > *best) best = move + 1;
		if (move[2] > *best) best = move + 2;
		if (move[3] > *best) best = move + 3;
		return *this;
	}
	inline select& operator <<(const board& b) { return operator ()(b); }
	inline void operator >>(std::vector<state>& path) const { path.push_back(*best); }
	inline void operator >>(board& b) const { *best >> b; }
	inline operator bool() const { return score() != -1; }
	inline i32 score() const { return best->score; }
	inline numeric esti() const { return best->esti; }
};
struct statistic {
	u64 limit;
	u64 loop;
	u64 check;

	struct record {
		u64 score;
		u64 win;
		u64 time;
		u64 opers;
		u32 max;
		u32 hash;
	} total, local;

	void init(const u64& max, const u64& chk = 1000) {
		limit = max * chk;
		loop = 1;
		check = chk;

		total = {};
		local = {};
		local.time = moporgic::millisec();
	}
	u64 operator++(int) { return (++loop) - 1; }
	u64 operator++() { return (++loop); }
	operator bool() { return loop <= limit; }

	void update(const u32& score, const u32& hash, const u32& opers) {
		local.score += score;
		local.hash |= hash;
		local.opers += opers;
		if (hash >= 2048) local.win++;
		local.max = std::max(local.max, score);

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
		snprintf(buf, sizeof(buf), "%03llu/%03llu %llums %.2fops",
				loop / check,
				limit / check,
				elapsedtime,
				local.opers * 1000.0 / elapsedtime);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), "local:  avg=%llu max=%u tile=%u win=%.2f%%",
				local.score / check,
				local.max,
				math::msb32(local.hash),
				local.win * 100.0 / check);
		std::cout << buf << std::endl;
		snprintf(buf, sizeof(buf), "total:  avg=%llu max=%u tile=%u win=%.2f%%",
				total.score / loop,
				total.max,
				math::msb32(total.hash),
				total.win * 100.0 / loop);
		std::cout << buf << std::endl;

		local.score = 0;
		local.win = 0;
		local.time = currtimept;
		local.opers = 0;
		local.hash = 0;
		local.max = 0;
	}
};

int main(int argc, const char* argv[]) {
//	randinit();
//	board bb;	bb.init();
//	for (int i = 0; i < 16; i++) bb.set(i, rand() % 16);
//	time_t start = moporgic::millisec();
//	for (u64 i = 0; i < 20000000000ULL; i++) {
//		//bb.transpose64();
//		bb.transpose2();
//		//bb.set(rand() % 16, rand() % 16);
//		bb.init();
//	}
//	std::cout << (moporgic::millisec() - start) << std::endl ;
//	bb.print();
//	for (int i = 0; i < 16; i++)
//		std::cout << i << "\t" << bb.count(i) << std::endl;
//	return 0;
//
//	start = moporgic::millisec();
//	for (u64 i = 0; i < 1000000000ULL; i++) {
//		bb.transpose64();
//	//	bb.transpose2();
//		bb.set(rand() % 16, rand() % 16);
//	}
//	std::cout << (moporgic::millisec() - start) ;

//	std::ifstream in;
//	in.open("X:\\bb.bin", std::ios::in | std::ios::binary);
//	bb << in;
//	board::print(bb);
//	bb.print();
//	bb.transpose2();
//	bb.print();
//	printf("%08x\n%08x\n",bb.mask(1), bb.mask(4));

//	std::ofstream out;
//	out.open("X:\\bb.bin", std::ios::out | std::ios::binary | std::ios::trunc);
//	bb >> out;
//	out.flush();
//	out.close();

//	int q[16] = {0};
//	for (int i = 0; i < 10000000; i++) {
//		bb.init();
//		for (int t = 0; t < 16; t++) q[t] += bb.at(t);
//	}
//	for (int i = 0; i < 16; i++)
//		std::cout << q[i] << "\t";
//	return 0;

	u32 train = 100;
	u32 test = 10;
	u32 seed = std::time(nullptr);
	struct tdlio {
		std::string input;
		std::string output;
	} weightio, featureio;

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
		case to_hash("-t"):
		case to_hash("--train"):
			train = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-e"):
		case to_hash("--test"):
			test = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("-s"):
		case to_hash("--seed"):
			seed = u32(std::stod(valueof(i, nullptr)));
			break;
		case to_hash("--weight-input-output"):
			weightio.input = weightio.output = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--weight-input"):
			weightio.input = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--weight-output"):
			weightio.output = valueof(i, "tdl2048.weight");
			break;
		case to_hash("--feature-input-output"):
			featureio.input = featureio.output = valueof(i, "tdl2048.feature");
			break;
		case to_hash("--feature-input"):
			featureio.input = valueof(i, "tdl2048.feature");
			break;
		case to_hash("--feature-output"):
			featureio.output = valueof(i, "tdl2048.feature");
			break;
		case to_hash("-w"):
		case to_hash("--weight"):
			for (std::string w; (w = valueof(i, "")).size(); ) {}
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
	printf("board::look[%d] = %lluM", (1 << 20), ((sizeof(board::cache) * (1 << 20)) >> 20));
	std::cout << std::endl;

	auto rotfx = [](int& i) {i = (3 - (i >> 2)) + ((i % 4) << 2);};
	auto mirfx = [](int& s) {s = ((s >> 2) << 2) + (3 - (s % 4));};
	std::vector<std::function<void(int&)>> mapfx = { rotfx, rotfx, rotfx, mirfx, rotfx, rotfx, rotfx, mirfx };
	std::vector<std::vector<int>> patt6t =
		{ { 0, 1, 2, 3, 6, 7 }, { 4, 5, 6, 7, 10, 11 }, { 0, 1, 2, 4, 5, 6 }, { 4, 5, 6, 8, 9, 10 }, };
	auto hashfx =
			[](std::vector<int>& p) -> u32 {u32 h = 0; for (int t : p) h = (h << 4) | t; return h;};

//	std::vector<std::string> ires = { "012367@#", "012456@#", "4567ab@#", "45689a@#",
//				"0123@#", "4567@#", "0145@#", "1256@#", "569a@#" };
//	std::vector<std::string> wres = { "00012367(6)", "00012456(6)", "004567ab(6)", "0045689a(6)", "fe000001/25", "ff000000/16" };
//	std::vector<std::string> fres = { "00012367:012367@#", "00012456:012456@#", "004567ab:4567ab@#", "0045689a:45689a@#",
//				"fe000001:fe000001", "ff000000:ff000000" };

	const u32 base = 16;
//
//	auto hash_to_patt = [](const u32& hash, const u32& size) -> std::vector<int> {
//		std::vector<int> patt;
//		for (size_t i = 0; i < size; i++)
//			patt.push_back((hash >> (((size - 1) - i) << 2)) & 0x0f);
//		return patt;
//	};
//	auto patt_to_hash = [](const std::vector<int>& patt) -> u32 {
//		u32 h = 0;
//		for (int t : patt) h = (h << 4) | t;
//		return h;
//	};
//	auto patt_map = [](std::vector<u32>& hashes, u32 size, std::vector<std::function<void(int&)>> mapfx) {
//		std::vector<u32> buf = hashes;
//		for (const u32& hash : buf) {
//			std::vector<int> p = hash_to_patt(hash, size);
//			for (auto fx : mapfx) {
//				std::for_each(p.begin(), p.end(), fx);
//				hashes.push_back(patt_to_hash(p));
//			}
//		}
//	};
//	auto make_patt = [](const std::string& res) -> std::pair<std::vector<u32>, u32> {
//		auto r = res.find('@');
//		auto m = res.find('#');
//		const std::string patt = res.substr(0, std::min(r, m));
//		std::vector<u32> sign = { std::stoul(patt, nullptr, 16) };
//		if (r != std::string::npos) {
//			patt_map(sign, patt.size(), { rotfx, rotfx, rotfx });
//		}
//		if (m != std::string::npos) {
//			patt_map(sign, patt.size(), { mirfx });
//		}
//		return std::make_pair(sign, patt.size());
//	};


//	auto index8t = [=](const board& b,
//			const int& p0, const int& p1, const int& p2,
//			const int& p3, const int& p4, const int& p5,
//			const int& p6, const int& p7) -> u64 { // pow(base, 8)-bit
//		register u64 index = 0;
//		index += b.at(p0) <<  0;
//		index += b.at(p1) <<  4;
//		index += b.at(p2) <<  8;
//		index += b.at(p3) << 12;
//		index += b.at(p4) << 16;
//		index += b.at(p5) << 20;
//		index += b.at(p6) << 24;
//		index += b.at(p7) << 28;
//		return index;
//	};
	auto index6t = [=](const board& b,
			const int& p0, const int& p1, const int& p2,
			const int& p3, const int& p4, const int& p5) -> u64 { // pow(base, 6)-bit
		register u64 index = 0;
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
//	auto index4t = [=](const board& b,
//			const int& p0, const int& p1, const int& p2, const int& p3) -> u64 { // pow(base, 4)-bit
//		register u64 index = 0;
//		index += b.at(p0) <<  0;
//		index += b.at(p1) <<  4;
//		index += b.at(p2) <<  8;
//		index += b.at(p3) << 12;
//		return index;
//	};
	auto indexmerge = [](const board& b) -> u64 { // 16-bit
		board q = b; q.transpose();
		u32 hori = 0, vert = 0;
		hori += b.query(0).merge << 0;
		hori += b.query(1).merge << 2;
		hori += b.query(2).merge << 4;
		hori += b.query(3).merge << 6;
		vert += q.query(0).merge << 0;
		vert += q.query(1).merge << 2;
		vert += q.query(2).merge << 4;
		vert += q.query(3).merge << 6;
		return hori | (vert << 8);
	};
//	auto indexnum0 = [](const board& b) -> u64 { // 12-bit
//		static u16 num[32];
//		b.count(num, 10, 16);
//		u64 index = 0;
//		index += (num[10] & 0x03) << 0; // 1k ~ 32k, 2-bit ea.
//		index += (num[11] & 0x03) << 2;
//		index += (num[12] & 0x03) << 4;
//		index += (num[13] & 0x03) << 6;
//		index += (num[14] & 0x03) << 8;
//		index += (num[15] & 0x03) << 10;
//		return index;
//	};
	auto indexnum1 = [](const board& b) -> u64 { // 25-bit
		static u16 num[32];
		b.count(num, 5, 16);
		u64 index = 0;
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
	};
//	auto indexmask = [](const board& b, const int& t) -> u64 { // 16-bit
//		return b.mask(t);
//	};

//	for (int hash = 0; hash < 0x00ffffff; hash++) {
//		int p[6];
//		for (int i = 0; i < 6; i++) p[i] = (hash >> ((5 - i) << 2)) & 0x0f;
//		indexer::make(hash,
//			std::bind(index6t, std::placeholders::_1, p[0], p[1], p[2], p[3], p[4], p[5]));
//	}

//	for (const std::string& res : ires) {
//		auto patt = make_patt(res);
//		for (u32 sign : patt.first) {
//			u64 size = patt.second;
//			auto p = hash_to_patt(sign, size);
//			using std::placeholders::_1;
//			switch (size) {
//			case 8:
//				indexer::make(sign, std::bind(index8t, _1, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7])));
//				break;
//			case 6:
//				indexer::make(sign, std::bind(index6t, _1, p[0], p[1], p[2], p[3], p[4], p[5]));
//				break;
//			case 4:
//				indexer::make(sign, std::bind(index4t, _1, p[0], p[1], p[2], p[3]));
//				break;
//			}
//		}
//	}
//
//	for (const std::string& res : wres) {
//		using std::string::npos;
//		auto pos = npos;
//		if ((pos = res.find(':')) != npos) {
//			u64 size = std::stoull(res.substr(pos + 1));
//			weight::make(std::stoul(res.substr(0, pos)), size);
//		} else if ((pos = res.find('/')) != npos) {
//			u64 size = 1ULL << std::stoul(res.substr(pos + 1));
//			weight::make(std::stoul(res.substr(0, pos)), size);
//		} else if ((pos = res.find('(')) != npos && res.find(')') != npos) {
//			u32 tiles = std::stoul(res.substr(pos + 1, res.find(')')));
//			u64 size = std::pow(base, tiles);
//			weight::make(std::stoul(res.substr(0, pos)), size);
//		} else {
//			auto patt = make_patt(res);
//			u64 size = std::pow(base, patt.second);
//			for (u32 sign : patt.first)
//				weight::make(sign, size);
//		}
//	}
//
//	for (const std::string& res : fres) {
//		std::string wgt = res.substr(0, res.find(':'));
//		std::string idx = res.substr(res.find(':') + 1);
//		std::vector<u32> wsign;
//		std::vector<u32> isign;
//		if (std::isxdigit(wgt.back())) {
//			wsign = { std::stoul(wgt, nullptr, 16) };
//		} else {
//			wsign = make_patt(wgt).first;
//		}
//		if (std::isxdigit(idx.back())) {
//			isign = { std::stoul(idx, nullptr, 16) };
//		} else {
//			isign = make_patt(idx).first;
//		}
//		while (wsign.size() < isign.size())
//			wsign.push_back(wsign.back());
//		while (isign.size() < wsign.size())
//			isign.push_back(isign.back());
//		for (u32 i = 0; i < wsign.size(); i++)
//			feature::make(wsign[i], isign[i]);
//	}

	for (auto& p : patt6t) {
		for (auto fx : mapfx) {
			indexer::make(hashfx(p),
				std::bind(index6t, std::placeholders::_1, p[0], p[1], p[2], p[3], p[4], p[5]));
			std::for_each(p.begin(), p.end(), fx);
		}
	}
//	indexer::make(0xfe000000, indexnum0);
	indexer::make(0xfe000001, indexnum1);
	indexer::make(0xff000000, indexmerge);
//	for (int i = 0; i < 16; i++)
//		indexer::make(0xfd000000 + (i << 16), std::bind(indexmask, std::placeholders::_1, i));

	if (weight::load(weightio.input) == false) {
		for (auto& p : patt6t) {
			weight::make(hashfx(p), std::pow(base, 6));
		}
//		weight::make(0xfe000000, 1 << 25);
		weight::make(0xfe000001, 1 << 25);
		weight::make(0xff000000, 1 << 16);
//		for (int i = 0; i < 16; i++) weight::make(0xfd000000 + (i << 16), 1 << 16);
	}


	if (feature::load(featureio.input) == false) {
		for (auto& p : patt6t) {
			const u32 wsign = hashfx(p);
			for (auto fx : mapfx) {
				feature::make(wsign, hashfx(p)); // FIXME
				std::for_each(p.begin(), p.end(), fx);
			}
		}
	//	feature::make(0xfe000000, 0xfe000000);
		feature::make(0xfe000001, 0xfe000001);
		feature::make(0xff000000, 0xff000000);
//		for (int i = 0; i < 16; i++) feature::make(0xfd000000 + (i << 16), 0xfd000000 + (i << 16));
	}

	// for 2nd layer-->
//	for (auto& p : patt6t) {
//		const u32 wsign = hashfx(p) | 0x10000000;
//		weight::make(wsign, std::pow(base, 6));
//		for (auto fx : mapfx) {
//			feature::make(wsign, hashfx(p)); // FIXME
//			std::for_each(p.begin(), p.end(), fx);
//		}
//	}
//	weight::make(0xfe100001, 1 << 25);
//	weight::make(0xff100000, 1 << 16);
//	feature::make(0xfe100001, 0xfe000001);
//	feature::make(0xff100000, 0xff000000);
	// <--for 2nd layer

	// for 3rd layer-->
//	for (auto& p : patt6t) {
//		const u32 wsign = hashfx(p) | 0x20000000;
//		weight::make(wsign, std::pow(base, 6));
//		for (auto fx : mapfx) {
//			feature::make(wsign, hashfx(p)); // FIXME
//			std::for_each(p.begin(), p.end(), fx);
//		}
//	}
//	weight::make(0xfe200001, 1 << 25);
//	weight::make(0xff200000, 1 << 16);
//	feature::make(0xfe200001, 0xfe000001);
//	feature::make(0xff200000, 0xff000000);
	// <--for 3rd layer


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

		stats.update(score, b.hash(), opers);
	}

//	auto s0begin = feature::begin();
//	auto s1begin = feature::find(0x10012367, 0x00012367);
////	auto s2begin = feature::find(0x20012367, 0x00012367);
//	auto s0end = s1begin;
//	auto s1end = feature::end(); //s2begin;
////	auto s2end = feature::end();
//	std::cout << "multi-stage hint: " << (s0end - s0begin) << " | " << (s1end - s1begin) << std::endl;
//
//	u64 s1upd = 0;
//	u64 s2upd = 0;
//	for (stats.init(train); stats; stats++) {
//
//		u32 score = 0;
//		u32 opers = 0;
//
////		for (b.init(); best(b, s0begin, s0end); b.next()) {
////			score += best.score();
////			opers += 1;
////			best >> path;
////			best >> b;
////		}
//
//		for (b.init(); best(b, s0begin, s0end); b.next()) {
//			score += best.score();
//			opers += 1;
//			best >> b;
//			if (b.hash() >= 16384) break;
//		}
//		for (b.next(); best(b, s1begin, s1end); b.next()) {
//			score += best.score();
//			opers += 1;
//			best >> path;
//			best >> b;
//		}
//
//		u32 hash = b.hash();
//
//		if (hash >= 16384) {
////			if (hash >= 16384 + 8192) {
////				for (numeric w = 0; path.size(); path.pop_back()) {
////					state& s = path.back();
////					if (s.move.hash() < 16384 + 8192) break;
////					w = s.update(w, alpha2, s2begin, s2end);
////				}
////				if (++s2upd % 1000 == 0) std::cout << "multi-stage hint: s2 " << s2upd << std::endl;
////			}
//			for (numeric u = 0; path.size(); path.pop_back()) {
//				state& s = path.back();
//				s.estimate(s1begin, s1end);
//				u = s.update(u, alpha1, s1begin, s1end);
//			}
//			if (++s1upd % 1000 == 0) std::cout << "multi-stage hint: s1 " << s1upd << std::endl;
//		} else {
////			for (numeric v = 0; path.size(); path.pop_back()) {
////				v = path.back().update(v, 0.0025 / 32, s0begin, s0end);
////			}
//		}
//		path.clear();
//
//		stats.update(score, hash, opers);
//	}
//	std::cout << "multi-stage hint: " << s1upd << " / " << s2upd << std::endl;

	weight::save(weightio.output);
	feature::save(featureio.output);


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

		stats.update(score, b.hash(), opers);
	}

//	std::cout << std::endl;
//	std::cout << "start testing..." << std::endl;
//	for (stats.init(test); stats; stats++) {
//
//		u32 score = 0;
//		u32 opers = 0;
//
//		for (b.init(); best(b, s0begin, s0end); b.next()) {
//			score += best.score();
//			opers += 1;
//			best >> b;
//			if (b.hash() >= 16384) break;
//		}
//		for (b.next(); best(b, s1begin, s1end); b.next()) {
//			score += best.score();
//			opers += 1;
//			best >> b;
//		}
//
//		stats.update(score, b.hash(), opers);
//	}


	return 0;
}

} // namespace moporgic

int main(int argc, const char* argv[]) {
	return moporgic::main(argc, argv);
}
