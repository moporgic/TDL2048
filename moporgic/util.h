/*
 * util.h mopoutil.h
 *
 *  Created on: 2015/10/24
 *      Author: moporgic
 */

#ifndef MOPORGIC_UTIL_H_
#define MOPORGIC_UTIL_H_

#ifndef DEBUG

#define DLOG(msg,...)
#define __constexpr constexpr

#else /* DEBUG */

#define DLOG(msg,...) printf(msg, ##__VA_ARGS__)
#define __constexpr

#endif /* DEBUG */

#ifndef MOPORGIC_SEQSTATIC_POOL
#define MOPORGIC_SEQSTATIC_POOL 1
#endif

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>

namespace moporgic {

inline uint64_t millisec() {
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	return ms.count();
}
inline uint64_t microsec() {
	auto now = std::chrono::high_resolution_clock::now();
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
	return us.count();
}
uint64_t mspush(std::vector<uint64_t>& stk) {
	uint64_t now = millisec();
	stk.push_back(now);
	return now;
}
uint64_t mspass(std::vector<uint64_t>& stk) {
	uint64_t now = millisec();
	uint64_t elapsed = now - stk.back();
	stk.pop_back();
	return elapsed;
}

const uint32_t seq32_static(const int& i = 0) {
	static uint32_t seq[MOPORGIC_SEQSTATIC_POOL] = {};
	return seq[i]++;
}
const uint64_t seq64_static(const int& i = 0) {
	static uint64_t seq[MOPORGIC_SEQSTATIC_POOL] = {};
	return seq[i]++;
}

__constexpr
uint32_t to_hash_tail(const char* str, const uint32_t& hash) noexcept {
	if (*str) return to_hash_tail(str + 1, (hash << 5) - hash + (*str));
	return hash;
}
__constexpr inline
uint32_t to_hash(const char* str) noexcept {
	return to_hash_tail(str, 0); // i' = 31 * i + c
}

inline
uint32_t to_hash(const std::string& str) noexcept {
	return to_hash(str.c_str());
}

inline
uint32_t rand24() {
#if   RAND_MAX == 0x7fffffff
	return rand() & 0x00ffffff;
#elif RAND_MAX == 0xffffffff
	return rand() & 0x00ffffff;
#elif RAND_MAX == 0x7fff
    return (rand() << 9) + (rand() >> 6);
#elif RAND_MAX == 0xffff
	return (rand() << 8) + (rand() >> 8);
#else
    return ((rand() << 9) + (rand() >> 6)) & 0x00ffffff;
#endif
}

inline float random() {
#if RAND_MAX == 0x7fff
	return static_cast<float>(rand24()) / static_cast<float>(0x01000000);
#else
	return static_cast<float>(static_cast<uint32_t>(rand())) / static_cast<float>(RAND_MAX);
#endif
}

inline
uint32_t rand16() {
#if RAND_MAX == 0x7fff
    return (rand() << 1) + (rand() & 1);
#else
    return rand() & 0xffff;
#endif
}

inline
uint32_t rand31() {
#if   RAND_MAX == 0x7fffffff
	return rand();
#elif RAND_MAX == 0xffffffff
	return rand() & 0x7fffffff;
#elif RAND_MAX == 0x7fff
    return (rand() << 16) + (rand() << 1) + (rand() & 1);
#elif RAND_MAX == 0xffff
	return (rand() << 16) + rand();
#else
    return (rand() << 16) + (rand() << 1) + (rand() & 1);
#endif
}

inline
uint32_t rand32() {
#if   RAND_MAX == 0x7fffffff
	return (rand() << 1) + (rand() & 1);
#elif RAND_MAX == 0xffffffff
	return rand();
#elif RAND_MAX == 0x7fff
    return (rand() << 17) + (rand() << 2) + (rand() & 3);
#elif RAND_MAX == 0xffff
	return (rand() << 16) + rand();
#else
    return (rand() << 17) + (rand() << 2) + (rand() & 3);
#endif
}

inline
uint64_t rand64() {
#if   RAND_MAX == 0x7fffffff
	return (static_cast<uint64_t>(rand()) << 33) + (static_cast<uint64_t>(rand()) << 2) + (static_cast<uint64_t>(rand()) & 3);
#elif RAND_MAX == 0xffffffff
	return (static_cast<uint64_t>(rand()) << 32) + (static_cast<uint64_t>(rand()));
#elif RAND_MAX == 0x7fff
    return (static_cast<uint64_t>(rand()) << 49) + (static_cast<uint64_t>(rand()) << 34)
    	 + (static_cast<uint64_t>(rand()) << 19) + (static_cast<uint64_t>(rand()) << 4) + (static_cast<uint64_t>(rand()) & 15);
#elif RAND_MAX == 0xffff
	return (static_cast<uint64_t>(rand()) << 48) + (static_cast<uint64_t>(rand()) << 32)
		 + (static_cast<uint64_t>(rand()) << 16) + (static_cast<uint64_t>(rand()));
#else
    return (static_cast<uint64_t>(rand()) << 49) + (static_cast<uint64_t>(rand()) << 34)
    	 + (static_cast<uint64_t>(rand()) << 19) + (static_cast<uint64_t>(rand()) << 4) + (static_cast<uint64_t>(rand()) & 15);
#endif
}

inline
uint32_t randx() {
#if   RAND_MAX == 0x7fffffff
	return rand();
#elif RAND_MAX == 0xffffffff
	return rand();
#elif RAND_MAX == 0x7fff
    return (rand() << 15) + (rand());
#elif RAND_MAX == 0xffff
	return (rand() << 16) + (rand());
#else
	return rand();
#endif
}

#undef __constexpr

} /* moporgic */

#endif /* MOPORGIC_UTIL_H_ */
