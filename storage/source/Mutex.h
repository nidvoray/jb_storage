#ifndef STORAGE_MUTEX_H
#define STORAGE_MUTEX_H

#ifdef USE_STDCXX_MUTEX
#include <shared_mutex>
#endif

namespace jb_storage
{

#ifdef USE_STDCXX_MUTEX
	using MutexType = std::shared_mutex;
#else
#error No mutex type choosen
#endif

}

#endif
