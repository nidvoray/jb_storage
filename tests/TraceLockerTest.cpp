#include "TraceLocker.h"

#include <gtest/gtest.h>

using namespace jb_storage;

namespace
{

	struct TestNode : public INode
	{
		std::shared_mutex Mutex;

		std::optional<Value> GetValue() const override										{ return std::nullopt; }
		bool GrowBranchAndSetValue(const utility::Path& path, const Value& value) override	{ return false; }

		INodePtr GetChild(const std::string& name) const override							{ return nullptr; }
		bool DeleteChild(const std::string& name) override									{ return false; }

		void lock() override 																{ }
		void unlock() override																{ }
		void lock_shared() override															{ Mutex.lock_shared(); }
		void unlock_shared() override														{ Mutex.unlock_shared(); }
	};

	using TestNodePtr = std::shared_ptr<TestNode>;

	constexpr auto depth{ 3 };

}

TEST(TraceLockerTest, Overflow)
{
	std::vector<INodePtr> nodes;
	utility::TraceLocker locker{ depth };

	for (auto i = 0; i < depth; ++i)
	{
		const auto node{ std::make_shared<TestNode>() };
		nodes.push_back(node);
		locker.Push(node);
	}

	ASSERT_THROW(locker.Push(std::make_shared<TestNode>()), std::out_of_range);
}

TEST(TraceLockerTest, Underflow)
{
	std::vector<INodePtr> nodes;
	utility::TraceLocker locker{ depth };

	for (auto i = 0; i < depth; ++i)
	{
		const auto node{ std::make_shared<TestNode>() };
		nodes.push_back(node);
		locker.Push(node);
	}

	for (auto i = 0; i < depth; ++i)
		locker.Pop();

	ASSERT_THROW(locker.Pop(), std::out_of_range);
}

TEST(TraceLockerTest, Consistency)
{
	std::vector<TestNodePtr> nodes;

	{
		utility::TraceLocker locker{ depth };

		for (auto i = 0; i < depth; ++i)
		{
			const auto node{ std::make_shared<TestNode>() };
			nodes.push_back(node);
			locker.Push(node);
		}

		for (auto i = 0; i < depth; ++i)
			ASSERT_FALSE(nodes[i]->Mutex.try_lock());

		locker.Pop();

		for (auto i = 0; i < depth - 1; ++i)
			ASSERT_FALSE(nodes[i]->Mutex.try_lock());

		ASSERT_TRUE(nodes[depth - 1]->Mutex.try_lock());
	}

	for (auto i = 0; i < depth - 1; ++i)
		ASSERT_TRUE(nodes[i]->Mutex.try_lock());
}

TEST(TraceLockerTest, KeepOnReturn)
{
	std::vector<TestNodePtr> nodes;
	const auto creator
	{
		[&nodes]()
		{
			utility::TraceLocker locker{ depth };

			for (auto i = 0; i < depth; ++i)
			{
				const auto node{ std::make_shared<TestNode>() };
				nodes.push_back(node);
				locker.Push(node);
			}

			return locker;
		}
	};

	{
		utility::TraceLocker locker{ creator() };

		for (auto i = 0; i < depth; ++i)
			ASSERT_FALSE(nodes[i]->Mutex.try_lock());
	}

	for (auto i = 0; i < depth; ++i)
		ASSERT_TRUE(nodes[i]->Mutex.try_lock());
}
