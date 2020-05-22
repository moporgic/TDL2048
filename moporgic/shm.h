#pragma once
/*
 * shm.h
 *  Created on: 2019-01-12
 *      Author: moporgic
 */

#include <cstdlib>
#include <cstdint>
#include <memory>
#if defined(__linux__)
#include <sys/shm.h>
#include <signal.h>
#include <string>
#include <fstream>
#include <utility>
#include <map>
#endif

namespace moporgic {
class shm {
#if defined(__linux__) && !defined(NOSHM)
public:
	static constexpr bool support() { return true; }

	template<typename type = void> static type* alloc(size_t size) {
		if (!enable<type>()) throw std::invalid_argument("shm is disabled");
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
		info().emplace(shm, std::make_pair(id, size));
		try {
			new (cast<type*>(shm)) type[size]();
		} catch (...) {}
		return cast<type*>(shm);
	}

	template<typename type = void> static void free(type* shm) {
		if (!enable<type>()) throw std::invalid_argument("shm is disabled");
		auto inf = info().at(shm);
		int id = inf.first;
		size_t size = inf.second;
		info().erase(shm);
		try {
			for (size_t i = 0; i < size; i++) cast<type*>(shm)[i].~type();
		} catch (...) {}
		shmdt(shm);
		shmctl(id, IPC_RMID, nullptr);
	}

protected:
	static void clear() {
		if (&info(false) == nullptr) return;
		for (auto blk : info()) {
			void* shm = blk.first;
			int id = blk.second.first;
			shmdt(shm);
			shmctl(id, IPC_RMID, nullptr);
		}
		info().clear();
	}

	static __attribute__((constructor)) void init() {
		signal(SIGTERM, shm::cleanup_handler);
		signal(SIGINT,  shm::cleanup_handler);
	//	signal(SIGSEGV, [](int i) { std::exit(i); });
	//	std::set_terminate([]() { clear(); __gnu_cxx::__verbose_terminate_handler(); });
	//	std::atexit(shm::clear);
	}
	static __attribute__((destructor)) void exit() {
		if (cleanup()) clear();
		if (&info(false)) delete &info();
	}
	static void cleanup_handler(int i) {
		if (cleanup()) clear();
		std::quick_exit(i);
	}

	static std::map<void*, std::pair<int, size_t>>& info(bool try_init = true) {
		static std::map<void*, std::pair<int, size_t>> *p = nullptr;
		if (try_init && !p) p = new std::map<void*, std::pair<int, size_t>>;
		return *p;
	}

#else /* if shm is not supported */
public:
	static constexpr bool support() { return false; }
	template<typename type = void> static type* alloc(size_t size) { throw std::bad_alloc(); }
	template<typename type = void> static void free(type* shm) { throw std::bad_alloc(); }
protected:
	static void clear() {}
#endif /* end if */

public:
	template<typename type = void> static bool enable() { return shm::use() && shm::use<type>(); }
	template<typename type = void> static void enable(bool use) {
		if (!support() && use) throw std::invalid_argument("shm is not supported");
		if (!use) clear();
		shm::use<type>() = use;
	}
	static bool auto_cleanup() { return shm::cleanup(); }
	static void auto_cleanup(bool use) {
		if (!support() && use) throw std::invalid_argument("shm is not supported");
		shm::cleanup() = use;
	}
private:
	template<typename type = void> static bool& use() { static bool use = support(); return use; }
	static bool& cleanup() { static bool use = support(); return use; }

public:
	template<typename type>
	class allocator : std::allocator<type> {
	public:
		inline type* allocate(std::size_t n) { return shm::alloc<type>(n); }
		inline void  deallocate(type* p, std::size_t n) { shm::free<type>(p); }
	};
};

} // namespace moporgic
