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

__constexpr
uint32_t to_hash_tail(const char* str, const uint32_t& hash) noexcept {
	if (*str) return to_hash_tail(str + 1, (hash << 5) - hash + (*str));
	return hash;
}
__constexpr inline
uint32_t to_hash(const char* str) noexcept {
	return to_hash_tail(str, 0); // i' = 31 * i + c
}

inline uint32_t to_hash(const std::string& str) noexcept {
	return to_hash(str.c_str());
}

#undef __constexpr

} /* moporgic */

#endif /* MOPORGIC_UTIL_H_ */
