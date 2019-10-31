#pragma once
/*
 * shm.h
 *  Created on: 2019-01-12
 *      Author: moporgic
 */

#include <cstdlib>
#include <cstdint>
#if defined(__linux__)
#include <sys/shm.h>
#include <signal.h>
#include <string>
#include <fstream>
#include <map>
#endif

namespace moporgic {
class shm {
#if defined(__linux__)
public:
	static inline constexpr bool support() { return true; }

	template<typename type> static inline type* alloc(size_t size) {
		static uint8_t seq = 0;
		static std::string hook = ({
			std::string path = ".";
			std::ifstream in("/proc/self/cmdline", std::ios::in);
			std::getline(in, path, '\0');
			path;
		});
		if (++seq == 0) throw std::bad_alloc();
		auto key = ftok(hook.c_str(), seq);
		int id = shmget(key, size * sizeof(type), IPC_CREAT | IPC_EXCL | 0600);
		void* shm = shmat(id, nullptr, 0);
		if (shm == (void*) -1ull) {
			if (errno & EEXIST) return alloc<type>(size);
			throw std::bad_alloc();
		}
		info().emplace(shm, id);
		std::fill_n((type*) shm, size, type());
		return (type*) shm;
	}

	static inline void free(void* shm) {
		shmdt(shm);
		shmctl(info().at(shm), IPC_RMID, nullptr);
		info().erase(shm);
	}

public:
	static __attribute__((constructor)) void init() {
		signal(SIGINT, shm::interrupt_handler);
	//	signal(SIGSEGV, [](int i) { std::exit(i); });
	//	std::set_terminate([]() { clear(); __gnu_cxx::__verbose_terminate_handler(); });
	//	std::atexit(shm::clear);
	}
	static __attribute__((destructor)) void clear() {
		for (auto blk : info()) {
			shmdt(blk.first);
			shmctl(blk.second, IPC_RMID, nullptr);
		}
		info().clear();
	}

	static void interrupt_handler(int i) {
		clear();
		std::quick_exit(i);
	}

private:
	static std::map<void*, int>& info() {
		static std::map<void*, int> i;
		return i;
	}
#else
public:
	static inline constexpr bool support() { return false; }
	template<typename type> static inline type* alloc(size_t size) { throw std::bad_alloc(); }
	static inline void free(void* shm) { throw std::bad_alloc(); }
#endif
};

} // namespace moporgic
