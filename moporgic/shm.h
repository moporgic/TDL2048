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
#if defined(__linux__) && !defined(NOSHM)
public:
	static constexpr bool support() { return true; }
	static void enable(bool use) { if (!use) clear(); shm::use() = use; }
	static void auto_cleanup(bool use) { shm::cleanup() = use; }

	template<typename type = void> static type* alloc(size_t size) {
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

	template<typename type = void> static void free(type* shm) {
		shmdt(shm);
		shmctl(info().at(shm), IPC_RMID, nullptr);
		info().erase(shm);
	}

	static void clear() {
		if (&info(false) == nullptr) return;
		for (auto blk : info()) {
			shmdt(blk.first);
			shmctl(blk.second, IPC_RMID, nullptr);
		}
		info().clear();
	}

protected:
	static __attribute__((constructor)) void init() {
		signal(SIGINT, shm::interrupt_handler);
	//	signal(SIGSEGV, [](int i) { std::exit(i); });
	//	std::set_terminate([]() { clear(); __gnu_cxx::__verbose_terminate_handler(); });
	//	std::atexit(shm::clear);
	}
	static __attribute__((destructor)) void exit() {
		if (cleanup()) clear();
		if (&info(false)) delete &info();
	}

	static void interrupt_handler(int i) {
		if (cleanup()) clear();
		std::quick_exit(i);
	}

private:
	static std::map<void*, int>& info(bool try_init = true) {
		static std::map<void*, int> *p = nullptr;
		if (try_init && !p) p = new std::map<void*, int>;
		return *p;
	}
#else /* if shm is not supported */
public:
	static constexpr bool support() { return false; }
	static void enable(bool use) { if (use) throw std::invalid_argument("shm is not supported"); }
	static void auto_cleanup(bool use) { if (use) throw std::invalid_argument("shm is not supported"); }
	template<typename type = void> static type* alloc(size_t size) { throw std::bad_alloc(); }
	template<typename type = void> static void free(type* shm) { throw std::bad_alloc(); }
#endif
public:
	static bool enable() { return shm::use(); }
	static bool auto_cleanup() { return shm::cleanup(); }
private:
	static bool& use() { static bool use = support(); return use; }
	static bool& cleanup() { static bool use = support(); return use; }
};

} // namespace moporgic
