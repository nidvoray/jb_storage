#include "TraceLocker.h"

#include "Mutex.h"

#include <gtest/gtest.h>

using namespace jb_storage;

namespace
{

	constexpr auto depth{ 3 };

}

TEST(TraceLockerTest, Overflow)
{
	std::vector<MutexType> locks{ depth };
	utility::TraceLocker<MutexType> locker{ locks.size() };

	for (auto& lock : locks)
		locker.Push(lock);

	MutexType lock;
	ASSERT_THROW(locker.Push(lock), std::out_of_range);
}

TEST(TraceLockerTest, Underflow)
{
	std::vector<MutexType> locks{ depth };
	utility::TraceLocker<MutexType> locker{ locks.size() };

	for (auto& lock : locks)
		locker.Push(lock);

	for ([[maybe_unused]] auto& lock : locks)
		locker.Pop();

	ASSERT_THROW(locker.Pop(), std::out_of_range);
}

TEST(TraceLockerTest, Consistency)
{
	std::vector<MutexType> locks{ depth };

	{
		utility::TraceLocker<MutexType> locker{ locks.size() };

		for (auto& lock : locks)
			locker.Push(lock);

		for (auto& lock : locks)
			ASSERT_FALSE(lock.try_lock());

		locker.Pop();

		std::for_each(locks.begin(), std::prev(locks.end()), [](auto& lock) { ASSERT_FALSE(lock.try_lock()); });

		ASSERT_TRUE(std::prev(locks.end())->try_lock());
	}

	std::for_each(locks.begin(), std::prev(locks.end()), [](auto& lock) { ASSERT_TRUE(lock.try_lock()); });
}

TEST(TraceLockerTest, KeepOnReturn)
{
	std::vector<MutexType> locks{ depth };
	const auto creator
	{
		[&locks]()
		{
			utility::TraceLocker<MutexType> locker{ locks.size() };

			for (auto& lock : locks)
				locker.Push(lock);

			return locker;
		}
	};

	{
		utility::TraceLocker locker{ creator() };

		for (auto& lock : locks)
			ASSERT_FALSE(lock.try_lock());
	}

	for (auto& lock : locks)
		ASSERT_TRUE(lock.try_lock());
}
