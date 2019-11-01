/*
 * mopoinc.h just an include header
 *
 *  Created on: 20150917
 *      Author: moporgic
 */

#include <cstdio>
#include <cstdlib>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <stack>
#include <queue>
#include <list>
#include <array>
#include <map>
#include <unordered_map>
#include <list>
#include <functional>
#include <algorithm>
#include <tuple>
#include <ctime>
#include <chrono>
#include <cstdarg>
#include <cctype>
#include <cmath>
#include <typeinfo>
#include <thread>
#include <memory>
#include <set>
#include <unordered_set>
// #define NDEBUG // to disable assert()
#include <cassert>
#include <numeric>
#include <limits>
#include <cstddef>
#include <clocale>
#include <cwchar>
#include <mutex>
#include <iterator>
#include <utility>
#include <iomanip>
#include <random>
#include <type_traits>
#include <regex>
#include <atomic>

#include <mmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#endif

#ifdef INC_MOPORGIOUS_LIB
#include "type.h"
#include "util.h"
#include "math.h"
#include "half.h"
#else
#define INC_MOPORGIOUS_LIB
#endif
